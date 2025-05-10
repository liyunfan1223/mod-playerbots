#include "PitOfSaronMultipliers.h"
#include "PitOfSaronActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "PitOfSaronTriggers.h"



float IckAndKrickMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return 1.0f;

    // Allow the IckAndKrickAction to run
    if (dynamic_cast<IckAndKrickAction*>(action))
        return 1.0f;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) || boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC)) && bot->GetExactDist2d(boss) < 20.0f)
        return 0.0f;  // Cancel all other actions when we need to handle Poison Nova

    if (bot->GetExactDist2d(boss) < 15.0f && bot->HasAura(SPELL_PURSUIT) && !botAI->IsTank(bot))
        return 0.0f;  // Cancel all other actions when we need to handle Pursuit

    if (!botAI->IsHeal(bot) && boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) || boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK)))
        return 0.0f;  // Cancel all other actions when we need to handle Explosive Barrage

    return 1.0f;
}

float GarfrostMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "garfrost");
    if (!boss)
        return 1.0f;

    return 1.0f;
}
