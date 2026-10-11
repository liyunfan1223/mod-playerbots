/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

// Lady Deathwhisper
bool IccDarkReckoningAction::Execute(Event /*event*/)
{
    constexpr float SAFE_DISTANCE_THRESHOLD = 2.0f;

    if (bot->HasAura(SPELL_DARK_RECKONING) &&
        bot->GetExactDist2d(ICC_DARK_RECKONING_SAFE_POSITION) > SAFE_DISTANCE_THRESHOLD)
    {
        return MoveTo(bot->GetMapId(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionX(),
                      ICC_DARK_RECKONING_SAFE_POSITION.GetPositionY(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool IccRangedPositionLadyDeathwhisperAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    float const currentDistance = bot->GetDistance2d(boss);
    float const minDistance = 7.0f;
    float const maxDistance = 30.0f;

    if (currentDistance < minDistance || currentDistance > maxDistance)
        return false;

    if (!PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    return MaintainRangedSpacing();
}

bool IccRangedPositionLadyDeathwhisperAction::MaintainRangedSpacing()
{
    float const safeSpacingRadius = 3.0f;
    float const moveIncrement = 2.0f;
    float const maxMoveDistance = 5.0f;
    bool const isRanged = PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot);

    if (!isRanged)
        return false;

    GuidVector const members = AI_VALUE(GuidVector, "group members");

    float totalX = 0.0f;
    float totalY = 0.0f;
    int nearbyCount = 0;

    for (auto const& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float const distance = bot->GetExactDist2d(member);
        if (distance < safeSpacingRadius)
        {
            float dx = bot->GetPositionX() - member->GetPositionX();
            float dy = bot->GetPositionY() - member->GetPositionY();

            float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
            totalX += dx * weight;
            totalY += dy * weight;
            ++nearbyCount;
        }
    }

    if (nearbyCount > 0)
    {
        float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
        if (magnitude > 0.001f)
        {
            totalX /= magnitude;
            totalY /= magnitude;

            float const moveDistance = std::min(moveIncrement, maxMoveDistance);

            float targetX = bot->GetPositionX() + totalX * moveDistance;
            float targetY = bot->GetPositionY() + totalY * moveDistance;
            float targetZ = bot->GetPositionZ();

            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            else
            {
                targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;
}

bool IccAddsLadyDeathwhisperAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig.EnableICCBuffs && boss->IsInCombat() && diff &&
        (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        if (!bot->HasAura(SPELL_NO_THREAT) && botAI->HasAggro(boss) && !PlayerbotAI::IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (PlayerbotAI::IsMainTank(bot) && !bot->HasAura(SPELL_SPITEFULL_FURY) && boss->GetVictim() != bot)
            bot->AddAura(SPELL_SPITEFULL_FURY, bot);
    }

    if (botAI->HasAura("Dominate Mind", bot, false, false) && !bot->HasAura(SPELL_CYCLONE))
        bot->AddAura(SPELL_CYCLONE, bot);
    else if (bot->HasAura(SPELL_CYCLONE) && !botAI->HasAura("Dominate Mind", bot, false, false))
        bot->RemoveAura(SPELL_CYCLONE);

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
            if (memberBotAI && !IsSelfBot(member))
                continue;

            if (botAI->HasAura("Dominate Mind", member, false, false) && !member->HasAura(SPELL_CYCLONE))
                member->AddAura(SPELL_CYCLONE, member);
            else if (member->HasAura(SPELL_CYCLONE) && !botAI->HasAura("Dominate Mind", member, false, false))
                member->RemoveAura(SPELL_CYCLONE);
        }
    }

    constexpr uint32 shadeEntryId = NPC_SHADE;

    if (PlayerbotAI::IsTank(bot))
    {
        if (bot->HasAura(SPELL_TOUCH_OF_INSIGNIFICANCE))
            bot->RemoveAura(SPELL_TOUCH_OF_INSIGNIFICANCE);

        if (IsTargetedByShade(shadeEntryId))
            return false;

        Unit* currentTarget = bot->GetVictim();
        bool const isAttackingAdd = currentTarget && IsAdd(currentTarget);
        if (isAttackingAdd && currentTarget->IsAlive())
        {
            if (PlayerbotAI::IsAssistTank(bot) || !IsAssistTankAlive())
            {
                constexpr float MARK_RANGE = 25.0f;
                Unit* nearAdd = FindAddNearBoss(boss, MARK_RANGE);
                UpdateRaidTargetIcon(nearAdd ? nearAdd : boss);
            }

            if (currentTarget->GetVictim() == bot)
            {
                constexpr float KITE_THRESHOLD = 25.0f;
                if (!boss->HealthAbovePct(95))
                {
                    Position const& tankPos = ICC_LDW_TANK_POSTION;
                    if (bot->GetExactDist2d(tankPos.GetPositionX(), tankPos.GetPositionY()) > KITE_THRESHOLD)
                        MoveTo(bot->GetMapId(), tankPos.GetPositionX(), tankPos.GetPositionY(), tankPos.GetPositionZ(),
                               false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
                else
                {
                    constexpr float CONSOLIDATE_THRESHOLD = 5.0f;
                    if (currentTarget->GetExactDist2d(boss) > CONSOLIDATE_THRESHOLD)
                        MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), false,
                               false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }

            return false;
        }

        Unit* targetAdd = FindAndCollectAdd(boss);
        if (targetAdd)
        {
            if (PlayerbotAI::IsAssistTank(bot) || !IsAssistTankAlive())
            {
                constexpr float MARK_RANGE = 25.0f;
                Unit* nearAdd = FindAddNearBoss(boss, MARK_RANGE);
                UpdateRaidTargetIcon(nearAdd ? nearAdd : boss);
            }
        }

        if (boss->IsAlive())
        {
            {
                constexpr float MARK_RANGE = 25.0f;
                Unit* nearAdd = FindAddNearBoss(boss, MARK_RANGE);
                UpdateRaidTargetIcon(nearAdd ? nearAdd : boss);
            }

            if (!boss->HealthAbovePct(95))
            {
                bool anyTankHasAggro = false;
                if (Group* group = bot->GetGroup())
                {
                    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                    {
                        Player* member = ref->GetSource();
                        if (member && member->IsAlive() && PlayerbotAI::IsTank(member) && boss->GetVictim() == member)
                        {
                            anyTankHasAggro = true;
                            break;
                        }
                    }
                }

                if (anyTankHasAggro)
                {
                    Position const& tankPos = ICC_LDW_TANK_POSTION;
                    if (bot->GetExactDist2d(tankPos.GetPositionX(), tankPos.GetPositionY()) > 5.0f)
                        MoveTo(bot->GetMapId(), tankPos.GetPositionX(), tankPos.GetPositionY(), tankPos.GetPositionZ(),
                               false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }

        return false;
    }

    // Phase 1 mana drain: every hunter keeps its own Viper Sting up on boss.
    // Per-hunter aura tracking via caster-GUID so multiple hunters stack cleanly.
    if (bot->getClass() == CLASS_HUNTER && boss && boss->HealthAbovePct(95) && boss->IsAlive() && !boss->HasAura(SPELL_VIPER_STING, bot->GetGUID()) && botAI->CanCastSpell(SPELL_VIPER_STING, boss))
        botAI->CastSpell("viper sting", boss);

    if (HandleNonTankAddEvasion())
        return false;

    ApplyNearbyAddCC();

    if (!boss->HealthAbovePct(95) && (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot)))
    {
        bool threatened = false;
        GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->IsAlive() && unit->GetVictim() == bot && (IsAdd(unit) || unit->GetEntry() == NPC_SHADE))
            {
                threatened = true;
                break;
            }
        }

        if (!threatened)
        {
            constexpr float REGROUP_DISTANCE = 18.0f;
            Position const& tankPos = ICC_LDW_TANK_POSTION;
            if (bot->GetExactDist2d(tankPos.GetPositionX(), tankPos.GetPositionY()) > REGROUP_DISTANCE)
                MoveTo(bot->GetMapId(), tankPos.GetPositionX(), tankPos.GetPositionY(), tankPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    if (IsAddsAlive())
        return false;

    if (boss->HealthAbovePct(95))
        return EngageBoss();

    return false;
}

Unit* IccAddsLadyDeathwhisperAction::FindAndCollectAdd(Unit* boss)
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Unit* bestAdd = nullptr;
    float bestScore = FLT_MAX;

    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!IsAdd(unit))
            continue;

        Unit* victim = unit->GetVictim();
        if (victim && victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer()))
            continue;

        float distance = bot->GetExactDist(unit);
        float score = distance;
        if (victim && victim != bot)
            score += 5.0f;

        if (score < bestScore)
        {
            bestScore = score;
            bestAdd = unit;
        }
    }

    if (!bestAdd)
        return nullptr;

    float const tauntRange = 30.0f;
    float const currentDist = bot->GetExactDist(bestAdd);

    if (currentDist > tauntRange)
    {
        float moveX = bot->GetPositionX();
        float moveY = bot->GetPositionY();
        float moveZ = bot->GetPositionZ();
        float step = 5.0f;
        float dx = bestAdd->GetPositionX() - bot->GetPositionX();
        float dy = bestAdd->GetPositionY() - bot->GetPositionY();
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.1f)
        {
            dx /= dist;
            dy /= dist;
            float moveDist = std::min(step, dist - (tauntRange - 5.0f));
            moveX = bot->GetPositionX() + dx * moveDist;
            moveY = bot->GetPositionY() + dy * moveDist;
        }
        MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        return nullptr;
    }

    bool taunted = false;
    if (botAI->CastSpell("taunt", bestAdd))
        taunted = true;
    else
    {
        switch (bot->getClass())
        {
            case CLASS_PALADIN:
                taunted = botAI->CastSpell("hand of reckoning", bestAdd);
                break;
            case CLASS_DEATH_KNIGHT:
                taunted = botAI->CastSpell("dark command", bestAdd);
                break;
            case CLASS_DRUID:
                taunted = botAI->CastSpell("growl", bestAdd);
                break;
            default:
                break;
        }
    }

    if (!taunted)
    {
        if (currentDist > 5.0f)
        {
            float dx = bestAdd->GetPositionX() - bot->GetPositionX();
            float dy = bestAdd->GetPositionY() - bot->GetPositionY();
            float dist = std::sqrt(dx * dx + dy * dy);
            dx /= dist;
            dy /= dist;
            float step = std::max(0.5f, std::min(3.0f, dist - 3.0f));
            float moveX = bot->GetPositionX() + dx * step;
            float moveY = bot->GetPositionY() + dy * step;
            MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT);
            return nullptr;
        }
    }

    if (boss && boss->HealthAbovePct(95))
    {
        constexpr float CONSOLIDATE_THRESHOLD = 5.0f;
        if (bestAdd->GetExactDist2d(boss) > CONSOLIDATE_THRESHOLD)
            MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
                   false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    return bestAdd;
}

bool IccAddsLadyDeathwhisperAction::HandleNonTankAddEvasion()
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    bool targetedByAdd = false;
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive() || unit->GetVictim() != bot)
            continue;

        if (IsAdd(unit))
        {
            targetedByAdd = true;
            ApplyCCToAdd(unit);
        }
    }

    if (!targetedByAdd)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    if (boss->HealthAbovePct(95))
    {
        Unit* closestAdd = nullptr;
        float closestDist = FLT_MAX;
        GuidVector const npcs2 = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto const& npcGuid : npcs2)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || !unit->IsAlive() || unit->GetVictim() != bot || !IsAdd(unit))
                continue;

            float d = bot->GetExactDist2d(unit);
            if (d < closestDist)
            {
                closestDist = d;
                closestAdd = unit;
            }
        }

        if (!closestAdd)
            return false;

        constexpr float KEEP_DISTANCE = 10.0f;
        if (closestDist >= KEEP_DISTANCE)
            return false;

        float dx = bot->GetPositionX() - closestAdd->GetPositionX();
        float dy = bot->GetPositionY() - closestAdd->GetPositionY();
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return false;

        dx /= len;
        dy /= len;
        float moveX = bot->GetPositionX() + dx * (KEEP_DISTANCE - closestDist + 2.0f);
        float moveY = bot->GetPositionY() + dy * (KEEP_DISTANCE - closestDist + 2.0f);
        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }
    else
    {
        constexpr float ARRIVAL_THRESHOLD = 5.0f;
        if (bot->GetExactDist2d(boss) <= ARRIVAL_THRESHOLD)
            return false;

        return MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT);
    }
}

bool IccAddsLadyDeathwhisperAction::ApplyNearbyAddCC()
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    constexpr float CC_RANGE = 10.0f;

    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!IsAdd(unit))
            continue;

        if (bot->GetDistance(unit) <= CC_RANGE)
            ApplyCCToAdd(unit);
    }

    return false;
}

bool IccAddsLadyDeathwhisperAction::ApplyCCToAdd(Unit* add)
{
    IccTryClassCC(bot, botAI, add);
    return false;
}

bool IccAddsLadyDeathwhisperAction::IsAddsAlive()
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->IsAlive() && IsAdd(unit))
            return true;
    }

    return false;
}

bool IccAddsLadyDeathwhisperAction::EngageBoss()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss || !boss->IsAlive())
        return false;

    bool const isMelee = PlayerbotAI::IsMelee(bot);
    bool const isRanged = PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot);

    if (isMelee)
    {
        float dist = bot->GetDistance(boss);
        if (dist > 25.0f)
        {
            float dx = boss->GetPositionX() - bot->GetPositionX();
            float dy = boss->GetPositionY() - bot->GetPositionY();
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.1f)
            {
                dx /= len;
                dy /= len;
                float step = std::min(3.0f, len);
                float moveX = bot->GetPositionX() + dx * step;
                float moveY = bot->GetPositionY() + dy * step;
                MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT);
                return false;
            }
        }
    }

    if (isRanged)
    {
        Position const rangedPos = ICC_LDW_RANGED_POSITION;
        float const distToPos = bot->GetDistance2d(rangedPos.GetPositionX(), rangedPos.GetPositionY());
        if (distToPos > 30.0f)
        {
            MoveTo(bot->GetMapId(), rangedPos.GetPositionX(), rangedPos.GetPositionY(), rangedPos.GetPositionZ(), false,
                   false, false, true, MovementPriority::MOVEMENT_COMBAT);
            return false;
        }
    }

    return false;
}

Unit* IccAddsLadyDeathwhisperAction::FindAddNearBoss(Unit* boss, float maxDist)
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* best = nullptr;
    float bestDist = maxDist;

    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || !unit->IsAlive() || !IsAdd(unit))
            continue;

        float dist = unit->GetExactDist2d(boss);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = unit;
        }
    }

    return best;
}

bool IccAddsLadyDeathwhisperAction::IsAdd(Unit* unit)
{
    if (!unit)
        return false;

    uint32 entry = unit->GetEntry();
    return std::find(addEntriesLady.begin(), addEntriesLady.end(), entry) != addEntriesLady.end();
}

bool IccAddsLadyDeathwhisperAction::IsAssistTankAlive()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (PlayerbotAI::IsTank(member) && !PlayerbotAI::IsMainTank(member))
            return true;
    }

    return false;
}

bool IccAddsLadyDeathwhisperAction::IsTargetedByShade(uint32 shadeEntry)
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == shadeEntry && unit->GetVictim() == bot)
            return true;
    }

    return false;
}

bool IccAddsLadyDeathwhisperAction::HandleAddTargeting(Unit* boss)
{
    GuidVector const targets = AI_VALUE(GuidVector, "possible targets no los");

    Unit* priorityTarget = nullptr;
    bool hasValidAdds = false;

    for (auto const& entry : addEntriesLady)
    {
        for (auto const& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            {
                priorityTarget = unit;
                hasValidAdds = true;
                break;
            }
        }

        if (priorityTarget)
            break;
    }

    if (!hasValidAdds && boss->IsAlive())
        priorityTarget = boss;

    if (priorityTarget)
        UpdateRaidTargetIcon(priorityTarget);

    return false;
}

bool IccAddsLadyDeathwhisperAction::UpdateRaidTargetIcon(Unit* target)
{
    static constexpr uint8 SKULL_ICON_INDEX = RtiTargetValue::skullIndex;

    if (!target || !target->IsAlive())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

    bool const currentIsAdd = currentSkullUnit && currentSkullUnit->IsAlive() && IsAdd(currentSkullUnit);
    bool const targetIsAdd = IsAdd(target);

    bool const shouldUpdate = !currentSkullUnit || !currentSkullUnit->IsAlive() || (targetIsAdd && !currentIsAdd);

    if (shouldUpdate)
        group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), target->GetGUID());

    return false;
}

bool IccShadeLadyDeathwhisperAction::Execute(Event /*event*/)
{
    static constexpr uint32 VENGEFUL_SHADE_ID = NPC_SHADE;
    static constexpr float SAFE_DISTANCE = 12.0f;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    float fleeX = 0.0f;
    float fleeY = 0.0f;
    float closestDist = SAFE_DISTANCE;
    int shadeCount = 0;

    for (auto const& npcGuid : npcs)
    {
        Unit* shade = botAI->GetUnit(npcGuid);

        if (!shade || shade->GetEntry() != VENGEFUL_SHADE_ID)
            continue;

        if (!shade->GetVictim() || shade->GetVictim()->GetGUID() != bot->GetGUID())
            continue;

        float const currentDistance = bot->GetDistance2d(shade);

        if (currentDistance >= SAFE_DISTANCE)
            continue;

        float dx = bot->GetPositionX() - shade->GetPositionX();
        float dy = bot->GetPositionY() - shade->GetPositionY();
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 0.001f)
            continue;

        float weight = (SAFE_DISTANCE - currentDistance) / dist;
        fleeX += dx * weight;
        fleeY += dy * weight;

        if (currentDistance < closestDist)
            closestDist = currentDistance;

        shadeCount++;
    }

    if (shadeCount == 0)
        return false;

    float fleeDist = std::sqrt(fleeX * fleeX + fleeY * fleeY);
    if (fleeDist < 0.001f)
        return false;

    fleeX /= fleeDist;
    fleeY /= fleeDist;

    float moveDistance = SAFE_DISTANCE - closestDist + 2.0f;
    float targetX = bot->GetPositionX() + fleeX * moveDistance;
    float targetY = bot->GetPositionY() + fleeY * moveDistance;
    float targetZ = bot->GetPositionZ();

    if (!bot->IsWithinLOS(targetX, targetY, targetZ))
    {
        moveDistance *= 0.5f;
        targetX = bot->GetPositionX() + fleeX * moveDistance;
        targetY = bot->GetPositionY() + fleeY * moveDistance;
    }

    botAI->Reset();
    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

// Rotting Giant WiP (waiting for core fix)
bool IccRottingFrostGiantTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("death plague", bot, false, false);
    if (aura)
        bot->RemoveAura(aura->GetId());

    /* TODO: code works for handling plague, but atm script is bugged and one bot can have 2 plagues at the same time or
       when cured, which should not happen, and it is immpossible to handle plague atm the legit way. const bool hasCure
       = botAI->GetAura("recently infected", bot) != nullptr;

        // Tank behavior - unchanged
        if (PlayerbotAI::IsTank(bot) && botAI->HasAggro(boss) && !isInfected)
            if (bot->GetExactDist2d(ICC_ROTTING_FROST_GIANT_TANK_POSITION) > 5.0f)
                return MoveTo(bot->GetMapId(), ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionX(),
                              ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionY(),
                              ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_NORMAL);

        if (PlayerbotAI::IsTank(bot))
            return false;

        // Handle infected bot behavior - move near a non-infected, non-cured bot
        if (isInfected)
        {
            const GuidVector members = AI_VALUE(GuidVector, "group members");

            // Count how many bots are targeting each potential target
            std::map<ObjectGuid, int> targetCounts;

            // First, identify all infected bots and their current targets (approximate)
            for (auto const& memberGuid : members)
            {
                Unit* member = botAI->GetUnit(memberGuid);
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;

                if (memberIsInfected)
                {
                    // Find the nearest non-infected bot to this infected bot (as a guess of its target)
                    float minDist = 5.0f;  // Only count if they're close enough to likely be targeting
                    Unit* likelyTarget = nullptr;

                    for (auto const& targetGuid : members)
                    {
                        Unit* potentialTarget = botAI->GetUnit(targetGuid);
                        if (!potentialTarget || !potentialTarget->IsAlive() || potentialTarget == member)
                            continue;

                        const bool targetIsInfected = botAI->GetAura("death plague", potentialTarget) != nullptr;
                        const bool targetHasCure = botAI->GetAura("recently infected", potentialTarget) != nullptr;

                        if (!targetIsInfected && !targetHasCure)
                        {
                            float dist = member->GetExactDist2d(potentialTarget);
                            if (dist < minDist)
                            {
                                minDist = dist;
                                likelyTarget = potentialTarget;
                            }
                        }
                    }

                    if (likelyTarget)
                    {
                        targetCounts[likelyTarget->GetGUID()]++;
                    }
                }
            }

            // Find viable targets and score them based on various factors
            std::vector<std::pair<Unit*, float>> viableTargets;

            // First try to find ranged, non-infected, non-cured bots
            for (auto const& memberGuid : members)
            {
                Unit* member = botAI->GetUnit(memberGuid);
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                const bool memberHasCure = botAI->GetAura("recently infected", member) != nullptr;
                const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;

                if (!memberIsInfected && !memberHasCure)
                {
                    // Base score is distance (lower is better)
                    float score = bot->GetExactDist2d(member);

                    // Prefer ranged targets
                    if (PlayerbotAI::IsRanged(bot))
                    {
                        score *= 0.7f;  // Bonus for ranged targets
                    }

                    // Apply penalty based on how many other infected bots are targeting this one
                    int targetingCount = targetCounts[member->GetGUID()];
                    score *= (1.0f + targetingCount * 0.5f);  // Increase score (worse) for heavily targeted bots

                    viableTargets.push_back(std::make_pair(member, score));
                }
            }

            // Sort targets by score (lowest/best first)
            std::sort(viableTargets.begin(), viableTargets.end(),
                      [](const std::pair<Unit*, float>& a, const std::pair<Unit*, float>& b)
                      { return a.second < b.second; });

            // Choose the best target
            Unit* targetBot = nullptr;
            if (!viableTargets.empty())
            {
                targetBot = viableTargets[0].first;
            }

            // Move to target bot if found
            if (targetBot)
            {
                // If we're already close enough (1 yard), no need to move
                if (bot->GetExactDist2d(targetBot) > 1.0f)
                {
                    // Calculate a unique angle based on bot's GUID to ensure different approach angles
                    // This helps spread infected bots around the target
                    uint32 guidLow = bot->GetGUID().GetCounter();
                    float angleOffset = float(guidLow % 628) / 100.0f;  // Random angle between 0 and 2π

                    // Calculate position 1 yard away from target at our unique angle
                    float angle = targetBot->GetOrientation() + angleOffset;
                    float targetX = targetBot->GetPositionX() + cos(angle) * 1.0f;
                    float targetY = targetBot->GetPositionY() + sin(angle) * 1.0f;
                    float targetZ = targetBot->GetPositionZ();

                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                                  MovementPriority::MOVEMENT_COMBAT);
                }
                return true;  // Already in position
            }
            // No suitable target found, continue with normal behavior
        }

        // For ranged bots, only spread from non-infected bots
        if (PlayerbotAI::IsRanged(bot))
        {
            const float safeSpacingRadius = 11.0f;
            const float moveIncrement = 2.0f;
            const float maxMoveDistance = 15.0f;

            const GuidVector members = AI_VALUE(GuidVector, "group members");

            // Calculate a combined vector representing all nearby NON-INFECTED members' positions
            float totalX = 0.0f;
            float totalY = 0.0f;
            int nearbyCount = 0;

            for (auto const& memberGuid : members)
            {
                Unit* member = botAI->GetUnit(memberGuid);
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                // Only spread from non-infected bots (can stay near infected or cured bots)
                const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;
                if (memberIsInfected)
                    continue;

                const float distance = bot->GetExactDist2d(member);
                if (distance < safeSpacingRadius)
                {
                    // Calculate vector from member to bot
                    float dx = bot->GetPositionX() - member->GetPositionX();
                    float dy = bot->GetPositionY() - member->GetPositionY();

                    // Weight by inverse distance (closer members have more influence)
                    float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
                    totalX += dx * weight;
                    totalY += dy * weight;
                    nearbyCount++;
                }
            }

            // If we have nearby non-infected members, move away in the combined direction
            if (nearbyCount > 0)
            {
                // Normalize the combined vector
                float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
                if (magnitude > 0.001f)  // Avoid division by zero
                {
                    totalX /= magnitude;
                    totalY /= magnitude;

                    // Calculate move distance based on nearest member
                    float moveDistance = std::min(moveIncrement, maxMoveDistance);

                    // Create target position in the combined direction
                    float targetX = bot->GetPositionX() + totalX * moveDistance;
                    float targetY = bot->GetPositionY() + totalY * moveDistance;
                    float targetZ = bot->GetPositionZ();  // Maintain current Z

                    // Check if the target position is valid and move there
                    if (bot->IsWithinLOS(targetX, targetY, targetZ))
                    {
                        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }
                    else
                    {
                        // If los check fails, try shorter distance
                        targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                        targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }
                }
            }
        }
    */
    return false;
}
