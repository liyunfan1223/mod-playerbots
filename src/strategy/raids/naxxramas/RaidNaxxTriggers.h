
#ifndef _PLAYERBOT_RAIDNAXXTRIGGERS_H
#define _PLAYERBOT_RAIDNAXXTRIGGERS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "RaidNaxxBossHelper.h"
#include "RaidNaxxScripts.h"
#include "Trigger.h"

class MutatingInjectionTrigger : public HasAuraTrigger
{
public:
    MutatingInjectionTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "mutating injection", 1) {}
};

class AuraRemovedTrigger : public Trigger
{
public:
    AuraRemovedTrigger(PlayerbotAI* botAI, std::string name) : Trigger(botAI, name, 1) { this->prev_check = false; }
    virtual bool IsActive() override;

protected:
    bool prev_check;
};

class MutatingInjectionRemovedTrigger : public HasNoAuraTrigger
{
public:
    MutatingInjectionRemovedTrigger(PlayerbotAI* ai) : HasNoAuraTrigger(ai, "mutating injection") {}
    virtual bool IsActive();
};

template <class T>
class BossEventTrigger : public Trigger
{
public:
    BossEventTrigger(PlayerbotAI* ai, uint32 event_id, uint32 boss_entry, uint32 boss_entry_secondary = 0, std::string name = "boss event")
        : Trigger(ai, name, 1)
    {
        this->event_id = event_id;
        this->boss_entry = boss_entry;
        this->boss_entry_secondary = boss_entry_secondary;
        this->last_event_time = -1;
    }
    virtual bool IsActive();

protected:
    uint32 event_id, boss_entry, boss_entry_secondary, last_event_time;
};

class GrobbulusCloudTrigger : public BossEventTrigger<Grobbulus::boss_grobbulus::boss_grobbulusAI>
{
public:
    GrobbulusCloudTrigger(PlayerbotAI* ai) : BossEventTrigger(ai, 2, 15931, 351003, "grobbulus cloud event") {}
    virtual bool IsActive();
};

class HeiganMeleeTrigger : public Trigger
{
public:
    HeiganMeleeTrigger(PlayerbotAI* ai) : Trigger(ai, "heigan melee") {}
    virtual bool IsActive();
};

class HeiganRangedTrigger : public Trigger
{
public:
    HeiganRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "heigan ranged") {}
    bool IsActive() override;
};

class RazuviousTankTrigger : public Trigger
{
public:
    RazuviousTankTrigger(PlayerbotAI* ai) : Trigger(ai, "instructor razuvious tank"), helper(ai) {}
    bool IsActive() override;

private:
    RazuviousBossHelper helper;
};

class RazuviousNontankTrigger : public Trigger
{
public:
    RazuviousNontankTrigger(PlayerbotAI* ai) : Trigger(ai, "instructor razuvious non-tank"), helper(ai) {}
    bool IsActive() override;

private:
    RazuviousBossHelper helper;
};

class KelthuzadTrigger : public Trigger
{
public:
    KelthuzadTrigger(PlayerbotAI* ai) : Trigger(ai, "kel'thuzad trigger"), helper(ai) {}
    bool IsActive() override;

private:
    KelthuzadBossHelper helper;
};

class AnubrekhanTrigger : public Trigger
{
public:
    AnubrekhanTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'rekhan") {}
    bool IsActive() override;
};

class ThaddiusPhasePetTrigger : public Trigger
{
public:
    ThaddiusPhasePetTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase pet"), helper(ai) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class ThaddiusPhasePetLoseAggroTrigger : public ThaddiusPhasePetTrigger
{
public:
    ThaddiusPhasePetLoseAggroTrigger(PlayerbotAI* ai) : ThaddiusPhasePetTrigger(ai) {}
    virtual bool IsActive()
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        return ThaddiusPhasePetTrigger::IsActive() && botAI->IsTank(bot) && target && target->GetVictim() != bot;
    }
};

class ThaddiusPhaseTransitionTrigger : public Trigger
{
public:
    ThaddiusPhaseTransitionTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase transition"), helper(ai) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class ThaddiusPhaseThaddiusTrigger : public Trigger
{
public:
    ThaddiusPhaseThaddiusTrigger(PlayerbotAI* ai) : Trigger(ai, "thaddius phase thaddius"), helper(ai) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class HorsemanAttractorsTrigger : public Trigger
{
public:
    HorsemanAttractorsTrigger(PlayerbotAI* ai) : Trigger(ai, "fourhorsemen attractors"), helper(ai) {}
    bool IsActive() override;

private:
    FourhorsemanBossHelper helper;
};

class HorsemanExceptAttractorsTrigger : public Trigger
{
public:
    HorsemanExceptAttractorsTrigger(PlayerbotAI* ai) : Trigger(ai, "fourhorsemen except attractors"), helper(ai) {}
    bool IsActive() override;

private:
    FourhorsemanBossHelper helper;
};

class SapphironGroundTrigger : public Trigger
{
public:
    SapphironGroundTrigger(PlayerbotAI* ai) : Trigger(ai, "sapphiron ground"), helper(ai) {}
    bool IsActive() override;

private:
    SapphironBossHelper helper;
};


class SapphironFlightTrigger : public Trigger
{
public:
    SapphironFlightTrigger(PlayerbotAI* ai) : Trigger(ai, "sapphiron flight"), helper(ai) {}
    bool IsActive() override;

private:
    SapphironBossHelper helper;
};

class GluthTrigger : public Trigger
{
public:
    GluthTrigger(PlayerbotAI* ai) : Trigger(ai, "gluth trigger"), helper(ai) {}
    bool IsActive() override;

private:
    GluthBossHelper helper;
};

class GluthMainTankMortalWoundTrigger : public Trigger
{
public:
    GluthMainTankMortalWoundTrigger(PlayerbotAI* ai) : Trigger(ai, "gluth main tank mortal wound trigger"), helper(ai)
    {
    }
    bool IsActive() override;

private:
    GluthBossHelper helper;
};

class LoathebTrigger : public Trigger
{
public:
    LoathebTrigger(PlayerbotAI* ai) : Trigger(ai, "loatheb"), helper(ai) {}
    bool IsActive() override;

private:
    LoathebBossHelper helper;
};

#endif