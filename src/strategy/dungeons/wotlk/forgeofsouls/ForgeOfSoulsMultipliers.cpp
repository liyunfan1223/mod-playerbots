#include "ForgeOfSoulsMultipliers.h"
#include "ForgeOfSoulsActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "ForgeOfSoulsTriggers.h"
#include "ForgeOfSoulsActions.h"


float BronjahmMultiplier::GetValue(Action* action) {
    Unit* boss = nullptr;
    boss = AI_VALUE2(Unit *, "find target", "bronjahm");
    if (!boss)
        return 1.0f;
    if (botAI->IsTank(bot))
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromBronjahmAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float AttackFragmentMultiplier::GetValue(Action* action)
{
    if (botAI->IsHeal(bot) && dynamic_cast<AttackCorruptedSoulFragmentAction*>(action))
        return 0.0f;

    Unit* fragment = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            fragment = unit;
            break;
        }
    }

    if (fragment && dynamic_cast<BronjahmDpsPositionAction*>(action))
        return 0.0f;

    if (fragment && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}
