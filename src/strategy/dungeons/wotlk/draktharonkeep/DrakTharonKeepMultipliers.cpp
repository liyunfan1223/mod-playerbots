#include "DrakTharonKeepMultipliers.h"
#include "DrakTharonKeepActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "DrakTharonKeepTriggers.h"
#include "Action.h"

float NovosMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
    if (!boss) { return 1.0f; }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_ARCANE_FIELD) && bot->GetTarget())
    {
        if (dynamic_cast<DpsAssistAction*>(action)
            || dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float TharonjaMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_GIFT_OF_THARONJA)) { return 1.0f; }
    
    // Suppress all skills that are not enabled in skeleton form.
    // Still allow non-ability actions such as movement
    if (dynamic_cast<CastSpellAction*>(action)
        && !dynamic_cast<CastSlayingStrikeAction*>(action)
        && !dynamic_cast<CastTauntAction*>(action)
        && !dynamic_cast<CastBoneArmorAction*>(action)
        && !dynamic_cast<CastTouchOfLifeAction*>(action))
    {
        return 0.0f;
    }
    // Also suppress FleeAction to prevent ranged characters from avoiding melee range
    if (dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    // Tanks should only taunt, no slaying strike
    if (botAI->IsTank(bot))
    {
        if (dynamic_cast<CastSlayingStrikeAction*>(action))
        {
            return 0.0f;
        }
    }
    // Dps & healer should not taunt
    else
    {
        if (dynamic_cast<CastTauntAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
