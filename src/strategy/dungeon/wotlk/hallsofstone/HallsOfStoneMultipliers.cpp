#include "HallsOfStoneMultipliers.h"
#include "HallsOfStoneActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "HallsOfStoneTriggers.h"
#include "Action.h"

float KrystallusMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "krystallus");
    if (!boss) { return 1.0f; }
    
    // Check both of these... the spell is applied first, debuff later.
    // Neither is active for the full duration so we need to trigger off both
    if (bot->HasAura(SPELL_GROUND_SLAM) || bot->HasAura(DEBUFF_GROUND_SLAM))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<ShatterSpreadAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float SjonnirMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sjonnir the ironshaper");
    if (!boss) { return 1.0f; }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_RING))
        {
            // Problematic since there's a lot of movement on this boss, will prevent players from positioning
            // well to deal with adds etc. during the channel period. Takes a bit of work to improve this though
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidLightningRingAction*>(action))
            {
                return 0.0f;
            }
        }
    return 1.0f;
}
