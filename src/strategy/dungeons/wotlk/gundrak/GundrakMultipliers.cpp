#include "GundrakMultipliers.h"
#include "GundrakActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "GundrakTriggers.h"
#include "Action.h"

float SladranMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return 1.0f; }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA))
        {
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidPoisonNovaAction*>(action))
            {
                return 0.0f;
            }
        }
    return 1.0f;
}

float GaldarahMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    if (!boss) { return 1.0f; }

    if (boss->HasAura(SPELL_WHIRLING_SLASH))
        {
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidWhirlingSlashAction*>(action))
            {
                return 0.0f;
            }
        }
    return 1.0f;
}
