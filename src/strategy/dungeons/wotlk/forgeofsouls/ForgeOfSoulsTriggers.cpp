#include "Playerbots.h"
#include "ForgeOfSoulsTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (!boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
        return false;

    if (!bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool SwitchToSoulFragment::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (!activeSoulExists)
        return false;

    return true;
}

bool BronjahmPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool DevourerOfSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    return true;
}
