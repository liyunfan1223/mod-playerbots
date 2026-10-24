/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Thorim.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <cmath>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldEncounter_Thorim.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include <TankAssistStrategy.h>

bool ThorimDpsPriorityAction::isUseful()
{
    ThorimDpsPriorityTrigger thorimDpsPriorityTrigger(botAI);
    return thorimDpsPriorityTrigger.IsActive();
}

bool ThorimDpsPriorityAction::Execute(Event /*event*/)
{
    // Before anything is read: a pinned target outranks every pick made below, so clearing has to
    // happen first or this action spends the pull losing to a mark it already replaced.
    ThorimClearStaleMarks(botAI, bot);

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Drop a forbidden target even with nothing better in reach: the arena target guard shuts down a bot
    // holding one, and an empty target lets the generic picker retry next tick.
    if (currentTarget && !ThorimDpsTargetAllowed(botAI, currentTarget))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
        currentTarget = nullptr;
        context->GetValue<Unit*>("current target")->Set(nullptr);
    }

    Unit* target = GetThorimDpsTarget(botAI, bot, currentTarget);
    if (!target || !ThorimDpsTargetAllowed(botAI, target))
        return false;

    if (target == currentTarget && (!PlayerbotAI::IsMelee(bot) || bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING)))
        return false;

    return Attack(target);
}

bool ThorimArenaPositioningAction::isUseful()
{
    ThorimArenaPositioningTrigger thorimArenaPositioningTrigger(botAI);
    return thorimArenaPositioningTrigger.IsActive();
}

bool ThorimArenaPositioningAction::Execute(Event /*event*/)
{
    Position anchor;
    if (!GetThorimArenaAnchor(botAI, bot, anchor))
        return false;

    FollowMasterStrategy followMasterStrategy(botAI);
    if (botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
        ThorimNoteFollowMasterStripped(bot);
    }

    // Reach then hold. A tight deadband has the bot sliding on its spot forever, and a moving bot
    // casts nothing.
    if (!ThorimArenaAnchorNeedsMove(botAI, bot, anchor))
        return false;

    return MoveTo(bot->GetMapId(), anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool ThorimFrostAuraAction::isUseful()
{
    ThorimFrostAuraTrigger thorimFrostAuraTrigger(botAI);
    return thorimFrostAuraTrigger.IsActive();
}

bool ThorimFrostAuraAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("frost resistance aura", Event(), true);
}

bool ThorimPetLeashAction::isUseful()
{
    ThorimPetLeashTrigger thorimPetLeashTrigger(botAI);
    return thorimPetLeashTrigger.IsActive();
}

bool ThorimPetLeashAction::Execute(Event /*event*/)
{
    std::vector<Unit*> pets;

    // All of them in one pass. A bot can have a pet and a guardian up at once, and leaving the second
    // one out there for another tick is a pack pulled for no reason.
    if (ThorimStrayPets(botAI, bot, pets))
        for (Unit* pet : pets)
            ThorimRecallPet(bot, pet);

    // Home before bearing: a pet out in the corridor pulls packs, one off its slot only bridges Chain Lightning.
    // The recall stamps the same throttle, so a pet just sent home isn't redirected.
    if (ThorimPetsOffRingSlot(botAI, bot, pets))
        for (Unit* pet : pets)
            ThorimSendPetToRingSlot(botAI, bot, pet);

    // False on purpose: this moved a pet, not the bot, so the bot's own node still gets this tick.
    return false;
}

bool ThorimChargedOrbAction::isUseful()
{
    ThorimChargedOrbTrigger thorimChargedOrbTrigger(botAI);
    return thorimChargedOrbTrigger.IsActive();
}

bool ThorimChargedOrbAction::Execute(Event /*event*/)
{
    Position escape;
    if (!ThorimChargedOrbEscape(botAI, bot, escape))
        return false;

    ThorimNoteOrbEscape(bot, escape);

    // No arrival latch here, unlike the anchor: the escape point stops being offered the moment the
    // bot is out of the field, so there is nothing for a deadband to damp.
    return MoveTo(bot->GetMapId(), escape.GetPositionX(), escape.GetPositionY(), escape.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool ThorimArenaLeashAction::isUseful()
{
    ThorimArenaLeashTrigger thorimArenaLeashTrigger(botAI);
    return thorimArenaLeashTrigger.IsActive();
}

bool ThorimArenaLeashAction::Execute(Event /*event*/)
{
    // Straying is the symptom; the master walking off down the corridor is usually the cause, so take
    // "follow master" here as well rather than waiting for the arena node to catch a quiet moment.
    FollowMasterStrategy followMasterStrategy(botAI);
    if (botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy(REMOVE_STRATEGY_CHAR + followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT);
        ThorimNoteFollowMasterStripped(bot);
    }

    // This bot's own spot, not the middle, or every fence trip collapses the formation the fence protects.
    // The middle is only the fallback, never the nearest box edge: there a rounding error is a wipe.
    Position anchor = ULDUAR_THORIM_NEAR_ARENA_CENTER;
    GetThorimArenaAnchor(botAI, bot, anchor);

    return MoveTo(bot->GetMapId(), anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool ThorimLaneMovementAction::MoveToGauntletWaypoint(bool leftLane, uint8 index, bool forceCombatPriority)
{
    Position const& waypoint = GetThorimGauntletWaypoint(leftLane, index);

    // The second waypoint moves at combat priority without the shortened delay, in both lanes: it's where
    // the squad rounds into the Colossus's line of sight.
    bool const combat = forceCombatPriority || index == 1;

    return MoveTo(bot->GetMapId(), waypoint.GetPositionX(), waypoint.GetPositionY(), waypoint.GetPositionZ(), false,
                  false, false, true,
                  combat ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL, !combat);
}

bool ThorimGauntletPositioningAction::isUseful()
{
    ThorimGauntletPositioningTrigger thorimGauntletPositioningTrigger(botAI);
    return thorimGauntletPositioningTrigger.IsActive();
}

bool ThorimGauntletPositioningAction::Execute(Event /*event*/)
{
    FollowMasterStrategy followMasterStrategy(botAI);

    Unit* master = botAI->GetMaster();
    if (!master)
        return false;

    if (master->GetDistance(ULDUAR_THORIM_NEAR_ENTRANCE_POSITION) < ULDUAR_THORIM_GATE_RADIUS &&
        bot->GetDistance2d(master) > ULDUAR_THORIM_GATE_FOLLOW_DISTANCE)
    {
        if (MoveTo(bot->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), false,
                   false, false, true, MovementPriority::MOVEMENT_NORMAL, true))
        {
            if (!botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
                botAI->ChangeStrategy(ADD_STRATEGY_CHAR + followMasterStrategy.getName(),
                                      BotState::BOT_STATE_NON_COMBAT);

            return true;
        }
    }

    uint8 index = 0;
    bool leftLane = false;
    if (ThorimGauntletLaneIndex(master, index, leftLane))
    {
        // The master's progress, but in the lane the last Runic Smash telegraph left safe, or the formation
        // walks everyone back into the blast.
        bool preferredLane = false;
        if (ThorimPreferredGauntletLane(botAI, preferredLane))
            leftLane = preferredLane;

        return MoveToGauntletWaypoint(leftLane, index, false);
    }

    return false;
}

bool ThorimBalconyAdvanceAction::isUseful()
{
    ThorimBalconyAdvanceTrigger thorimBalconyAdvanceTrigger(botAI);
    return thorimBalconyAdvanceTrigger.IsActive();
}

bool ThorimBalconyAdvanceAction::Execute(Event /*event*/)
{
    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive())
        return false;

    // Still walked as the chain once he's down: the edge and both Paralytic Field bunnies sit on the centre line,
    // so a straight line from south of the second doors crosses one. The chain ends at the jump point anyway.
    bool const bossDown = boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;

    // Before the jump test, or the latch never reaches the end and a bot on the edge never jumps.
    uint8 const step = ThorimAdvanceBalconyStep(bot);

    // Ranged wait out the first Chain Lightning up here, spread. Ahead of the jump since hold 2 is inside the
    // edge test, and only past BALCONY_5: a line to a hold from any earlier waypoint crosses a bunny's field.
    Position hold;
    if (bossDown && step >= ULDUAR_THORIM_BALCONY_WAYPOINTS - 1 && ThorimBalconyHoldSpot(botAI, bot, hold))
    {
        if (bot->GetExactDist2d(&hold) <= ULDUAR_THORIM_BALCONY_HOLD_TOLERANCE)
            return false;

        return MoveTo(bot->GetMapId(), hold.GetPositionX(), hold.GetPositionY(), hold.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true);
    }

    // Distance, never the latch alone: a step left over from an earlier pull fires the jump from the top of the
    // ramp. Same tolerance as the step latch, so a bot the walk counts as arrived also jumps.
    bool const atEdge =
        bot->GetExactDist2d(&ULDUAR_THORIM_JUMP_START_POINT) <= ULDUAR_THORIM_BALCONY_ARRIVE_TOLERANCE;

    if (bossDown && atEdge)
        return JumpTo(bot->GetMapId(), ULDUAR_THORIM_JUMP_END_POINT.GetPositionX(),
                      ULDUAR_THORIM_JUMP_END_POINT.GetPositionY(), ULDUAR_THORIM_JUMP_END_POINT.GetPositionZ(),
                      MovementPriority::MOVEMENT_COMBAT);

    // Standing on the edge with him still up: done walking, and the tick belongs to whatever shoots him.
    if (step >= ULDUAR_THORIM_BALCONY_WAYPOINTS && atEdge)
        return false;

    // Clamps to the last waypoint, the edge, so a bot knocked past the end walks back instead of stalling.
    // Both bunnies are well south, so nothing to cross from up there.
    Position const& waypoint = GetThorimBalconyWaypoint(step);

    // Combat priority: at normal, every step loses to "reach melee" on a boss nothing up here can reach, which
    // holds the mover up to 5s and aims straight up the middle, over both Paralytic Field bunnies.
    return MoveTo(bot->GetMapId(), waypoint.GetPositionX(), waypoint.GetPositionY(), waypoint.GetPositionZ(), false,
                  false, false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool ThorimRunicSmashAction::isUseful()
{
    ThorimRunicSmashTrigger thorimRunicSmashTrigger(botAI);
    return thorimRunicSmashTrigger.IsActive();
}

bool ThorimRunicSmashAction::Execute(Event /*event*/)
{
    bool safeLane = false;
    if (!ThorimPreferredGauntletLane(botAI, safeLane))
        return false;

    uint8 index = 0;
    if (!ThorimResolveGauntletIndex(botAI, bot, index))
        return false;

    return MoveToGauntletWaypoint(safeLane, index, true);
}

bool ThorimRunicBarrierBailAction::isUseful()
{
    ThorimRunicBarrierBailTrigger thorimRunicBarrierBailTrigger(botAI);
    return thorimRunicBarrierBailTrigger.IsActive();
}

bool ThorimRunicBarrierBailAction::Execute(Event /*event*/)
{
    Unit* colossus = GetThorimRunicColossus(botAI);
    if (!colossus)
        return false;

    float const currentDistance = bot->GetDistance(colossus);
    if (currentDistance >= ULDUAR_THORIM_BARRIER_BAIL_DISTANCE)
        return false;

    // No AttackStop: the target stays, so everything but a melee swing keeps landing from out here.
    // Not FleePosition: it clamps travel to Playerbots.FleeDistance and can leave the bot inside the shield's reach.
    return MoveAway(colossus, ULDUAR_THORIM_BARRIER_BAIL_DISTANCE - currentDistance);
}

bool ThorimResetEncounterStateAction::Execute(Event /*event*/)
{
    // Hand "follow master" back before the record of having taken it goes with the rest of the state,
    // or an arena squad bot never follows anyone again after the kill.
    if (ThorimFollowMasterStripped(bot))
    {
        FollowMasterStrategy followMasterStrategy(botAI);
        if (!botAI->HasStrategy(followMasterStrategy.getName(), BotState::BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy(ADD_STRATEGY_CHAR + followMasterStrategy.getName(),
                                  BotState::BOT_STATE_NON_COMBAT);
    }

    ResetThorimEncounterState(bot, false);

    // Never claims the tick: clearing state is bookkeeping, and the pull still needs every node below
    // this one to run on the same tick.
    return false;
}

bool ThorimFallFromFloorAction::Execute(Event /*event*/)
{
    Player* master = botAI->GetMaster();
    if (!master || master->GetMap() != bot->GetMap())
        return false;

    bot->NearTeleportTo(master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                        master->GetOrientation());
    return true;
}

bool ThorimFallFromFloorAction::isUseful()
{
    ThorimFallFromFloorTrigger thorimFallFromFloorTrigger(botAI);
    return thorimFallFromFloorTrigger.IsActive();
}

bool ThorimPhase2PositioningAction::Execute(Event /*event*/)
{
    ThorimPhase2Role const role = GetThorimPhase2Role(botAI, bot);

    Position targetPosition;
    if (!TryGetThorimPhase2Spot(botAI, bot, role, targetPosition))
        return false;

    bool const ringSlot = role == ThorimPhase2Role::MeleeRing;

    // Arrival is set by the trigger and cleared here as the move goes out, so the "stay" branch below only
    // runs if the spot changed between the trigger's ask and this one.
    if (ringSlot)
    {
        if (!ThorimRingWantsMove(botAI, bot, targetPosition))
        {
            ThorimRingMarkArrived(bot);
            return false;
        }

        ThorimRingClearArrived(bot);
    }

    // The main tank backs into his spot so he keeps facing the boss he is dragging south.
    bool const backward = role == ThorimPhase2Role::MainTank;

    MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ(),
           false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, backward);

    if (ringSlot)
        return true;

    return bot->GetDistance(targetPosition) <= ULDUAR_THORIM_PHASE2_SPOT_TOLERANCE;
}

bool ThorimPhase2PositioningAction::isUseful()
{
    ThorimPhase2PositioningTrigger thorimPhase2PositioningTrigger(botAI);
    return thorimPhase2PositioningTrigger.IsActive();
}

bool ThorimSifBlizzardAction::Execute(Event event)
{
    if (!ThorimPhase2Active(botAI) || GetThorimPhase2Role(botAI, bot) != ThorimPhase2Role::Ranged)
        return MoveAwayFromCreatureAction::Execute(event);

    // Nothing clear and cone-safe in reach: stand and take the tick rather than run the trail.
    Position escape;
    if (!ThorimCampBlizzardEscape(botAI, bot, escape))
        return false;

    // Forced, or a walk home already under way at combat priority turns the step away with "wait" until
    // it finishes, and the bot keeps standing in the zone meanwhile.
    return MoveTo(bot->GetMapId(), escape.GetPositionX(), escape.GetPositionY(), escape.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true);
}

bool ThorimUnbalancingStrikeSwapAction::isUseful()
{
    ThorimUnbalancingStrikeSwapTrigger thorimUnbalancingStrikeSwapTrigger(botAI);
    return thorimUnbalancingStrikeSwapTrigger.IsActive();
}

bool ThorimTakeBossAction::Execute(Event event)
{
    Unit* boss = GetThorim(botAI);
    if (!boss || !boss->IsAlive())
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    if (boss->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    return false;
}

bool ThorimTankPickupAction::isUseful()
{
    ThorimTankPickupTrigger thorimTankPickupTrigger(botAI);
    return thorimTankPickupTrigger.IsActive();
}
