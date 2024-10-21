#include "Playerbots.h"
#include "UtgardeKeepTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool KelesethFrostTombTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_FROST_TOMB))
        {
            return true;
        }
    }
    return false;
}

bool DalronnNontankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!boss) { return false; }

    return !botAI->IsTank(bot);
}

bool IngvarStaggeringRoarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_STAGGERING_ROAR))
        {
            return true;
        }
    }
    return false;
}

bool IngvarDreadfulRoarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        boss->FindCurrentSpellBySpellId(SPELL_DREADFUL_ROAR))
    {
        return true;
    }
    return false;
}

bool IngvarSmashTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss || !botAI->IsTank(bot)) { return false; }

    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_SMASH) ||
            boss->FindCurrentSpellBySpellId(SPELL_DARK_SMASH))
            {
                return true;
            }
    }
    return false;
}

bool IngvarSmashTankReturnTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    // if (!boss || !botAI->IsTank(bot) || boss->HasUnitState(UNIT_STATE_CASTING))
    // Ignore casting state as Ingvar will sometimes chain-cast a roar after a smash..
    // We don't want this to prevent our tank from repositioning properly.
    if (!boss || !botAI->IsTank(bot)) { return false; }

    return true;
}

bool NotBehindIngvarTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss || botAI->IsTank(bot)) { return false; }
    
    return AI_VALUE2(bool, "behind", "current target");
}
