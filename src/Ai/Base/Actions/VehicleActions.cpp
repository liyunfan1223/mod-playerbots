/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VehicleActions.h"
#include "BattlegroundIC.h"
#include "BattlegroundSA.h"
#include "BattleGroundTactics.h"
#include "ItemVisitors.h"
#include "ObjectDefines.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "QuestValues.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"

// SotA: the team that is not the attacker defends. BattlegroundSA keeps Attackers private, but ResetObjs spawns the
// defender portals with the defender faction (same approach as SaGetAttackerTeam in BattleGroundTactics.cpp)
static bool SaIsDefender(Player* bot)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID() != BATTLEGROUND_SA)
        return false;

    GameObject* portal = bg->GetBGObject(BG_SA_PORTAL_DEFFENDER_BLUE);
    if (!portal)
        return false;  // round not set up yet, do not guess

    uint32 const defenderFaction = portal->GetFaction();
    if (defenderFaction == BG_SA_Factions[TEAM_ALLIANCE])
        return bot->GetTeamId() == TEAM_ALLIANCE;
    if (defenderFaction == BG_SA_Factions[TEAM_HORDE])
        return bot->GetTeamId() == TEAM_HORDE;

    return false;
}

// TODO methods to enter/exit vehicle should be added to BGTactics or MovementAction (so that we can better control
// whether bot is in vehicle, eg: get out of vehicle to cap flag, if we're down to final boss, etc),
// right now they will enter vehicle based only what's available here, then they're stuck in vehicle until they die
// (LeaveVehicleAction doesnt do much seeing as they, or another bot, will get in immediately after exit)
bool EnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Player* master = botAI->GetMaster();
    // Triggered by a chat command
    if (event.getOwner() && master && master->GetTarget())
    {
        Unit* vehicleBase = botAI->GetUnit(master->GetTarget());
        if (!vehicleBase)
            return false;
        Vehicle* veh = vehicleBase->GetVehicleKit();
        if (vehicleBase->IsVehicle() && veh && veh->GetAvailableSeatCount())
        {
            return EnterVehicle(vehicleBase, false);
        }
        return false;
    }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        // dont let them get in the cannons as they'll stay forever and do nothing useful
        // dont let them in catapult they cant use them at all
        if (NPC_KEEP_CANNON == vehicleBase->GetEntry() || NPC_CATAPULT == vehicleBase->GetEntry())
            continue;

        // healers stay out of the SotA demolishers and towers: inside them they cannot heal
        if ((NPC_DEMOLISHER_SA == vehicleBase->GetEntry() || NPC_ANTI_PERSONNAL_CANNON == vehicleBase->GetEntry()) &&
            PlayerbotAI::IsHeal(bot, true))
            continue;

        // SotA anti-personnel towers: only the defenders use them (to shoot the enemy demolishers), and only when
        // they are close; attackers never get in
        // TODO: not tested in-game whether bots in the tower really fire Rocket Blast (49872)
        if (NPC_ANTI_PERSONNAL_CANNON == vehicleBase->GetEntry())
        {
            if (!SaIsDefender(bot) || ServerFacade::instance().GetDistance2d(bot, vehicleBase) > 25.0f)
                continue;

            // the gate it covers is down: nothing to shoot, do not climb back in
            if (SaTowerGateDestroyed(bot->GetBattleground(), vehicleBase))
                continue;

            // only the towers of the gate stage being held (beach gates first); the others stay empty
            if (!SaTowerPhaseActive(bot->GetBattleground(), vehicleBase))
                continue;
        }

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        // this will avoid adding passengers (which dont really do much for the IOC vehicles which is the only place
        // this code is used)
        if (vehicleBase->GetVehicleKit()->IsVehicleInUse())
            continue;

        if (EnterVehicle(vehicleBase, true))
            return true;
    }

    return false;
}

bool EnterVehicleAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = ServerFacade::instance().GetDistance2d(bot, vehicleBase);
    if (dist > 40.0f)
        return false;

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);
    // Use HandleSpellClick instead of Unit::EnterVehicle to handle special vehicle script (ulduar)
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // a siege position is only for the vehicle it was set for: the tactics set one for this vehicle
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo siegePos = posMap["bg siege"];
    siegePos.Reset();
    posMap["bg siege"] = siegePos;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool LeaveVehicleAction::Execute(Event /*event*/)
{
    Vehicle* myVehicle = bot->GetVehicle();
    if (!myVehicle)
        return false;

    VehicleSeatEntry const* seat = myVehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket p;
    bot->GetSession()->HandleRequestVehicleExit(p);

    return true;
}
