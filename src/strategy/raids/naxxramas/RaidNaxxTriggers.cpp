#include "EventMap.h"
#include "Playerbots.h"
#include "RaidNaxxTriggers.h"
#include "ScriptedCreature.h"
#include "Trigger.h"

bool AuraRemovedTrigger::IsActive() {
    bool check = botAI->HasAura(name, bot, false, false, -1, true);
    bool ret = false;
    if (prev_check && !check) {
        ret = true;
    }
    prev_check = check;
    return ret;
}

bool MutatingInjectionRemovedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss) {
        return false;
    }
    return HasNoAuraTrigger::IsActive() && botAI->GetState() == BOT_STATE_COMBAT && botAI->IsRanged(bot);
}

template<class T>
bool BossEventTrigger<T>::IsActive()
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != boss_entry) {
        return false;
    }
    T* ai = dynamic_cast<T*>(boss->GetAI());
    EventMap *eventMap = &ai->events;
    if (!eventMap) {
        return false;
    }
    const uint32 event_time = eventMap->GetNextEventTime(event_id);
    if (event_time != last_event_time) {
        last_event_time = event_time;
        return true;
    }
    return false;
}

template<class T>
bool BossPhaseTrigger<T>::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", boss_name);
    if (!boss) {
        return false;
    }
    if (this->phase_mask == 0) {
        return true;
    }
    T* boss_ai = dynamic_cast<T*>(boss->GetAI());
    EventMap* eventMap = &boss_ai->events;
    uint8 phase_mask = eventMap->GetPhaseMask();
    // bot->Yell("phase mask detected: " + to_string(phase_mask) + " compare with " + to_string(this->phase_mask), LANG_UNIVERSAL);
    return phase_mask == this->phase_mask;
}

bool GrobbulusCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss || boss->GetEntry() != boss_entry) {
        return false;
    }
    if (!botAI->IsMainTank(bot)) {
        return false;
    }
    // bot->Yell("has aggro on " + boss->GetName() + " : " + to_string(AI_VALUE2(bool, "has aggro", "boss target")), LANG_UNIVERSAL);
    return AI_VALUE2(bool, "has aggro", "boss target");
}

bool HeiganMeleeTrigger::IsActive()
{
    
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan) {
        return false;
    }
    return !botAI->IsRanged(bot);
}

bool HeiganRangedTrigger::IsActive()
{
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan) {
        return false;
    }
    return botAI->IsRanged(bot);
}

bool RazuviousTankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL) {
        return helper.UpdateBossAI() && botAI->IsTank(bot);
    }
    return helper.UpdateBossAI() && bot->getClass() == CLASS_PRIEST;
}

bool RazuviousNontankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL) {
        return helper.UpdateBossAI() && !(botAI->IsTank(bot));
    }
    return helper.UpdateBossAI() && !(bot->getClass() == CLASS_PRIEST);
}

// bool HorsemanAttractorsTrigger::IsActive()
// {
//     Difficulty diff = bot->GetRaidDifficulty();
//     if (diff == RAID_DIFFICULTY_25MAN_NORMAL) {
//         return BossPhaseTrigger::IsActive() && (botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 0) || 
//            botAI->IsHealAssistantOfIndex(bot, 1) || botAI->IsHealAssistantOfIndex(bot, 2));
//     }
//     return BossPhaseTrigger::IsActive() && (botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 0));
// }

// bool HorsemanExceptAttractorsTrigger::IsActive()
// {
//     return BossPhaseTrigger::IsActive() && 
//         !(botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 0) || 
//           botAI->IsHealAssistantOfIndex(bot, 1) || botAI->IsHealAssistantOfIndex(bot, 2));
// }

bool SapphironGroundTrigger::IsActive()
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    return helper.IsPhaseGround();
}

bool SapphironFlightTrigger::IsActive()
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    return helper.IsPhaseFlight();
}

// bool SapphironGroundExceptMainTankTrigger::IsActive()
// {
//     return BossPhaseTrigger::IsActive() && !botAI->IsMainTank(bot);
// }

// bool SapphironFlightTrigger::IsActive()
// {
//     return BossPhaseTrigger::IsActive();
// }

// bool SapphironGroundChillTrigger::IsActive()
// {
//     return BossPhaseTrigger::IsActive() && !botAI->IsMainTank(bot) && botAI->HasAura("chill", bot);
// }

bool GluthTrigger::IsActive()
{
    return helper.UpdateBossAI();
}

bool GluthMainTankMortalWoundTrigger::IsActive()
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    if (!botAI->IsAssistTankOfIndex(bot, 0)) {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt) {
        return false;
    }
    Aura* aura = botAI->GetAura("mortal wound", mt, false, true);
    if (!aura || aura->GetStackAmount() < 5) {
        return false;
    }
    return true;
}

bool KelthuzadTrigger::IsActive() 
{
    return helper.UpdateBossAI();
}

template bool BossEventTrigger<boss_grobbulus::boss_grobbulusAI>::IsActive();
template bool BossPhaseTrigger<boss_anubrekhan::boss_anubrekhanAI>::IsActive();