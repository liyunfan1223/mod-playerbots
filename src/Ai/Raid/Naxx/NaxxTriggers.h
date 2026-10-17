/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXXTRIGGERS_H
#define PLAYERBOTS_NAXXTRIGGERS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "NaxxBossHelper.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

class MutatingInjectionTrigger : public HasAuraTrigger
{
public:
    MutatingInjectionTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "mutating injection", 1) {}
};

class MutatingInjectionMeleeTrigger : public MutatingInjectionTrigger
{
public:
    MutatingInjectionMeleeTrigger(PlayerbotAI* botAI) : MutatingInjectionTrigger(botAI) {}
    bool IsActive() override;
};

class MutatingInjectionRangedTrigger : public MutatingInjectionTrigger
{
public:
    MutatingInjectionRangedTrigger(PlayerbotAI* botAI) : MutatingInjectionTrigger(botAI) {}
    bool IsActive() override;
};

class AuraRemovedTrigger : public Trigger
{
public:
    AuraRemovedTrigger(PlayerbotAI* botAI, std::string name) : Trigger(botAI, name, 1)
    {
        this->prev_check = false;
    }
    virtual bool IsActive() override;

protected:
    bool prev_check;
};

class MutatingInjectionRemovedTrigger : public HasNoAuraTrigger
{
public:
    MutatingInjectionRemovedTrigger(PlayerbotAI* botAI) : HasNoAuraTrigger(botAI, "mutating injection") {}
    virtual bool IsActive();
};

class GrobbulusCloudTrigger : public Trigger
{
public:
    GrobbulusCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "grobbulus cloud event"), last_cloud_ms(0) {}
    bool IsActive() override;

private:
    uint32 last_cloud_ms;
    static constexpr uint32 CloudRotationDelayMs = 15000;
};

class HeiganMeleeTrigger : public Trigger
{
public:
    explicit HeiganMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "heigan melee"), helper(botAI) {}
    bool IsActive() override;

private:
    HeiganBossHelper helper;
};

class HeiganRangedTrigger : public Trigger
{
public:
    explicit HeiganRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "heigan ranged"), helper(botAI) {}
    bool IsActive() override;

private:
    HeiganBossHelper helper;
};

class RazuviousTankTrigger : public Trigger
{
public:
    RazuviousTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "instructor razuvious tank"), helper(botAI) {}
    bool IsActive() override;

private:
    RazuviousBossHelper helper;
};

class RazuviousNontankTrigger : public Trigger
{
public:
    RazuviousNontankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "instructor razuvious non-tank"), helper(botAI) {}
    bool IsActive() override;

private:
    RazuviousBossHelper helper;
};

class KelthuzadTrigger : public Trigger
{
public:
    KelthuzadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kel'thuzad trigger"), helper(botAI) {}
    bool IsActive() override;

private:
    KelthuzadBossHelper helper;
};

class AnubrekhanTrigger : public Trigger
{
public:
    AnubrekhanTrigger(PlayerbotAI* botAI) : Trigger(botAI, "anub'rekhan") {}
    bool IsActive() override;
};

 class FaerlinaTrigger : public Trigger
 {
 public:
     FaerlinaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faerlina") {}
     bool IsActive() override;
 };

class MaexxnaTrigger : public Trigger
{
public:
    MaexxnaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maexxna") {}
    bool IsActive() override;
};

//class PatchwerkTankTrigger : public Trigger
//{
//public:
//    PatchwerkTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "patchwerk tank") {}
//    bool IsActive() override;
//};
//
//class PatchwerkNonTankTrigger : public Trigger
//{
//public:
//    PatchwerkNonTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "patchwerk non-tank") {}
//    bool IsActive() override;
//};
//
//class PatchwerkRangedTrigger : public Trigger
//{
//public:
//    PatchwerkRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "patchwerk ranged") {}
//    bool IsActive() override;
//};

class ThaddiusPhasePetTrigger : public Trigger
{
public:
    ThaddiusPhasePetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thaddius phase pet"), helper(botAI) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class ThaddiusPhasePetLoseAggroTrigger : public ThaddiusPhasePetTrigger
{
public:
    ThaddiusPhasePetLoseAggroTrigger(PlayerbotAI* botAI) : ThaddiusPhasePetTrigger(botAI) {}
    virtual bool IsActive()
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        return ThaddiusPhasePetTrigger::IsActive() && PlayerbotAI::IsTank(bot) && target && target->GetVictim() != bot;
    }
};

class ThaddiusPhaseTransitionTrigger : public Trigger
{
public:
    ThaddiusPhaseTransitionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thaddius phase transition"), helper(botAI) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class ThaddiusPhaseThaddiusTrigger : public Trigger
{
public:
    ThaddiusPhaseThaddiusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thaddius phase thaddius"), helper(botAI) {}
    bool IsActive() override;

private:
    ThaddiusBossHelper helper;
};

class FourHorsemenAttractorsTrigger : public Trigger
{
public:
    FourHorsemenAttractorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "four horsemen attractors"), helper(botAI) {}
    bool IsActive() override;

private:
    FourHorsemenBossHelper helper;
};

class FourHorsemenExceptAttractorsTrigger : public Trigger
{
public:
    FourHorsemenExceptAttractorsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "four horsemen except attractors"), helper(botAI) {}
    bool IsActive() override;

private:
    FourHorsemenBossHelper helper;
};

class SapphironGroundTrigger : public Trigger
{
public:
    SapphironGroundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sapphiron ground"), helper(botAI) {}
    bool IsActive() override;

private:
    SapphironBossHelper helper;
};

class SapphironFlightTrigger : public Trigger
{
public:
    SapphironFlightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sapphiron flight"), helper(botAI) {}
    bool IsActive() override;

private:
    SapphironBossHelper helper;
};

class GluthTrigger : public Trigger
{
public:
    GluthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gluth trigger"), helper(botAI) {}
    bool IsActive() override;

private:
    GluthBossHelper helper;
};

class GluthMainTankMortalWoundTrigger : public Trigger
{
public:
    GluthMainTankMortalWoundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gluth main tank mortal wound trigger"), helper(botAI) {}
    bool IsActive() override;

private:
    GluthBossHelper helper;
};

class LoathebTrigger : public Trigger
{
public:
    LoathebTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loatheb"), helper(botAI) {}
    bool IsActive() override;

private:
    LoathebBossHelper helper;
};

#endif
