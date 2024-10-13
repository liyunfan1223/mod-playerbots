#include "Playerbots.h"
#include "GundrakTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool SladranPoisonNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA);
}

bool GaldarahWhirlingSlashTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    return boss && boss->HasAura(SPELL_WHIRLING_SLASH);
}
