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
