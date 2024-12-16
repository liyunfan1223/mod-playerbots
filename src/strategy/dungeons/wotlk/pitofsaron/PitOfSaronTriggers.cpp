#include "Playerbots.h"
#include "PitOfSaronTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool MoveFromIckPursuitTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");

    if (boss && boss->FindCurrentSpellBySpellId(SPELL_PURSUIT) && bot->HasAura(SPELL_PURSUIT))
        return true;

    return false;
}

bool MoveFromPoisonNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");

    if (boss && boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_NORMAL) && bot->HasAura(SPELL_POISON_NOVA_NORMAL))
        return true;
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_HEROIC) && bot->HasAura(SPELL_POISON_NOVA_HEROIC))
        return true;

    return false;
}

bool MoveFromExplodingOrbTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");

    if (boss && boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_NORMAL) && bot->HasAura(SPELL_EXPLOSIVE_BARRAGE_NORMAL))
        return true;
    if (boss && boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_HEROIC) && bot->HasAura(SPELL_EXPLOSIVE_BARRAGE_HEROIC))
        return true;

    return false;
}
