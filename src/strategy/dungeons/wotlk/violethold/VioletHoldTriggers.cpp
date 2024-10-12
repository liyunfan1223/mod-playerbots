#include "Playerbots.h"
#include "VioletHoldTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool ErekemTargetTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "erekem") && botAI->IsDps(bot);
}

bool IchoronTargetTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "ichoron") && !botAI->IsHeal(bot);
}

bool VoidShiftTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    return boss && bot->HasAura(SPELL_VOID_SHIFTED) && !botAI->IsHeal(bot);
}

bool ShroudOfDarknessTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    return boss && boss->HasAura(SPELL_SHROUD_OF_DARKNESS);
}

bool CyanigosaPositioningTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "cyanigosa");
    // Include healers here for now, otherwise they stand in things
    return boss && !botAI->IsTank(bot) && !botAI->IsRangedDps(bot);
    // return boss && botAI->IsMelee(bot) && !botAI->IsTank(bot);
}

// bool CorpseExplodeTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "trollgore");
//     if (!boss) { return false; }

//     float distance = 6.0f;  // 5 unit radius, 1 unit added as buffer
//     GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
//     for (auto i = corpses.begin(); i != corpses.end(); ++i)
//     {
//         Unit* unit = botAI->GetUnit(*i);
//         if (unit && unit->GetEntry() == NPC_DRAKKARI_INVADER)
//         {
//             if (bot->GetExactDist2d(unit) < distance)
//             {
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// bool CrystalHandlerTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
//     if (!boss) { return false; }

//     // Target is not findable from threat table using AI_VALUE2(),
//     // therefore need to search manually for the unit name
//     GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

//     for (auto i = targets.begin(); i != targets.end(); ++i)
//     {
//         Unit* unit = botAI->GetUnit(*i);
//         if (unit && unit->GetEntry() == NPC_CRYSTAL_HANDLER)
//         {
//             return true;
//         }
//     }
//     return false;
// }
