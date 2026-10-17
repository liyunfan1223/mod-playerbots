/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCScripts.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Timer.h"
#include "Vehicle.h"
#include <array>
#include <cmath>
#include <map>

// Rotface
bool IccRotfaceTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");

    MarkBossWithSkull(boss);

    if (PlayerbotAI::IsMainTank(bot))
    {
        bool assistTankAlive = false;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && member != bot && PlayerbotAI::IsAssistTank(member))
                {
                    assistTankAlive = true;
                    break;
                }
            }
        }

        Unit* bigOoze = bot->FindNearestCreature(NPC_BIG_OOZE, 100.0f);
        bool const bigOozeHandled = bigOoze && bigOoze->IsAlive() && bigOoze->GetVictim() &&
                                    bigOoze->GetVictim()->IsPlayer() &&
                                    PlayerbotAI::IsAssistTank(bigOoze->GetVictim()->ToPlayer());

        if (bigOoze && bigOoze->IsAlive() && !assistTankAlive && !bigOozeHandled)
            return HandleAssistTankPositioning(boss);

        return PositionMainTankAndMelee(boss, smallOoze);
    }

    if (PlayerbotAI::IsAssistTank(bot))
        return HandleAssistTankPositioning(boss);

    return false;
}

bool IccRotfaceTankPositionAction::MarkBossWithSkull(Unit* boss)
{
    IccEnsureIconOn(bot, botAI, RtiTargetValue::skullIndex, boss);
    return false;
}

bool IccRotfaceTankPositionAction::PositionMainTankAndMelee(Unit* boss, Unit*)
{
    bool isBossCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
        isBossCasting = true;

    if (PlayerbotAI::IsMainTank(bot) && boss && boss->GetVictim() == bot)
    {
        bool const bossInPosition = boss->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION_BOSS) <= 4.0f;

        if (!bossInPosition)
        {
            // Step 2y in the direction from boss toward center — tank leads, boss follows
            float dirX = ICC_ROTFACE_CENTER_POSITION_BOSS.GetPositionX() - boss->GetPositionX();
            float dirY = ICC_ROTFACE_CENTER_POSITION_BOSS.GetPositionY() - boss->GetPositionY();
            float const len = std::sqrt(dirX * dirX + dirY * dirY);
            if (len > 0.5f)
            {
                dirX /= len;
                dirY /= len;
            }
            float const destX = bot->GetPositionX() + dirX * 2.0f;
            float const destY = bot->GetPositionY() + dirY * 2.0f;
            MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT);
        }
        else if (bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION) > 7.0f)
        {
            MoveTo(bot->GetMapId(), ICC_ROTFACE_CENTER_POSITION.GetPositionX(),
                   ICC_ROTFACE_CENTER_POSITION.GetPositionY(), ICC_ROTFACE_CENTER_POSITION.GetPositionZ(), false, false,
                   false, true, MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            if (bot->GetVictim() != boss)
                bot->Attack(boss, true);
        }
    }

    if (boss && isBossCasting && !PlayerbotAI::IsTank(bot))
    {
        float const x = boss->GetPositionX();
        float const y = boss->GetPositionY();
        float const z = boss->GetPositionZ();

        if (bot->GetExactDist2d(x, y) > 0.5f)
            MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                   false);
    }

    return false;
}

bool IccRotfaceTankPositionAction::HandleAssistTankPositioning(Unit*)
{
    GuidVector bigOozes = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> activeBigOozes;

    for (auto const& guid : bigOozes)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_BIG_OOZE && unit->IsVisible())
            activeBigOozes.push_back(unit);
    }

    if (activeBigOozes.empty())
        return false;

    Unit* uncollectedOoze = nullptr;
    float minUncollectedDist = FLT_MAX;
    for (Unit* ooze : activeBigOozes)
    {
        if (ooze->GetVictim() == bot)
            continue;

        if (!bot->HasAura(SPELL_SPITEFULL_FURY))
            bot->AddAura(SPELL_SPITEFULL_FURY, bot);
        IccCastClassTaunt(bot, botAI, ooze);

        float const dist = bot->GetExactDist2d(ooze);
        if (dist < minUncollectedDist)
        {
            minUncollectedDist = dist;
            uncollectedOoze = ooze;
        }
    }

    if (uncollectedOoze)
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);
        return MoveTo(bot->GetMapId(), uncollectedOoze->GetPositionX(), uncollectedOoze->GetPositionY(),
                      uncollectedOoze->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    Unit* targetOoze = nullptr;
    float minDist = FLT_MAX;
    for (Unit* ooze : activeBigOozes)
    {
        float const dist = bot->GetExactDist2d(ooze);
        if (dist < minDist)
        {
            minDist = dist;
            targetOoze = ooze;
        }
    }

    if (!targetOoze)
        return false;

    return HandleBigOozeKiting(targetOoze);
}

Unit* IccRotfaceTankPositionAction::FindAssignedBigOoze(Unit* /*boss*/, std::vector<Unit*>& bigOozes)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* bestOoze = nullptr;
    float minDistance = FLT_MAX;

    for (Unit* ooze : bigOozes)
    {
        if (!ooze || !ooze->IsAlive() || !ooze->IsVisible())
            continue;

        ObjectGuid oozeGuid = ooze->GetGUID();

        bool isAssignedToOther = false;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsInWorld() || member == bot)
                continue;

            Unit* memberTarget = botAI->GetUnit(member->GetTarget());
            if (memberTarget && memberTarget->GetGUID() == oozeGuid)
            {
                isAssignedToOther = true;
                break;
            }
        }

        if (!isAssignedToOther)
        {
            float const dist = bot->GetExactDist2d(ooze);
            if (dist < minDistance)
            {
                minDistance = dist;
                bestOoze = ooze;
            }
        }
    }

    return bestOoze;
}

bool IccRotfaceTankPositionAction::HandleBigOozeKiting(Unit* bigOoze)
{
    if (bigOoze->GetVictim() != bot && bigOoze->IsAlive())
    {
        if (!bot->HasAura(SPELL_SPITEFULL_FURY))
            bot->AddAura(SPELL_SPITEFULL_FURY, bot);
        IccCastClassTaunt(bot, botAI, bigOoze);
    }

    float const oozeDistance = bot->GetExactDist2d(bigOoze);

    if (oozeDistance > 12.0f)
    {
        bot->SetTarget(bigOoze->GetGUID());
        bot->SetFacingToObject(bigOoze);
        if (bigOoze->GetVictim() != bot)
        {
            if (!bot->HasAura(SPELL_SPITEFULL_FURY))
                bot->AddAura(SPELL_SPITEFULL_FURY, bot);
            IccCastClassTaunt(bot, botAI, bigOoze);
        }
        return false;
    }

    float const minRadius = 24.0f;
    float const maxRadius = 34.0f;
    float const safeDistanceFromOoze = 13.0f;

    float const currentDistance = bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION);

    if (currentDistance < minRadius || currentDistance > maxRadius)
    {
        float dirX = bot->GetPositionX() - ICC_ROTFACE_CENTER_POSITION.GetPositionX();
        float dirY = bot->GetPositionY() - ICC_ROTFACE_CENTER_POSITION.GetPositionY();
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        dirX /= length;
        dirY /= length;

        float const targetX = ICC_ROTFACE_CENTER_POSITION.GetPositionX() + dirX * maxRadius;
        float const targetY = ICC_ROTFACE_CENTER_POSITION.GetPositionY() + dirY * maxRadius;

        if (bigOoze->GetExactDist2d(targetX, targetY) >= safeDistanceFromOoze)
            return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
    }

    float currentAngle = std::atan2(bot->GetPositionY() - ICC_ROTFACE_CENTER_POSITION.GetPositionY(),
                                    bot->GetPositionX() - ICC_ROTFACE_CENTER_POSITION.GetPositionX());

    for (int32 i = 0; i < 16; ++i)
    {
        float const angleOffset = (i % 2 == 0 ? 1 : -1) * (M_PI / 16.0f) * (i / 2.0f);
        float const newAngle = currentAngle + angleOffset;

        float const newX = ICC_ROTFACE_CENTER_POSITION.GetPositionX() + maxRadius * std::cos(newAngle);
        float const newY = ICC_ROTFACE_CENTER_POSITION.GetPositionY() + maxRadius * std::sin(newAngle);

        if (bigOoze->GetExactDist2d(newX, newY) >= safeDistanceFromOoze)
        {
            GuidVector puddles = AI_VALUE(GuidVector, "nearest hostile npcs");
            bool isSafeFromPuddles = true;

            for (auto const& puddleGuid : puddles)
            {
                Unit* puddle = botAI->GetUnit(puddleGuid);
                if (puddle && botAI->GetAura("Ooze Flood", puddle))
                {
                    float const puddleDistance = puddle->GetDistance2d(newX, newY);
                    if (puddleDistance < 30.0f)
                    {
                        isSafeFromPuddles = false;
                        break;
                    }
                }
            }

            if (isSafeFromPuddles)
            {
                MoveTo(bot->GetMapId(), newX, newY, bot->GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT);
                return true;
            }
        }
    }

    return false;
}

bool IccRotfaceGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    bool const hasOozeFlood = botAI->HasAura("Ooze Flood", bot);
    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");

    if (!PlayerbotAI::IsTank(bot) && HandlePuddleAvoidance(boss))
        return true;

    if (HandleOozeTargeting())
        return true;

    if (!(smallOoze && smallOoze->GetVictim() == bot) && !hasOozeFlood && PositionRangedAndHealers(boss, smallOoze))
        return true;

    return false;
}

bool IccRotfaceGroupPositionAction::HandlePuddleAvoidance(Unit* boss)
{
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !botAI->HasAura("Ooze Flood", unit))
            continue;

        float const puddleDistance = bot->GetExactDist2d(unit);
        float const bossDistance = bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION);

        if (bossDistance < 15.0f)
            return false;

        if (puddleDistance < 30.0f)
            return MoveAwayFromPuddle(boss, unit, puddleDistance);
    }

    return false;
}

bool IccRotfaceGroupPositionAction::MoveAwayFromPuddle(Unit* boss, Unit* puddle, float)
{
    if (!boss || !puddle)
        return false;

    float const dx = puddle->GetPositionX() - bot->GetPositionX();
    float const dy = puddle->GetPositionY() - bot->GetPositionY();
    float const angle = std::atan2(dy, dx);

    float const stepSize = 7.0f;
    float const minPuddleDistance = 30.0f;
    float const minCenterDistance = 15.0f;
    float const maxCenterDistance = 25.0f;
    int32 const directions = 8;

    for (int32 i = 0; i < directions; ++i)
    {
        float const testAngle = angle + M_PI + (i * M_PI / 4);
        float const moveX = bot->GetPositionX() + stepSize * std::cos(testAngle);
        float const moveY = bot->GetPositionY() + stepSize * std::sin(testAngle);
        float const moveZ = bot->GetPositionZ();

        float const newPuddleDistance = puddle->GetDistance2d(moveX, moveY);
        float const newCenterDistance = std::sqrt(std::pow(moveX - ICC_ROTFACE_CENTER_POSITION.GetPositionX(), 2) +
                                                  std::pow(moveY - ICC_ROTFACE_CENTER_POSITION.GetPositionY(), 2));

        if (newPuddleDistance >= minPuddleDistance && newCenterDistance >= minCenterDistance &&
            newCenterDistance <= maxCenterDistance && bot->IsWithinLOS(moveX, moveY, moveZ))
        {
            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool IccRotfaceGroupPositionAction::HandleOozeTargeting()
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Player* mainTankWithOoze = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !PlayerbotAI::IsMainTank(member))
                continue;

            for (auto const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SMALL_OOZE && unit->GetVictim() == member)
                {
                    mainTankWithOoze = member;
                    break;
                }
            }
            break;
        }
    }

    Aura* infectionAura = botAI->GetAura("Mutated Infection", bot, false, false);

    // Find assist tank currently kiting a Big Ooze — preferred merge point
    Player* assistTankKiting = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !PlayerbotAI::IsAssistTank(member))
                continue;

            for (auto const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && unit->IsAlive() && unit->GetEntry() == NPC_BIG_OOZE && unit->GetVictim() == member)
                {
                    assistTankKiting = member;
                    break;
                }
            }
            if (assistTankKiting)
                break;
        }
    }

    bool const hasSmallOoze = [&]() -> bool {
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SMALL_OOZE && unit->GetVictim() == bot)
                return true;
        }
        return false;
    }();

    bool const needsMerge = hasSmallOoze || (infectionAura && infectionAura->GetDuration() <= 4000);

    if (needsMerge)
    {
        if (assistTankKiting && bot->GetExactDist2d(assistTankKiting) > 2.0f)
            return MoveTo(bot->GetMapId(), assistTankKiting->GetPositionX(), assistTankKiting->GetPositionY(),
                          assistTankKiting->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED);

        if (!assistTankKiting)
        {
            if (mainTankWithOoze && bot->GetExactDist2d(mainTankWithOoze) > 2.0f)
                return MoveTo(bot->GetMapId(), mainTankWithOoze->GetPositionX(), mainTankWithOoze->GetPositionY(),
                              mainTankWithOoze->GetPositionZ(), false, false, false, true,
                              MovementPriority::MOVEMENT_FORCED);

            return HandleOozeMemberPositioning(nullptr);
        }
    }

    return false;
}

bool IccRotfaceGroupPositionAction::HandleOozeMemberPositioning(Unit* /*mySmallOoze*/)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> bigOozes;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->IsVisible() && unit->GetEntry() == NPC_BIG_OOZE)
            bigOozes.push_back(unit);
    }

    if (!bigOozes.empty())
    {
        Unit* target = nullptr;
        float minDist = FLT_MAX;

        for (Unit* ooze : bigOozes)
        {
            Unit* victim = ooze->GetVictim();
            if (victim && victim->IsPlayer() && PlayerbotAI::IsAssistTank(victim->ToPlayer()))
            {
                float const dist = bot->GetExactDist2d(ooze);
                if (dist < minDist)
                {
                    minDist = dist;
                    target = ooze;
                }
            }
        }

        if (!target)
        {
            for (Unit* ooze : bigOozes)
            {
                float const dist = bot->GetExactDist2d(ooze);
                if (dist < minDist)
                {
                    minDist = dist;
                    target = ooze;
                }
            }
        }

        if (target && bot->GetExactDist2d(target) > 2.0f && !PlayerbotAI::IsAssistTank(bot))
            return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_FORCED);

        return false;
    }

    if (bot->GetExactDist2d(ICC_ROTFACE_BIG_OOZE_POSITION) > 5.0f)
        return MoveTo(bot->GetMapId(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionX(),
                      ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionY(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED);

    return false;
}

bool IccRotfaceGroupPositionAction::PositionRangedAndHealers(Unit* boss, Unit* smallOoze)
{
    if (!(PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot)))
        return false;

    if (smallOoze && smallOoze->GetVictim() == bot)
        return false;

    Difficulty const diff = bot->GetRaidDifficulty();
    bool isBossCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_SLIME_SPRAY))
        isBossCasting = true;

    bool const isHeroic = (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC);

    if (boss && isBossCasting && !isHeroic)
    {
        float const x = boss->GetPositionX();
        float const y = boss->GetPositionY();
        float const z = boss->GetPositionZ();

        if (bot->GetExactDist2d(x, y) > 0.5f)
        {
            MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                   false);
        }
        return false;
    }

    if (!isHeroic && !isBossCasting && boss && bot->getClass() != CLASS_HUNTER &&
        (bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) < 2.0f ||
         bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) > 5.0f))
    {
        float const angle =
            std::atan2(bot->GetPositionY() - boss->GetPositionY(), bot->GetPositionX() - boss->GetPositionX());
        float const destX = boss->GetPositionX() + 3.5f * std::cos(angle);
        float const destY = boss->GetPositionY() + 3.5f * std::sin(angle);
        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    if (!isHeroic)
        return false;

    return PositionHeroicGrid(boss);
}

bool IccRotfaceGroupPositionAction::PositionHeroicGrid(Unit* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static Position const rangedSpots[] = {
        ICC_ROTFACE_RANGED_POSITION_HC_1,  ICC_ROTFACE_RANGED_POSITION_HC_2,  ICC_ROTFACE_RANGED_POSITION_HC_3,
        ICC_ROTFACE_RANGED_POSITION_HC_4,  ICC_ROTFACE_RANGED_POSITION_HC_5,  ICC_ROTFACE_RANGED_POSITION_HC_6,
        ICC_ROTFACE_RANGED_POSITION_HC_7,  ICC_ROTFACE_RANGED_POSITION_HC_8,  ICC_ROTFACE_RANGED_POSITION_HC_9,
        ICC_ROTFACE_RANGED_POSITION_HC_10, ICC_ROTFACE_RANGED_POSITION_HC_11, ICC_ROTFACE_RANGED_POSITION_HC_12,
        ICC_ROTFACE_RANGED_POSITION_HC_13, ICC_ROTFACE_RANGED_POSITION_HC_14, ICC_ROTFACE_RANGED_POSITION_HC_15,
        ICC_ROTFACE_RANGED_POSITION_HC_16, ICC_ROTFACE_RANGED_POSITION_HC_17, ICC_ROTFACE_RANGED_POSITION_HC_18,
        ICC_ROTFACE_RANGED_POSITION_HC_19, ICC_ROTFACE_RANGED_POSITION_HC_20, ICC_ROTFACE_RANGED_POSITION_HC_21,
        ICC_ROTFACE_RANGED_POSITION_HC_22,
    };
    constexpr int32 totalSpots = 22;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> floodPuddles;
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && botAI->HasAura("Ooze Flood", unit))
            floodPuddles.push_back(unit);
    }

    bool spotFlooded[totalSpots] = {};
    for (int32 i = 0; i < totalSpots; ++i)
    {
        for (Unit* puddle : floodPuddles)
        {
            if (puddle->GetExactDist2d(rangedSpots[i].GetPositionX(), rangedSpots[i].GetPositionY()) < 30.0f)
            {
                spotFlooded[i] = true;
                break;
            }
        }
    }

    std::vector<ObjectGuid> hunterGuids;
    std::vector<ObjectGuid> rangedGuids;
    std::vector<ObjectGuid> healerGuids;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;

        if (!GET_PLAYERBOT_AI(member))
            continue;

        ObjectGuid const guid = member->GetGUID();
        if (PlayerbotAI::IsHeal(member))
            healerGuids.push_back(guid);
        else if (PlayerbotAI::IsRanged(member))
        {
            if (member->getClass() == CLASS_HUNTER)
                hunterGuids.push_back(guid);
            else
                rangedGuids.push_back(guid);
        }
    }
    std::sort(hunterGuids.begin(), hunterGuids.end());
    std::sort(rangedGuids.begin(), rangedGuids.end());
    std::sort(healerGuids.begin(), healerGuids.end());

    std::vector<ObjectGuid> ordered;
    for (auto const& g : hunterGuids)
        ordered.push_back(g);
    for (auto const& g : rangedGuids)
        ordered.push_back(g);
    for (auto const& g : healerGuids)
        ordered.push_back(g);

    auto it = std::find(ordered.begin(), ordered.end(), bot->GetGUID());
    if (it == ordered.end())
        return false;

    int32 const botRank = static_cast<int32>(std::distance(ordered.begin(), it));
    int32 const totalBots = static_cast<int32>(ordered.size());

    bool const hasHomeSpot = (botRank < totalSpots);
    bool const homeSpotSafe = hasHomeSpot && !spotFlooded[botRank];

    auto moveTowardSpot = [&](Position const& spot) -> bool
    {
        float const dx = spot.GetPositionX() - bot->GetPositionX();
        float const dy = spot.GetPositionY() - bot->GetPositionY();
        float const dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= 1.0f)
            return false;
        constexpr float moveStep = 7.0f;
        float const step = std::min(moveStep, dist);
        float const targetX = bot->GetPositionX() + (dx / dist) * step;
        float const targetY = bot->GetPositionY() + (dy / dist) * step;

        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);

        return MoveTo(bot->GetMapId(), targetX, targetY, spot.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);
    };

    if (homeSpotSafe)
        return moveTowardSpot(rangedSpots[botRank]);

    std::vector<int32> tempSpots;
    for (int32 i = 0; i < totalSpots; ++i)
    {
        if (i >= totalBots && !spotFlooded[i])
            tempSpots.push_back(i);
    }

    if (hasHomeSpot)
    {
        int32 displacedRank = 0;
        for (int32 rank = 0; rank < totalSpots && rank < totalBots; ++rank)
        {
            if (rank == botRank)
                break;
            if (spotFlooded[rank])
                displacedRank++;
        }

        if (displacedRank < static_cast<int32>(tempSpots.size()))
            return moveTowardSpot(rangedSpots[tempSpots[displacedRank]]);
    }

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && !PlayerbotAI::IsTank(bot))
    {
        float const bx = boss->GetPositionX();
        float const by = boss->GetPositionY();
        float const bz = boss->GetPositionZ();
        if (bot->GetExactDist2d(bx, by) > 0.5f)
            return MoveTo(bot->GetMapId(), bx, by, bz, false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                          true, false);
        return false;
    }

    if (Group* grp = bot->GetGroup())
    {
        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;
            if (PlayerbotAI::IsMainTank(member))
            {
                if (bot->getClass() == CLASS_HUNTER)
                {
                    float const distToTank = bot->GetExactDist2d(member->GetPositionX(), member->GetPositionY());
                    if (distToTank <= 12.0f)
                        return false;

                    if (!bot->HasAura(SPELL_NITRO_BOOSTS))
                        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

                    return MoveTo(bot->GetMapId(), member->GetPositionX(), member->GetPositionY(),
                                  member->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
                }

                Position const mainTankPos(member->GetPositionX(), member->GetPositionY(), member->GetPositionZ());
                return moveTowardSpot(mainTankPos);
            }
        }
    }
    return false;
}

bool IccRotfaceMoveAwayFromExplosionAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Creature* bigOoze = bot->FindNearestCreature(NPC_BIG_OOZE, 100.0f);
    bool const castingNow = bigOoze && bigOoze->IsAlive() && bigOoze->HasUnitState(UNIT_STATE_CASTING) &&
                            bigOoze->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION);

    uint32 const now = getMSTime();
    bool const stillHolding = _hasEscape && _holdUntil != 0 && now < _holdUntil;

    if (!castingNow && !stillHolding)
    {
        _hasEscape = false;
        _holdUntil = 0;
        return false;
    }

    if (!bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

    Position const anchor = ICC_ROTFACE_CENTER_POSITION_BOSS;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> puddles;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && botAI->HasAura("Ooze Flood", unit))
            puddles.push_back(unit);
    }

    auto isSlotSafe = [&](float x, float y) -> bool
    {
        for (Unit* puddle : puddles)
        {
            if (puddle->GetDistance2d(x, y) < 30.0f)
                return false;
        }
        return true;
    };

    constexpr uint32 TOTAL_SLOTS = 25;
    constexpr float ESCAPE_RADIUS = 40.0f;

    auto slotPos = [&](uint32 i) -> std::pair<float, float>
    {
        float const angle = static_cast<float>(i) * (2.0f * static_cast<float>(M_PI) / static_cast<float>(TOTAL_SLOTS));
        return {anchor.GetPositionX() + ESCAPE_RADIUS * std::cos(angle),
                anchor.GetPositionY() + ESCAPE_RADIUS * std::sin(angle)};
    };

    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    auto& sExplosionSlotMemory = IcecrownHelpers::IccState(instanceId).rfExplosionSlotMemory;
    auto const myKey = bot->GetGUID();

    // count how many OTHER bots in this instance occupy each slot
    std::array<int32, TOTAL_SLOTS> otherCount{};
    for (auto const& [key, slot] : sExplosionSlotMemory)
    {
        if (key != bot->GetGUID())
            ++otherCount[slot];
    }

    auto assignSlot = [&]() -> int32
    {
        int32 bestSlot = -1;
        float bestDist = FLT_MAX;
        for (uint32 i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (otherCount[i] >= 2)
                continue;
            auto [x, y] = slotPos(i);
            if (!isSlotSafe(x, y))
                continue;
            float const d = bot->GetExactDist2d(x, y);
            if (d < bestDist)
            {
                bestDist = d;
                bestSlot = static_cast<int32>(i);
            }
        }
        return bestSlot;
    };

    auto currentSlotIt = sExplosionSlotMemory.find(myKey);
    bool needsAssignment = true;

    if (currentSlotIt != sExplosionSlotMemory.end())
    {
        int32 const mySlot = currentSlotIt->second;
        auto [x, y] = slotPos(mySlot);
        if (otherCount[mySlot] < 2 && isSlotSafe(x, y))
            needsAssignment = false;
    }

    if (needsAssignment)
    {
        int32 const slot = assignSlot();
        if (slot >= 0)
            sExplosionSlotMemory[myKey] = slot;
        else
            sExplosionSlotMemory.erase(myKey);
    }

    auto finalSlotIt = sExplosionSlotMemory.find(myKey);

    Position escapePos;
    if (finalSlotIt != sExplosionSlotMemory.end())
    {
        auto [x, y] = slotPos(finalSlotIt->second);
        escapePos = Position(x, y, anchor.GetPositionZ());
    }
    else
    {
        escapePos = anchor;
    }

    _escapePosition = escapePos;
    _hasEscape = true;

    if (castingNow)
    {
        _holdUntil = now + 2000;
        if (bot->GetExactDist2d(_escapePosition) > 2.0f)
            return MoveTo(bot->GetMapId(), _escapePosition.GetPositionX(), _escapePosition.GetPositionY(),
                          _escapePosition.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED);
        return false;
    }

    // Cast finished but still inside 2s hold window: stay at escape spot.
    if (bot->GetExactDist2d(_escapePosition) <= 2.0f)
        return true;

    return false;
}

bool IccRotfaceAvoidVileGasAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();

    IcecrownHelpers::IccInstanceState& st = IcecrownHelpers::IccState(bot->GetMap()->GetInstanceId());
    bool const isVictim =
        st.rotfaceVileGas.victimGuid == bot->GetGUID() &&
        getMSTimeDiff(st.rotfaceVileGas.castTime, now) < 8000;
    bool const hasAura = botAI->HasAura("Vile Gas", bot);

    auto& waitMap = st.rotfaceVileGasWaitUntil;
    auto waitIt = waitMap.find(bot->GetGUID());
    bool const inWait = waitIt != waitMap.end() && now < waitIt->second;

    if (!isVictim && !hasAura && !inWait)
    {
        if (waitIt != waitMap.end())
            waitMap.erase(waitIt);
        _hasSafeSpot = false;
        return false;
    }

    Position const anchor = ICC_ROTFACE_CENTER_POSITION_BOSS;

    Creature* bigOoze = bot->FindNearestCreature(NPC_BIG_OOZE, 100.0f);

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> puddles;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && botAI->HasAura("Ooze Flood", unit))
            puddles.push_back(unit);
    }

    auto isSafe = [&](float x, float y, float z) -> bool
    {
        if (bigOoze && bigOoze->IsAlive() && bigOoze->GetDistance2d(x, y) < 20.0f)
            return false;

        for (Unit* puddle : puddles)
        {
            if (puddle->GetDistance2d(x, y) < 30.0f)
                return false;
        }

        if (!bot->IsWithinLOS(x, y, z))
            return false;

        return true;
    };

    if (!_hasSafeSpot)
    {
        float const baseAngle = std::atan2(bot->GetPositionY() - anchor.GetPositionY(),
                                           bot->GetPositionX() - anchor.GetPositionX());

        constexpr float escapeDistance = 40.0f;
        constexpr int32 sweepSteps = 24;
        constexpr float stepAngle = 2.0f * static_cast<float>(M_PI) / sweepSteps;

        float chosenAngle = baseAngle;
        for (int32 i = 0; i < sweepSteps; ++i)
        {
            int32 const sign = (i % 2 == 0) ? 1 : -1;
            int32 const magnitude = (i + 1) / 2;
            float const angle = baseAngle + sign * magnitude * stepAngle;
            float const x = anchor.GetPositionX() + escapeDistance * std::cos(angle);
            float const y = anchor.GetPositionY() + escapeDistance * std::sin(angle);
            float const z = anchor.GetPositionZ();
            if (isSafe(x, y, z))
            {
                chosenAngle = angle;
                break;
            }
        }

        _safeSpot = Position(anchor.GetPositionX() + escapeDistance * std::cos(chosenAngle),
                             anchor.GetPositionY() + escapeDistance * std::sin(chosenAngle),
                             anchor.GetPositionZ());
        _hasSafeSpot = true;
    }

    if (bot->GetExactDist2d(_safeSpot) > 2.0f)
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);

        return MoveTo(bot->GetMapId(), _safeSpot.GetPositionX(), _safeSpot.GetPositionY(),
                      _safeSpot.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED);
    }

    if (!inWait)
        waitMap[bot->GetGUID()] = now + 4000;

    return true;
}
