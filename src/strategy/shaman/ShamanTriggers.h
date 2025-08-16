/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAMANTRIGGERS_H
#define _PLAYERBOT_SHAMANTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include <set>
#include "TotemsShamanStrategy.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include <ctime>

class PlayerbotAI;

// Buff and Out of Combat Triggers

class MainHandWeaponNoImbueTrigger : public BuffTrigger
{
public:
    MainHandWeaponNoImbueTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "main hand", 1) {}
    virtual bool IsActive();
};

class OffHandWeaponNoImbueTrigger : public BuffTrigger
{
public:
    OffHandWeaponNoImbueTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "off hand", 1) {}
    virtual bool IsActive();
};

class WaterShieldTrigger : public BuffTrigger
{
public:
    WaterShieldTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water shield") {}
};

class LightningShieldTrigger : public BuffTrigger
{
public:
    LightningShieldTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "lightning shield") {}
};

class WaterWalkingTrigger : public BuffTrigger
{
public:
    WaterWalkingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water walking", 7) {}

    bool IsActive() override;
};

class WaterBreathingTrigger : public BuffTrigger
{
public:
    WaterBreathingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water breathing", 5 * 2000) {}

    bool IsActive() override;
};

class WaterWalkingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterWalkingOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "water walking on party", 2 * 2000) {}

    bool IsActive() override;
};

class WaterBreathingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterBreathingOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "water breathing on party", 2 * 2000) {}

    bool IsActive() override;
};

// Boost Triggers

class HeroismTrigger : public BoostTrigger
{
public:
    HeroismTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "heroism") {}
};

class BloodlustTrigger : public BoostTrigger
{
public:
    BloodlustTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "bloodlust") {}
};

class ElementalMasteryTrigger : public BuffTrigger
{
public:
    ElementalMasteryTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "elemental mastery") {}

    bool IsActive() override;
};

class SpiritWalkTrigger : public Trigger
{
public:
    SpiritWalkTrigger(PlayerbotAI* ai) : Trigger(ai, "spirit walk ready") {}

    bool IsActive() override;

private:
    time_t lastSpiritWalkTime = 0;
};

class FireElementalTotemTrigger : public BoostTrigger
{
public:
    FireElementalTotemTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "fire elemental totem") {}
};

    // CC, Interrupt, and Dispel Triggers

class WindShearInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    WindShearInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "wind shear") {}
};

class WindShearInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    WindShearInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "wind shear") {}
};

class PurgeTrigger : public TargetAuraDispelTrigger
{
public:
    PurgeTrigger(PlayerbotAI* botAI) : TargetAuraDispelTrigger(botAI, "purge", DISPEL_MAGIC) {}
};

class CleanseSpiritPoisonTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritPoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_POISON) {}
};

class PartyMemberCleanseSpiritPoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritPoisonTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_POISON)
    {
    }
};

class CleanseSpiritCurseTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritCurseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_CURSE) {}
};

class PartyMemberCleanseSpiritCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritCurseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_CURSE)
    {
    }
};

class CleanseSpiritDiseaseTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_DISEASE) {}
};

class PartyMemberCleanseSpiritDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritDiseaseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_DISEASE)
    {
    }
};

class CurePoisonTrigger : public NeedCureTrigger
{
public:
    CurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

class CureDiseaseTrigger : public NeedCureTrigger
{
public:
    CureDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure disease", DISPEL_DISEASE) {}
};

class PartyMemberCureDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCureDiseaseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cure disease", DISPEL_DISEASE) {}
};

// Damage and Debuff Triggers

class ShockTrigger : public DebuffTrigger
{
public:
    ShockTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "earth shock", 1, true) {}

    bool IsActive() override;
};

class FrostShockSnareTrigger : public SnareTargetTrigger
{
public:
    FrostShockSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "frost shock") {}
};

class MaelstromWeaponTrigger : public HasAuraStackTrigger
{
public:
    MaelstromWeaponTrigger(PlayerbotAI* botAI, int stack = 5) : HasAuraStackTrigger(botAI, "maelstrom weapon", stack) {}
};

class FlameShockTrigger : public DebuffTrigger
{
public:
    FlameShockTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "flame shock", 1, true, 6.0f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class EarthShockExecuteTrigger : public Trigger
{
public:
    EarthShockExecuteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "earth shock execute") {}

    bool IsActive() override;
};

class MaelstromWeapon5AndMediumAoeTrigger : public TwoTriggers
{
public:
    MaelstromWeapon5AndMediumAoeTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "maelstrom weapon 5", "medium aoe") {}
};

class MaelstromWeapon4AndMediumAoeTrigger : public TwoTriggers
{
public:
    MaelstromWeapon4AndMediumAoeTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "maelstrom weapon 4", "medium aoe") {}
};

class ChainLightningNoCdTrigger : public SpellNoCooldownTrigger
{
public:
    ChainLightningNoCdTrigger(PlayerbotAI* ai) : SpellNoCooldownTrigger(ai, "chain lightning") {}
};

// Healing Triggers

class EarthShieldOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    EarthShieldOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "earth shield", false) {}
};

// Totem Triggers

class TotemTrigger : public Trigger
{
public:
    TotemTrigger(PlayerbotAI* botAI, std::string const spell, uint32 attackerCount = 0)
        : Trigger(botAI, spell), attackerCount(attackerCount)
    {
    }

    bool IsActive() override;

protected:
    uint32 attackerCount;
};

class CallOfTheElementsTrigger : public Trigger
{
public:
    CallOfTheElementsTrigger(PlayerbotAI* ai) : Trigger(ai, "call of the elements") {}
    bool IsActive() override;
};

class TotemicRecallTrigger : public Trigger
{
public:
    TotemicRecallTrigger(PlayerbotAI* ai) : Trigger(ai, "totemic recall") {}
    bool IsActive() override;
};

class NoEarthTotemTrigger : public Trigger
{
public:
    NoEarthTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "no earth totem") {}
    bool IsActive() override;
};

class NoFireTotemTrigger : public Trigger
{
public:
    NoFireTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "no fire totem") {}
    bool IsActive() override;
};

class NoWaterTotemTrigger : public Trigger
{
public:
    NoWaterTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "no water totem") {}
    bool IsActive() override;
};

class NoAirTotemTrigger : public Trigger
{
public:
    NoAirTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "no air totem") {}
    bool IsActive() override;
};

class CallOfTheElementsAndEnemyWithinMeleeTrigger : public TwoTriggers
{
public:
    CallOfTheElementsAndEnemyWithinMeleeTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "call of the elements", "enemy within melee") {}
};

// Set Strategy Assigned Totems

class SetStrengthOfEarthTotemTrigger : public Trigger
{
public:
    SetStrengthOfEarthTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set strength of earth totem") {}
    bool IsActive() override;
};

class SetStoneskinTotemTrigger : public Trigger
{
public:
    SetStoneskinTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set stoneskin totem") {}
    bool IsActive() override;
};

class SetTremorTotemTrigger : public Trigger
{
public:
    SetTremorTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set tremor totem") {}
    bool IsActive() override;
};

class SetEarthbindTotemTrigger : public Trigger
{
public:
    SetEarthbindTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set earthbind totem") {}
    bool IsActive() override;
};

class SetSearingTotemTrigger : public Trigger
{
public:
    SetSearingTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set searing totem") {}
    bool IsActive() override;
};

class SetMagmaTotemTrigger : public Trigger
{
public:
    SetMagmaTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set magma totem") {}
    bool IsActive() override;
};

class SetFlametongueTotemTrigger : public Trigger
{
public:
    SetFlametongueTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set flametongue totem") {}
    bool IsActive() override;
};

class SetTotemOfWrathTrigger : public Trigger
{
public:
    SetTotemOfWrathTrigger(PlayerbotAI* ai) : Trigger(ai, "set totem of wrath") {}
    bool IsActive() override;
};

class SetFrostResistanceTotemTrigger : public Trigger
{
public:
    SetFrostResistanceTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set frost resistance totem") {}
    bool IsActive() override;
};

class SetHealingStreamTotemTrigger : public Trigger
{
public:
    SetHealingStreamTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set healing stream totem") {}
    bool IsActive() override;
};

class SetManaSpringTotemTrigger : public Trigger
{
public:
    SetManaSpringTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set mana spring totem") {}
    bool IsActive() override;
};

class SetCleansingTotemTrigger : public Trigger
{
public:
    SetCleansingTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set cleansing totem") {}
    bool IsActive() override;
};

class SetFireResistanceTotemTrigger : public Trigger
{
public:
    SetFireResistanceTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set fire resistance totem") {}
    bool IsActive() override;
};

class SetWrathOfAirTotemTrigger : public Trigger
{
public:
    SetWrathOfAirTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set wrath of air totem") {}
    bool IsActive() override;
};

class SetWindfuryTotemTrigger : public Trigger
{
public:
    SetWindfuryTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set windfury totem") {}
    bool IsActive() override;
};

class SetNatureResistanceTotemTrigger : public Trigger
{
public:
    SetNatureResistanceTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set nature resistance totem") {}
    bool IsActive() override;
};

class SetGroundingTotemTrigger : public Trigger
{
public:
    SetGroundingTotemTrigger(PlayerbotAI* ai) : Trigger(ai, "set grounding totem") {}
    bool IsActive() override;
};

#endif
