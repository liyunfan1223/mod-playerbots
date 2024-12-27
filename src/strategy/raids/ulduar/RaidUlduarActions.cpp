
#include "RaidUlduarActions.h"

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
#include "Player.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaidUlduarBossHelper.h"
#include "RaidUlduarScripts.h"
#include "RaidUlduarStrategy.h"
#include "RtiValue.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "Vehicle.h"

const std::vector<uint32> availableVehicles = {NPC_VEHICLE_CHOPPER, NPC_SALVAGED_DEMOLISHER,
                                               NPC_SALVAGED_DEMOLISHER_TURRET, NPC_SALVAGED_SIEGE_ENGINE,
                                               NPC_SALVAGED_SIEGE_ENGINE_TURRET};

const std::vector<Position> corners = {
    {183.53f, 66.53f, 409.80f}, {383.03f, 75.10f, 411.71f}, {379.74f, -133.05f, 410.88f}, {158.67f, -137.54f, 409.80f}};

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
        if (unit->GetEntry() == 33113) // Flame Leviathan
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
                return true; // Bot is within the danger distance
            }
        }
    }

    return false; // No nearby flames or bot is at a safe distance
}

bool RazorscaleAvoidSentinelAction::Execute(Event event)
{
    bool isTank = botAI->IsTank(bot);
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
    
    if (mainTank && !GET_PLAYERBOT_AI(mainTank)) // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot)) // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group && lowestHealthSentinel)
                {
                    int8 skullIndex = 7; // Skull
                    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    
                    // If there's no skull set yet, or the skull is on a different target, set the sentinel
                    if (!currentSkullTarget || (lowestHealthSentinel->GetGUID() != currentSkullTarget))
                    {
                        group->SetTargetIcon(skullIndex, bot->GetGUID(), lowestHealthSentinel->GetGUID());
                    }
                }
                break; // Stop after finding the first valid bot tank
            }
        }
    }
    else if (isMainTank && lowestHealthSentinel) // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            int8 skullIndex = 7; // Skull
            ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    
            // If there's no skull set yet, or the skull is on a different target, set the sentinel
            if (!currentSkullTarget || (lowestHealthSentinel->GetGUID() != currentSkullTarget))
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), lowestHealthSentinel->GetGUID());
            }
        }
    }


    return movedAway; // Return true if moved
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
    if (mainTank && !GET_PLAYERBOT_AI(mainTank)) // Main tank is a human player
    {
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot)) // Bot is a valid tank
            {
                return true; // This bot should assist with marking
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
            if (unit->HasAura(RazorscaleBossHelper::SPELL_SENTINEL_WHIRLWIND) || unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
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
        if (bot->GetDistance2d(
                RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) > RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + 25.0f)
        {
            return true; // Movement to the center is the top priority for all bots
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
        int8 moonIndex = 4; // Moon marker index
        ObjectGuid currentMoonTarget = group->GetTargetIcon(moonIndex);
        if (currentMoonTarget == boss->GetGUID())
        {
            return false; // Moon marker is already correctly set, no further action needed
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
        if (mainTank && !GET_PLAYERBOT_AI(mainTank)) // Main tank is a human player
        {
        for (int i = 0; i < 3; ++i) // Only iterate through the first 3 indexes
            {
                if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot)) // Valid bot tank
                {
                    return true; // This bot should assign the marker
                }
            }
        }
    }

    return false;
}

bool RazorscaleIgnoreBossAction::Execute(Event event)
{
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
    if (bot->GetDistance2d(
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y) > RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS + 25.0f)
    {
        return MoveInside(
            ULDUAR_MAP_ID,
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y,
            bot->GetPositionZ(),
            RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 10.0f,
            MovementPriority::MOVEMENT_NORMAL
        );
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
        return false; // Moon marker is already correctly set
    }

    // Get the main tank and determine role
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    // If the main tank is a human, assign the moon marker using the lowest-indexed bot tank
    if (mainTank && !GET_PLAYERBOT_AI(mainTank)) // Main tank is a real player
    {
        for (int i = 0; i < 3; ++i) // Only iterate through the first 3 indexes
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot)) // Bot is a valid tank
            {
                group->SetTargetIcon(moonIndex, bot->GetGUID(), boss->GetGUID());
                SetNextMovementDelay(1000);
                break; // Assign the moon marker and stop
            }
        }
    }
    else if (mainTankUnit == bot) // If this bot is the main tank
    {
        group->SetTargetIcon(moonIndex, bot->GetGUID(), boss->GetGUID());
        SetNextMovementDelay(1000);
    }

    // Tanks move inside the arena
    return MoveInside(
        ULDUAR_MAP_ID,
        RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
        RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y,
        bot->GetPositionZ(),
        RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 10.0f,
        MovementPriority::MOVEMENT_NORMAL
    );
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

        bool atInitialLandingPosition = (fabs(bossX - landingX) < 2.0f) &&
                                        (fabs(bossY - landingY) < 2.0f) &&
                                        (fabs(bossZ - landingZ) < 1.0f);

        constexpr float initialLandingRadius = 14.0f;
        constexpr float normalRadius = 12.0f;

        if (atInitialLandingPosition)
        {
            float adjustedCenterX = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X;
            float adjustedCenterY = RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y - 20.0f;

            float distanceToAdjustedCenter = bot->GetDistance2d(adjustedCenterX, adjustedCenterY);
            return distanceToAdjustedCenter > initialLandingRadius;
        }

        float distanceToCenter = bot->GetDistance2d(RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X, RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y);
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
    
    if (mainTank && !GET_PLAYERBOT_AI(mainTank)) // Main tank is a human player
    {
        // Iterate through the first 3 bot tanks to handle the moon marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot)) // Bot is a valid tank
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
    else if (botAI->IsMainTank(bot)) // Bot is the main tank
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

        bool atInitialLandingPosition = (fabs(bossX - landingX) < 2.0f) &&
                                        (fabs(bossY - landingY) < 2.0f) &&
                                        (fabs(bossZ - landingZ) < 1.0f);

        if (atInitialLandingPosition)
        {
            // If at the initial landing position, use 12-yard radius with a
            // 20 yard offset on the Y axis so everyone is behind the boss
            return MoveInside(
                ULDUAR_MAP_ID,
                RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
                RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y - 20.0f,
                bot->GetPositionZ(),
                RazorscaleBossHelper::RAZORSCALE_ARENA_RADIUS - 12.0f,
                MovementPriority::MOVEMENT_COMBAT
            );
        }

        // Otherwise, move inside a 12-yard radius around the arena center
        return MoveInside(
            ULDUAR_MAP_ID,
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_X,
            RazorscaleBossHelper::RAZORSCALE_ARENA_CENTER_Y,
            bot->GetPositionZ(),
            12.0f,
            MovementPriority::MOVEMENT_COMBAT
        );
    }
    return false;
}

bool RazorscaleHarpoonAction::Execute(Event event)
{
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
        return MoveTo(bot->GetMapId(),
                      closestHarpoon->GetPositionX(),
                      closestHarpoon->GetPositionY(),
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
