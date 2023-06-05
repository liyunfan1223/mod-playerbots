#include "Playerbots.h"
#include "RaidNaxxTrigger.h"
#include "ScriptedCreature.h"

// bool MutatingInjectionRemovedTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
//     if (!boss) {
//         return false;
//     }
//     return HasNotAuraTrigger::IsActive() && botAI->GetCurrentState() == BOT_STATE_COMBAT && botAI->IsRanged(bot);
// }

// bool BossEventTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE(Unit*, "boss target");
//     if (!boss || boss->GetEntry() != boss_entry) {
//         return false;
//     }
//     BossAI* boss_ai = dynamic_cast<BossAI*>(boss->GetAI());
//     EventMap* eventMap = boss_botAI->GetEvents();
//     const uint32 event_time = eventMap->GetNextEventTime(event_id);
//     if (event_time != last_event_time) {
//         last_event_time = event_time;
//         return true;
//     }
//     return false;
// }

// bool BossPhaseTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", boss_name);
//     if (!boss) {
//         return false;
//     }
//     if (this->phase_mask == 0) {
//         return true;
//     }
//     BossAI* boss_ai = dynamic_cast<BossAI*>(boss->GetAI());
//     EventMap* eventMap = boss_botAI->GetEvents();
//     uint8 phase_mask = eventMap->GetPhaseMask();
//     // bot->Yell("phase mask detected: " + to_string(phase_mask) + " compare with " + to_string(this->phase_mask), LANG_UNIVERSAL);
//     return phase_mask == this->phase_mask;
// }

// bool GrobbulusCloudTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE(Unit*, "boss target");
//     if (!boss || boss->GetEntry() != boss_entry) {
//         return false;
//     }
//     if (!botAI->IsMainTank(bot)) {
//         return false;
//     }
//     // bot->Yell("has aggro on " + boss->GetName() + " : " + to_string(AI_VALUE2(bool, "has aggro", "boss target")), LANG_UNIVERSAL);
//     return AI_VALUE2(bool, "has aggro", "boss target");
// }

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

// bool RazuviousTankTrigger::IsActive()
// {
//     Difficulty diff = bot->GetRaidDifficulty();
//     if (diff == RAID_DIFFICULTY_10MAN_NORMAL) {
//         return BossPhaseTrigger::IsActive() && botAI->IsTank(bot);
//     }
//     return BossPhaseTrigger::IsActive() && bot->getClass() == CLASS_PRIEST;
// }

// bool RazuviousNontankTrigger::IsActive()
// {
//     Difficulty diff = bot->GetRaidDifficulty();
//     if (diff == RAID_DIFFICULTY_10MAN_NORMAL) {
//         return BossPhaseTrigger::IsActive() && !(botAI->IsTank(bot));
//     }
//     return BossPhaseTrigger::IsActive() && !(bot->getClass() == CLASS_PRIEST);
// }

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

// bool SapphironGroundMainTankTrigger::IsActive()
// {
//     return BossPhaseTrigger::IsActive() && botAI->IsMainTank(bot) && AI_VALUE2(bool, "has aggro", "boss target");
// }

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

// bool GluthMainTankMortalWoundTrigger::IsActive()
// {
//     if (!BossPhaseTrigger::IsActive()) {
//         return false;
//     }
//     if (!botAI->IsAssistTankOfIndex(bot, 0)) {
//         return false;
//     }
//     Unit* mt = AI_VALUE(Unit*, "main tank");
//     if (!mt) {
//         return false;
//     }
//     Aura* aura = botAI->GetAuraWithDuration("mortal wound", mt);
//     if (!aura || aura->GetStackAmount() < 5) {
//         return false;
//     }
//     // bot->Yell("Time to taunt!", LANG_UNIVERSAL);
//     return true;
// }