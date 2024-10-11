#include "Playerbots.h"
#include "DrakTharonKeepTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool CorpseExplodeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "trollgore");
    if (!boss) { return false; }

    float distance = 6.0f;  // 5 unit radius, 1 unit added as buffer
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_DRAKKARI_INVADER)
        {
            if (bot->GetExactDist2d(unit) < distance)
            {
                return true;
            }
        }
    }
    return false;
}

bool ArcaneFieldTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
    if (boss)
    {
        return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_FIELD);
    }
    return false;
}

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

bool GiftOfTharonjaTrigger::IsActive()
{
    return bot->HasAura(SPELL_GIFT_OF_THARONJA);
}
