/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Razorscale.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldData.h"
#include "UldEncounter_Razorscale.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "RtiValue.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Timer.h"
#include "Unit.h"
#include "Vehicle.h"
#include <RtiTargetValue.h>
#include <TankAssistStrategy.h>

using namespace EncounterHelpers;

RazorscaleAvoidDevouringFlameAction::FlameScan const& RazorscaleAvoidDevouringFlameAction::Scan()
{
    uint32 const now = getMSTime();
    if (now == _scan.atMs && _scan.atMs)
        return _scan;

    _scan = FlameScan();
    _scan.atMs = now;

    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return _scan;

    // Widened for the main tank while she is airborne so he can hold the Dark Rune adds away from the
    // patches; on the ground he only has to clear his own footprint.
    bool const airborne = boss->GetPositionZ() >= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD;
    float const multiplier =
        (PlayerbotAI::IsMainTank(bot) && airborne) ? RazorscaleBossHelper::DEVOURING_FLAME_AIRBORNE_TANK_SCALE : 1.0f;
    _scan.clearRadius = RazorscaleBossHelper::DEVOURING_FLAME_CLEAR_RADIUS * multiplier;

    if (Unit* flame = RazorscaleBossHelper::FindDevouringFlameNear(botAI, _scan.clearRadius))
        _scan.flame = flame->GetGUID();

    return _scan;
}

bool RazorscaleAvoidDevouringFlameAction::StepClearOfFlames(Unit* flame, float clearRadius)
{
    // Measured from where the bot is standing, not from the patch: MoveAway and MoveTo both step by a
    // distance rather than to one, so a bot on the centre needs the whole radius.
    float const step = std::max(clearRadius - bot->GetDistance2d(flame) + 1.0f, 1.0f);
    float const initAngle = flame->GetAngle(bot);

    // Every candidate below sits within `step` of the bot, so one collect around him covers the whole sweep.
    std::vector<Position> flames;
    RazorscaleBossHelper::CollectDevouringFlames(bot, step + RazorscaleBossHelper::DEVOURING_FLAME_CLEAR_RADIUS,
                                                 flames);

    // Counted in whole steps: a float delta accumulating M_PI/8 overshoots the M_PI/2 bound on the
    // last iteration, which silently drops the two widest escape bearings.
    constexpr int SWEEP_STEPS = 4;
    constexpr double SWEEP_STEP_ANGLE = M_PI / 8.0;
    for (int i = 0; i <= SWEEP_STEPS; ++i)
    {
        float const delta = static_cast<float>(i) * static_cast<float>(SWEEP_STEP_ANGLE);

        for (float sign : {1.0f, -1.0f})
        {
            if (i == 0 && sign < 0.0f)
                continue;

            float const angle = initAngle + sign * delta;
            float x = bot->GetPositionX() + std::cos(angle) * step;
            float y = bot->GetPositionY() + std::sin(angle) * step;
            float z = bot->GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                                bot->GetPositionZ(), x, y, z))
                continue;

            // A destination inside another patch turns one dodge into a chain. She drops one every 6-12s
            // and they stack up.
            if (RazorscaleBossHelper::DevouringFlameBlocks(flames, x, y))
                continue;

            if (MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
                return true;
        }
    }

    // Nothing validated clear. Any step out beats standing in it.
    return MoveAway(flame, step);
}

bool RazorscaleAvoidDevouringFlameAction::Execute(Event /*event*/)
{
    FlameScan const& scan = Scan();

    Unit* flame = botAI->GetUnit(scan.flame);
    if (!flame)
        return false;

    return StepClearOfFlames(flame, scan.clearRadius);
}

bool RazorscaleAvoidDevouringFlameAction::isUseful()
{
    // Standing clear is not this action's problem: the razorscale rule in her definition keeps the
    // reach and formation nodes from walking the bot back onto a patch, and it costs no tick to do it.
    return !Scan().flame.IsEmpty();
}

bool RazorscaleAvoidSentinelAction::Execute(Event /*event*/)
{
    // No marking here: "razorscale kill target action" owns the skull all fight, so it can hand it to
    // the boss the moment she lands.
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    float const radius = RazorscaleBossHelper::SENTINEL_AVOID_RADIUS;
    // Copied: the walk continues after MoveAway, so a refill mid-loop can't pull the list out from under it.
    GuidVector const npcs = GetRazorscaleScan(botAI).Hostiles();

    bool movedAway = false;
    for (ObjectGuid const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            bot->GetDistance2d(unit) < radius)
            movedAway = MoveAway(unit, radius) || movedAway;
    }

    return movedAway;
}

bool RazorscaleAvoidSentinelAction::isUseful()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            bot->GetDistance2d(unit) < RazorscaleBossHelper::SENTINEL_AVOID_RADIUS)
            return true;
    }

    return false;
}

bool RazorscaleAvoidWhirlwindAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    float const radius = RazorscaleBossHelper::SENTINEL_AVOID_RADIUS;
    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            bot->GetDistance2d(unit) < radius)
            return MoveAway(unit, radius);
    }

    return false;
}

bool RazorscaleAvoidWhirlwindAction::isUseful()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            (unit->HasAura(RazorscaleBossHelper::SPELL_SENTINEL_WHIRLWIND) ||
             unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL)) &&
            bot->GetDistance2d(unit) < RazorscaleBossHelper::SENTINEL_AVOID_RADIUS)
            return true;
    }

    return false;
}

bool RazorscaleIgnoreBossAction::isUseful()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    if (boss->GetPositionZ() >= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        if (bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                               RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) >
            RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + RazorscaleBossHelper::RAZORSCALE_ARENA_LEASH_MARGIN)
            return true;

        if (!PlayerbotAI::IsTank(bot))
            return false;

        Group* group = bot->GetGroup();
        if (!group)
            return false;

        ObjectGuid currentMoonTarget = group->GetTargetIcon(RtiTargetValue::moonIndex);
        if (currentMoonTarget == boss->GetGUID())
            return false;

        Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
        Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

        if (mainTankUnit == bot)
            return true;

        if (mainTank && !GET_PLAYERBOT_AI(mainTank))
        {
            for (int i = 0; i < RazorscaleBossHelper::RAZORSCALE_ASSIST_TANK_SLOTS; ++i)
                if (PlayerbotAI::IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))
                    return true;
        }
    }

    return false;
}

bool RazorscaleIgnoreBossAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float const returnRadius =
        RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - RazorscaleBossHelper::RAZORSCALE_ARENA_RETURN_INSET;

    if (bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                           RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) >
        RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + RazorscaleBossHelper::RAZORSCALE_ARENA_LEASH_MARGIN)
        return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                          RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(), returnRadius,
                          MovementPriority::MOVEMENT_NORMAL);

    if (!PlayerbotAI::IsTank(bot))
        return false;

    ObjectGuid currentMoonTarget = group->GetTargetIcon(RtiTargetValue::moonIndex);
    if (currentMoonTarget == boss->GetGUID())
        return false;

    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))
    {
        for (int i = 0; i < RazorscaleBossHelper::RAZORSCALE_ASSIST_TANK_SLOTS; ++i)
        {
            if (PlayerbotAI::IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))
            {
                group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), boss->GetGUID());
                SetNextMovementDelay(1000);
                break;
            }
        }
    }
    else if (mainTankUnit == bot)
    {
        group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), boss->GetGUID());
        SetNextMovementDelay(1000);
    }

    return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                      RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(), returnRadius,
                      MovementPriority::MOVEMENT_NORMAL);
}

bool RazorscaleGroundedAction::isUseful()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive() || boss->GetPositionZ() > RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        ObjectGuid currentMoonTarget = group->GetTargetIcon(RtiTargetValue::moonIndex);

        return currentMoonTarget == boss->GetGUID();
    }

    if (PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsMainTank(bot))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        Player* mainTank = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && PlayerbotAI::IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }

        if (mainTank)
        {
            constexpr float maxDistance = 2.0f;
            float distanceToMainTank = bot->GetDistance2d(mainTank);
            return (distanceToMainTank > maxDistance);
        }
    }

    if (PlayerbotAI::IsMelee(bot))
        return false;

    if (PlayerbotAI::IsRanged(bot))
    {
        constexpr float initialLandingRadius = 14.0f;

        if (IsRazorscaleAtFirstLanding(boss))
        {
            float adjustedCenterX = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X;
            float adjustedCenterY = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y -
                                    RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_STACK_OFFSET_Y;

            float distanceToAdjustedCenter = bot->GetDistance2d(adjustedCenterX, adjustedCenterY);
            return distanceToAdjustedCenter > initialLandingRadius;
        }

        float distanceToCenter = bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                                                    RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y);
        return distanceToCenter > RazorscaleBossHelper::RAZORSCALE_RANGED_STACK_RADIUS;
    }

    return false;
}

bool RazorscaleGroundedAction::Execute(Event /*event*/)
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive() || boss->GetPositionZ() > RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))
    {
        for (int i = 0; i < RazorscaleBossHelper::RAZORSCALE_ASSIST_TANK_SLOTS; ++i)
        {
            if (PlayerbotAI::IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))
            {
                ObjectGuid currentMoonTarget = group->GetTargetIcon(RtiTargetValue::moonIndex);

                if (currentMoonTarget == boss->GetGUID())
                {
                    group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ObjectGuid::Empty);
                    SetNextMovementDelay(1000);
                    return true;
                }
            }
        }
    }
    else if (PlayerbotAI::IsMainTank(bot))
    {
        ObjectGuid currentMoonTarget = group->GetTargetIcon(RtiTargetValue::moonIndex);

        if (currentMoonTarget == boss->GetGUID())
        {
            group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ObjectGuid::Empty);
            SetNextMovementDelay(1000);
            return true;
        }
    }

    if (mainTank && (PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsMainTank(bot)))
    {
        constexpr float followDistance = 2.0f;
        return MoveNear(mainTank, followDistance, MovementPriority::MOVEMENT_COMBAT);
    }

    if (PlayerbotAI::IsRanged(bot))
    {
        if (IsRazorscaleAtFirstLanding(boss))
            return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                              RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y -
                                  RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_STACK_OFFSET_Y,
                              bot->GetPositionZ(), RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_STACK_RADIUS,
                              MovementPriority::MOVEMENT_COMBAT);

        return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                          RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(),
                          RazorscaleBossHelper::RAZORSCALE_RANGED_STACK_RADIUS, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool RazorscaleHarpoonAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    RazorscaleBossHelper razorscaleHelper(botAI);

    if (!razorscaleHelper.UpdateBossAI())
        return false;

    Unit* boss = razorscaleHelper.GetBoss();
    if (!boss || !boss->IsAlive())
        return false;

    GameObject* closestHarpoon = GetRazorscaleClosestReadyHarpoon(botAI);
    if (!closestHarpoon)
        return false;

    Player* closestRangedDPS = nullptr;
    float minDistance = std::numeric_limits<float>::max();
    GuidVector groupBots = AI_VALUE(GuidVector, "group members");

    for (ObjectGuid const& guid : groupBots)
    {
        Player* member = ObjectAccessor::FindPlayer(guid);
        if (member && member->IsAlive() && IsRazorscaleHarpoonCrew(botAI, member))
        {
            float distance = member->GetDistance2d(closestHarpoon);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestRangedDPS = member;
            }
        }
    }

    if (closestRangedDPS != bot)
        return false;

    float botDist = bot->GetDistance(closestHarpoon);
    if (botDist > INTERACTION_DISTANCE - 1.0f)
        return MoveTo(bot->GetMapId(), closestHarpoon->GetPositionX(), closestHarpoon->GetPositionY(),
                      closestHarpoon->GetPositionZ());

    SetNextMovementDelay(1000);

    {
        WorldPacket usePacket(CMSG_GAMEOBJ_USE);
        usePacket << closestHarpoon->GetGUID();
        bot->GetSession()->HandleGameObjectUseOpcode(usePacket);
    }

    {
        WorldPacket reportPacket(CMSG_GAMEOBJ_REPORT_USE);
        reportPacket << closestHarpoon->GetGUID();
        bot->GetSession()->HandleGameobjectReportUse(reportPacket);
    }

    RazorscaleBossHelper::SetHarpoonOnCooldown(closestHarpoon);

    return true;
}

bool RazorscaleHarpoonAction::isUseful()
{
    RazorscaleBossHelper razorscaleHelper(botAI);

    if (!razorscaleHelper.UpdateBossAI())
        return false;

    Unit* boss = razorscaleHelper.GetBoss();
    if (!boss || !boss->IsAlive())
        return false;

    // Before the harpoon search: anyone else gets false whatever it finds.
    if (!IsRazorscaleHarpoonCrew(botAI, bot))
        return false;

    return GetRazorscaleClosestReadyHarpoon(botAI) != nullptr;
}

bool RazorscaleFuseArmorAction::isUseful()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Aura* fuseArmor = bot->GetAura(RazorscaleBossHelper::SPELL_FUSE_ARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
            return true;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (PlayerbotAI::IsMainTank(member) && member != bot)
        {
            Aura* fuseArmor = member->GetAura(RazorscaleBossHelper::SPELL_FUSE_ARMOR);
            if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
                return true;
        }
    }

    return false;
}

bool RazorscaleFuseArmorAction::Execute(Event /*event*/)
{
    RazorscaleBossHelper bossHelper(botAI);
    bossHelper.AssignRolesBasedOnHealth();
    return true;
}

bool RazorscaleKillTargetAction::isUseful()
{
    RazorscaleKillTargetTrigger razorscaleKillTargetTrigger(botAI);
    return razorscaleKillTargetTrigger.IsActive();
}

bool RazorscaleKillTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetRazorscaleKillTarget(botAI);
    if (!target)
        return false;

    MarkTargetWithSkull(bot, target);
    SetRtiTarget(botAI, "skull", target);
    return true;
}

bool RazorscalePetControlAction::isUseful()
{
    RazorscalePetControlTrigger razorscalePetControlTrigger(botAI);
    return razorscalePetControlTrigger.IsActive();
}

bool RazorscalePetControlAction::Execute(Event /*event*/)
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    if (boss->GetPositionZ() >= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        // She takes no damage up there, so the pet's whole contribution is the adds.
        if (Unit* add = GetRazorscaleAddKillTarget(botAI))
            CommandPetAttack(botAI, add);
        else
            StopPet(botAI);
    }
    else
    {
        CommandPetAttack(botAI, boss);
    }

    // Deliberately reports failure: the order is a side effect, and the tick still belongs to the
    // dodge and positioning nodes below.
    return false;
}

bool RazorscaleFlameBreathAction::isUseful()
{
    RazorscaleFlameBreathTrigger razorscaleFlameBreathTrigger(botAI);
    return razorscaleFlameBreathTrigger.IsActive();
}

bool RazorscaleFlameBreathAction::Execute(Event /*event*/)
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Sidestep the shortest way out of the frontal cone while keeping current range
    Position const dest = GetPositionOutsideFrontalCone(bot, boss, RazorscaleBossHelper::FLAME_BREATH_CONE);
    return MoveTo(boss->GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_COMBAT);
}
