/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RtiTriggers.h"
#include "Playerbots.h"

bool NoRtiTrigger::IsActive()
{
    // Do not auto-react to raid icons while out of combat.
    // Out-of-combat RTI usage (explicit chat commands) is handled by chat triggers,
    // not by this generic trigger.
    if (!bot->IsInCombat())
        return false;

    Unit* target = AI_VALUE(Unit*, "rti target");
    return target == nullptr;
}

// Fires when the RTI CC target should be crowd controlled by this spell.
// Standard path: the target is already in the attackers list and "cc target" matches the RTI
// mark — delegates to HasCcTargetTrigger to confirm no one else is already CCing it.
bool RtiCcTrigger::IsActive()
{
    Unit* rtiCcTarget = AI_VALUE(Unit*, "rti cc target");
    if (!rtiCcTarget)
        return false;

    Unit* ccTarget = AI_VALUE2(Unit*, "cc target", getName());
    if (ccTarget && ccTarget == rtiCcTarget)
        return IsCcTargetFree(ccTarget, rtiCcTarget);

    return botAI->CanCastSpell(getName(), rtiCcTarget);
}
