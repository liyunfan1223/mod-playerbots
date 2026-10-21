/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSActions.h"
#include "CharmInfo.h"
#include "GameObject.h"
#include "Group.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Totem.h"

#include <limits>

bool IckAndKrickAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "Ick");
    if (!boss)
        return false;

    std::vector<Unit*> orbs;
    bool orb = false;

    // First gather all orbs
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs1)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
        {
            orb = true;
            break;
        }
    }

    // Only the fixated player kites, tank included; everyone else keeps fighting
    bool pursuit = bot->HasAura(SPELL_PURSUIT);
    bool poisonNova = boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) || boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC));
    bool explosiveBarrage = orb || (boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) || boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK)));
    bool isTank = PlayerbotAI::IsTank(bot);

    if (pursuit && Pursuit(pursuit, boss))
        return true;

    if (poisonNova && PoisonNova(poisonNova, boss))
        return true;

    if (explosiveBarrage && ExplosiveBarrage(explosiveBarrage, boss))
        return true;

    if (isTank && !pursuit && !poisonNova && !explosiveBarrage)
    {
        if (TankPosition(boss))
            return true;
    }

    return false;
}

bool IckAndKrickAction::TankPosition(Unit* boss)
{
    if (botAI->HasAggro(boss))
    {
        float distance = bot->GetExactDist2d(ICKANDKRICK_TANK_POSITION.GetPositionX(), ICKANDKRICK_TANK_POSITION.GetPositionY());
        // A Toxic Waste pool on the spot: tank him where he is until it fades rather than drag him into it
        std::vector<PoSGroundHazard> hazards = FindPoSGroundHazards(bot);
        if (InPoSGroundHazard(hazards, ICKANDKRICK_TANK_POSITION.GetPositionX(), ICKANDKRICK_TANK_POSITION.GetPositionY(), 3.0f))
            return false;

        if (distance > 7.0f)
        {
            // Calculate direction vector
            float dirX = ICKANDKRICK_TANK_POSITION.GetPositionX() - bot->GetPositionX();
            float dirY = ICKANDKRICK_TANK_POSITION.GetPositionY() - bot->GetPositionY();
            float length = sqrt(dirX * dirX + dirY * dirY);
            dirX /= length;
            dirY /= length;

            // Move in increments of 3.0f
            float moveX = bot->GetPositionX() + dirX * 3.0f;
            float moveY = bot->GetPositionY() + dirY * 3.0f;
            if (InPoSGroundHazard(hazards, moveX, moveY, 1.5f))
                return false;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool IckAndKrickAction::Pursuit(bool pursuit, Unit* boss)
{
    if (!pursuit)
        return false;

    // Keep running a circle around the room for the whole 12 s fixate, always on the side away from Ick;
    // the room centre is the script's evade anchor, so the circle never pulls him out of it
    float distanceToBoss = bot->GetExactDist2d(boss);
    if (distanceToBoss > 18.0f && bot->isMoving())
        return true;

    Position const center = ICKANDKRICK_ROOM_CENTER;
    float botAngle = std::atan2(bot->GetPositionY() - center.GetPositionY(), bot->GetPositionX() - center.GetPositionX());
    float bossAngle = std::atan2(boss->GetPositionY() - center.GetPositionY(), boss->GetPositionX() - center.GetPositionX());
    float ahead = Position::NormalizeOrientation(botAngle - bossAngle) < float(M_PI) ? 1.0f : -1.0f;

    std::vector<PoSGroundHazard> hazards = FindPoSGroundHazards(bot);
    for (float radius : {20.0f, 16.0f, 24.0f})
    {
        for (float step : {0.9f, 1.3f, 0.6f})
        {
            float angle = botAngle + ahead * step;
            float x = center.GetPositionX() + radius * std::cos(angle);
            float y = center.GetPositionY() + radius * std::sin(angle);
            float z = bot->GetPositionZ();
            if (InPoSGroundHazard(hazards, x, y, 1.5f))
                continue;

            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > 3.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
                continue;

            botAI->Reset();
            return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }

    return MoveAway(boss, 10.0f);
}

bool IckAndKrickAction::PoisonNova(bool poisonNova, Unit* boss)
{
    // The nova reaches 15 yd: end up at 18 yd however close we start (a fixed 11 yd step left melee at 14)
    float distance = bot->GetExactDist2d(boss);
    if (distance < POISON_NOVA_SAFE_DISTANCE && poisonNova)
        return MoveAway(boss, POISON_NOVA_SAFE_DISTANCE - distance + 1.0f);
    // Clear of the nova: wait it out, unless in a pool; the hazard step then keeps the same distance
    else if (poisonNova)
        return !InPoSGroundHazard(FindPoSGroundHazards(bot), bot->GetPositionX(), bot->GetPositionY(), 1.0f);

    return false;
}

bool IckAndKrickAction::ExplosiveBarrage(bool /*explosiveBarrage*/, Unit* boss)
{
    std::vector<Unit*> orbs;
    Unit* closestOrb = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    // First gather all orbs
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs1)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->HasAura(SPELL_EXPLODING_ORB_VISUAL))
        {
            orbs.push_back(unit);
            float dist = bot->GetDistance(unit);
            if (dist < closestDistance)
            {
                closestDistance = dist;
                closestOrb = unit;
            }
        }
    }

    if (!orbs.empty() && closestOrb && bot->GetExactDist2d(closestOrb) < 7.0f)
    {
        // Get player positions to avoid moving toward them
        std::vector<Position> playerPositions;
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (Player* member = ref->GetSource())
                {
                    if (member != bot && member->IsAlive() && bot->GetMap() == member->GetMap() &&
                        bot->GetExactDist2d(member) < 20.0f)
                    {
                        playerPositions.push_back(member->GetPosition());
                    }
                }
            }
        }

        // Calculate several potential escape positions
        const int NUM_ANGLES = 16;  // Check 16 different directions
        const float SAFE_DISTANCE = 7.0f;
        const float MAX_BOSS_DISTANCE = 30.0f;  // Maximum allowed distance from boss
        const float ANGLE_INCREMENT = 2 * M_PI / NUM_ANGLES;

        Position bestPos;
        bool foundValidPos = false;
        float bestScore = -1.0f;

        for (int i = 0; i < NUM_ANGLES; i++)
        {
            float angle = i * ANGLE_INCREMENT;
            Position potentialPos;
            potentialPos.Relocate(bot->GetPositionX() + SAFE_DISTANCE * cos(angle),
                                  bot->GetPositionY() + SAFE_DISTANCE * sin(angle),
                                  bot->GetPositionZ());

            // Check if position is valid (not in a wall)
            if (!bot->IsWithinLOS(potentialPos.GetPositionX(), potentialPos.GetPositionY(),
                                  potentialPos.GetPositionZ()))
                continue;

            // Check if position is within maximum allowed distance from boss
            if (boss && boss->GetDistance2d(potentialPos.GetPositionX(), potentialPos.GetPositionY()) > MAX_BOSS_DISTANCE)
                continue;

            // Score this position based on:
            // 1. Distance from all orbs (farther is better)
            // 2. Distance from other players (farther is better)
            // 3. Distance from boss (closer is better, but still safe)
            float score = 0.0f;

            // Check distance from all orbs
            float minOrbDist = std::numeric_limits<float>::max();
            for (Unit* orb : orbs)
            {
                float orbDist = orb->GetDistance2d(potentialPos.GetPositionX(), potentialPos.GetPositionY());
                minOrbDist = std::min(minOrbDist, orbDist);
            }
            score += minOrbDist * 2.0f;  // Weight orb distance more heavily

            // Check distance from other players
            for (Position const& playerPos : playerPositions)
            {
                float playerDist = sqrt(pow(potentialPos.GetPositionX() - playerPos.GetPositionX(), 2) +
                                        pow(potentialPos.GetPositionY() - playerPos.GetPositionY(), 2));
                score += std::min(playerDist, 10.0f);  // Cap player distance contribution
            }

            // Factor in proximity to boss (closer is better, as long as we're safe from orbs)
            if (boss)
            {
                float bossDist = boss->GetDistance2d(potentialPos.GetPositionX(), potentialPos.GetPositionY());
                // Add points for being closer to boss (inverse relationship)
                // but only if we're safely away from orbs
                if (minOrbDist > SAFE_DISTANCE)
                {
                    score += (MAX_BOSS_DISTANCE - bossDist) * 0.5f;
                }
            }

            // If this is the best position so far, remember it
            if (score > bestScore)
            {
                bestScore = score;
                bestPos = potentialPos;
                foundValidPos = true;
            }
        }

        // If we found a valid position, move there
        if (foundValidPos)
        {
            botAI->Reset();
            // Use MoveTo instead of FleePosition since we already calculated the exact position
            return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bot->GetPositionZ(), false,
                          false,
                          false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            return FleePosition(closestOrb->GetPosition(), 7.0f, 250U);
        }
    }

    return false;
}

bool TyrannusAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    bool rangedSpread = false;

    if (PlayerbotAI::IsRanged(bot) && boss->HealthBelowPct(99))
        rangedSpread = true;

    if (rangedSpread && RangedSpread(rangedSpread))
        return true;

    return false;
}

bool TyrannusAction::RangedSpread(bool rangedSpread)
{
    float radius = 10.0f;
    float moveIncrement = 3.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (PlayerbotAI::IsRanged(bot) && rangedSpread)
    {
        // Ranged: spread from other members
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return FleePosition(unit->GetPosition(), moveDistance, 250U);
            }
        }
    }

    return false;
}

static Player* FindGroupHealer(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->IsInWorld() && PlayerbotAI::IsHeal(member))
            return member;
    }
    return nullptr;
}

bool GarfrostTakeCoverAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss)
        return false;

    // Hide on the far side of a boulder; everyone gathers at the healer's boulder (or a close one in the
    // healer's sight) so nobody drops out of heals, and casters stay in range of the boss
    std::list<GameObject*> rocks;
    boss->GetGameObjectListWithEntryInGrid(rocks, GO_SARONITE_ROCK, 100.0f);
    Player* healer = PlayerbotAI::IsHeal(bot) ? nullptr : FindGroupHealer(bot);
    Unit* tank = boss->GetVictim();
    bool forging = IsGarfrostForging(boss);
    bool ranged = PlayerbotAI::IsRanged(bot);

    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (GameObject* rock : rocks)
    {
        if (rock->IsInvisibleDueToDespawn())
            continue;

        float angle = boss->GetAngle(rock);
        for (float back : {3.0f, 5.0f})
        {
            // No line-of-sight test from the bot: the boulder itself blocks the view of the spot behind it
            float x = rock->GetPositionX() + back * std::cos(angle);
            float y = rock->GetPositionY() + back * std::sin(angle);
            float z = bot->GetPositionZ();
            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > 4.0f)
                continue;

            float bossDist = boss->GetExactDist2d(x, y);
            if (ranged && !forging && bossDist > 30.0f)
                continue;

            // Boulders block heals too: the healer never hides where the tank drops out of sight
            if (PlayerbotAI::IsHeal(bot) && !forging && tank && tank != bot &&
                (tank->GetExactDist2d(x, y) > 38.0f || !tank->IsWithinLOS(x, y, z + 1.0f)))
                continue;

            float score = bot->GetExactDist2d(x, y) + GARFROST_ROOM_MIDDLE.GetExactDist2d(x, y) * 0.5f;
            if (healer)
            {
                score = healer->GetExactDist2d(x, y) * 2.0f + score * 0.5f;
                if (!healer->IsWithinLOS(x, y, z + 1.0f))
                    score += 200.0f;
            }

            if (score < bestScore)
            {
                bestScore = score;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    if (bestScore == std::numeric_limits<float>::max() || bot->GetExactDist2d(bestX, bestY) < 1.0f)
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool GarfrostTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss || IsGarfrostForging(boss) || boss->GetVictim() != bot)
        return false;

    float distance = bot->GetExactDist2d(GARFROST_ROOM_MIDDLE);
    if (distance < 6.0f)
        return false;

    // Walk him back in small steps so he stays in melee the whole way
    float step = std::min(distance, 4.0f);
    float angle = bot->GetAngle(&GARFROST_ROOM_MIDDLE);
    float x = bot->GetPositionX() + step * std::cos(angle);
    float y = bot->GetPositionY() + step * std::sin(angle);
    float z = bot->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);
    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool GarfrostDodgeSaroniteAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    Position landing;
    if (!boss || !GetSaroniteLanding(boss, landing))
        return false;

    // The boulder follows its target, so the target cannot dodge it: it walks away from the others instead,
    // and everyone else steps away from the target
    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_THROW_SARONITE);
    bool isTarget = spell && spell->m_targets.GetUnitTarget() == bot;
    std::vector<Player*> others;
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member != bot && member->IsAlive() && member->IsInWorld() && member->GetMapId() == bot->GetMapId())
                    others.push_back(member);

    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (int i = 0; i < 16; ++i)
    {
        float angle = float(i) * float(M_PI) / 8.0f;
        float radius = GARFROST_SARONITE_AVOID_RADIUS + 1.0f;
        float x = (isTarget ? bot->GetPositionX() : landing.GetPositionX()) + radius * std::cos(angle);
        float y = (isTarget ? bot->GetPositionY() : landing.GetPositionY()) + radius * std::sin(angle);
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        if (std::fabs(z - bot->GetPositionZ()) > 3.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
            continue;

        float score = bot->GetExactDist2d(x, y);
        if (isTarget)
        {
            float nearest = std::numeric_limits<float>::max();
            for (Player* other : others)
                nearest = std::min(nearest, other->GetExactDist2d(x, y));
            score = -nearest;  // as far from the closest party member as possible
        }
        if (score < bestScore)
        {
            bestScore = score;
            bestX = x;
            bestY = y;
            bestZ = z;
        }
    }

    if (bestScore == std::numeric_limits<float>::max())
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool PoSAvoidGroundHazardAction::Execute(Event /*event*/)
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss)
        return false;

    std::vector<PoSGroundHazard> hazards = FindPoSGroundHazards(bot);
    bool inHazard = InPoSGroundHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), 1.0f);
    bool nova = IsPoisonNovaCasting(boss);
    bool dragBoss = !inHazard && !nova && PlayerbotAI::IsTank(bot) &&
                    InPoSGroundHazard(hazards, boss->GetPositionX(), boss->GetPositionY(), 0.0f);
    if (!inHazard && !dragBoss)
        return false;

    // Melee step out on the side nearest the boss; the tank pulls the boss clear so he leaves the patch too;
    // everyone else takes the shortest safe step
    // Icy Blast lays three 7 yd patches every 15 s that last a minute, so the near ground fills up: search wide,
    // and if nothing is clear take the spot under the fewest patches rather than standing still
    bool stayNearBoss = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    float clearance = dragBoss ? boss->GetCombatReach() + 2.0f : 1.5f;
    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (float step : {4.0f, 7.0f, 10.0f, 14.0f, 18.0f, 22.0f, 26.0f, 30.0f})
    {
        for (int i = 0; i < 24; ++i)
        {
            float angle = float(i) * float(M_PI) / 12.0f;
            float x = bot->GetPositionX() + step * std::cos(angle);
            float y = bot->GetPositionY() + step * std::sin(angle);
            float z = bot->GetPositionZ();
            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > 3.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
                continue;

            int covering = 0;
            for (PoSGroundHazard const& hazard : hazards)
            {
                std::vector<PoSGroundHazard> one = {hazard};
                if (InPoSGroundHazard(one, x, y, clearance))
                    ++covering;
            }

            // Ranged and healers stay within casting range of the boss; nobody steps back into a nova
            float bossDist = boss->GetExactDist2d(x, y);
            if (nova && bossDist < POISON_NOVA_SAFE_DISTANCE)
                continue;
            float score = covering * 1000.0f + (stayNearBoss ? bossDist : step) + (bossDist > 30.0f ? 500.0f : 0.0f);
            if (score < bestScore)
            {
                bestScore = score;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    if (bestScore == std::numeric_limits<float>::max() || bot->GetExactDist2d(bestX, bestY) < 1.0f)
        return false;

    // Moving onto ground under as many patches as here gains nothing
    int bestCovering = int(bestScore / 1000.0f);
    int hereCovering = 0;
    for (PoSGroundHazard const& hazard : hazards)
    {
        std::vector<PoSGroundHazard> one = {hazard};
        if (InPoSGroundHazard(one, dragBoss ? boss->GetPositionX() : bot->GetPositionX(),
                              dragBoss ? boss->GetPositionY() : bot->GetPositionY(), dragBoss ? 0.0f : 1.0f))
            ++hereCovering;
    }
    if (bestCovering >= hereCovering)
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

// Totems are left out: TotemAI ignores react state and picks its own target
static std::vector<Creature*> PoSControlledCreatures(Player* bot)
{
    std::vector<Creature*> pets;
    for (Unit* controlled : bot->m_Controlled)
    {
        Creature* pet = controlled ? controlled->ToCreature() : nullptr;
        if (pet && pet->IsAlive() && !pet->IsTotem())
            pets.push_back(pet);
    }
    return pets;
}

static bool PoSTotemDealsDamage(Totem* totem)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(totem->GetSpell());
    if (!info)
        return false;

    // Searing Totem hits directly; Magma Totem pulses a triggered spell
    for (SpellEffectInfo const& effect : info->Effects)
    {
        if (effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
            return true;
        if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
            if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(effect.TriggerSpell))
                if (triggered->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE))
                    return true;
    }
    return false;
}

static std::vector<Totem*> PoSDamagingTotems(Player* bot)
{
    std::vector<Totem*> totems;
    for (Unit* controlled : bot->m_Controlled)
    {
        Totem* totem = controlled && controlled->IsTotem() ? controlled->ToTotem() : nullptr;
        if (totem && totem->IsAlive() && PoSTotemDealsDamage(totem))
            totems.push_back(totem);
    }
    return totems;
}

bool IsBrandDamagingTotemSpell(std::string const& spell)
{
    return spell == "searing totem" || spell == "magma totem" || spell == "fire elemental totem";
}

bool TyrannusBrandStopAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    // Every heal from the branded player also heals Tyrannus, so any cast counts, not only those on him
    if (bot->GetVictim() || (bot->IsNonMeleeSpellCast(true) && !BrandedHealerMayHeal(bot, boss)) ||
        !PoSDamagingTotems(bot).empty())
        return true;

    for (Creature* pet : PoSControlledCreatures(bot))
    {
        if (pet->GetVictim() || pet->GetReactState() != REACT_PASSIVE)
            return true;
    }
    return false;
}

bool TyrannusBrandStopAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss || !BrandedHealerMayHeal(bot, boss))
        bot->InterruptNonMeleeSpells(true);

    // The multiplier keeps the shaman from dropping them again until the brand fades
    for (Totem* totem : PoSDamagingTotems(bot))
        totem->UnSummon();

    // Passive until the brand fades, or the pet's AI picks a target straight back up
    for (Creature* pet : PoSControlledCreatures(bot))
    {
        TyrannusRememberSuspendedPet(bot, pet);
        pet->SetReactState(REACT_PASSIVE);
        pet->AttackStop();
        pet->CastStop();
        if (CharmInfo* ci = pet->GetCharmInfo())
        {
            ci->SetPlayerReactState(REACT_PASSIVE);
            ci->SetCommandState(COMMAND_FOLLOW);
            ci->SetIsCommandAttack(false);
            ci->SetIsAtStay(false);
            ci->SetIsReturning(true);
            ci->SetIsFollowing(true);
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        }
    }
    return true;
}

bool TyrannusBrandRestorePetsAction::Execute(Event /*event*/)
{
    // Only the pets the brand suspended, back to the stance they had
    bool restored = false;
    for (auto const& [guid, state] : TyrannusTakeSuspendedPets(bot))
    {
        Creature* pet = ObjectAccessor::GetCreature(*bot, guid);
        if (!pet || !pet->IsAlive())
            continue;

        pet->SetReactState(state);
        if (CharmInfo* ci = pet->GetCharmInfo())
            ci->SetPlayerReactState(state);
        restored = true;
    }
    return restored;
}
