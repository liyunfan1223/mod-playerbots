#include "ForgeOfSoulsMultipliers.h"
#include "ForgeOfSoulsActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "ForgeOfSoulsTriggers.h"
#include "ForgeOfSoulsActions.h"


float BronjahmMultiplier::GetValue(Action* action) {
    Unit* boss = AI_VALUE2(Unit *, "find target", "bronjahm");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL) &&
        bot->HasAura(SPELL_CORRUPT_SOUL))
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

    if (fragment && botAI->IsDps(bot) && dynamic_cast<BronjahmGroupPositionAction*>(action))
        return 0.0f;

    return 1.0f;
}
