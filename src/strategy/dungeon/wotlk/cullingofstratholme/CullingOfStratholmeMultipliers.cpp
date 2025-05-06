#include "CullingOfStratholmeMultipliers.h"
#include "CullingOfStratholmeActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "CullingOfStratholmeTriggers.h"
#include "Action.h"

float EpochMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono-lord epoch");
    if (!boss) { return 1.0f; }

    if (bot->getClass() == CLASS_HUNTER) { return 1.0f; }
    
    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

    return 1.0f;
}
