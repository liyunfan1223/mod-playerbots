#include "FosMultipliers.h"
#include "FosActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "FosTriggers.h"


float BronjahmMultiplier::GetValue(Action* action) {
    Unit* boss = nullptr;
    boss = AI_VALUE2(Unit *， "find target", "bronjahm");
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromBronjahmAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f; }

float AttackFragmentMultiplier::GetValue(Action* action)
{
    auto isTank = botAI->IsTank();
    if (isTank && dynamic_cast<AttackCorruptedSoulFragmentAction>(action))
        return 0.0f;
    return 1.0f; }
