#include "Playerbots.h"
#include "HallsOfLightningTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool StormforgedLieutenantTrigger::IsActive()
{
    if (botAI->IsTank(bot) || botAI->IsHeal(bot)) { return false; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            return true;
        }
    }
    return false;
}

bool BjarngrimWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "general bjarngrim");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND_BJARNGRIM);
}

bool VolkhanTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volkhan");
    return boss && !botAI->IsTank(bot) && !botAI->IsHeal(bot);
}

bool IonarStaticOverloadTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_STATIC_OVERLOAD))
        {
            return true;
        }
    }
    return false;
}

bool IonarBallLightningTrigger::IsActive()
{
    if (botAI->IsMelee(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BALL_LIGHTNING);
}

bool IonarTankAggroTrigger::IsActive()
{
    if (!botAI->IsTank(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }
    
    return AI_VALUE2(bool, "has aggro", "current target");
}

bool IonarDisperseTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }

    return !bot->CanSeeOrDetect(boss) || boss->FindCurrentSpellBySpellId(SPELL_DISPERSE);
}

bool LokenRangedTrigger::IsActive()
{
    return !botAI->IsMelee(bot) && AI_VALUE2(Unit*, "find target", "loken");
}

bool LokenLightningNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loken");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA);
}
