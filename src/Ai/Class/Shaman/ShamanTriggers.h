/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHAMANTRIGGERS_H
#define PLAYERBOTS_SHAMANTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"
#include "TotemsShamanStrategy.h"
#include "Trigger.h"
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
    MainHandWeaponNoImbueTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "main hand", 1) {}
    virtual bool IsActive();
};

class OffHandWeaponNoImbueTrigger : public BuffTrigger
{
public:
    OffHandWeaponNoImbueTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "off hand", 1) {}
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
    WaterBreathingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water breathing", 10 * IN_MILLISECONDS) {}

    bool IsActive() override;
};

class WaterWalkingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterWalkingOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "water walking on party", 4 * IN_MILLISECONDS) {}

    bool IsActive() override;
};

class WaterBreathingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterBreathingOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "water breathing on party", 4 * IN_MILLISECONDS) {}

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
    SpiritWalkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "spirit walk ready") {}

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
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_POISON) {}
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
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_CURSE) {}
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
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_DISEASE) {}
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
    FlameShockTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "flame shock", 1, true, 6.0f) {}
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
    MaelstromWeapon5AndMediumAoeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "maelstrom weapon 5", "medium aoe") {}
};

class MaelstromWeapon4AndMediumAoeTrigger : public TwoTriggers
{
public:
    MaelstromWeapon4AndMediumAoeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "maelstrom weapon 4", "medium aoe") {}
};

class ChainLightningNoCdTrigger : public SpellNoCooldownTrigger
{
public:
    ChainLightningNoCdTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "chain lightning") {}
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
    CallOfTheElementsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "call of the elements") {}
    bool IsActive() override;
};

class TotemicRecallTrigger : public Trigger
{
public:
    TotemicRecallTrigger(PlayerbotAI* botAI) : Trigger(botAI, "totemic recall") {}
    bool IsActive() override;
};

class NoEarthTotemTrigger : public Trigger
{
public:
    NoEarthTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no earth totem") {}
    bool IsActive() override;
};

class NoFireTotemTrigger : public Trigger
{
public:
    NoFireTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no fire totem") {}
    bool IsActive() override;
};

class NoWaterTotemTrigger : public Trigger
{
public:
    NoWaterTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no water totem") {}
    bool IsActive() override;
};

class NoAirTotemTrigger : public Trigger
{
public:
    NoAirTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no air totem") {}
    bool IsActive() override;
};

class CallOfTheElementsAndEnemyWithinMeleeTrigger : public TwoTriggers
{
public:
    CallOfTheElementsAndEnemyWithinMeleeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "call of the elements", "enemy within melee") {}
};

// Set Strategy Assigned Totems

class SetTotemTrigger : public Trigger
{
public:
    // Template constructor: infers N (size of the id array) at compile time
    template <size_t N>
    SetTotemTrigger(PlayerbotAI* botAI, std::string const& spellName, const uint32 (&ids)[N], int actionButtonId)
        : Trigger(botAI, "set " + spellName)
        , totemSpellIds(ids)
        , totemSpellIdsCount(N)
        , actionButtonId(actionButtonId)
    {}
    bool IsActive() override;

private:
    uint32 const* totemSpellIds;
    size_t totemSpellIdsCount;
    int actionButtonId;
};

class SetStrengthOfEarthTotemTrigger : public SetTotemTrigger
{
public:
    SetStrengthOfEarthTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "strength of earth totem", STRENGTH_OF_EARTH_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetStoneskinTotemTrigger : public SetTotemTrigger
{
public:
    SetStoneskinTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "stoneskin totem", STONESKIN_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetTremorTotemTrigger : public SetTotemTrigger
{
public:
    SetTremorTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "tremor totem", TREMOR_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetEarthbindTotemTrigger : public SetTotemTrigger
{
public:
    SetEarthbindTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "earthbind totem", EARTHBIND_TOTEM, TOTEM_BAR_SLOT_EARTH) {}
};

class SetSearingTotemTrigger : public SetTotemTrigger
{
public:
    SetSearingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "searing totem", SEARING_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetMagmaTotemTrigger : public SetTotemTrigger
{
public:
    SetMagmaTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "magma totem", MAGMA_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetFlametongueTotemTrigger : public SetTotemTrigger
{
public:
    SetFlametongueTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "flametongue totem", FLAMETONGUE_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetTotemOfWrathTrigger : public SetTotemTrigger
{
public:
    SetTotemOfWrathTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "totem of wrath", TOTEM_OF_WRATH, TOTEM_BAR_SLOT_FIRE) {}
};

class SetFrostResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFrostResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "frost resistance totem", FROST_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_FIRE) {}
};

class SetHealingStreamTotemTrigger : public SetTotemTrigger
{
public:
    SetHealingStreamTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "healing stream totem", HEALING_STREAM_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetManaSpringTotemTrigger : public SetTotemTrigger
{
public:
    SetManaSpringTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "mana spring totem", MANA_SPRING_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetCleansingTotemTrigger : public SetTotemTrigger
{
public:
    SetCleansingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "cleansing totem", CLEANSING_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetFireResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFireResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "fire resistance totem", FIRE_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_WATER) {}
};

class SetWrathOfAirTotemTrigger : public SetTotemTrigger
{
public:
    SetWrathOfAirTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "wrath of air totem", WRATH_OF_AIR_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetWindfuryTotemTrigger : public SetTotemTrigger
{
public:
    SetWindfuryTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "windfury totem", WINDFURY_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetNatureResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetNatureResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "nature resistance totem", NATURE_RESISTANCE_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

class SetGroundingTotemTrigger : public SetTotemTrigger
{
public:
    SetGroundingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "grounding totem", GROUNDING_TOTEM, TOTEM_BAR_SLOT_AIR) {}
};

#endif
