
#include "RaidUlduarActions.h"

#include <cmath>

#include "DBCEnums.h"
#include "GameObject.h"
#include "LastMovementValue.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaidUlduarBossHelper.h"
#include "RaidUlduarScripts.h"
#include "RaidUlduarStrategy.h"
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