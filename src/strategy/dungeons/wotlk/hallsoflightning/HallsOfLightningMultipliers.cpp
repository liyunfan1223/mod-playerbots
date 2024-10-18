#include "HallsOfLightningMultipliers.h"
#include "HallsOfLightningActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "HallsOfLightningTriggers.h"
#include "Action.h"

float BjarngrimMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "general bjarngrim");
    if (!boss || botAI->IsHeal(bot)) { return 1.0f; }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND_BJARNGRIM))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidWhirlwindAction*>(action))
        {
            return 0.0f;
        }
    }

    // Detect boss adds this way as sometimes they don't get added to threat table on dps bots,
    // and some dps just stand at range and don't engage the boss at all as they can't find the adds
    // Unit* boss_add = AI_VALUE2(Unit*, "find target", "stormforged lieutenant");
    Unit* boss_add = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            boss_add = unit;
            break;
        }
    }

    if (!boss_add || botAI->IsTank(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }
    
    return 1.0f;
}

float VolkhanMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volkhan");
    if (!boss || botAI->IsTank(bot) || botAI->IsHeal(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }
    
    return 1.0f;
}

float IonarMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return 1.0f; }

    if(!bot->CanSeeOrDetect(boss))
        {
            if (dynamic_cast<MovementAction*>(action)
                && !dynamic_cast<DispersePositionAction*>(action)
                && !dynamic_cast<StaticOverloadSpreadAction*>(action))
            {
                return 0.0f;
            }
        }
    return 1.0f;
}

float LokenMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loken");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

    if (boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA)
        && dynamic_cast<MovementAction*>(action)
        && !dynamic_cast<AvoidLightningNovaAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
