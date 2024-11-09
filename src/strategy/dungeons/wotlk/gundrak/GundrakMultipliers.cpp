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

    if (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidPoisonNovaAction*>(action))
        {
            return 0.0f;
        }
    }

    if (!botAI->IsDps(bot)) { return 1.0f; }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    Unit* snakeWrap = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_SNAKE_WRAP)
        {
            snakeWrap = unit;
            break;
        }
    }
    // Prevent auto-target acquisition during snake wraps
    if (snakeWrap && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
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
