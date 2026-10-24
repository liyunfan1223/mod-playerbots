/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_Thorim.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldEncounter_Thorim.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <set>
#include <string>

using namespace EncounterHelpers;

// Target pickers. They all derive from AttackAction, so a guard zeroing AttackAction wholesale must spare these
// or a bot holding a bad target is stuck on it for the rest of the pull.
static bool ThorimIsTargetSelectionAction(Action* action)
{
    // The encounter's own picker too, which nothing below catches. Without it the arena target guard kills the
    // node that drops the very target the guard fires on.
    return dynamic_cast<ThorimDpsPriorityAction*>(action) || dynamic_cast<DpsAssistAction*>(action) ||
           dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
           dynamic_cast<AggressiveTargetAction*>(action) || dynamic_cast<AttackAnythingAction*>(action) ||
           dynamic_cast<AttackLeastHpTargetAction*>(action);
}

float ThorimRunicBarrierMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    // Only the swing and the walk to it: a blanket stop throws away DPS the shield never punishes, and the
    // gauntlet is on the hard-mode clock.
    if (!dynamic_cast<MeleeAction*>(action) && !dynamic_cast<ReachTargetAction*>(action))
        return 1.0f;

    if (!ThorimBarrierBailLatched(botAI, bot))
        return 1.0f;

    // Gated on the current target, so a switch onto anything else releases this for free.
    Unit* colossus = GetThorimRunicColossus(botAI);
    return colossus && AI_VALUE(Unit*, "current target") == colossus ? 0.0f : 1.0f;
}

float ThorimArenaTargetGuardMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<ReachTargetAction*>(action))
        return 1.0f;

    // The escape hatch. Zeroing these too is a deadlock: the guard fires because the target is wrong,
    // and the action it kills is the one that would pick a different one.
    if (ThorimIsTargetSelectionAction(action))
        return 1.0f;

    if (!ThorimSplitActive(botAI) || GetThorimSquad(botAI, bot) != ThorimSquad::Arena)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !ThorimInArenaBox(target) ? 0.0f : 1.0f;
}

float ThorimBalconyGuardMultiplier::GetValue(Action* action)
{
    if (!action || (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<ReachTargetAction*>(action)))
        return 1.0f;

    // Above the floor line and in the corridor half. The arena squad never gets up here, and a bot
    // that has already dropped for phase 2 is the phase 2 nodes' business.
    if (bot->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD ||
        GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return 1.0f;

    // Same escape hatch the arena guard needs: the node this would kill is the one that drops the bad
    // target.
    if (ThorimIsTargetSelectionAction(action))
        return 1.0f;

    // He is on the floor now and there is no walkable way down from here, so a reach walks the bot back
    // through the hallway. It is also what would pull a ranged bot off its wait spot up here.
    if (dynamic_cast<ReachTargetAction*>(action) && ThorimPhase2Active(botAI))
        return 0.0f;

    if (!ThorimSplitActive(botAI))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    Unit* boss = GetThorim(botAI);
    return (target == boss || target->GetEntry() == NPC_SIF) ? 0.0f : 1.0f;
}
