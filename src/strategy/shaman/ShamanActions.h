/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAMANACTIONS_H
#define _PLAYERBOT_SHAMANACTIONS_H

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "TotemsShamanStrategy.h"

class PlayerbotAI;

// Buff and Out of Combat Actions

class CastWaterShieldAction : public CastBuffSpellAction
{
public:
    CastWaterShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water shield") {}
};

class CastLightningShieldAction : public CastBuffSpellAction
{
public:
    CastLightningShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "lightning shield") {}
};

class CastEarthlivingWeaponAction : public CastEnchantItemAction
{
public:
    CastEarthlivingWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "earthliving weapon") {}
};

class CastRockbiterWeaponAction : public CastEnchantItemAction
{
public:
    CastRockbiterWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "rockbiter weapon") {}
};

class CastFlametongueWeaponAction : public CastEnchantItemAction
{
public:
    CastFlametongueWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "flametongue weapon") {}
};

class CastFrostbrandWeaponAction : public CastEnchantItemAction
{
public:
    CastFrostbrandWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "frostbrand weapon") {}
};

class CastWindfuryWeaponAction : public CastEnchantItemAction
{
public:
    CastWindfuryWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "windfury weapon") {}
};

class CastAncestralSpiritAction : public ResurrectPartyMemberAction
{
public:
    CastAncestralSpiritAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "ancestral spirit") {}
};

class CastWaterBreathingAction : public CastBuffSpellAction
{
public:
    CastWaterBreathingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water breathing") {}
};

class CastWaterWalkingAction : public CastBuffSpellAction
{
public:
    CastWaterWalkingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water walking") {}
};

class CastWaterBreathingOnPartyAction : public BuffOnPartyAction
{
public:
    CastWaterBreathingOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "water breathing") {}
};

class CastWaterWalkingOnPartyAction : public BuffOnPartyAction
{
public:
    CastWaterWalkingOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "water walking") {}
};

// Boost Actions

class CastHeroismAction : public CastBuffSpellAction
{
public:
    CastHeroismAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "heroism") {}
};

class CastBloodlustAction : public CastBuffSpellAction
{
public:
    CastBloodlustAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bloodlust") {}
};

class CastElementalMasteryAction : public CastBuffSpellAction
{
public:
    CastElementalMasteryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "elemental mastery") {}
};

class CastShamanisticRageAction : public CastBuffSpellAction
{
public:
    CastShamanisticRageAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shamanistic rage") {}
};

class CastFeralSpiritAction : public CastSpellAction
{
public:
    CastFeralSpiritAction(PlayerbotAI* ai) : CastSpellAction(ai, "feral spirit") {}
};

class CastSpiritWalkAction : public Action
{
public:
    CastSpiritWalkAction(PlayerbotAI* botAI) : Action(botAI, "spirit walk") {}

    bool Execute(Event event) override;
};

// CC, Interrupt, and Dispel Actions

class CastWindShearAction : public CastSpellAction
{
public:
    CastWindShearAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "wind shear") {}
};

class CastWindShearOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastWindShearOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "wind shear") {}
};

class CastPurgeAction : public CastSpellAction
{
public:
    CastPurgeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "purge") {}
};

class CastCleanseSpiritAction : public CastCureSpellAction
{
public:
    CastCleanseSpiritAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse spirit") {}
};

class CastCleanseSpiritPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritPoisonOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_POISON)
    {
    }

    std::string const getName() override { return "cleanse spirit poison on party"; }
};

class CastCleanseSpiritCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritCurseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_CURSE)
    {
    }

    std::string const getName() override { return "cleanse spirit curse on party"; }
};

class CastCleanseSpiritDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritDiseaseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_DISEASE)
    {
    }

    std::string const getName() override { return "cleanse spirit disease on party"; }
};

class CastCurePoisonActionSham : public CastCureSpellAction
{
public:
    CastCurePoisonActionSham(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cure poison") {}
};

class CastCurePoisonOnPartyActionSham : public CurePartyMemberAction
{
public:
    CastCurePoisonOnPartyActionSham(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cure poison", DISPEL_POISON) {}

    std::string const getName() override { return "cure poison on party"; }
};

class CastCureDiseaseActionSham : public CastCureSpellAction
{
public:
    CastCureDiseaseActionSham(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cure disease") {}
};

class CastCureDiseaseOnPartyActionSham : public CurePartyMemberAction
{
public:
    CastCureDiseaseOnPartyActionSham(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cure disease", DISPEL_DISEASE)
    {
    }

    std::string const getName() override { return "cure disease on party"; }
};

// Damage and Debuff Actions

class CastFireNovaAction : public CastMeleeSpellAction
{
public:
    CastFireNovaAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "fire nova") {}
    bool isUseful() override;
};

class CastStormstrikeAction : public CastMeleeSpellAction
{
public:
    CastStormstrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "stormstrike") {}
};

class CastLavaLashAction : public CastMeleeSpellAction
{
public:
    CastLavaLashAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "lava lash") {}
};

class CastFlameShockAction : public CastDebuffSpellAction
{
public:
    CastFlameShockAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "flame shock", true, 6.0f) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastEarthShockAction : public CastSpellAction
{
public:
    CastEarthShockAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "earth shock") {}
};

class CastFrostShockAction : public CastSnareSpellAction
{
public:
    CastFrostShockAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "frost shock") {}
};

class CastChainLightningAction : public CastSpellAction
{
public:
    CastChainLightningAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chain lightning") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastLightningBoltAction : public CastSpellAction
{
public:
    CastLightningBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "lightning bolt") {}
};

class CastThunderstormAction : public CastSpellAction
{
public:
    CastThunderstormAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "thunderstorm") {}
};

class CastLavaBurstAction : public CastSpellAction
{
public:
    CastLavaBurstAction(PlayerbotAI* ai) : CastSpellAction(ai, "lava burst") {}
    bool isUseful() override;
};


// Healing Actions

class CastLesserHealingWaveAction : public CastHealingSpellAction
{
public:
    CastLesserHealingWaveAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lesser healing wave") {}
};

class CastLesserHealingWaveOnPartyAction : public HealPartyMemberAction
{
public:
    CastLesserHealingWaveOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "lesser healing wave", 25.0f, HealingManaEfficiency::LOW)
    {
    }
};

class CastHealingWaveAction : public CastHealingSpellAction
{
public:
    CastHealingWaveAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "healing wave") {}
};

class CastHealingWaveOnPartyAction : public HealPartyMemberAction
{
public:
    CastHealingWaveOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "healing wave", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastChainHealAction : public HealPartyMemberAction
{
public:
    CastChainHealAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "chain heal", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastRiptideAction : public CastHealingSpellAction
{
public:
    CastRiptideAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "riptide") {}
};

class CastRiptideOnPartyAction : public HealPartyMemberAction
{
public:
    CastRiptideOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "riptide", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastEarthShieldAction : public CastBuffSpellAction
{
public:
    CastEarthShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "earth shield") {}
};

class CastEarthShieldOnPartyAction : public BuffOnPartyAction
{
public:
    CastEarthShieldOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "earth shield") {}
};

class CastEarthShieldOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastEarthShieldOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "earth shield", false) {}
};

// Totem Spells

class CastTotemAction : public CastBuffSpellAction
{
public:
    CastTotemAction(PlayerbotAI* botAI, std::string const spell, std::string const buffName = "")
        : CastBuffSpellAction(botAI, spell)
    {
        buff = (buffName == "") ? spell : buffName;
    }

    bool isUseful() override;

protected:
    std::string buff;
};

class CastCallOfTheElementsAction : public CastSpellAction
{
public:
    CastCallOfTheElementsAction(PlayerbotAI* ai) : CastSpellAction(ai, "call of the elements") {}
};

class CastTotemicRecallAction : public CastSpellAction
{
public:
    CastTotemicRecallAction(PlayerbotAI* ai) : CastSpellAction(ai, "totemic recall") {}
};

class CastStrengthOfEarthTotemAction : public CastTotemAction
{
public:
    CastStrengthOfEarthTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "strength of earth totem", "strength of earth") {}
};

class CastStoneskinTotemAction : public CastTotemAction
{
public:
    CastStoneskinTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "stoneskin totem", "stoneskin") {}
};

class CastTremorTotemAction : public CastTotemAction
{
public:
    CastTremorTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "tremor totem", "") {}
};

class CastEarthbindTotemAction : public CastTotemAction
{
public:
    CastEarthbindTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "earthbind totem", "") {}
};

class CastStoneclawTotemAction : public CastTotemAction
{
public:
    CastStoneclawTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "stoneclaw totem", "") {}
    bool isUseful() override;
};

class CastSearingTotemAction : public CastTotemAction
{
public:
    CastSearingTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "searing totem", "") {}
};

class CastMagmaTotemAction : public CastTotemAction
{
public:
    CastMagmaTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "magma totem", "") {}
    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

class CastFlametongueTotemAction : public CastTotemAction
{
public:
    CastFlametongueTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "flametongue totem", "flametongue totem") {}
};

class CastTotemOfWrathAction : public CastTotemAction
{
public:
    CastTotemOfWrathAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "totem of wrath", "totem of wrath") {}
};

class CastFrostResistanceTotemAction : public CastTotemAction
{
public:
    CastFrostResistanceTotemAction(PlayerbotAI* botAI)
        : CastTotemAction(botAI, "frost resistance totem", "frost resistance") {}
};

class CastFireElementalTotemAction : public CastTotemAction
{
public:
    CastFireElementalTotemAction(PlayerbotAI* ai) : CastTotemAction(ai, "fire elemental totem", "") {}
    virtual std::string const GetTargetName() override { return "self target"; }
    virtual bool isUseful() override { return CastTotemAction::isUseful(); }
};

class CastFireElementalTotemMeleeAction : public CastTotemAction
{
public:
    CastFireElementalTotemMeleeAction(PlayerbotAI* ai) : CastTotemAction(ai, "fire elemental totem", "") {}
    virtual std::string const GetTargetName() override { return "self target"; }
    virtual bool isUseful() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target || !bot->IsWithinMeleeRange(target))
            return false;
        return CastTotemAction::isUseful();
    }
};

class CastHealingStreamTotemAction : public CastTotemAction
{
public:
    CastHealingStreamTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "healing stream totem", "") {}
};

class CastManaSpringTotemAction : public CastTotemAction
{
public:
    CastManaSpringTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "mana spring totem", "mana spring") {}
};

class CastCleansingTotemAction : public CastTotemAction
{
public:
    CastCleansingTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "cleansing totem", "") {}
    virtual bool isUseful();
};

class CastManaTideTotemAction : public CastTotemAction
{
public:
    CastManaTideTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "mana tide totem", "") {}
    std::string const GetTargetName() override { return "self target"; }
};

class CastFireResistanceTotemAction : public CastTotemAction
{
public:
    CastFireResistanceTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "fire resistance totem", "fire resistance") {}
};

class CastWrathOfAirTotemAction : public CastTotemAction
{
public:
    CastWrathOfAirTotemAction(PlayerbotAI* ai) : CastTotemAction(ai, "wrath of air totem", "wrath of air totem") {}
};

class CastWindfuryTotemAction : public CastTotemAction
{
public:
    CastWindfuryTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "windfury totem", "windfury totem") {}
};

class CastNatureResistanceTotemAction : public CastTotemAction
{
public:
    CastNatureResistanceTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "nature resistance totem", "nature resistance") {}
};

// Set Strategy Assigned Totems

class SetStrengthOfEarthTotemAction : public Action
{
public:
    SetStrengthOfEarthTotemAction(PlayerbotAI* ai) : Action(ai, "set strength of earth totem") {}
    bool Execute(Event event) override;
};

class SetStoneskinTotemAction : public Action
{
public:
    SetStoneskinTotemAction(PlayerbotAI* ai) : Action(ai, "set stoneskin totem") {}
    bool Execute(Event event) override;
};

class SetTremorTotemAction : public Action
{
public:
    SetTremorTotemAction(PlayerbotAI* ai) : Action(ai, "set tremor totem") {}
    bool Execute(Event event) override;
};

class SetEarthbindTotemAction : public Action
{
public:
    SetEarthbindTotemAction(PlayerbotAI* ai) : Action(ai, "set earthbind totem") {}
    bool Execute(Event event) override;
};

class SetSearingTotemAction : public Action
{
public:
    SetSearingTotemAction(PlayerbotAI* ai) : Action(ai, "set searing totem") {}
    bool Execute(Event event) override;
};

class SetMagmaTotemAction : public Action
{
public:
    SetMagmaTotemAction(PlayerbotAI* ai) : Action(ai, "set magma totem") {}
    bool Execute(Event event) override;
};

class SetFlametongueTotemAction : public Action
{
public:
    SetFlametongueTotemAction(PlayerbotAI* ai) : Action(ai, "set flametongue totem") {}
    bool Execute(Event event) override;
};

class SetTotemOfWrathAction : public Action
{
public:
    SetTotemOfWrathAction(PlayerbotAI* ai) : Action(ai, "set totem of wrath") {}
    bool Execute(Event event) override;
};

class SetFrostResistanceTotemAction : public Action
{
public:
    SetFrostResistanceTotemAction(PlayerbotAI* ai) : Action(ai, "set frost resistance totem") {}
    bool Execute(Event event) override;
};

class SetHealingStreamTotemAction : public Action
{
public:
    SetHealingStreamTotemAction(PlayerbotAI* ai) : Action(ai, "set healing stream totem") {}
    bool Execute(Event event) override;
};

class SetManaSpringTotemAction : public Action
{
public:
    SetManaSpringTotemAction(PlayerbotAI* ai) : Action(ai, "set mana spring totem") {}
    bool Execute(Event event) override;
};

class SetCleansingTotemAction : public Action
{
public:
    SetCleansingTotemAction(PlayerbotAI* ai) : Action(ai, "set cleansing totem") {}
    bool Execute(Event event) override;
};

class SetFireResistanceTotemAction : public Action
{
public:
    SetFireResistanceTotemAction(PlayerbotAI* ai) : Action(ai, "set fire resistance totem") {}
    bool Execute(Event event) override;
};

class SetWrathOfAirTotemAction : public Action
{
public:
    SetWrathOfAirTotemAction(PlayerbotAI* ai) : Action(ai, "set wrath of air totem") {}
    bool Execute(Event event) override;
};

class SetWindfuryTotemAction : public Action
{
public:
    SetWindfuryTotemAction(PlayerbotAI* ai) : Action(ai, "set windfury totem") {}
    bool Execute(Event event) override;
};

class SetNatureResistanceTotemAction : public Action
{
public:
    SetNatureResistanceTotemAction(PlayerbotAI* ai) : Action(ai, "set nature resistance totem") {}
    bool Execute(Event event) override;
};

class SetGroundingTotemAction : public Action
{
public:
    SetGroundingTotemAction(PlayerbotAI* ai) : Action(ai, "set grounding totem") {}
    bool Execute(Event event) override;
};

#endif
