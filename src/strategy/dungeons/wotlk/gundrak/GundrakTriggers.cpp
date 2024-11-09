#include "Playerbots.h"
#include "GundrakTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool SladranPoisonNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "slad'ran");
    if (!boss) { return false; }
    
    return bool(boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA));
}

bool SladranSnakeWrapTrigger::IsActive()
{
    if (!botAI->IsDps(bot)) { return false; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_SNAKE_WRAP)
        {
            return true;
        }
    }
    return false;
}

bool GaldarahWhirlingSlashTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gal'darah");
    return boss && boss->HasAura(SPELL_WHIRLING_SLASH);
}
