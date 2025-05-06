#include "RaidNaxxTriggers.h"

#include "EventMap.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "Trigger.h"

bool AuraRemovedTrigger::IsActive()
{
    bool check = botAI->HasAura(name, bot, false, false, -1, true);
    bool ret = false;
    if (prev_check && !check)
    {
        ret = true;
    }
    prev_check = check;
    return ret;
}

bool MutatingInjectionRemovedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return false;
    }
    return HasNoAuraTrigger::IsActive() && botAI->GetState() == BOT_STATE_COMBAT && botAI->IsRanged(bot);
}

template <class T>
bool BossEventTrigger<T>::IsActive()
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != boss_entry)
    {
        return false;
    }
    T* ai = dynamic_cast<T*>(boss->GetAI());
    EventMap* eventMap = &ai->events;
    if (!eventMap)
    {
        return false;
    }
    const uint32 event_time = eventMap->GetNextEventTime(event_id);
    if (event_time != last_event_time)
    {
        last_event_time = event_time;
        return true;
    }
    return false;
}

bool GrobbulusCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != boss_entry)
    {
        return false;
    }
    if (!botAI->IsMainTank(bot))
    {
        return false;
    }
    // bot->Yell("has aggro on " + boss->GetName() + " : " + to_string(AI_VALUE2(bool, "has aggro", "boss target")),
    // LANG_UNIVERSAL);
    return AI_VALUE2(bool, "has aggro", "boss target");
}

bool HeiganMeleeTrigger::IsActive()
{
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan)
    {
        return false;
    }
    return !botAI->IsRanged(bot);
}

bool HeiganRangedTrigger::IsActive()
{
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan)
    {
        return false;
    }
    return botAI->IsRanged(bot);
}

bool RazuviousTankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return helper.UpdateBossAI() && botAI->IsTank(bot);
    }
    return helper.UpdateBossAI() && bot->getClass() == CLASS_PRIEST;
}

bool RazuviousNontankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return helper.UpdateBossAI() && !(botAI->IsTank(bot));
    }
    return helper.UpdateBossAI() && !(bot->getClass() == CLASS_PRIEST);
}

bool HorsemanAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsAttracter(bot);
}

bool HorsemanExceptAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return !helper.IsAttracter(bot);
}

bool SapphironGroundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseGround();
}

bool SapphironFlightTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseFlight();
}

bool GluthTrigger::IsActive() { return helper.UpdateBossAI(); }

bool GluthMainTankMortalWoundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    if (!botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
    {
        return false;
    }
    Aura* aura = botAI->GetAura("mortal wound", mt, false, true);
    if (!aura || aura->GetStackAmount() < 5)
    {
        return false;
    }
    return true;
}

bool KelthuzadTrigger::IsActive() { return helper.UpdateBossAI(); }

bool AnubrekhanTrigger::IsActive() {
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss)
    {
        return false;
    }
    return true;
}

bool LoathebTrigger::IsActive() { return helper.UpdateBossAI(); }

bool ThaddiusPhasePetTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhasePet();
}

bool ThaddiusPhaseTransitionTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseTransition();
}

bool ThaddiusPhaseThaddiusTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseThaddius();
}

template bool BossEventTrigger<Grobbulus::boss_grobbulus::boss_grobbulusAI>::IsActive();
