/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Thorim.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Thorim.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "RangeTriggers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>

using namespace EncounterHelpers;

bool ThorimDpsPriorityTrigger::IsActive()
{
    // Cheap gate first: this runs for every bot every tick, and everything below walks the nearby-unit list. The
    // wing, not a flat 110 yd: the hallway runs past that, and only this node pulls a forbidden target off a bot.
    if (!NearThorimEncounter(bot))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // A healer's target drives its wand and its offensive dispels rather than a rotation, so steering
    // it every tick would fight the healing. Pull it off something it must not be holding, no more.
    if (PlayerbotAI::IsHeal(bot))
        return currentTarget && !ThorimDpsTargetAllowed(botAI, currentTarget);

    // Same for a tank in phase 1, where "tank assist" steers it and two live pickers on one bot is the oscillation.
    // Phase 2 mutes tank assist, so this is the only picker an off-tank waiting to swap has.
    if (PlayerbotAI::IsTank(bot) && !ThorimPhase2Active(botAI))
        return currentTarget && !ThorimDpsTargetAllowed(botAI, currentTarget);

    if (currentTarget && !ThorimDpsTargetAllowed(botAI, currentTarget))
        return true;

    Unit* target = GetThorimDpsTarget(botAI, bot, currentTarget);
    return target && target != currentTarget;
}

bool ThorimGauntletPositioningTrigger::IsActive()
{
    if (bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > ULDUAR_THORIM_FLOOR_RANGE)
        return false;

    if (GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return false;

    Unit* master = botAI->GetMaster();
    if (!master)
        return false;

    if (master->GetDistance(ULDUAR_THORIM_NEAR_ENTRANCE_POSITION) < ULDUAR_THORIM_GATE_RADIUS &&
        bot->GetDistance2d(master) > ULDUAR_THORIM_GATE_FOLLOW_DISTANCE)
        return true;

    uint8 masterIndex = 0;
    bool leftLane = false;
    if (ThorimGauntletLaneIndex(master, masterIndex, leftLane))
    {
        bool preferredLane = false;
        if (ThorimPreferredGauntletLane(botAI, preferredLane))
            leftLane = preferredLane;

        uint8 botIndex = 0;
        if (!ThorimGauntletLaneIndexInLane(bot, leftLane, botIndex))
            return bot->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;
    }

    return false;
}

bool ThorimBalconyAdvanceTrigger::IsActive()
{
    if (GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return false;

    // No master check on purpose: the hallway is the last thing before phase 2 and gets walked with or without a
    // human up here to follow.
    return ThorimBalconyOpen(botAI, bot);
}

bool ThorimArenaPositioningTrigger::IsActive()
{
    Position anchor;
    if (!GetThorimArenaAnchor(botAI, bot, anchor))
        return false;

    // Surviving beats standing on a spot, and letting the anchor fight a dodge is what has a bot step
    // out of a hazard and get walked straight back into it.
    ThorimChargedOrbTrigger chargedOrb(botAI);
    if (chargedOrb.IsActive())
        return false;

    ThorimSifBlizzardTrigger blizzard(botAI);
    if (blizzard.IsActive())
        return false;

    return ThorimArenaAnchorNeedsMove(botAI, bot, anchor);
}

bool ThorimFallFromFloorTrigger::IsActive()
{
    if (bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > ULDUAR_THORIM_FLOOR_RANGE)
        return false;

    return bot->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_PATHING_ISSUE_DETECT;
}

bool ThorimPhase2PositioningTrigger::IsActive()
{
    if (!ThorimPhase2Active(botAI))
        return false;

    // Same stand-down the arena node carries, or the anchor walks a bot straight back into the zone it just left.
    ThorimSifBlizzardTrigger blizzard(botAI);
    if (blizzard.IsActive())
        return false;

    ThorimPhase2Role const role = GetThorimPhase2Role(botAI, bot);
    if (role == ThorimPhase2Role::None)
        return false;

    Position spot;
    if (!TryGetThorimPhase2Spot(botAI, bot, role, spot))
        return false;

    Unit* boss = GetThorim(botAI);
    bool const holdsBoss = boss && boss->GetVictim() == bot;

    // Only whoever has him walks him to the anchor, a second tank would drag him back. The off-tank too: his ring
    // point is measured off the boss, so towing from it drifts the pair across the room.
    if (role == ThorimPhase2Role::MainTank || (role == ThorimPhase2Role::OffTank && holdsBoss))
        return holdsBoss && bot->GetDistance(spot) > ULDUAR_THORIM_PHASE2_SPOT_TOLERANCE;

    if (role == ThorimPhase2Role::Ranged)
    {
        if (bot->GetDistance(spot) <= ULDUAR_THORIM_PHASE2_SPOT_TOLERANCE)
            return false;

        // Wait out a zone on the spot or on the straight walk there, rather than walk in and get flung off again.
        // Not while under the cone though, that hits far harder than a tick.
        return !ThorimWalkUnderBlizzard(bot, *bot, spot) || ThorimCampSpotInLitCone(botAI, *bot);
    }

    // Arrival is set here, since the action only runs when there's a move and never sees "stay". Safe in a
    // trigger: it only widens the tolerance, so a second ask this tick still says stay.
    if (!ThorimRingWantsMove(botAI, bot, spot))
    {
        ThorimRingMarkArrived(bot);
        return false;
    }

    return true;
}

bool ThorimRunicSmashTrigger::IsActive()
{
    bool safeLane = false;
    if (!ThorimRunicSmashImminent(botAI) || !ThorimPreferredGauntletLane(botAI, safeLane))
        return false;

    // A bot still up on the balcony has its own node to get down and nothing to dodge up there.
    if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    // Corridor squad only: ThorimResolveGauntletIndex falls back to the master's waypoint, so an arena bot would
    // answer the telegraph by walking 110 yd down the corridor and never come back.
    if (GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return false;

    // The centre line is inside the blast too, so this asks for the safe lane rather than "not the
    // hot one".
    uint8 index = 0;
    if (ThorimGauntletLaneIndexInLane(bot, safeLane, index))
        return false;

    return ThorimResolveGauntletIndex(botAI, bot, index);
}

bool ThorimRunicBarrierBailTrigger::IsActive()
{
    if (!ThorimBarrierBailLatched(botAI, bot))
        return false;

    Unit* colossus = GetThorimRunicColossus(botAI);
    return colossus && bot->GetDistance(colossus) < ULDUAR_THORIM_BARRIER_BAIL_DISTANCE;
}

bool ThorimFrostAuraTrigger::IsActive()
{
    return IsThorimFrostAuraPaladin(bot) && !botAI->HasAura("frost resistance aura", bot);
}

bool ThorimPetLeashTrigger::IsActive()
{
    std::vector<Unit*> pets;
    return ThorimStrayPets(botAI, bot, pets) || ThorimPetsOffRingSlot(botAI, bot, pets);
}

bool ThorimChargedOrbTrigger::IsActive()
{
    Position spot;
    return ThorimChargedOrbEscape(botAI, bot, spot);
}

bool ThorimArenaLeashTrigger::IsActive() { return ThorimArenaLeashBreached(botAI, bot); }

bool ThorimResetEncounterStateTrigger::IsActive()
{
    return ThorimBotHasEncounterState(bot) && ThorimEncounterStateIsStale(botAI);
}

bool ThorimUnbalancingStrikeSwapTrigger::IsActive()
{
    // The only tank node that reads the boss with no aura on the bot to prove it's near, and the boss handle
    // answers from anywhere on the map.
    if (!NearThorimEncounter(bot))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsInWorld() || boss->IsDuringRemoveFromWorld())
        return false;

    if (!boss->IsAlive() || !boss->IsHostileTo(bot))
        return false;

    bool const isMainTank = PlayerbotAI::IsMainTank(bot);
    bool const isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0);
    if (!isMainTank && !isFirstAssistTank)
        return false;

    Unit* activeTank = boss->GetVictim();
    if (!activeTank || activeTank == bot)
        return false;

    Player* activeTankPlayer = activeTank->ToPlayer();
    if (!activeTankPlayer)
        return false;

    // The active tank must be this bot's swap partner so the taunt is symmetric both ways
    bool const partnerIsSwapTank = isMainTank ? PlayerbotAI::IsAssistTankOfIndex(activeTankPlayer, 0)
                                              : PlayerbotAI::IsMainTank(activeTankPlayer);
    if (!partnerIsSwapTank)
        return false;

    if (bot->HasAura(SPELL_UNBALANCING_STRIKE))
        return false;

    return activeTank->HasAura(SPELL_UNBALANCING_STRIKE);
}

bool ThorimTankPickupTrigger::IsActive()
{
    if (!ThorimPhase2Active(botAI))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsInWorld() || boss->IsDuringRemoveFromWorld())
        return false;

    if (!boss->IsAlive() || !boss->IsHostileTo(bot))
        return false;

    // Nothing to reach from up there, and the walk back is the whole corridor. The balcony node
    // drops the bot first, same as the phase 2 spot does.
    if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    Unit* victim = boss->GetVictim();
    if (victim == bot)
        return false;

    // A tank already has him, so this is the swap node's call from here. Answering anyway is the
    // second taunter that drags him back and forth.
    if (Player* holder = victim ? victim->ToPlayer() : nullptr)
        if (PlayerbotAI::IsTank(holder))
            return false;

    ThorimPhase2Role const role = GetThorimPhase2Role(botAI, bot);
    if (role == ThorimPhase2Role::MainTank)
        return true;

    if (role != ThorimPhase2Role::OffTank)
        return false;

    // Off-tank covers only with no main tank left: he's the corridor tank, still running back at the phase change,
    // and two of them answering is the ping-pong again.
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid const mainTank = PlayerbotAI::GetMainTankGuid(group);
    if (mainTank.IsEmpty())
        return true;

    Player* tank = ObjectAccessor::FindPlayer(mainTank);
    return !tank || !tank->IsAlive() || !NearThorimEncounter(tank);
}

bool ThorimSifBlizzardTrigger::IsActive()
{
    // No tank dodges in phase 2, by role: the holder check alone drops out for the half second after a human
    // tank's taunt, and the 30 yd flee then tows the boss into the ranged camp.
    Unit* boss = GetThorim(botAI);
    if (boss && boss->GetVictim() == bot)
        return false;

    if (ThorimPhase2Active(botAI))
    {
        ThorimPhase2Role const role = GetThorimPhase2Role(botAI, bot);
        if (role == ThorimPhase2Role::MainTank || role == ThorimPhase2Role::OffTank)
            return false;

        // The ring slides along its own radius to a clear bearing instead. This node's flee takes the furthest of
        // eight rays out to 30 yd, out of melee.
        if (role == ThorimPhase2Role::MeleeRing)
            return false;

        // The camp tests the zones at their real reach, not the bunny at 15. Under the cone the shelter walk wins.
        // Flat test, hence the floor check, or a bot still on the balcony dodges zones on the floor below.
        if (role == ThorimPhase2Role::Ranged)
            return bot->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD && ThorimSpotUnderBlizzard(bot, *bot) &&
                   !ThorimCampSpotInLitCone(botAI, *bot);
    }

    TooCloseToCreatureTrigger tooCloseToBlizzard(botAI);
    return tooCloseToBlizzard.TooCloseToCreature(NPC_SIF_BLIZZARD, ULDUAR_THORIM_SIF_BLIZZARD_RADIUS);
}
