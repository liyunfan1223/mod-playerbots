/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinAiObjectContext.h"

#include "DpsPaladinStrategy.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "HealPaladinStrategy.h"
#include "NamedObjectContext.h"
#include "PaladinActions.h"
#include "PaladinBuffStrategies.h"
#include "PaladinTriggers.h"
#include "Playerbots.h"
#include "TankPaladinStrategy.h"

class PaladinStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinStrategyFactoryInternal()
    {
        creators["nc"] = &PaladinStrategyFactoryInternal::nc;
        creators["cure"] = &PaladinStrategyFactoryInternal::cure;
        creators["boost"] = &PaladinStrategyFactoryInternal::boost;
        creators["cc"] = &PaladinStrategyFactoryInternal::cc;
        creators["bthreat"] = &PaladinStrategyFactoryInternal::bthreat;
        creators["healer dps"] = &PaladinStrategyFactoryInternal::healer_dps;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericPaladinNonCombatStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new PaladinCureStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new PaladinBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new PaladinCcStrategy(botAI); }
    static Strategy* bthreat(PlayerbotAI* botAI) { return new PaladinBuffThreatStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new PaladinHealerDpsStrategy(botAI); }
};

class PaladinResistanceStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinResistanceStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["rshadow"] = &PaladinResistanceStrategyFactoryInternal::rshadow;
        creators["rfrost"] = &PaladinResistanceStrategyFactoryInternal::rfrost;
        creators["rfire"] = &PaladinResistanceStrategyFactoryInternal::rfire;
        creators["baoe"] = &PaladinResistanceStrategyFactoryInternal::baoe;
        creators["barmor"] = &PaladinResistanceStrategyFactoryInternal::barmor;
        creators["bcast"] = &PaladinResistanceStrategyFactoryInternal::bcast;
    }

private:
    static Strategy* rshadow(PlayerbotAI* botAI) { return new PaladinShadowResistanceStrategy(botAI); }
    static Strategy* rfrost(PlayerbotAI* botAI) { return new PaladinFrostResistanceStrategy(botAI); }
    static Strategy* rfire(PlayerbotAI* botAI) { return new PaladinFireResistanceStrategy(botAI); }
    static Strategy* baoe(PlayerbotAI* botAI) { return new PaladinBuffAoeStrategy(botAI); }
    static Strategy* barmor(PlayerbotAI* botAI) { return new PaladinBuffArmorStrategy(botAI); }
    static Strategy* bcast(PlayerbotAI* botAI) { return new PaladinBuffCastStrategy(botAI); }
};

class PaladinBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bhealth"] = &PaladinBuffStrategyFactoryInternal::bhealth;
        creators["bmana"] = &PaladinBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &PaladinBuffStrategyFactoryInternal::bdps;
        creators["bstats"] = &PaladinBuffStrategyFactoryInternal::bstats;
    }

private:
    static Strategy* bhealth(PlayerbotAI* botAI) { return new PaladinBuffHealthStrategy(botAI); }
    static Strategy* bmana(PlayerbotAI* botAI) { return new PaladinBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new PaladinBuffDpsStrategy(botAI); }
    static Strategy* bstats(PlayerbotAI* botAI) { return new PaladinBuffStatsStrategy(botAI); }
};

class PaladinCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &PaladinCombatStrategyFactoryInternal::tank;
        creators["dps"] = &PaladinCombatStrategyFactoryInternal::dps;
        creators["heal"] = &PaladinCombatStrategyFactoryInternal::heal;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankPaladinStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsPaladinStrategy(botAI); }
    static Strategy* heal(PlayerbotAI* botAI) { return new HealPaladinStrategy(botAI); }
};

class PaladinTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    PaladinTriggerFactoryInternal()
    {
        creators["judgement"] = &PaladinTriggerFactoryInternal::judgement;
        creators["judgement of wisdom"] = &PaladinTriggerFactoryInternal::judgement_of_wisdom;
        creators["judgement of light"] = &PaladinTriggerFactoryInternal::judgement_of_light;
        creators["blessing"] = &PaladinTriggerFactoryInternal::blessing;
        creators["seal"] = &PaladinTriggerFactoryInternal::seal;
        creators["art of war"] = &PaladinTriggerFactoryInternal::art_of_war;
        creators["blessing on party"] = &PaladinTriggerFactoryInternal::blessing_on_party;
        creators["crusader aura"] = &PaladinTriggerFactoryInternal::crusader_aura;
        creators["retribution aura"] = &PaladinTriggerFactoryInternal::retribution_aura;
        creators["devotion aura"] = &PaladinTriggerFactoryInternal::devotion_aura;
        creators["sanctity aura"] = &PaladinTriggerFactoryInternal::sanctity_aura;
        creators["concentration aura"] = &PaladinTriggerFactoryInternal::concentration_aura;
        creators["shadow resistance aura"] = &PaladinTriggerFactoryInternal::shadow_resistance_aura;
        creators["frost resistance aura"] = &PaladinTriggerFactoryInternal::frost_resistance_aura;
        creators["fire resistance aura"] = &PaladinTriggerFactoryInternal::fire_resistance_aura;
        creators["hammer of justice snare"] = &PaladinTriggerFactoryInternal::hammer_of_justice_snare;
        creators["hammer of justice interrupt"] = &PaladinTriggerFactoryInternal::hammer_of_justice_interrupt;
        creators["cleanse cure disease"] = &PaladinTriggerFactoryInternal::CleanseCureDisease;
        creators["cleanse party member cure disease"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberDisease;
        creators["cleanse cure poison"] = &PaladinTriggerFactoryInternal::CleanseCurePoison;
        creators["cleanse party member cure poison"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberPoison;
        creators["cleanse cure magic"] = &PaladinTriggerFactoryInternal::CleanseCureMagic;
        creators["cleanse party member cure magic"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberMagic;
        creators["righteous fury"] = &PaladinTriggerFactoryInternal::righteous_fury;
        creators["holy shield"] = &PaladinTriggerFactoryInternal::holy_shield;
        creators["hammer of justice on enemy healer"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_enemy_target;
        creators["hammer of justice on snare target"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_snare_target;
        creators["divine favor"] = &PaladinTriggerFactoryInternal::divine_favor;
        creators["turn undead"] = &PaladinTriggerFactoryInternal::turn_undead;
        creators["avenger's shield"] = &PaladinTriggerFactoryInternal::avenger_shield;
        creators["consecration"] = &PaladinTriggerFactoryInternal::consecration;
        creators["repentance on enemy healer"] = &PaladinTriggerFactoryInternal::repentance_on_enemy_healer;
        creators["repentance on snare target"] = &PaladinTriggerFactoryInternal::repentance_on_snare_target;
        creators["repentance interrupt"] = &PaladinTriggerFactoryInternal::repentance_interrupt;
        creators["beacon of light on main tank"] = &PaladinTriggerFactoryInternal::beacon_of_light_on_main_tank;
        creators["sacred shield on main tank"] = &PaladinTriggerFactoryInternal::sacred_shield_on_main_tank;

        creators["blessing of kings on party"] = &PaladinTriggerFactoryInternal::blessing_of_kings_on_party;
        creators["blessing of wisdom on party"] = &PaladinTriggerFactoryInternal::blessing_of_wisdom_on_party;
        creators["blessing of might on party"] = &PaladinTriggerFactoryInternal::blessing_of_might_on_party;

        creators["avenging wrath"] = &PaladinTriggerFactoryInternal::avenging_wrath;
    }

private:
    static Trigger* turn_undead(PlayerbotAI* botAI) { return new TurnUndeadTrigger(botAI); }
    static Trigger* divine_favor(PlayerbotAI* botAI) { return new DivineFavorTrigger(botAI); }
    static Trigger* holy_shield(PlayerbotAI* botAI) { return new HolyShieldTrigger(botAI); }
    static Trigger* righteous_fury(PlayerbotAI* botAI) { return new RighteousFuryTrigger(botAI); }
    static Trigger* judgement(PlayerbotAI* botAI) { return new JudgementTrigger(botAI); }
    static Trigger* judgement_of_wisdom(PlayerbotAI* botAI) { return new JudgementOfWisdomTrigger(botAI); }
    static Trigger* judgement_of_light(PlayerbotAI* botAI) { return new JudgementOfLightTrigger(botAI); }
    static Trigger* blessing(PlayerbotAI* botAI) { return new BlessingTrigger(botAI); }
    static Trigger* seal(PlayerbotAI* botAI) { return new SealTrigger(botAI); }
    static Trigger* art_of_war(PlayerbotAI* botAI) { return new ArtOfWarTrigger(botAI); }
    static Trigger* blessing_on_party(PlayerbotAI* botAI) { return new BlessingOnPartyTrigger(botAI); }
    static Trigger* crusader_aura(PlayerbotAI* botAI) { return new CrusaderAuraTrigger(botAI); }
    static Trigger* retribution_aura(PlayerbotAI* botAI) { return new RetributionAuraTrigger(botAI); }
    static Trigger* devotion_aura(PlayerbotAI* botAI) { return new DevotionAuraTrigger(botAI); }
    static Trigger* sanctity_aura(PlayerbotAI* botAI) { return new SanctityAuraTrigger(botAI); }
    static Trigger* concentration_aura(PlayerbotAI* botAI) { return new ConcentrationAuraTrigger(botAI); }
    static Trigger* shadow_resistance_aura(PlayerbotAI* botAI) { return new ShadowResistanceAuraTrigger(botAI); }
    static Trigger* frost_resistance_aura(PlayerbotAI* botAI) { return new FrostResistanceAuraTrigger(botAI); }
    static Trigger* fire_resistance_aura(PlayerbotAI* botAI) { return new FireResistanceAuraTrigger(botAI); }
    static Trigger* hammer_of_justice_snare(PlayerbotAI* botAI) { return new HammerOfJusticeSnareTrigger(botAI); }
    static Trigger* hammer_of_justice_interrupt(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeInterruptSpellTrigger(botAI);
    }
    static Trigger* CleanseCureDisease(PlayerbotAI* botAI) { return new CleanseCureDiseaseTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberDisease(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberDiseaseTrigger(botAI);
    }
    static Trigger* CleanseCurePoison(PlayerbotAI* botAI) { return new CleanseCurePoisonTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberPoison(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberPoisonTrigger(botAI);
    }
    static Trigger* CleanseCureMagic(PlayerbotAI* botAI) { return new CleanseCureMagicTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberMagic(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberMagicTrigger(botAI);
    }
    static Trigger* hammer_of_justice_on_enemy_target(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeEnemyHealerTrigger(botAI);
    }
    static Trigger* hammer_of_justice_on_snare_target(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeSnareTrigger(botAI);
    }
    static Trigger* avenger_shield(PlayerbotAI* botAI) { return new AvengerShieldTrigger(botAI); }
    static Trigger* consecration(PlayerbotAI* botAI) { return new ConsecrationTrigger(botAI); }
    static Trigger* repentance_on_enemy_healer(PlayerbotAI* botAI) { return new RepentanceOnHealerTrigger(botAI); }
    static Trigger* repentance_on_snare_target(PlayerbotAI* botAI) { return new RepentanceSnareTrigger(botAI); }
    static Trigger* repentance_interrupt(PlayerbotAI* botAI) { return new RepentanceInterruptTrigger(botAI); }
    static Trigger* beacon_of_light_on_main_tank(PlayerbotAI* ai) { return new BeaconOfLightOnMainTankTrigger(ai); }
    static Trigger* sacred_shield_on_main_tank(PlayerbotAI* ai) { return new SacredShieldOnMainTankTrigger(ai); }

    static Trigger* blessing_of_kings_on_party(PlayerbotAI* botAI) { return new BlessingOfKingsOnPartyTrigger(botAI); }
    static Trigger* blessing_of_wisdom_on_party(PlayerbotAI* botAI)
    {
        return new BlessingOfWisdomOnPartyTrigger(botAI);
    }
    static Trigger* blessing_of_might_on_party(PlayerbotAI* botAI) { return new BlessingOfMightOnPartyTrigger(botAI); }

    static Trigger* avenging_wrath(PlayerbotAI* botAI) { return new AvengingWrathTrigger(botAI); }
};

class PaladinAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    PaladinAiObjectContextInternal()
    {
        creators["seal of command"] = &PaladinAiObjectContextInternal::seal_of_command;
        creators["seal of vengeance"] = &PaladinAiObjectContextInternal::seal_of_vengeance;
        creators["seal of corruption"] = &PaladinAiObjectContextInternal::seal_of_corruption;
        creators["blessing of might"] = &PaladinAiObjectContextInternal::blessing_of_might;
        creators["blessing of wisdom"] = &PaladinAiObjectContextInternal::blessing_of_wisdom;
        creators["blessing of kings"] = &PaladinAiObjectContextInternal::blessing_of_kings;
        creators["blessing of sanctuary"] = &PaladinAiObjectContextInternal::blessing_of_sanctuary;
        creators["divine storm"] = &PaladinAiObjectContextInternal::divine_storm;
        creators["blessing of kings on party"] = &PaladinAiObjectContextInternal::blessing_of_kings_on_party;
        creators["blessing of might on party"] = &PaladinAiObjectContextInternal::blessing_of_might_on_party;
        creators["blessing of wisdom on party"] = &PaladinAiObjectContextInternal::blessing_of_wisdom_on_party;
        creators["redemption"] = &PaladinAiObjectContextInternal::redemption;
        creators["crusader strike"] = &PaladinAiObjectContextInternal::crusader_strike;
        creators["crusader aura"] = &PaladinAiObjectContextInternal::crusader_aura;
        creators["seal of light"] = &PaladinAiObjectContextInternal::seal_of_light;
        creators["devotion aura"] = &PaladinAiObjectContextInternal::devotion_aura;
        creators["concentration aura"] = &PaladinAiObjectContextInternal::concentration_aura;
        creators["holy wrath"] = &PaladinAiObjectContextInternal::holy_wrath;
        creators["consecration"] = &PaladinAiObjectContextInternal::consecration;
        creators["cleanse disease"] = &PaladinAiObjectContextInternal::cleanse_disease;
        creators["cleanse poison"] = &PaladinAiObjectContextInternal::cleanse_poison;
        creators["cleanse magic"] = &PaladinAiObjectContextInternal::cleanse_magic;
        creators["purify disease"] = &PaladinAiObjectContextInternal::purify_disease;
        creators["purify poison"] = &PaladinAiObjectContextInternal::purify_poison;
        creators["cleanse poison on party"] = &PaladinAiObjectContextInternal::cleanse_poison_on_party;
        creators["cleanse disease on party"] = &PaladinAiObjectContextInternal::cleanse_disease_on_party;
        creators["cleanse magic on party"] = &PaladinAiObjectContextInternal::cleanse_magic_on_party;
        creators["purify poison on party"] = &PaladinAiObjectContextInternal::purify_poison_on_party;
        creators["purify disease on party"] = &PaladinAiObjectContextInternal::purify_disease_on_party;
        creators["seal of wisdom"] = &PaladinAiObjectContextInternal::seal_of_wisdom;
        creators["seal of justice"] = &PaladinAiObjectContextInternal::seal_of_justice;
        creators["seal of righteousness"] = &PaladinAiObjectContextInternal::seal_of_righteousness;
        creators["flash of light"] = &PaladinAiObjectContextInternal::flash_of_light;
        creators["hand of reckoning"] = &PaladinAiObjectContextInternal::hand_of_reckoning;
        creators["avenger's shield"] = &PaladinAiObjectContextInternal::avengers_shield;
        creators["exorcism"] = &PaladinAiObjectContextInternal::exorcism;
        creators["judgement"] = &PaladinAiObjectContextInternal::judgement;
        creators["judgement of light"] = &PaladinAiObjectContextInternal::judgement_of_light;
        creators["judgement of wisdom"] = &PaladinAiObjectContextInternal::judgement_of_wisdom;
        creators["divine shield"] = &PaladinAiObjectContextInternal::divine_shield;
        creators["divine protection"] = &PaladinAiObjectContextInternal::divine_protection;
        creators["divine protection on party"] = &PaladinAiObjectContextInternal::divine_protection_on_party;
        creators["hammer of justice"] = &PaladinAiObjectContextInternal::hammer_of_justice;
        creators["flash of light on party"] = &PaladinAiObjectContextInternal::flash_of_light_on_party;
        creators["holy light"] = &PaladinAiObjectContextInternal::holy_light;
        creators["holy light on party"] = &PaladinAiObjectContextInternal::holy_light_on_party;
        creators["lay on hands"] = &PaladinAiObjectContextInternal::lay_on_hands;
        creators["lay on hands on party"] = &PaladinAiObjectContextInternal::lay_on_hands_on_party;
        creators["judgement of justice"] = &PaladinAiObjectContextInternal::judgement_of_justice;
        creators["hammer of wrath"] = &PaladinAiObjectContextInternal::hammer_of_wrath;
        creators["holy shield"] = &PaladinAiObjectContextInternal::holy_shield;
        creators["hammer of the righteous"] = &PaladinAiObjectContextInternal::hammer_of_the_righteous;
        creators["retribution aura"] = &PaladinAiObjectContextInternal::retribution_aura;
        creators["shadow resistance aura"] = &PaladinAiObjectContextInternal::shadow_resistance_aura;
        creators["frost resistance aura"] = &PaladinAiObjectContextInternal::frost_resistance_aura;
        creators["fire resistance aura"] = &PaladinAiObjectContextInternal::fire_resistance_aura;
        creators["righteous fury"] = &PaladinAiObjectContextInternal::righteous_fury;
        creators["hammer of justice on enemy healer"] =
            &PaladinAiObjectContextInternal::hammer_of_justice_on_enemy_healer;
        creators["hammer of justice on snare target"] =
            &PaladinAiObjectContextInternal::hammer_of_justice_on_snare_target;
        creators["divine favor"] = &PaladinAiObjectContextInternal::divine_favor;
        creators["turn undead"] = &PaladinAiObjectContextInternal::turn_undead;
        creators["blessing of protection on party"] = &PaladinAiObjectContextInternal::blessing_of_protection_on_party;
        creators["righteous defense"] = &PaladinAiObjectContextInternal::righteous_defense;
        creators["repentance"] = &PaladinAiObjectContextInternal::repentance;
        creators["repentance on snare target"] = &PaladinAiObjectContextInternal::repentance_on_snare_target;
        creators["repentance on enemy healer"] = &PaladinAiObjectContextInternal::repentance_on_enemy_healer;
        creators["sanctity aura"] = &PaladinAiObjectContextInternal::sanctity_aura;
        creators["holy shock"] = &PaladinAiObjectContextInternal::holy_shock;
        creators["holy shock on party"] = &PaladinAiObjectContextInternal::holy_shock_on_party;
        creators["divine plea"] = &PaladinAiObjectContextInternal::divine_plea;
        creators["shield of righteousness"] = &PaladinAiObjectContextInternal::shield_of_righteousness;
        creators["beacon of light on main tank"] = &PaladinAiObjectContextInternal::beacon_of_light_on_main_tank;
        creators["sacred shield on main tank"] = &PaladinAiObjectContextInternal::sacred_shield_on_main_tank;
        creators["avenging wrath"] = &PaladinAiObjectContextInternal::avenging_wrath;
        creators["divine illumination"] = &PaladinAiObjectContextInternal::divine_illumination;
        creators["divine sacrifice"] = &PaladinAiObjectContextInternal::divine_sacrifice;
        creators["cancel divine sacrifice"] = &PaladinAiObjectContextInternal::cancel_divine_sacrifice;
    }

private:
    static Action* blessing_of_protection_on_party(PlayerbotAI* botAI)
    {
        return new CastBlessingOfProtectionProtectAction(botAI);
    }
    static Action* turn_undead(PlayerbotAI* botAI) { return new CastTurnUndeadAction(botAI); }
    static Action* divine_favor(PlayerbotAI* botAI) { return new CastDivineFavorAction(botAI); }
    static Action* righteous_fury(PlayerbotAI* botAI) { return new CastRighteousFuryAction(botAI); }
    static Action* seal_of_command(PlayerbotAI* botAI) { return new CastSealOfCommandAction(botAI); }
    static Action* seal_of_vengeance(PlayerbotAI* botAI) { return new CastSealOfVengeanceAction(botAI); }
    static Action* seal_of_corruption(PlayerbotAI* botAI) { return new CastSealOfCorruptionAction(botAI); }
    static Action* blessing_of_sanctuary(PlayerbotAI* botAI) { return new CastBlessingOfSanctuaryAction(botAI); }
    static Action* blessing_of_might(PlayerbotAI* botAI) { return new CastBlessingOfMightAction(botAI); }
    static Action* blessing_of_wisdom(PlayerbotAI* botAI) { return new CastBlessingOfWisdomAction(botAI); }
    static Action* blessing_of_kings(PlayerbotAI* botAI) { return new CastBlessingOfKingsAction(botAI); }
    static Action* divine_storm(PlayerbotAI* botAI) { return new CastDivineStormAction(botAI); }
    static Action* blessing_of_kings_on_party(PlayerbotAI* botAI)
    {
        return new CastBlessingOfKingsOnPartyAction(botAI);
    }
    static Action* blessing_of_might_on_party(PlayerbotAI* botAI)
    {
        return new CastBlessingOfMightOnPartyAction(botAI);
    }
    static Action* blessing_of_wisdom_on_party(PlayerbotAI* botAI)
    {
        return new CastBlessingOfWisdomOnPartyAction(botAI);
    }
    static Action* redemption(PlayerbotAI* botAI) { return new CastRedemptionAction(botAI); }
    static Action* crusader_strike(PlayerbotAI* botAI) { return new CastCrusaderStrikeAction(botAI); }
    static Action* crusader_aura(PlayerbotAI* botAI) { return new CastCrusaderAuraAction(botAI); }
    static Action* seal_of_light(PlayerbotAI* botAI) { return new CastSealOfLightAction(botAI); }
    static Action* devotion_aura(PlayerbotAI* botAI) { return new CastDevotionAuraAction(botAI); }
    static Action* concentration_aura(PlayerbotAI* botAI) { return new CastConcentrationAuraAction(botAI); }
    static Action* holy_wrath(PlayerbotAI* botAI) { return new CastHolyWrathAction(botAI); }
    static Action* consecration(PlayerbotAI* botAI) { return new CastConsecrationAction(botAI); }
    static Action* cleanse_poison(PlayerbotAI* botAI) { return new CastCleansePoisonAction(botAI); }
    static Action* cleanse_disease(PlayerbotAI* botAI) { return new CastCleanseDiseaseAction(botAI); }
    static Action* cleanse_magic(PlayerbotAI* botAI) { return new CastCleanseMagicAction(botAI); }
    static Action* purify_poison(PlayerbotAI* botAI) { return new CastPurifyPoisonAction(botAI); }
    static Action* purify_disease(PlayerbotAI* botAI) { return new CastPurifyDiseaseAction(botAI); }
    static Action* cleanse_poison_on_party(PlayerbotAI* botAI) { return new CastCleansePoisonOnPartyAction(botAI); }
    static Action* cleanse_disease_on_party(PlayerbotAI* botAI) { return new CastCleanseDiseaseOnPartyAction(botAI); }
    static Action* cleanse_magic_on_party(PlayerbotAI* botAI) { return new CastCleanseMagicOnPartyAction(botAI); }
    static Action* purify_poison_on_party(PlayerbotAI* botAI) { return new CastPurifyPoisonOnPartyAction(botAI); }
    static Action* purify_disease_on_party(PlayerbotAI* botAI) { return new CastPurifyDiseaseOnPartyAction(botAI); }
    static Action* seal_of_wisdom(PlayerbotAI* botAI) { return new CastSealOfWisdomAction(botAI); }
    static Action* seal_of_justice(PlayerbotAI* botAI) { return new CastSealOfJusticeAction(botAI); }
    static Action* seal_of_righteousness(PlayerbotAI* botAI) { return new CastSealOfRighteousnessAction(botAI); }
    static Action* flash_of_light(PlayerbotAI* botAI) { return new CastFlashOfLightAction(botAI); }
    static Action* hand_of_reckoning(PlayerbotAI* botAI) { return new CastHandOfReckoningAction(botAI); }
    static Action* avengers_shield(PlayerbotAI* botAI) { return new CastAvengersShieldAction(botAI); }
    static Action* exorcism(PlayerbotAI* botAI) { return new CastExorcismAction(botAI); }
    static Action* judgement(PlayerbotAI* botAI) { return new CastJudgementAction(botAI); }
    static Action* judgement_of_light(PlayerbotAI* botAI) { return new CastJudgementOfLightAction(botAI); }
    static Action* judgement_of_wisdom(PlayerbotAI* botAI) { return new CastJudgementOfWisdomAction(botAI); }
    static Action* divine_shield(PlayerbotAI* botAI) { return new CastDivineShieldAction(botAI); }
    static Action* divine_protection(PlayerbotAI* botAI) { return new CastDivineProtectionAction(botAI); }
    static Action* divine_protection_on_party(PlayerbotAI* botAI)
    {
        return new CastDivineProtectionOnPartyAction(botAI);
    }
    static Action* hammer_of_justice(PlayerbotAI* botAI) { return new CastHammerOfJusticeAction(botAI); }
    static Action* flash_of_light_on_party(PlayerbotAI* botAI) { return new CastFlashOfLightOnPartyAction(botAI); }
    static Action* holy_light(PlayerbotAI* botAI) { return new CastHolyLightAction(botAI); }
    static Action* holy_light_on_party(PlayerbotAI* botAI) { return new CastHolyLightOnPartyAction(botAI); }
    static Action* lay_on_hands(PlayerbotAI* botAI) { return new CastLayOnHandsAction(botAI); }
    static Action* lay_on_hands_on_party(PlayerbotAI* botAI) { return new CastLayOnHandsOnPartyAction(botAI); }
    static Action* judgement_of_justice(PlayerbotAI* botAI) { return new CastJudgementOfJusticeAction(botAI); }
    static Action* hammer_of_wrath(PlayerbotAI* botAI) { return new CastHammerOfWrathAction(botAI); }
    static Action* holy_shield(PlayerbotAI* botAI) { return new CastHolyShieldAction(botAI); }
    static Action* hammer_of_the_righteous(PlayerbotAI* botAI) { return new CastHammerOfTheRighteousAction(botAI); }
    static Action* retribution_aura(PlayerbotAI* botAI) { return new CastRetributionAuraAction(botAI); }
    static Action* shadow_resistance_aura(PlayerbotAI* botAI) { return new CastShadowResistanceAuraAction(botAI); }
    static Action* frost_resistance_aura(PlayerbotAI* botAI) { return new CastFrostResistanceAuraAction(botAI); }
    static Action* fire_resistance_aura(PlayerbotAI* botAI) { return new CastFireResistanceAuraAction(botAI); }
    static Action* hammer_of_justice_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new CastHammerOfJusticeOnEnemyHealerAction(botAI);
    }
    static Action* hammer_of_justice_on_snare_target(PlayerbotAI* botAI)
    {
        return new CastHammerOfJusticeSnareAction(botAI);
    }
    static Action* righteous_defense(PlayerbotAI* botAI) { return new CastRighteousDefenseAction(botAI); }
    static Action* repentance(PlayerbotAI* botAI) { return new CastRepentanceAction(botAI); }
    static Action* repentance_on_snare_target(PlayerbotAI* botAI) { return new CastRepentanceSnareAction(botAI); }
    static Action* repentance_on_enemy_healer(PlayerbotAI* botAI) { return new CastRepentanceOnHealerAction(botAI); }
    static Action* sanctity_aura(PlayerbotAI* botAI) { return new CastSanctityAuraAction(botAI); }
    static Action* holy_shock(PlayerbotAI* botAI) { return new CastHolyShockAction(botAI); }
    static Action* holy_shock_on_party(PlayerbotAI* botAI) { return new CastHolyShockOnPartyAction(botAI); }
    static Action* divine_plea(PlayerbotAI* ai) { return new CastDivinePleaAction(ai); }
    static Action* shield_of_righteousness(PlayerbotAI* ai) { return new ShieldOfRighteousnessAction(ai); }
    static Action* beacon_of_light_on_main_tank(PlayerbotAI* ai) { return new CastBeaconOfLightOnMainTankAction(ai); }
    static Action* sacred_shield_on_main_tank(PlayerbotAI* ai) { return new CastSacredShieldOnMainTankAction(ai); }
    static Action* avenging_wrath(PlayerbotAI* ai) { return new CastAvengingWrathAction(ai); }
    static Action* divine_illumination(PlayerbotAI* ai) { return new CastDivineIlluminationAction(ai); }
    static Action* divine_sacrifice(PlayerbotAI* ai) { return new CastDivineSacrificeAction(ai); }
    static Action* cancel_divine_sacrifice(PlayerbotAI* ai) { return new CastCancelDivineSacrificeAction(ai); }
};

PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new PaladinStrategyFactoryInternal());
    strategyContexts.Add(new PaladinCombatStrategyFactoryInternal());
    strategyContexts.Add(new PaladinBuffStrategyFactoryInternal());
    strategyContexts.Add(new PaladinResistanceStrategyFactoryInternal());
    actionContexts.Add(new PaladinAiObjectContextInternal());
    triggerContexts.Add(new PaladinTriggerFactoryInternal());
}
