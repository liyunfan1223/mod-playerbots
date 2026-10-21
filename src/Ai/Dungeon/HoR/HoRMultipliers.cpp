/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRMultipliers.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"

float MarwynSharedSufferingMultiplier::GetValue(Action* action)
{
    // Dispelling Shared Suffering splits its remaining damage over the whole party; groups healed through it
    bool partyCure = dynamic_cast<CurePartyMemberAction*>(action) != nullptr;
    if (!partyCure && !dynamic_cast<CastCureSpellAction*>(action))
        return 1.0f;

    Unit* target = partyCure ? action->GetTarget() : bot;
    if (target && (target->HasAura(SPELL_SHARED_SUFFERING) || target->HasAura(SPELL_SHARED_SUFFERING_HC)))
        return 0.0f;

    return 1.0f;
}
