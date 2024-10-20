#include "UtgardeKeepMultipliers.h"
#include "UtgardeKeepActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "UtgardeKeepTriggers.h"

float PrinceKelesethMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float SkarvaldAndDalronnMultiplier::GetValue(Action* action)
{
    // Only need to deal with Dalronn here. If he's dead, just fall back to normal dps strat
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float IngvarThePlundererMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    bool isTank = botAI->IsTank(bot);
    if (!boss) { return 1.0f; }

    // Prevent movement actions overriding current movement, we're probably dodging a slam
    if (isTank && bot->isMoving() && dynamic_cast<MovementAction*>(action))
    {
        return 0.0f;
    }

    // If boss is casting a roar, do not allow beginning a spell cast that is non-instant
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_STAGGERING_ROAR) ||
            boss->FindCurrentSpellBySpellId(SPELL_DREADFUL_ROAR))
        {
            if (dynamic_cast<CastSpellAction*>(action))
            {
                uint32 spellId = AI_VALUE2(uint32, "spell id", action->getName());
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo) { return 1.0f; }

                uint32 castTime = spellInfo->CalcCastTime(bot);
                if (castTime != 0)
                {
                    return 0.0f;
                }
            }
        }
        // Done with non-tank logic
        if (!isTank) { return 1.0f; }
        
        // TANK ONLY
        if (boss->FindCurrentSpellBySpellId(SPELL_SMASH) ||
            boss->FindCurrentSpellBySpellId(SPELL_DARK_SMASH))
        {
            // Prevent movement actions during smash which can mess up boss position.
            // Allow through IngvarDodgeSmashAction only, as well as any non-movement actions.
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<IngvarDodgeSmashAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    return 1.0f;
}
