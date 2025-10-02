/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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

const uint32 SPELL_EARTHBIND_TOTEM_RANK_1 = 2484;
const uint32 SPELL_SEARING_TOTEM_RANK_1 = 3599;
const uint32 SPELL_WRATH_OF_AIR_TOTEM_RANK_1 = 3738;
const uint32 SPELL_HEALING_STREAM_TOTEM_RANK_1 = 5394;
const uint32 SPELL_MANA_SPRING_TOTEM_RANK_1 = 5675;
const uint32 SPELL_STONESKIN_TOTEM_RANK_1 = 8071;
const uint32 SPELL_STRENGTH_OF_EARTH_TOTEM_RANK_1 = 8075;
const uint32 SPELL_TREMOR_TOTEM_RANK_1 = 8143;
const uint32 SPELL_CLEANSING_TOTEM_RANK_1 = 8170;
const uint32 SPELL_GROUNDING_TOTEM_RANK_1 = 8177;
const uint32 SPELL_FROST_RESISTANCE_TOTEM_RANK_1 = 8181;
const uint32 SPELL_FIRE_RESISTANCE_TOTEM_RANK_1 = 8184;
const uint32 SPELL_MAGMA_TOTEM_RANK_1 = 8190;
const uint32 SPELL_FLAMETONGUE_TOTEM_RANK_1 = 8227;
const uint32 SPELL_WINDFURY_TOTEM_RANK_1 = 8512;
const uint32 SPELL_NATURE_RESISTANCE_TOTEM_RANK_1 = 10595;
const uint32 SPELL_TOTEM_OF_WRATH_RANK_1 = 30706;
const uint32 SPELL_TOTEMIC_RECALL = 36936;
const uint32 SPELL_CALL_OF_THE_ELEMENTS = 66842;

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

class SetTotemTrigger : public Trigger
{
public:
    // Template constructor: infers N (size of the id array) at compile time
    template <size_t N>
    SetTotemTrigger(PlayerbotAI* ai, std::string const& spellName, uint32 requiredSpellId,
                    const uint32 (&ids)[N], int actionButtonId)
        : Trigger(ai, "set " + spellName)
        , requiredSpellId(requiredSpellId)
        , totemSpellIds(ids)
        , totemSpellIdsCount(N)
        , actionButtonId(actionButtonId)
    {}
    bool IsActive() override;

private:
    uint32 requiredSpellId;
    uint32 const* totemSpellIds;
	size_t totemSpellIdsCount;
    int actionButtonId;
};

class SetStrengthOfEarthTotemTrigger : public SetTotemTrigger
{
public:
    SetStrengthOfEarthTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "strength of earth totem", SPELL_STRENGTH_OF_EARTH_TOTEM_RANK_1, STRENGTH_OF_EARTH_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetStoneskinTotemTrigger : public SetTotemTrigger
{
public:
    SetStoneskinTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "stoneskin totem", SPELL_STONESKIN_TOTEM_RANK_1, STONESKIN_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetTremorTotemTrigger : public SetTotemTrigger
{
public:
    SetTremorTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "tremor totem", SPELL_TREMOR_TOTEM_RANK_1, TREMOR_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetEarthbindTotemTrigger : public SetTotemTrigger
{
public:
    SetEarthbindTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "earthbind totem", SPELL_EARTHBIND_TOTEM_RANK_1, EARTHBIND_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetSearingTotemTrigger : public SetTotemTrigger
{
public:
    SetSearingTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "searing totem", SPELL_SEARING_TOTEM_RANK_1, SEARING_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetMagmaTotemTrigger : public SetTotemTrigger
{
public:
    SetMagmaTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "magma totem", SPELL_MAGMA_TOTEM_RANK_1, MAGMA_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetFlametongueTotemTrigger : public SetTotemTrigger
{
public:
    SetFlametongueTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "flametongue totem", SPELL_FLAMETONGUE_TOTEM_RANK_1, FLAMETONGUE_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetTotemOfWrathTrigger : public SetTotemTrigger
{
public:
    SetTotemOfWrathTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "totem of wrath", SPELL_TOTEM_OF_WRATH_RANK_1, TOTEM_OF_WRATH, TOTEM_BAR_SLOT_FIRE) {}
};

class SetFrostResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFrostResistanceTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "frost resistance totem", SPELL_FROST_RESISTANCE_TOTEM_RANK_1, FROST_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetHealingStreamTotemTrigger : public SetTotemTrigger
{
public:
    SetHealingStreamTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "healing stream totem", SPELL_HEALING_STREAM_TOTEM_RANK_1, HEALING_STREAM_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetManaSpringTotemTrigger : public SetTotemTrigger
{
public:
    SetManaSpringTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "mana spring totem", SPELL_MANA_SPRING_TOTEM_RANK_1, MANA_SPRING_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetCleansingTotemTrigger : public SetTotemTrigger
{
public:
    SetCleansingTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "cleansing totem", SPELL_CLEANSING_TOTEM_RANK_1, CLEANSING_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetFireResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFireResistanceTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "fire resistance totem", SPELL_FIRE_RESISTANCE_TOTEM_RANK_1, FIRE_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetWrathOfAirTotemTrigger : public SetTotemTrigger
{
public:
    SetWrathOfAirTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "wrath of air totem", SPELL_WRATH_OF_AIR_TOTEM_RANK_1, WRATH_OF_AIR_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetWindfuryTotemTrigger : public SetTotemTrigger
{
public:
    SetWindfuryTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "windfury totem", SPELL_WINDFURY_TOTEM_RANK_1, WINDFURY_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetNatureResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetNatureResistanceTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "nature resistance totem", SPELL_NATURE_RESISTANCE_TOTEM_RANK_1, NATURE_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetGroundingTotemTrigger : public SetTotemTrigger
{
public:
    SetGroundingTotemTrigger(PlayerbotAI* ai)
        : SetTotemTrigger(ai, "grounding totem", SPELL_GROUNDING_TOTEM_RANK_1, GROUNDING_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

#endif
