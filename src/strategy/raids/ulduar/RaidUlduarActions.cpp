
#include "RaidUlduarActions.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <cmath>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaidUlduarBossHelper.h"
#include "RaidUlduarScripts.h"
#include "RaidUlduarStrategy.h"
#include "RaidUlduarTriggers.h"
#include "RtiValue.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "Vehicle.h"

const std::string ADD_STRATEGY_CHAR = "+";
const std::string REMOVE_STRATEGY_CHAR = "-";

const std::vector<uint32> availableVehicles = {NPC_VEHICLE_CHOPPER, NPC_SALVAGED_DEMOLISHER,
                                               NPC_SALVAGED_DEMOLISHER_TURRET, NPC_SALVAGED_SIEGE_ENGINE,
                                               NPC_SALVAGED_SIEGE_ENGINE_TURRET};

const std::vector<Position> corners = {
    {183.53f, 66.53f, 409.80f}, {383.03f, 75.10f, 411.71f}, {379.74f, -133.05f, 410.88f}, {158.67f, -137.54f, 409.80f}};

const Position ULDUAR_KOLOGARN_RESTORE_POSITION = Position(1764.3749f, -24.02903f, 448.0f, 0.00087690353f);
const Position ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION = Position(1781.2051f, 9.34402f, 449.0f, 0.00087690353f);
const Position ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION = Position(1763.2561f, -24.44305f, 449.0f, 0.00087690353f);
const Position ULDUAR_THORIM_JUMP_START_POINT = Position(2137.137f, -291.19025f, 438.24753f, 1.7059844f);

bool FlameLeviathanVehicleAction::Execute(Event event)
{
    vehicleBase_ = bot->GetVehicleBase();
    vehicle_ = bot->GetVehicle();
    if (!vehicleBase_ || !vehicle_)
        return false;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    Unit* target = nullptr;
    Unit* flame = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;
        if (unit->GetEntry() == 33139)  // Flame Leviathan Turret
            continue;
        if (unit->GetEntry() == 33142)  // Leviathan Defense Turret
            continue;
        if (unit->GetEntry() == 33113)  // Flame Leviathan
        {
            flame = unit;
            continue;
        }
        if (!target || bot->GetExactDist(target) > bot->GetExactDist(unit))
        {
            target = unit;
        }
    }
    // Flame Leviathan is chasing me
    if (flame && flame->GetVictim() == vehicleBase_)
        if (MoveAvoidChasing(flame))
            return true;

    uint32 entry = vehicleBase_->GetEntry();
    switch (entry)
    {
        case NPC_SALVAGED_DEMOLISHER:
            return DemolisherAction(flame ? flame : target);
        case NPC_SALVAGED_DEMOLISHER_TURRET:
            return DemolisherTurretAction(target ? target: flame);
        case NPC_SALVAGED_SIEGE_ENGINE:
            return SiegeEngineAction(flame ? flame : target);
        case NPC_SALVAGED_SIEGE_ENGINE_TURRET:
            return SiegeEngineTurretAction(target ? target: flame);
        case NPC_VEHICLE_CHOPPER:
            return ChopperAction(target ? target: flame);
        default:
            break;
    }
    return false;
}

bool FlameLeviathanVehicleAction::MoveAvoidChasing(Unit* target)
{
    if (!target)
        return false;
    if (avoidChaseIdx == -1)
    {
        for (int i = 0; i < corners.size(); i++)
        {
            if (bot->GetExactDist(corners[i]) > target->GetExactDist(corners[i]))
                continue;
            if (avoidChaseIdx == -1 || bot->GetExactDist(corners[i]) > bot->GetExactDist(corners[avoidChaseIdx]))
                avoidChaseIdx = i;
        }
        if (avoidChaseIdx == -1)
            avoidChaseIdx = 0;
    }
    else
    {
        if (bot->GetExactDist(corners[avoidChaseIdx]) < 5.0f && target->GetExactDist(bot) < 50.0f)
            avoidChaseIdx = (avoidChaseIdx + 1) % corners.size();
    }
    const Position& to = corners[avoidChaseIdx];
    return MoveTo(bot->GetMap()->GetId(), to.GetPositionX(), to.GetPositionY(), to.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT);
}

bool FlameLeviathanVehicleAction::DemolisherAction(Unit* target)
{
    if (!target)
        return false;
    Aura* bluePyrite = target->GetAura(68605);
    if (!bluePyrite || (vehicleBase_->GetPower(POWER_ENERGY) >= 20) || bluePyrite->GetDuration() <= 5000)
    {
        uint32 spellId = 62490;
        if (botAI->CanCastVehicleSpell(spellId, target))
            if (botAI->CastVehicleSpell(spellId, target))
            {
                vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
                return true;
            }
    }
    uint32 spellId = 62306;
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    return false;
}

bool FlameLeviathanVehicleAction::DemolisherTurretAction(Unit* target)
{
    int32 liquidCount = 0;
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (!unit)
                continue;
            if (unit->GetEntry() != 33189)
                continue;
            if (unit->GetDistance(bot) >= 49.0f)
                continue;
            ++liquidCount;
            if (vehicleBase_->GetPower(POWER_ENERGY) <= 25) // Liquid Pyrite
            {
                uint32 spellId = 62479;
                if (botAI->CanCastVehicleSpell(spellId, unit))
                    if (botAI->CastVehicleSpell(spellId, unit))
                    {
                        vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
                        return true;
                    }
            }
        }
    }
    if (liquidCount <= 10)
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        for (auto i = targets.begin(); i != targets.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (!unit)
                continue;
            if (unit->GetEntry() == 33214) // Mechanolift 304-A
            {
                uint32 spellId = 64979;
                if (botAI->CanCastVehicleSpell(spellId, unit))
                    if (botAI->CastVehicleSpell(spellId, unit))
                    {
                        vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
                        return true;
                    }
            }
        }
    }
    if (!target)
        return false;
    uint32 spellId = 62634;
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    return false;
}

bool FlameLeviathanVehicleAction::SiegeEngineAction(Unit* target)
{
    if (!target)
        return false;
    if (target->GetCurrentSpell(CURRENT_CHANNELED_SPELL) || target->HasAura(62396))
    {
        uint32 spellId = 62522;
        if (botAI->CanCastVehicleSpell(spellId, target))
            if (botAI->CastVehicleSpell(spellId, target))
            {
                vehicleBase_->AddSpellCooldown(spellId, 0, 10000);
                return true;
            }
    }
    uint32 spellId = 62345;
    if (vehicleBase_->GetPower(POWER_ENERGY) >= 80 && botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    return false;
}

bool FlameLeviathanVehicleAction::SiegeEngineTurretAction(Unit* target)
{
    if (!target)
        return false;
    uint32 spellId = 62358;
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    return false;
}

bool FlameLeviathanVehicleAction::ChopperAction(Unit* target)
{
    if (!target)
        return false;
    uint32 spellId = 62286;
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 15000);
            return true;
        }
    spellId = 62974;
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase_->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    return false;
}

bool FlameLeviathanEnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;
    Unit* vehicleToEnter = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles far");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (!ShouldEnter(vehicleBase))
            continue;

        if (!vehicleToEnter || bot->GetExactDist(vehicleToEnter) > bot->GetExactDist(vehicleBase))
            vehicleToEnter = vehicleBase;
    }

    if (!vehicleToEnter)
        return false;

    if (EnterVehicle(vehicleToEnter, true))
        return true;

    return false;
}

bool FlameLeviathanEnterVehicleAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    botAI->RemoveShapeshift();
    // Use HandleSpellClick instead of Unit::EnterVehicle to handle special vehicle script (ulduar)
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool FlameLeviathanEnterVehicleAction::ShouldEnter(Unit* target)
{
    Vehicle* vehicleKit = target->GetVehicleKit();
    if (!vehicleKit)
        return false;

    bool isMelee = botAI->IsMelee(bot);
    bool allMain = AllMainVehiclesOnUse();
    bool inUse = vehicleKit->IsVehicleInUse();
    int32 entry = target->GetEntry();
    if (entry != NPC_SALVAGED_DEMOLISHER && entry != NPC_SALVAGED_SIEGE_ENGINE && entry != NPC_VEHICLE_CHOPPER)
        return false;
    // two phase enter (make all main vehicles in use -> next player enter)
    if (!allMain)
    {
        if (inUse)
            return false;
        if (entry != NPC_SALVAGED_DEMOLISHER && entry != NPC_SALVAGED_SIEGE_ENGINE)
            return false;
        if (entry == NPC_SALVAGED_DEMOLISHER && isMelee)
            return false;
        if (entry == NPC_SALVAGED_SIEGE_ENGINE && !isMelee)
            return false;
        return true;
    }

    if (!vehicleKit->GetAvailableSeatCount())
        return false;

    // do not enter useless seat
    if (entry == NPC_SALVAGED_SIEGE_ENGINE)
    {
        Unit* turret = vehicleKit->GetPassenger(7);
        if (!turret)
            return false;
        Vehicle* turretVehicle = turret->GetVehicleKit();
        if (!turretVehicle)
            return false;
        if (turretVehicle->IsVehicleInUse())
            return false;
        return true;
    }

    if (entry == NPC_SALVAGED_DEMOLISHER)
    {
        if (vehicleKit->GetPassenger(0))
        {
            Unit* target2 = vehicleKit->GetPassenger(1);
            if (!target2)
                return false;
            Vehicle* vehicle2 = target2->GetVehicleKit();
            if (!vehicle2)
                return false;
            if (vehicle2->GetPassenger(0))
                return false;
        }
        return true;
    }

    if (entry == NPC_VEHICLE_CHOPPER && vehicleKit->GetAvailableSeatCount() <= 1)
        return false;

    return true;
}

bool FlameLeviathanEnterVehicleAction::AllMainVehiclesOnUse()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    int demolisher = 0;
    int siege = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        Unit* vehicleBase = player->GetVehicleBase();
        if (!vehicleBase)
            continue;
        if (vehicleBase->GetEntry() == NPC_SALVAGED_DEMOLISHER)
            ++demolisher;
        else if (vehicleBase->GetEntry() == NPC_SALVAGED_SIEGE_ENGINE)
            ++siege;
    }
    Difficulty diff = bot->GetRaidDifficulty();
    int maxC = (diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_10MAN_HEROIC) ? 2 : 5;
    return demolisher >= maxC && siege >= maxC;
}

bool RazorscaleAvoidDevouringFlameAction::Execute(Event event)
{
    RazorscaleBossHelper razorscaleHelper(botAI);

    if (!razorscaleHelper.UpdateBossAI())
    {
        return false;
    }

    bool isMainTank = botAI->IsMainTank(bot);
    const float flameRadius = 3.5f;

    // Main tank moves further so they can hold adds away from flames, but only during the air phases
    const float safeDistanceMultiplier = (isMainTank && !razorscaleHelper.IsGroundPhase()) ? 2.3f : 1.0f;
    const float safeDistance = flameRadius * safeDistanceMultiplier;

    // Get the boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
    {
        return false;
    }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestFlame = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    // Find the closest Devouring Flame
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DEVOURING_FLAME)
        {
            float distance = bot->GetDistance2d(unit);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestFlame = unit;
            }
        }
    }

    // Off tanks are following the main tank during grounded and should prioritise stacking
    if (razorscaleHelper.IsGroundPhase() && (botAI->IsTank(bot) && !botAI->IsMainTank(bot)))
    {
        return false;
    }

    // Handle movement from flames
    if (closestDistance < safeDistance)
    {
        return MoveAway(closestFlame, safeDistance);
    }
    return false;
}

bool RazorscaleAvoidDevouringFlameAction::isUseful()
{
    bool isMainTank = botAI->IsMainTank(bot);

    const float flameRadius = 3.5f;
    const float safeDistanceMultiplier = isMainTank ? 2.3f : 1.0f;
    const float safeDistance = flameRadius * safeDistanceMultiplier;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DEVOURING_FLAME)
        {
            float distance = bot->GetDistance2d(unit);
            if (distance < safeDistance)
            {
                return true;  // Bot is within the danger distance
            }
        }
    }

    return false;  // No nearby flames or bot is at a safe distance
}

bool RazorscaleAvoidSentinelAction::Execute(Event event)
{
    bool isMainTank = botAI->IsMainTank(bot);
    bool isRanged = botAI->IsRanged(bot);
    const float radius = 8.0f;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Unit* lowestHealthSentinel = nullptr;
    uint32 lowestHealth = UINT32_MAX;
    bool movedAway = false;

    // Iterate through all nearby NPCs
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            // Check if this sentinel has the lowest health
            if (unit->GetHealth() < lowestHealth)
            {
                lowestHealth = unit->GetHealth();
                lowestHealthSentinel = unit;
            }

            // Move away if ranged and too close
            if (isRanged && bot->GetDistance2d(unit) < radius)
            {
                movedAway = MoveAway(unit, radius) || movedAway;
            }
        }
    }

    // Check if the main tank is a human player
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group && lowestHealthSentinel)
                {
                    int8 skullIndex = 7;  // Skull
                    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

                    // If there's no skull set yet, or the skull is on a different target, set the sentinel
                    if (!currentSkullTarget || (lowestHealthSentinel->GetGUID() != currentSkullTarget))
                    {
                        group->SetTargetIcon(skullIndex, bot->GetGUID(), lowestHealthSentinel->GetGUID());
                    }
                }
                break;  // Stop after finding the first valid bot tank
            }
        }
    }
    else if (isMainTank && lowestHealthSentinel)  // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            int8 skullIndex = 7;  // Skull
            ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

            // If there's no skull set yet, or the skull is on a different target, set the sentinel
            if (!currentSkullTarget || (lowestHealthSentinel->GetGUID() != currentSkullTarget))
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), lowestHealthSentinel->GetGUID());
            }
        }
    }

    return movedAway;  // Return true if moved
}

bool RazorscaleAvoidSentinelAction::isUseful()
{
    bool isMainTank = botAI->IsMainTank(bot);
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    // If this bot is the main tank, it should always try to mark
    if (isMainTank)
    {
        return true;
    }

    // If the main tank is a human, check if this bot is one of the first three valid bot tanks
    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a human player
    {
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                return true;  // This bot should assist with marking
            }
        }
    }

    bool isRanged = botAI->IsRanged(bot);
    const float radius = 8.0f;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            if (isRanged && bot->GetDistance2d(unit) < radius)
            {
                return true;
            }
        }
    }

    return false;
}

bool RazorscaleAvoidWhirlwindAction::Execute(Event event)
{
    if (botAI->IsTank(bot))
    {
        return false;
    }

    const float radius = 8.0f;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            float currentDistance = bot->GetDistance2d(unit);
            if (currentDistance < radius)
            {
                return MoveAway(unit, radius);
            }
        }
    }
    return false;
}

bool RazorscaleAvoidWhirlwindAction::isUseful()
{
    // Tanks do not avoid Whirlwind
    if (botAI->IsTank(bot))
    {
        return false;
    }

    const float radius = 8.0f;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            if (unit->HasAura(RazorscaleBossHelper::SPELL_SENTINEL_WHIRLWIND) ||
                unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                if (bot->GetDistance2d(unit) < radius)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool RazorscaleIgnoreBossAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
    {
        return false;
    }

    // Check if the boss is flying
    if (boss->GetPositionZ() >= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        // Check if the bot is outside the designated area
        if (bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                               RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) >
            RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + 25.0f)
        {
            return true;  // Movement to the center is the top priority for all bots
        }

        if (!botAI->IsTank(bot))
        {
            return false;
        }

        Group* group = bot->GetGroup();
        if (!group)
        {
            return false;
        }

        // Check if the boss is already set as the moon marker
        int8 moonIndex = 4;  // Moon marker index
        ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);
        if (currentMoonTarget == boss->GetGUID())
        {
            return false;  // Moon marker is already correctly set, no further action needed
        }

        // Proceed to tank-specific logic
        Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
        Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

        // If this bot is the main tank, it needs to set the moon marker
        if (mainTankUnit == bot)
        {
            return true;
        }

        // If the main tank is a human, check if this bot is the lowest-indexed bot tank
        if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a human player
        {
            for (int i = 0; i < 3; ++i)  // Only iterate through the first 3 indexes
            {
                if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Valid bot tank
                {
                    return true;  // This bot should assign the marker
                }
            }
        }
    }

    return false;
}

bool RazorscaleIgnoreBossAction::Execute(Event event)
{
    if (!bot)
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    // Check if the bot is outside the designated area and move inside first
    if (bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                           RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) >
        RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + 25.0f)
    {
        return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                          RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(),
                          RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 10.0f, MovementPriority::MOVEMENT_NORMAL);
    }

    if (!botAI->IsTank(bot))
    {
        return false;
    }

    // Check if the boss is already set as the moon marker
    int8 moonIndex = 4;
    ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);
    if (currentMoonTarget == boss->GetGUID())
    {
        return false;  // Moon marker is already correctly set
    }

    // Get the main tank and determine role
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    // If the main tank is a human, assign the moon marker using the lowest-indexed bot tank
    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        for (int i = 0; i < 3; ++i)  // Only iterate through the first 3 indexes
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                group->SetTargetIcon(moonIndex, bot->GetGUID(), boss->GetGUID());
                SetNextMovementDelay(1000);
                break;  // Assign the moon marker and stop
            }
        }
    }
    else if (mainTankUnit == bot)  // If this bot is the main tank
    {
        group->SetTargetIcon(moonIndex, bot->GetGUID(), boss->GetGUID());
        SetNextMovementDelay(1000);
    }

    // Tanks move inside the arena
    return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                      RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(),
                      RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 10.0f, MovementPriority::MOVEMENT_NORMAL);
}

bool RazorscaleGroundedAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss || !boss->IsAlive() || boss->GetPositionZ() > RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        return false;
    }

    if (botAI->IsMainTank(bot))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // Check if the boss is marked with Moon
        int8 moonIndex = 4;
        ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);

        // Useful only if the boss is currently marked with Moon
        return currentMoonTarget == boss->GetGUID();
    }

    if (botAI->IsTank(bot) && !botAI->IsMainTank(bot))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // Find the main tank
        Player* mainTank = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
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

    if (botAI->IsMelee(bot))
    {
        return false;
    }

    if (botAI->IsRanged(bot))
    {
        constexpr float landingX = 588.0f;
        constexpr float landingY = -166.0f;
        constexpr float landingZ = 391.1f;

        float bossX = boss->GetPositionX();
        float bossY = boss->GetPositionY();
        float bossZ = boss->GetPositionZ();

        bool atInitialLandingPosition =
            (fabs(bossX - landingX) < 2.0f) && (fabs(bossY - landingY) < 2.0f) && (fabs(bossZ - landingZ) < 1.0f);

        constexpr float initialLandingRadius = 14.0f;
        constexpr float normalRadius = 12.0f;

        if (atInitialLandingPosition)
        {
            float adjustedCenterX = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X;
            float adjustedCenterY = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y - 20.0f;

            float distanceToAdjustedCenter = bot->GetDistance2d(adjustedCenterX, adjustedCenterY);
            return distanceToAdjustedCenter > initialLandingRadius;
        }

        float distanceToCenter = bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                                                    RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y);
        return distanceToCenter > normalRadius;
    }

    return false;
}

bool RazorscaleGroundedAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss || !boss->IsAlive() || boss->GetPositionZ() > RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a human player
    {
        // Iterate through the first 3 bot tanks to handle the moon marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                int8 moonIndex = 4;
                ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);

                // If the moon marker is set to the boss, reset it
                if (currentMoonTarget == boss->GetGUID())
                {
                    group->SetTargetIcon(moonIndex, bot->GetGUID(), ObjectGuid::Empty);
                    SetNextMovementDelay(1000);
                    return true;
                }
            }
        }
    }
    else if (botAI->IsMainTank(bot))  // Bot is the main tank
    {
        int8 moonIndex = 4;
        ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);

        // If the moon marker is set to the boss, reset it
        if (currentMoonTarget == boss->GetGUID())
        {
            group->SetTargetIcon(moonIndex, bot->GetGUID(), ObjectGuid::Empty);
            SetNextMovementDelay(1000);
            return true;
        }
    }

    if (mainTank && (botAI->IsTank(bot) && !botAI->IsMainTank(bot)))
    {
        constexpr float followDistance = 2.0f;
        return MoveNear(mainTank, followDistance, MovementPriority::MOVEMENT_COMBAT);
    }

    if (botAI->IsRanged(bot))
    {
        constexpr float landingX = 588.0f;
        constexpr float landingY = -166.0f;
        constexpr float landingZ = 391.1f;

        float bossX = boss->GetPositionX();
        float bossY = boss->GetPositionY();
        float bossZ = boss->GetPositionZ();

        bool atInitialLandingPosition =
            (fabs(bossX - landingX) < 2.0f) && (fabs(bossY - landingY) < 2.0f) && (fabs(bossZ - landingZ) < 1.0f);

        if (atInitialLandingPosition)
        {
            // If at the initial landing position, use 12-yard radius with a
            // 20 yard offset on the Y axis so everyone is behind the boss
            return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                              RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y - 20.0f, bot->GetPositionZ(),
                              RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 12.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        // Otherwise, move inside a 12-yard radius around the arena center
        return MoveInside(ULDUAR_MAP_ID, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                          RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y, bot->GetPositionZ(), 12.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    return false;
}

bool RazorscaleHarpoonAction::Execute(Event event)
{
    if (!bot)
    {
        return false;
    }

    RazorscaleBossHelper razorscaleHelper(botAI);

    // Update the boss AI context
    if (!razorscaleHelper.UpdateBossAI())
        return false;

    Unit* boss = razorscaleHelper.GetBoss();
    if (!boss || !boss->IsAlive())
        return false;

    // Retrieve harpoon data from the helper
    const std::vector<RazorscaleBossHelper::HarpoonData>& harpoonData = razorscaleHelper.GetHarpoonData();

    GameObject* closestHarpoon = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    // Find the nearest harpoon that hasn't been fired and is not on cooldown
    for (const auto& harpoon : harpoonData)
    {
        if (razorscaleHelper.IsHarpoonFired(harpoon.chainSpellId))
            continue;

        if (GameObject* harpoonGO = bot->FindNearestGameObject(harpoon.gameObjectEntry, 200.0f))
        {
            if (RazorscaleBossHelper::IsHarpoonReady(harpoonGO))
            {
                float distance = bot->GetDistance2d(harpoonGO);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestHarpoon = harpoonGO;
                }
            }
        }
    }

    if (!closestHarpoon)
        return false;

    // Find the nearest ranged DPS (not a healer) to the harpoon
    Player* closestRangedDPS = nullptr;
    minDistance = std::numeric_limits<float>::max();
    GuidVector groupBots = AI_VALUE(GuidVector, "group members");

    for (auto& guid : groupBots)
    {
        Player* member = ObjectAccessor::FindPlayer(guid);
        if (member && member->IsAlive() && botAI->IsRanged(member) && botAI->IsDps(member) && !botAI->IsHeal(member))
        {
            float distance = member->GetDistance2d(closestHarpoon);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestRangedDPS = member;
            }
        }
    }

    // Only proceed if this bot is the closest ranged DPS
    if (closestRangedDPS != bot)
        return false;

    float botDist = bot->GetDistance(closestHarpoon);
    if (botDist > INTERACTION_DISTANCE - 1.0f)
    {
        return MoveTo(bot->GetMapId(), closestHarpoon->GetPositionX(), closestHarpoon->GetPositionY(),
                      closestHarpoon->GetPositionZ());
    }

    SetNextMovementDelay(1000);

    // Interact with the harpoon
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

    // Update the boss AI context to ensure we have the latest info
    if (!razorscaleHelper.UpdateBossAI())
        return false;

    Unit* boss = razorscaleHelper.GetBoss();
    if (!boss || !boss->IsAlive())
        return false;

    const std::vector<RazorscaleBossHelper::HarpoonData>& harpoonData = razorscaleHelper.GetHarpoonData();

    for (const auto& harpoon : harpoonData)
    {
        if (razorscaleHelper.IsHarpoonFired(harpoon.chainSpellId))
            continue;

        if (GameObject* harpoonGO = bot->FindNearestGameObject(harpoon.gameObjectEntry, 200.0f))
        {
            if (RazorscaleBossHelper::IsHarpoonReady(harpoonGO))
            {
                // Check if this bot is a ranged DPS (not a healer)
                if (botAI->IsRanged(bot) && botAI->IsDps(bot) && !botAI->IsHeal(bot))
                    return true;
            }
        }
    }

    return false;
}

bool RazorscaleFuseArmorAction::isUseful()
{
    // If this bot cannot tank at all, no need to do anything
    if (!botAI->IsTank(bot))
        return false;

    // If this bot is the main tank AND has Fuse Armor at the threshold, return true immediately
    if (botAI->IsMainTank(bot))
    {
        Aura* fuseArmor = bot->GetAura(RazorscaleBossHelper::SPELL_FUSEARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
            return true;
    }

    // Otherwise, check if there's any other main tank with high Fuse Armor
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (botAI->IsMainTank(member) && member != bot)
        {
            Aura* fuseArmor = member->GetAura(RazorscaleBossHelper::SPELL_FUSEARMOR);
            if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
            {
                // There is another main tank with high Fuse Armor
                return true;
            }
        }
    }

    return false;
}

bool RazorscaleFuseArmorAction::Execute(Event event)
{
    // We already know from isUseful() that:
    //  1) This bot can tank, AND
    //  2) There is at least one main tank (possibly this bot) with Fuse Armor >= threshold.

    RazorscaleBossHelper bossHelper(botAI);

    // Attempt to reassign the roles based on health/Fuse Armor debuff
    bossHelper.AssignRolesBasedOnHealth();
    return true;
}

bool IronAssemblyLightningTendrilsAction::isUseful()
{
    IronAssemblyLightningTendrilsTrigger ironAssemblyLightningTendrilsTrigger(botAI);
    return ironAssemblyLightningTendrilsTrigger.IsActive();
}

bool IronAssemblyLightningTendrilsAction::Execute(Event event)
{
    const float radius = 18.0f + 10.0f;  // 18 yards + 10 yards for safety

    Unit* boss = AI_VALUE2(Unit*, "find target", "stormcaller brundir");
    if (!boss)
        return false;

    float currentDistance = bot->GetDistance2d(boss);

    if (currentDistance < radius)
    {
        return MoveAway(boss, radius - currentDistance);
    }

    return false;
}

bool IronAssemblyOverloadAction::isUseful()
{
    IronAssemblyOverloadTrigger ironAssemblyOverloadTrigger(botAI);
    return ironAssemblyOverloadTrigger.IsActive();
}

bool IronAssemblyOverloadAction::Execute(Event event)
{
    const float radius = 20.0f + 5.0f;  // 20 yards + 5 yards for safety

    Unit* boss = AI_VALUE2(Unit*, "find target", "stormcaller brundir");
    if (!boss)
        return false;

    float currentDistance = bot->GetDistance2d(boss);

    if (currentDistance < radius)
    {
        return MoveAway(boss, radius - currentDistance);
    }

    return false;
}

bool IronAssemblyRuneOfPowerAction::isUseful()
{
    IronAssemblyRuneOfPowerTrigger ironAssemblyRuneOfPowerTrigger(botAI);
    return ironAssemblyRuneOfPowerTrigger.IsActive();
}

bool IronAssemblyRuneOfPowerAction::Execute(Event event)
{
    Unit* target = botAI->GetUnit(bot->GetTarget());
    if (!target || !target->IsAlive())
        return false;

    return MoveAway(target, 10.0f, true);
}

bool KologarnMarkDpsTargetAction::isUseful()
{
    KologarnMarkDpsTargetTrigger kologarnMarkDpsTargetTrigger(botAI);
    return kologarnMarkDpsTargetTrigger.IsActive();
}

bool KologarnMarkDpsTargetAction::Execute(Event event)
{
    Unit* targetToMark = nullptr;
    Unit* additionalTargetToMark = nullptr;
    Unit* targetToCcMark = nullptr;
    int8 skullIndex = 7;  // Skull
    int8 crossIndex = 6;  // Cross
    int8 moonIndex = 4;   // Moon

    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");
    if (!boss || !boss->IsAlive())
        return false;

    // Check that there is rubble to mark
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target)
            continue;

        uint32 creatureId = target->GetEntry();
        if (target->GetEntry() == NPC_RUBBLE && target->IsAlive())
        {
            targetToMark = target;
            additionalTargetToMark = boss;
        }
    }

    if (!targetToMark)
    {
        Unit* rightArm = AI_VALUE2(Unit*, "find target", "right arm");
        if (rightArm && rightArm->IsAlive())
        {
            targetToMark = rightArm;
            additionalTargetToMark = boss;
        }
    }

    if (!targetToMark)
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");
        if (boss && boss->IsAlive())
        {
            targetToMark = boss;
        }
    }

    if (!targetToMark)
    {
        return false;  // No target to mark
    }

    Unit* leftArm = AI_VALUE2(Unit*, "find target", "left arm");
    if (leftArm && leftArm->IsAlive())
    {
        targetToCcMark = leftArm;
    }

    bool isMainTank = botAI->IsMainTank(bot);
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group)
                {
                    group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
                    if (targetToCcMark)
                    {
                        group->SetTargetIcon(moonIndex, bot->GetGUID(), targetToCcMark->GetGUID());
                    }
                    if (additionalTargetToMark)
                    {
                        group->SetTargetIcon(crossIndex, bot->GetGUID(), additionalTargetToMark->GetGUID());
                    }

                    return true;
                }
                break;  // Stop after finding the first valid bot tank
            }
        }
    }
    else if (isMainTank && bot->IsAlive())  // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
            if (targetToCcMark)
            {
                group->SetTargetIcon(moonIndex, bot->GetGUID(), targetToCcMark->GetGUID());
            }
            if (additionalTargetToMark)
            {
                group->SetTargetIcon(crossIndex, bot->GetGUID(), additionalTargetToMark->GetGUID());
            }
            return true;
        }
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot) && bot->IsAlive())  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group)
                {
                    group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
                    if (targetToCcMark)
                    {
                        group->SetTargetIcon(moonIndex, bot->GetGUID(), targetToCcMark->GetGUID());
                    }
                    if (additionalTargetToMark)
                    {
                        group->SetTargetIcon(crossIndex, bot->GetGUID(), additionalTargetToMark->GetGUID());
                    }
                    return true;
                }
                break;  // Stop after finding the first valid bot tank
            }
        }
    }

    return false;
}

bool KologarnFallFromFloorAction::Execute(Event event)
{
    return bot->TeleportTo(bot->GetMapId(), ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionX(),
                           ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionY(),
                           ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionZ(),
                           ULDUAR_KOLOGARN_RESTORE_POSITION.GetOrientation());
}

bool KologarnFallFromFloorAction::isUseful()
{
    KologarnFallFromFloorTrigger kologarnFallFromFloorTrigger(botAI);
    return kologarnFallFromFloorTrigger.IsActive();
}

bool KologarnRubbleSlowdownAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int8 skullIndex = 7;
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);
    if (!currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit->GetEntry() != NPC_RUBBLE)
        return false;

    return botAI->CastSpell("frost trap", currentSkullUnit);
}

bool KologarnEyebeamAction::Execute(Event event)
{
    float distanceToLeftPoint = bot->GetExactDist(ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION);
    float distanceToRightPoint = bot->GetExactDist(ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION);

    bool runToLeftSide;
    if (!distanceToLeftPoint)
    {
        runToLeftSide = true;
    }
    else if (!distanceToRightPoint)
    {
        runToLeftSide = false;
    }
    else
    {
        runToLeftSide = distanceToRightPoint > distanceToLeftPoint;
    }

    bool teleportedToPoint;
    KologarnEyebeamTrigger kologarnEyebeamTrigger(botAI);
    if (runToLeftSide)
    {
        teleportedToPoint = bot->TeleportTo(bot->GetMapId(), ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION.GetPositionX(),
                                            ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION.GetPositionY(),
                                            ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION.GetPositionZ(),
                                            ULDUAR_KOLOGARN_EYEBEAM_LEFT_POSITION.GetOrientation());
    }
    else
    {
        teleportedToPoint = bot->TeleportTo(bot->GetMapId(), ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION.GetPositionX(),
                                            ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION.GetPositionY(),
                                            ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION.GetPositionZ(),
                                            ULDUAR_KOLOGARN_EYEBEAM_RIGHT_POSITION.GetOrientation());
    }

    if (teleportedToPoint)
        SetNextMovementDelay(5000);

    return teleportedToPoint;
}

bool KologarnEyebeamAction::isUseful()
{
    KologarnEyebeamTrigger kologarnEyebeamTrigger(botAI);
    if (!kologarnEyebeamTrigger.IsActive())
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::raid);
}

bool KologarnRtiTargetAction::isUseful()
{
    KologarnRtiTargetTrigger kologarnRtiTargetTrigger(botAI);
    return kologarnRtiTargetTrigger.IsActive();
}

bool KologarnRtiTargetAction::Execute(Event event)
{
    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        context->GetValue<std::string>("rti")->Set("cross");
        return true;
    }

    context->GetValue<std::string>("rti")->Set("skull");
    return true;
}

bool KologarnCrunchArmorAction::isUseful()
{
    KologarnCrunchArmorTrigger kologarnCrunchArmorTrigger(botAI);
    if (!kologarnCrunchArmorTrigger.IsActive())
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::raid);
}

bool KologarnCrunchArmorAction::Execute(Event event)
{
    bot->RemoveAura(SPELL_CRUNCH_ARMOR);
    return true;
}

bool AuriayaFallFromFloorAction::Execute(Event event)
{
    Player* master = botAI->GetMaster();

    if (!master)
        return false;

    return bot->TeleportTo(bot->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                           master->GetOrientation());
}

bool AuriayaFallFromFloorAction::isUseful()
{
    AuriayaFallFromFloorTrigger auriayaFallFromFloorTrigger(botAI);
    return auriayaFallFromFloorTrigger.IsActive();
}

bool HodirMoveSnowpackedIcicleAction::isUseful()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "hodir");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if boss is casting Flash Freeze
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_FLASH_FREEZE))
    {
        return false;
    }

    // Find the nearest Snowpacked Icicle Target
    Creature* target = bot->FindNearestCreature(NPC_SNOWPACKED_ICICLE, 100.0f);
    if (!target)
        return false;

    // Check that bot is stacked on Snowpacked Icicle
    if (bot->GetDistance2d(target->GetPositionX(), target->GetPositionY()) <= 5.0f)
    {
        return false;
    }

    return true;
}

bool HodirMoveSnowpackedIcicleAction::Execute(Event event)
{
    Creature* target = bot->FindNearestCreature(NPC_SNOWPACKED_ICICLE, 100.0f);
    if (!target)
        return false;

    return MoveTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false,
                  false, false, true, MovementPriority::MOVEMENT_NORMAL, true);
}

bool HodirBitingColdJumpAction::Execute(Event event)
{
    bot->RemoveAurasDueToSpell(SPELL_BITING_COLD_PLAYER_AURA);

    return true;

    // Backup when the overall strategy without cheat will be more vialable

    // int mapId = bot->GetMap()->GetId();
    // int x = bot->GetPositionX();
    // int y = bot->GetPositionY();
    // int z = bot->GetPositionZ() + 3.98f;
    // float speed = 7.96f;

    // UpdateMovementState();
    // if (!IsMovingAllowed(mapId, x, y, z))
    //{
    //     return false;
    // }
    // MovementPriority priority;
    // if (IsWaitingForLastMove(priority))
    //{
    //     return false;
    // }

    // MotionMaster& mm = *bot->GetMotionMaster();
    // mm.Clear();
    // mm.MoveJump(x, y, z, speed, speed, 1, AI_VALUE(Unit*, "current target"));
    // mm.MoveFall(0, true);
    // AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), 1000, priority);

    // return true;
}

bool HodirBitingColdJumpAction::isUseful()
{
    return botAI->HasCheat(BotCheatMask::raid);
}

bool FreyaMoveAwayNatureBombAction::isUseful()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Find the nearest Nature Bomb
    GameObject* target = bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, 12.0f);
    if (!target)
        return false;

    return true;
}

bool FreyaMoveAwayNatureBombAction::Execute(Event event)
{
    GameObject* target = bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, 12.0f);
    if (!target)
        return false;

    return FleePosition(target->GetPosition(), 13.0f);
}

bool FreyaMarkDpsTargetAction::isUseful()
{
    FreyaMarkDpsTargetTrigger freyaMarkDpsTargetTrigger(botAI);
    return freyaMarkDpsTargetTrigger.IsActive();
}

bool FreyaMarkDpsTargetAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
        return false;

    Unit* targetToMark = nullptr;

    // Check which adds is up
    Unit* eonarsGift = nullptr;
    Unit* ancientConservator = nullptr;
    Unit* snaplasher = nullptr;
    Unit* ancientWaterSpirit = nullptr;
    Unit* stormLasher = nullptr;
    Unit* firstDetonatingLasher = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_EONARS_GIFT)
        {
            eonarsGift = target;
        }
        else if (target->GetEntry() == NPC_ANCIENT_CONSERVATOR)
        {
            ancientConservator = target;
        }
        else if (target->GetEntry() == NPC_SNAPLASHER)
        {
            snaplasher = target;
        }
        else if (target->GetEntry() == NPC_ANCIENT_WATER_SPIRIT)
        {
            ancientWaterSpirit = target;
        }
        else if (target->GetEntry() == NPC_STORM_LASHER)
        {
            stormLasher = target;
        }
        else if (target->GetEntry() == NPC_DETONATING_LASHER && !firstDetonatingLasher)
        {
            firstDetonatingLasher = target;
        }
    }

    // Check that eonars gift is need to be mark
    if (eonarsGift)
    {
        targetToMark = eonarsGift;
    }

    // Check that ancient conservator is need to be mark
    if (ancientConservator && !targetToMark)
    {
        targetToMark = ancientConservator;
    }

    // Check that trio of adds is need to be mark
    if ((snaplasher || ancientWaterSpirit || stormLasher) && !targetToMark)
    {
        Unit* highestHealthUnit = nullptr;
        uint32 highestHealth = 0;

        if (snaplasher && snaplasher->GetHealth() > highestHealth)
        {
            highestHealth = snaplasher->GetHealth();
            highestHealthUnit = snaplasher;
        }
        if (ancientWaterSpirit && ancientWaterSpirit->GetHealth() > highestHealth)
        {
            highestHealth = ancientWaterSpirit->GetHealth();
            highestHealthUnit = ancientWaterSpirit;
        }
        if (stormLasher && stormLasher->GetHealth() > highestHealth)
        {
            highestHealthUnit = stormLasher;
        }

        // If the highest health unit is not already marked, mark it
        if (highestHealthUnit)
        {
            targetToMark = highestHealthUnit;
        }
    }

    // Check that detonating lasher is need to be mark
    if (firstDetonatingLasher && !targetToMark)
    {
        targetToMark = firstDetonatingLasher;
    }

    if (!targetToMark)
    {
        return false;  // No target to mark
    }

    bool isMainTank = botAI->IsMainTank(bot);
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;
    int8 squareIndex = 5;  // Square
    int8 skullIndex = 7;   // Skull

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group)
                {
                    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

                    if (!currentSkullTarget || (targetToMark->GetGUID() != currentSkullTarget))
                    {
                        group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
                        group->SetTargetIcon(squareIndex, bot->GetGUID(), boss->GetGUID());
                        return true;
                    }
                }
                break;
            }
        }
    }
    else if (isMainTank)  // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

            if (!currentSkullTarget || (targetToMark->GetGUID() != currentSkullTarget))
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
                group->SetTargetIcon(squareIndex, bot->GetGUID(), boss->GetGUID());
                botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
                return true;
            }
        }
    }

    return false;
}

bool FreyaMoveToHealingSporeAction::isUseful()
{
    FreyaMoveToHealingSporeTrigger freyaMoveToHealingSporeTrigger(botAI);
    return freyaMoveToHealingSporeTrigger.IsActive();
}

bool FreyaMoveToHealingSporeAction::Execute(Event event)
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");
    Creature* nearestSpore = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        // Check if this unit is a healthy spore and alive
        if (unit->GetEntry() != NPC_HEALTHY_SPORE || !unit->IsAlive())
            continue;

        float distance = bot->GetDistance2d(unit);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestSpore = static_cast<Creature*>(unit);
        }
    }

    if (!nearestSpore)
        return false;

    return MoveTo(nearestSpore->GetMapId(), nearestSpore->GetPositionX(), nearestSpore->GetPositionY(),
                  nearestSpore->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool ThorimUnbalancingStrikeAction::isUseful()
{
    ThorimUnbalancingStrikeTrigger thorimUnbalancingStrikeTrigger(botAI);
    if (!thorimUnbalancingStrikeTrigger.IsActive())
    {
        return false;
    }

    return botAI->HasCheat(BotCheatMask::raid);
}

bool ThorimUnbalancingStrikeAction::Execute(Event event)
{
    bot->RemoveAura(SPELL_UNBALANCING_STRIKE);
    return true;
}

bool ThorimMarkDpsTargetAction::isUseful()
{
    ThorimMarkDpsTargetTrigger thorimMarkDpsTargetTrigger(botAI);
    return thorimMarkDpsTargetTrigger.IsActive();
}

bool ThorimMarkDpsTargetAction::Execute(Event event)
{
    Unit* targetToMark = nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);
    Unit* currentMoonUnit = botAI->GetUnit(currentMoonTarget);
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");
    if (!currentMoonUnit && boss && boss->IsAlive() && boss->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
    {
        group->SetTargetIcon(moonIndex, bot->GetGUID(), boss->GetGUID());
    }

    if (currentMoonUnit && boss && currentMoonUnit->GetEntry() == boss->GetEntry() &&
        boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
    {
        group->SetTargetIcon(skullIndex, bot->GetGUID(), boss->GetGUID());
        return true;
    }

    if (botAI->IsMainTank(bot))
    {
        ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
        Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);
        if (currentSkullUnit && !currentSkullUnit->IsAlive())
        {
            currentSkullUnit = nullptr;
        }

        Unit* acolyte = AI_VALUE2(Unit*, "find target", "dark rune acolyte");
        Unit* evoker = AI_VALUE2(Unit*, "find target", "dark rune evoker");

        if (acolyte && acolyte->IsAlive() && bot->GetDistance(acolyte) < 50.0f &&
            (!currentSkullUnit || currentSkullUnit->GetEntry() != acolyte->GetEntry()))
            targetToMark = acolyte;
        else if (evoker && evoker->IsAlive() && bot->GetDistance(evoker) < 50.0f &&
                 (!currentSkullUnit || currentSkullUnit->GetEntry() != evoker->GetEntry()))
            targetToMark = evoker;
        else
            return false;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        ObjectGuid currentCrossTarget = group->GetTargetIcon(crossIndex);
        Unit* currentCrossUnit = botAI->GetUnit(currentCrossTarget);
        if (currentCrossUnit && !currentCrossUnit->IsAlive())
        {
            currentCrossUnit = nullptr;
        }

        Unit* acolyte = AI_VALUE2(Unit*, "find target", "dark rune acolyte");
        Unit* runicColossus = AI_VALUE2(Unit*, "find target", "runic colossus");
        Unit* ancientRuneGiant = AI_VALUE2(Unit*, "find target", "ancient rune giant");
        Unit* ironHonorGuard = AI_VALUE2(Unit*, "find target", "iron ring guard");
        Unit* ironRingGuard = AI_VALUE2(Unit*, "find target", "iron honor guard");

        if (acolyte && acolyte->IsAlive() && (!currentCrossUnit || currentCrossUnit->GetEntry() != acolyte->GetEntry()))
            targetToMark = acolyte;
        else if (runicColossus && runicColossus->IsAlive() &&
                 (!currentCrossUnit || currentCrossUnit->GetEntry() != runicColossus->GetEntry()))
            targetToMark = runicColossus;
        else if (ancientRuneGiant && ancientRuneGiant->IsAlive() &&
                 (!currentCrossUnit || currentCrossUnit->GetEntry() != ancientRuneGiant->GetEntry()))
            targetToMark = ancientRuneGiant;
        else if (ironHonorGuard && ironHonorGuard->IsAlive() &&
                 (!currentCrossUnit || currentCrossUnit->GetEntry() != ironHonorGuard->GetEntry()))
            targetToMark = ironHonorGuard;
        else if (ironRingGuard && ironRingGuard->IsAlive() &&
                 (!currentCrossUnit || currentCrossUnit->GetEntry() != ironRingGuard->GetEntry()))
            targetToMark = ironRingGuard;
        else
            return false;
    }

    if (!targetToMark)
        return false;  // No target to mark

    if (botAI->IsMainTank(bot))
    {
        group->SetTargetIcon(skullIndex, bot->GetGUID(), targetToMark->GetGUID());
        return true;
    }

    if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        group->SetTargetIcon(crossIndex, bot->GetGUID(), targetToMark->GetGUID());
        return true;
    }

    return false;
}

bool ThorimArenaPositioningAction::isUseful()
{
    ThorimArenaPositioningTrigger thorimArenaPositioningTrigger(botAI);
    return thorimArenaPositioningTrigger.IsActive();
}

bool ThorimArenaPositioningAction::Execute(Event event)
{
    FollowMasterStrategy followMasterStrategy(botAI);

    MoveTo(bot->GetMapId(), ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionX(),
           ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionY(), ULDUAR_THORIM_NEAR_ARENA_CENTER.GetPositionZ(), false, false,
           false, true, MovementPriority::MOVEMENT_COMBAT, true);

    if (botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
    }

    return true;
}

bool ThorimGauntletPositioningAction::isUseful()
{
    ThorimGauntletPositioningTrigger thorimGauntletPositioningTrigger(botAI);
    return thorimGauntletPositioningTrigger.IsActive();
}

bool ThorimGauntletPositioningAction::Execute(Event event)
{
    FollowMasterStrategy followMasterStrategy(botAI);

    Unit* master = botAI->GetMaster();

    std::string const rti = AI_VALUE(std::string, "rti");
    if (rti != "cross")
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
    }

    if (master->GetDistance(ULDUAR_THORIM_NEAR_ENTRANCE_POSITION) < 10.0f && (bot->GetDistance2d(master) > 5.0f))
    {
        if (MoveTo(bot->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), false,
                   false, false, true, MovementPriority::MOVEMENT_NORMAL, true))
        {
            if (!botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
            {
                botAI->ChangeStrategy(ADD_STRATEGY_CHAR + followMasterStrategy.getName(),
                                      BotState::BOT_STATE_NON_COMBAT);
            }

            return true;
        }
    }

    if (master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1) < 6.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2) < 6.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1) < 5.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1) < 10.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2) < 10.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3) < 10.0f)
    {
        float distance1 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1);
        float distance2 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2);
        float distance3 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1);
        float distance4 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1);
        float distance5 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2);
        float distance6 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3);

        float smallestDistance = std::min({distance1, distance2, distance3, distance4, distance5, distance6});

        Position targetPosition;

        if (smallestDistance == distance1)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance2)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
        }
        else if (smallestDistance == distance3)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance4)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance5)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance6)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else
            return false;
    }

    if (master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1) < 6.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2) < 6.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1) < 5.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1) < 10.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2) < 10.0f ||
        master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3) < 10.0f)
    {
        float distance1 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1);
        float distance2 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2);
        float distance3 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1);
        float distance4 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1);
        float distance5 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2);
        float distance6 = master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3);

        float smallestDistance = std::min({distance1, distance2, distance3, distance4, distance5, distance6});

        Position targetPosition;

        if (smallestDistance == distance1)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance2)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
        }
        else if (smallestDistance == distance3)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance4)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance5)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else if (smallestDistance == distance6)
        {
            return MoveTo(bot->GetMapId(), ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3.GetPositionX(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3.GetPositionY(),
                          ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL, true);
        }
        else
            return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");
    if (boss && boss->IsAlive() && bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD &&
        boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
    {
        MoveTo(bot->GetMapId(), ULDUAR_THORIM_JUMP_START_POINT.GetPositionX(),
               ULDUAR_THORIM_JUMP_START_POINT.GetPositionY(), ULDUAR_THORIM_JUMP_START_POINT.GetPositionZ(), false,
               false, false, true, MovementPriority::MOVEMENT_NORMAL, true);

        if (bot->GetDistance(ULDUAR_THORIM_JUMP_START_POINT) > 0.5f)
            return false;

        JumpTo(bot->GetMapId(), ULDUAR_THORIM_JUMP_END_POINT.GetPositionX(),
               ULDUAR_THORIM_JUMP_END_POINT.GetPositionY(), ULDUAR_THORIM_JUMP_END_POINT.GetPositionZ(),
               MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool ThorimFallFromFloorAction::Execute(Event event)
{
    Player* master = botAI->GetMaster();

    if (!master)
        return false;

    return bot->TeleportTo(bot->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                           master->GetOrientation());
}

bool ThorimFallFromFloorAction::isUseful()
{
    ThorimFallFromFloorTrigger thorimFallFromFloorTrigger(botAI);
    return thorimFallFromFloorTrigger.IsActive();
}

bool ThorimPhase2PositioningAction::Execute(Event event)
{
    Position targetPosition;
    bool backward = false;

    if (botAI->IsMainTank(bot))
    {
        targetPosition = ULDUAR_THORIM_PHASE2_TANK_SPOT;
        backward = true;
    }
    else
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        uint32 memberPositionNumber = 0;
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member)
                continue;

            if (botAI->IsRanged(member) || botAI->IsHeal(member))
            {
                if (bot->GetGUID() == member->GetGUID())
                    break;

                memberPositionNumber++;

                if (memberPositionNumber == 3)
                    memberPositionNumber = 0;
            }
        }

        if (memberPositionNumber == 0)
            targetPosition = ULDUAR_THORIM_PHASE2_RANGE1_SPOT;

        if (memberPositionNumber == 1)
            targetPosition = ULDUAR_THORIM_PHASE2_RANGE2_SPOT;

        if (memberPositionNumber == 2)
            targetPosition = ULDUAR_THORIM_PHASE2_RANGE3_SPOT;
    }

    MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
           false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, backward);

    if (bot->GetDistance(targetPosition) > 1.0f)
        return false;

    return true;
}

bool ThorimPhase2PositioningAction::isUseful()
{
    ThorimPhase2PositioningTrigger thorimPhase2PositioningTrigger(botAI);
    return thorimPhase2PositioningTrigger.IsActive();
}

bool MimironShockBlastAction::Execute(Event event)
{
    Unit* leviathanMkII = nullptr;
    Unit* vx001 = nullptr;
    Unit* aerialCommandUnit = nullptr;

    float radius = 20.0f;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_LEVIATHAN_MKII)
        {
            leviathanMkII = target;
        }
        else if (target->GetEntry() == NPC_VX001)
        {
            vx001 = target;
        }
        else if (target->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
        {
            aerialCommandUnit = target;
        }
    }

    if (!leviathanMkII)
    {
        return false;
    }

    if (!vx001 && !aerialCommandUnit)
    {
        float currentDistance = bot->GetDistance2d(leviathanMkII);

        MoveAway(leviathanMkII, radius - currentDistance);

        if (botAI->IsMelee(bot))
        {
            botAI->SetNextCheckDelay(100);
        }
        return true;
    }
    else
    {
        float init_angle = leviathanMkII->GetAngle(bot);
        float distance = radius - bot->GetDistance2d(leviathanMkII);
        for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8)
        {
            float angle = init_angle + delta;
            float dx = bot->GetPositionX() + cos(angle) * distance;
            float dy = bot->GetPositionY() + sin(angle) * distance;
            float dz = bot->GetPositionZ();
            if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                               bot->GetPositionZ(), dx, dy, dz))
            {
                bot->TeleportTo(target->GetMapId(), dx, dy, dz, target->GetOrientation());
                return true;
            }
        }
        return false;
    }
}

bool MimironShockBlastAction::isUseful()
{
    MimironShockBlastTrigger mimironShockBlastTrigger(botAI);
    return mimironShockBlastTrigger.IsActive();
}

bool MimironPhase1PositioningAction::Execute(Event event)
{
    SET_AI_VALUE(float, "disperse distance", 6.0f);
    return true;
}

bool MimironPhase1PositioningAction::isUseful()
{
    MimironPhase1PositioningTrigger mimironPhase1PositioningTrigger(botAI);
    return mimironPhase1PositioningTrigger.IsActive();
}

bool MimironP3Wx2LaserBarrageAction::Execute(Event event)
{
    auto master = botAI->GetMaster();
    if (!master || !master->IsAlive())
    {
        return false;
    }

    if (bot->GetDistance2d(master) > 15.0f)
    {
        return bot->TeleportTo(master->GetMapId(), master->GetPositionX(), master->GetPositionY(),
                               master->GetPositionZ(), master->GetOrientation());
    }

    return MoveTo(master->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), false,
                  false, false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool MimironRapidBurstAction::isUseful()
{
    MimironRapidBurstTrigger mimironRapidBurstTrigger(botAI);
    return mimironRapidBurstTrigger.IsActive();
}

bool MimironRapidBurstAction::Execute(Event event)
{
    Unit* leviathanMkII = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_LEVIATHAN_MKII)
        {
            leviathanMkII = target;
        }
    }

    Position targetPosition;

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    uint32 memberSpotNumber = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (bot->GetGUID() == member->GetGUID())
        {
            if (botAI->IsRanged(bot))
            {
                switch (memberSpotNumber)
                {
                    case 0:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE1RANGE_SPOT;
                        break;
                    case 1:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE2RANGE_SPOT;
                        break;
                    case 2:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE3RANGE_SPOT;
                        break;
                    default:
                        break;
                }
            }
            else if (botAI->IsMainTank(bot) && leviathanMkII)
            {
                targetPosition = ULDUAR_MIMIRON_PHASE4_TANK_SPOT;
            }
            else
            {
                switch (memberSpotNumber)
                {
                    case 0:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE1MELEE_SPOT;
                        break;
                    case 1:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE2MELEE_SPOT;
                        break;
                    case 2:
                        targetPosition = ULDUAR_MIMIRON_PHASE2_SIDE3MELEE_SPOT;
                        break;
                    default:
                        break;
                }
            }
        }

        memberSpotNumber++;

        if (memberSpotNumber == 3)
        {
            memberSpotNumber = 0;
        }
    }

    MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
           false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);

    if (AI_VALUE(float, "disperse distance") != 0.0f)
    {
        SET_AI_VALUE(float, "disperse distance", 0.0f);
    }

    TankFaceStrategy tankFaceStrategy(botAI);
    if (botAI->HasStrategy(tankFaceStrategy.getName(), BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + tankFaceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    }
    if (botAI->HasStrategy(tankFaceStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + tankFaceStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
    }

    if (bot->GetDistance(targetPosition) > 1.0f)
        return false;

    return true;
}

bool MimironAerialCommandUnitAction::Execute(Event event)
{
    Unit* boss = nullptr;
    Unit* bombBot = nullptr;
    Unit* assaultBot = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
        {
            boss = target;
        }
        else if (target->GetEntry() == NPC_BOMB_BOT)
        {
            bombBot = target;
        }
        else if (target->GetEntry() == NPC_ASSAULT_BOT)
        {
            assaultBot = target;
        }
    }

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        Group* group = bot->GetGroup();
        if (!group)
        {
            return false;
        }

        if (bombBot)
        {
            group->SetTargetIcon(crossIndex, bot->GetGUID(), bombBot->GetGUID());
        }
        else if (boss)
        {
            group->SetTargetIcon(crossIndex, bot->GetGUID(), boss->GetGUID());
        }

        if (assaultBot)
        {
            ObjectGuid skullTarget = group->GetTargetIcon(skullIndex);
            Unit* skullUnit = botAI->GetUnit(skullTarget);
            if (!skullTarget || !skullUnit || !skullUnit->IsAlive())
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), assaultBot->GetGUID());
            }
        }

        return true;
    }

    if (AI_VALUE(float, "disperse distance") != 5.0f)
    {
        SET_AI_VALUE(float, "disperse distance", 5.0f);
    }
    botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
    return true;
}

bool MimironRocketStrikeAction::isUseful()
{
    MimironRocketStrikeTrigger mimironRocketStrikeTrigger(botAI);
    return mimironRocketStrikeTrigger.IsActive();
}

bool MimironRocketStrikeAction::Execute(Event event)
{
    Unit* leviathanMkII = nullptr;
    Unit* vx001 = nullptr;
    Unit* aerialCommandUnit = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_LEVIATHAN_MKII)
        {
            leviathanMkII = target;
        }
        else if (target->GetEntry() == NPC_VX001)
        {
            vx001 = target;
        }
        else if (target->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
        {
            aerialCommandUnit = target;
        }
    }

    Creature* rocketStrikeN = bot->FindNearestCreature(NPC_ROCKET_STRIKE_N, 100.0f);

    if (!rocketStrikeN)
    {
        return false;
    }

    if (!vx001 && !aerialCommandUnit)
    {
        MoveAway(rocketStrikeN, 10.0f);
        return true;
    }
    else
    {
        float init_angle = rocketStrikeN->GetAngle(bot);
        float distance = 10.0f - bot->GetDistance2d(rocketStrikeN);
        for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8)
        {
            float angle = init_angle + delta;
            float dx = bot->GetPositionX() + cos(angle) * distance;
            float dy = bot->GetPositionY() + sin(angle) * distance;
            float dz = bot->GetPositionZ();
            if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                               bot->GetPositionZ(), dx, dy, dz))
            {
                bot->TeleportTo(target->GetMapId(), dx, dy, dz, target->GetOrientation());
                return true;
            }
        }
        return false;
    }
}

bool MimironPhase4MarkDpsAction::Execute(Event event)
{
    Unit* leviathanMkII = nullptr;
    Unit* vx001 = nullptr;
    Unit* aerialCommandUnit = nullptr;

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_LEVIATHAN_MKII)
        {
            leviathanMkII = target;
        }
        else if (target->GetEntry() == NPC_VX001)
        {
            vx001 = target;
        }
        else if (target->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
        {
            aerialCommandUnit = target;
        }
    }

    if (!leviathanMkII || !vx001 || !aerialCommandUnit)
    {
        return false;
    }

    if (botAI->IsMainTank(bot))
    {
        Unit* highestHealthUnit = nullptr;
        uint32 highestHealth = 0;
        if (leviathanMkII && leviathanMkII->GetHealth() > highestHealth)
        {
            highestHealth = leviathanMkII->GetHealth();
            highestHealthUnit = leviathanMkII;
        }
        if (vx001 && vx001->GetHealth() > highestHealth)
        {
            highestHealth = vx001->GetHealth();
            highestHealthUnit = vx001;
        }
        if (aerialCommandUnit && aerialCommandUnit->GetHealth() > highestHealth)
        {
            highestHealthUnit = aerialCommandUnit;
        }

        group->SetTargetIcon(skullIndex, bot->GetGUID(), highestHealthUnit->GetGUID());
        if (highestHealthUnit == leviathanMkII)
        {
            if (AI_VALUE(std::string, "rti") == "skull")
            {
                botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("skull");
            }
        }
        else
        {
            group->SetTargetIcon(crossIndex, bot->GetGUID(), leviathanMkII->GetGUID());
            if (AI_VALUE(std::string, "rti") != "cross")
            {
                botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
            }
        }

        botAI->DoSpecificAction("attack rti target");
        return true;
    }
    else
    {
        /*if (AI_VALUE(float, "disperse distance") != 0.0f)
        {
            SET_AI_VALUE(float, "disperse distance", 0.0f);
        }*/
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("skull");
        return true;
    }
}

bool MimironCheatAction::Execute(Event event)
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");
    for (const ObjectGuid& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PROXIMITY_MINE)
        {
            unit->Kill(bot, unit);
        }
        else if (unit->GetEntry() == NPC_BOMB_BOT)
        {
            unit->Kill(bot, unit);
        }
    }

    return true;
}

bool VezaxCheatAction::Execute(Event event)
{
    // Restore bot's mana to full
    uint32 maxMana = bot->GetMaxPower(POWER_MANA);
    if (maxMana > 0)
    {
        bot->SetPower(POWER_MANA, maxMana);
    }

    return true;
}

bool VezaxShadowCrashAction::Execute(Event event)
{
    // Find General Vezax boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "general vezax");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Get bot's current position relative to boss
    float bossX = boss->GetPositionX();
    float bossY = boss->GetPositionY();
    float bossZ = boss->GetPositionZ();

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();

    // Calculate current angle and distance from boss
    float currentAngle = atan2(botY - bossY, botX - bossX);
    float currentDistance = bot->GetDistance2d(boss);

    // Set desired distance from boss (stay close enough for melee, far enough for ranged)
    float desiredDistance = 15.0f;

    // If too close or too far, adjust distance first
    if (currentDistance < desiredDistance - 2.0f || currentDistance > desiredDistance + 2.0f)
    {
        currentDistance = desiredDistance;
    }

    // Calculate movement increment - move in increments around the boss
    float angleIncrement = M_PI / 10;
    float newAngle = currentAngle + angleIncrement;

    // Calculate new position
    float newX = bossX + currentDistance * cos(newAngle);
    float newY = bossY + currentDistance * sin(newAngle);
    float newZ = bossZ;  // Keep same Z level as boss

    // Move to the new position
    return MoveTo(boss->GetMapId(), newX, newY, newZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT,
                  true);
}

bool VezaxMarkOfTheFacelessAction::Execute(Event event)
{
    return MoveTo(bot->GetMapId(), ULDUAR_VEZAX_MARK_OF_THE_FACELESS_SPOT.GetPositionX(),
                  ULDUAR_VEZAX_MARK_OF_THE_FACELESS_SPOT.GetPositionY(),
                  ULDUAR_VEZAX_MARK_OF_THE_FACELESS_SPOT.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}
