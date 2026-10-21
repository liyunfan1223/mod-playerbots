// Marwyn bot tactics: leave Shared Suffering on its target and heal through it.

#include "GenericSpellActions.h"
#include "HoRMultipliers.h"
#include "HoRPolicy.h"
#include "Playerbots.h"

float MarwynSharedSufferingMultiplier::GetValue(Action* action)
{
    bool partyCure = dynamic_cast<CurePartyMemberAction*>(action) != nullptr;
    if (!partyCure && !dynamic_cast<CastCureSpellAction*>(action))
        return 1.0f;

    Unit* target = partyCure ? action->GetTarget() : bot;
    bool suffering = target && (target->HasAura(SPELL_SHARED_SUFFERING) || target->HasAura(SPELL_SHARED_SUFFERING_HC));
    return HallsOfReflection::SharedSufferingAllows(true, suffering) ? 1.0f : 0.0f;
}
