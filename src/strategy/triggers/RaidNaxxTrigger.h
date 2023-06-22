
#ifndef _PLAYERBOT_RAIDNAXXTRIGGER_H
#define _PLAYERBOT_RAIDNAXXTRIGGER_H

#include "EventMap.h"
#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "../../../../src/server/scripts/Northrend/Naxxramas/boss_grobbulus.h"
#include "../../../../src/server/scripts/Northrend/Naxxramas/boss_anubrekhan.h"

using namespace std;

class MutatingInjectionTrigger : public HasAuraTrigger
{
public:
    MutatingInjectionTrigger(PlayerbotAI* ai): HasAuraTrigger(ai, "mutating injection", 1) {}
};

class AuraRemovedTrigger : public Trigger
{
public:
    AuraRemovedTrigger(PlayerbotAI* botAI, string name): Trigger(botAI, name, 1) {
        this->prev_check = false;
    }
    virtual bool IsActive() override;
protected:
    bool prev_check;
};

class MutatingInjectionRemovedTrigger : public HasNoAuraTrigger
{
public:
    MutatingInjectionRemovedTrigger(PlayerbotAI* ai): HasNoAuraTrigger(ai, "mutating injection") {}
    virtual bool IsActive();
};

template<class T>
class BossEventTrigger : public Trigger
{
public:
    BossEventTrigger(PlayerbotAI* ai, uint32 boss_entry, uint32 event_id, string name = "boss event"): Trigger(ai, name, 1) {
        this->boss_entry = boss_entry;
        this->event_id = event_id;
        this->last_event_time = -1;
    }
    virtual bool IsActive();
protected:
    uint32 boss_entry, event_id, last_event_time;
};

template<class T>
class BossPhaseTrigger : public Trigger
{
public:
    BossPhaseTrigger(PlayerbotAI* ai, string boss_name, uint32 phase_mask, string name = "boss event"): Trigger(ai, name, 1) {
        this->boss_name = boss_name;
        this->phase_mask = phase_mask;
    }
    virtual bool IsActive();
protected:
    string boss_name;
    uint32 phase_mask;
};

class GrobbulusCloudTrigger : public BossEventTrigger<boss_grobbulus::boss_grobbulusAI>
{
public:
    GrobbulusCloudTrigger(PlayerbotAI* ai): BossEventTrigger(ai, 15931, 2, "grobbulus cloud event") { }
    virtual bool IsActive();
};

class HeiganMeleeTrigger : public Trigger
{
public:
    HeiganMeleeTrigger(PlayerbotAI* ai): Trigger(ai, "heigan melee") {}
    virtual bool IsActive();
};

class HeiganRangedTrigger : public Trigger
{
public:
    HeiganRangedTrigger(PlayerbotAI* ai): Trigger(ai, "heigan ranged") {}
    virtual bool IsActive();
};

// class ThaddiusPhasePetTrigger : public BossPhaseTrigger
// {
// public:
//     ThaddiusPhasePetTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "thaddius", 1 << (2 - 1), "thaddius phase pet") {}
// };

// class ThaddiusPhasePetLoseAggroTrigger : public ThaddiusPhasePetTrigger
// {
// public:
//     ThaddiusPhasePetLoseAggroTrigger(PlayerbotAI* ai) : ThaddiusPhasePetTrigger(ai) {}
//     virtual bool IsActive() {
//         Unit* target = AI_VALUE(Unit*, "current target");
//         return ThaddiusPhasePetTrigger::IsActive() && ai->IsTank(bot) && target && target->GetVictim() != bot;
//     }
// };

// class ThaddiusPhaseTransitionTrigger : public BossPhaseTrigger
// {
// public:
//     ThaddiusPhaseTransitionTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "thaddius", 1 << (3 - 1), "thaddius phase transition") {}
// };

// class ThaddiusPhaseThaddiusTrigger : public BossPhaseTrigger
// {
// public:
//     ThaddiusPhaseThaddiusTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "thaddius", 1 << (4 - 1), "thaddius phase thaddius") {}
// };

// class RazuviousTankTrigger : public BossPhaseTrigger
// {
// public:
//     RazuviousTankTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "instructor razuvious", 0, "razuvious tank") {}
//     virtual bool IsActive();
// };

// class RazuviousNontankTrigger : public BossPhaseTrigger
// {
// public:
//     RazuviousNontankTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "instructor razuvious", 0, "razuvious nontank") {}
//     virtual bool IsActive();
// };

// class HorsemanAttractorsTrigger : public BossPhaseTrigger
// {
// public:
//     HorsemanAttractorsTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sir zeliek", 0, "horseman attractors") {}
//     virtual bool IsActive();
// };

// class HorsemanExceptAttractorsTrigger : public BossPhaseTrigger
// {
// public:
//     HorsemanExceptAttractorsTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sir zeliek", 0, "horseman except attractors") {}
//     virtual bool IsActive();
// };

// class SapphironGroundMainTankTrigger : public BossPhaseTrigger
// {
// public:
//     SapphironGroundMainTankTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sapphiron", (1 << (2 - 1)), "sapphiron ground main tank") {}
//     virtual bool IsActive();
// };

// class SapphironGroundExceptMainTankTrigger : public BossPhaseTrigger
// {
// public:
//     SapphironGroundExceptMainTankTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sapphiron", (1 << (2 - 1)), "sapphiron ground except main tank") {}
//     virtual bool IsActive();
// };

// class SapphironFlightTrigger : public BossPhaseTrigger
// {
// public:
//     SapphironFlightTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sapphiron", (1 << (3 - 1)), "sapphiron flight") {}
//     virtual bool IsActive();
// };

// class SapphironGroundChillTrigger : public BossPhaseTrigger
// {
// public:
//     SapphironGroundChillTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "sapphiron", 0, "sapphiron chill") {}
//     virtual bool IsActive();
// };

// class KelthuzadTrigger : public BossPhaseTrigger
// {
// public:
//     KelthuzadTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "kel'thuzad", 0, "kel'thuzad trigger") {}
// };

class AnubrekhanTrigger : public BossPhaseTrigger<boss_anubrekhan::boss_anubrekhanAI>
{
public:
    AnubrekhanTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "anub'rekhan", 0, "anub'rekhan trigger") {}
};

// class KelthuzadPhaseTwoTrigger : public BossPhaseTrigger
// {
// public:
//     KelthuzadPhaseTwoTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "kel'thuzad", 1 << (2 - 1), "kel'thuzad trigger") {}
// };

// class GluthTrigger : public BossPhaseTrigger
// {
// public:
//     GluthTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "gluth", 0, "gluth trigger") {}
// };

// class GluthMainTankMortalWoundTrigger : public BossPhaseTrigger
// {
// public:
//     GluthMainTankMortalWoundTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "gluth", 0, "gluth main tank mortal wound trigger") {}
//     virtual bool IsActive();
// };

// class LoathebTrigger : public BossPhaseTrigger
// {
// public:
//     LoathebTrigger(PlayerbotAI* ai) : BossPhaseTrigger(ai, "loatheb", 0, "loatheb trigger") {}
// };
    

// template BossEventTrigger<class boss_grobbulus::boss_grobbulusAI>;
#endif