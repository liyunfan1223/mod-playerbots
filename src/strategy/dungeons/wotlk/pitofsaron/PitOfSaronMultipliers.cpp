#include "PitOfSaronMultipliers.h"
#include "PitOfSaronActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "PitOfSaronTriggers.h"
#include "PitOfSaronActions.h"


float IckMultiplier::GetValue(Action* action) {
    Unit* boss = AI_VALUE2(Unit *, "find target", "ick");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;
    
    if (boss->FindCurrentSpellBySpellId(SPELL_PURSUIT &&
        bot->HasAura(SPELL_PURSUIT)))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromIckPursuitAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_NORMAL) &&
        bot->HasAura(SPELL_POISON_NOVA_NORMAL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromPoisonNovaAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_HEROIC) &&
        bot->HasAura(SPELL_POISON_NOVA_HEROIC))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromPoisonNovaAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_NORMAL) &&
        bot->HasAura(SPELL_EXPLOSIVE_BARRAGE_NORMAL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromExplodingOrbAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_HEROIC) &&
        bot->HasAura(SPELL_EXPLOSIVE_BARRAGE_HEROIC))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromExplodingOrbAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
