#include "Playerbots.h"
#include "AzjolNerubTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool KrikthirWebWrapTrigger::IsActive()
{
    if (!botAI->IsDps(bot)) { return false; }
    
    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_WEB_WRAP)
        {
            return true;
        }
    }

    return false;
}

bool KrikthirWatchersTrigger::IsActive()
{
    if (!botAI->IsDps(bot)) { return false; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_KRIKTHIR)
        {
            return true;
        }
    }
    return false;
}

// bool AnubarakImpaleTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "anub'arak");
//     if (!boss) { return false; }
//     GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
//     for (auto i = triggers.begin(); i != triggers.end(); i++)
//     {
//         Unit* unit = botAI->GetUnit(*i);
        
//         if (unit)
//         {
//             bot->Yell("TRIGGER="+unit->GetName(), LANG_UNIVERSAL);
//         }
//     }
//     return false;
// }

bool AnubarakPoundTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'arak");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_POUND);
}
