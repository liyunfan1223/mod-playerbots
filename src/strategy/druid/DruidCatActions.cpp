#include "DruidCatActions.h"

bool CastMangleCatAction::isUseful()
{
    return CastMeleeDebuffSpellAction::isUseful() && !botAI->HasAura("mangle (bear)", GetTarget(), false, isOwner);
}