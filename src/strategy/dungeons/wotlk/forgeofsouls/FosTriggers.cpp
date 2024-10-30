#include "FosTriggers.h"
#include "Playerbots.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = nullptr;
    boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
            return true;
    }
    return false;
}

bool SwitchToSoulFragment::IsActive()
{
    Unit* fragment = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            return true;
        }
    }

    return false;
    
}

bool BronjahmPositionTrigger::IsActive()
{

    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (boss)
        return true;

    return false;
}
