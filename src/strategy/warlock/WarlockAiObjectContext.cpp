/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockAiObjectContext.h"
#include "AfflictionWarlockStrategy.h"
#include "DemonologyWarlockStrategy.h"
#include "DestructionWarlockStrategy.h"
#include "TankWarlockStrategy.h"
#include "DpsWarlockStrategy.h"
#include "GenericTriggers.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"
#include "Strategy.h"
#include "UseItemAction.h"
#include "WarlockActions.h"
#include "WarlockTriggers.h"

class WarlockStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockStrategyFactoryInternal()
    { 
        creators["nc"] = &WarlockStrategyFactoryInternal::nc;
        creators["pull"] = &WarlockStrategyFactoryInternal::pull;
        creators["boost"] = &WarlockStrategyFactoryInternal::boost;
        creators["cc"] = &WarlockStrategyFactoryInternal::cc;
        creators["pet"] = &WarlockStrategyFactoryInternal::pet;
        creators["affli"] = &WarlockStrategyFactoryInternal::affliction;
        creators["affli aoe"] = &WarlockStrategyFactoryInternal::affliction_aoe;
        creators["demo"] = &WarlockStrategyFactoryInternal::demonology;
        creators["demo aoe"] = &WarlockStrategyFactoryInternal::demonology_aoe;
        creators["destro"] = &WarlockStrategyFactoryInternal::destruction;
        creators["destro aoe"] = &WarlockStrategyFactoryInternal::destruction_aoe;
        creators["meta melee"] = &WarlockStrategyFactoryInternal::meta_melee_aoe;
        creators["dps"] = &WarlockStrategyFactoryInternal::dps;
        creators["aoe"] = &WarlockStrategyFactoryInternal::aoe;
        creators["curse of elements"] = &WarlockStrategyFactoryInternal::curse_of_elements;
    }

private:
    static Strategy* pet(PlayerbotAI* botAI) { return new WarlockPetStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarlockNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* boost(PlayerbotAI* botAI) { return new WarlockBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new WarlockCcStrategy(botAI); }
    static Strategy* affliction(PlayerbotAI* botAI) { return new AfflictionWarlockStrategy(botAI); }
    static Strategy* affliction_aoe(PlayerbotAI* botAI) { return new AfflictionWarlockAoeStrategy(botAI); }
    static Strategy* demonology(PlayerbotAI* botAI) { return new DemonologyWarlockStrategy(botAI); }
    static Strategy* demonology_aoe(PlayerbotAI* botAI) { return new DemonologyWarlockAoeStrategy(botAI); }
    static Strategy* destruction(PlayerbotAI* botAI) { return new DestructionWarlockStrategy(botAI); }
    static Strategy* destruction_aoe(PlayerbotAI* botAI) { return new DestructionWarlockAoeStrategy(botAI); }
    static Strategy* meta_melee_aoe(PlayerbotAI* botAI) { return new MetaMeleeAoeStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsWarlockStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new DpsAoeWarlockStrategy(botAI); }
    static Strategy* curse_of_elements(PlayerbotAI* botAI) { return new WarlockCurseOfTheElementsStrategy(botAI); }
};

class WarlockCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &WarlockCombatStrategyFactoryInternal::tank;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarlockStrategy(botAI); }
};

class NonCombatBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    NonCombatBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["imp"] = &NonCombatBuffStrategyFactoryInternal::imp;
        creators["voidwalker"] = &NonCombatBuffStrategyFactoryInternal::voidwalker;
        creators["succubus"] = &NonCombatBuffStrategyFactoryInternal::succubus;
        creators["felhunter"] = &NonCombatBuffStrategyFactoryInternal::felhunter;
        creators["felguard"] = &NonCombatBuffStrategyFactoryInternal::felguard;
        creators["ss self"] = &NonCombatBuffStrategyFactoryInternal::soulstone_self;
        creators["ss master"] = &NonCombatBuffStrategyFactoryInternal::soulstone_master;
        creators["ss tank"] = &NonCombatBuffStrategyFactoryInternal::soulstone_tank;
        creators["ss healer"] = &NonCombatBuffStrategyFactoryInternal::soulstone_healer;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpStrategy(ai); }
    static Strategy* voidwalker(PlayerbotAI* ai) { return new SummonVoidwalkerStrategy(ai); }
    static Strategy* succubus(PlayerbotAI* ai) { return new SummonSuccubusStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardStrategy(ai); }
    static Strategy* soulstone_self(PlayerbotAI* ai) { return new SoulstoneSelfStrategy(ai); }
    static Strategy* soulstone_master(PlayerbotAI* ai) { return new SoulstoneMasterStrategy(ai); }
    static Strategy* soulstone_tank(PlayerbotAI* ai) { return new SoulstoneTankStrategy(ai); }
    static Strategy* soulstone_healer(PlayerbotAI* ai) { return new SoulstoneHealerStrategy(ai); }
};

class WarlockTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarlockTriggerFactoryInternal()
    {
        creators["shadow trance"] = &WarlockTriggerFactoryInternal::shadow_trance;
        creators["demon armor"] = &WarlockTriggerFactoryInternal::demon_armor;
        creators["soul link"] = &WarlockTriggerFactoryInternal::soul_link;
        creators["no healthstone"] = &WarlockTriggerFactoryInternal::HasHealthstone;
        creators["no firestone"] = &WarlockTriggerFactoryInternal::HasFirestone;
        creators["no spellstone"] = &WarlockTriggerFactoryInternal::HasSpellstone;
        creators["no soulstone"] = &WarlockTriggerFactoryInternal::HasSoulstone;
        creators["firestone"] = &WarlockTriggerFactoryInternal::firestone;
        creators["spellstone"] = &WarlockTriggerFactoryInternal::spellstone;
        creators["soulstone"] = &WarlockTriggerFactoryInternal::soulstone;
        creators["banish"] = &WarlockTriggerFactoryInternal::banish;
        creators["fear"] = &WarlockTriggerFactoryInternal::fear;
        creators["spell lock"] = &WarlockTriggerFactoryInternal::spell_lock;
        creators["devour magic purge"] = &WarlockTriggerFactoryInternal::devour_magic_purge;
        creators["devour magic cleanse"] = &WarlockTriggerFactoryInternal::devour_magic_cleanse;
        creators["backlash"] = &WarlockTriggerFactoryInternal::backlash;
        creators["corruption"] = &WarlockTriggerFactoryInternal::corruption;
        creators["corruption on attacker"] = &WarlockTriggerFactoryInternal::corruption_on_attacker;
        creators["curse of agony"] = &WarlockTriggerFactoryInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockTriggerFactoryInternal::curse_of_agony_on_attacker;
        creators["immolate"] = &WarlockTriggerFactoryInternal::immolate;
        creators["immolate on attacker"] = &WarlockTriggerFactoryInternal::immolate_on_attacker;
        creators["unstable affliction"] = &WarlockTriggerFactoryInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockTriggerFactoryInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockTriggerFactoryInternal::haunt;
        creators["curse of the elements"] = &WarlockTriggerFactoryInternal::curse_of_the_elements;
        creators["decimation"] = &WarlockTriggerFactoryInternal::decimation;
        creators["life tap"] = &WarlockTriggerFactoryInternal::life_tap;
        creators["life tap glyph buff"] = &WarlockTriggerFactoryInternal::life_tap_glyph_buff;
        creators["molten core"] = &WarlockTriggerFactoryInternal::molten_core;
        creators["metamorphosis"] = &WarlockTriggerFactoryInternal::metamorphosis;
        creators["demonic empowerment"] = &WarlockTriggerFactoryInternal::demonic_empowerment;
        creators["immolation aura active"] = &WarlockTriggerFactoryInternal::immolation_aura_active;
    }

private:
    static Trigger* shadow_trance(PlayerbotAI* botAI) { return new ShadowTranceTrigger(botAI); }
    static Trigger* demon_armor(PlayerbotAI* botAI) { return new DemonArmorTrigger(botAI); }
    static Trigger* soul_link(PlayerbotAI* botAI) { return new SoulLinkTrigger(botAI); }
    static Trigger* HasHealthstone(PlayerbotAI* botAI) { return new HasHealthstoneTrigger(botAI); }
    static Trigger* HasFirestone(PlayerbotAI* botAI) { return new HasFirestoneTrigger(botAI); }
    static Trigger* HasSpellstone(PlayerbotAI* botAI) { return new HasSpellstoneTrigger(botAI); }
    static Trigger* HasSoulstone(PlayerbotAI* botAI) { return new HasSoulstoneTrigger(botAI); }
    static Trigger* firestone(PlayerbotAI* botAI) { return new FirestoneTrigger(botAI); }
    static Trigger* spellstone(PlayerbotAI* botAI) { return new SpellstoneTrigger(botAI); }
    static Trigger* soulstone(PlayerbotAI* botAI) { return new SoulstoneTrigger(botAI); }
    static Trigger* corruption(PlayerbotAI* botAI) { return new CorruptionTrigger(botAI); }
    static Trigger* corruption_on_attacker(PlayerbotAI* botAI) { return new CorruptionOnAttackerTrigger(botAI); }
    static Trigger* curse_of_agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CurseOfAgonyOnAttackerTrigger(botAI); }
    static Trigger* banish(PlayerbotAI* botAI) { return new BanishTrigger(botAI); }
    static Trigger* fear(PlayerbotAI* botAI) { return new FearTrigger(botAI); }
    static Trigger* spell_lock(PlayerbotAI* botAI) { return new SpellLockInterruptSpellTrigger(botAI); }
    static Trigger* devour_magic_purge(PlayerbotAI* botAI) { return new DevourMagicPurgeTrigger(botAI); }
    static Trigger* devour_magic_cleanse(PlayerbotAI* botAI) { return new DevourMagicCleanseTrigger(botAI); }
    static Trigger* backlash(PlayerbotAI* botAI) { return new BacklashTrigger(botAI); }
    static Trigger* immolate(PlayerbotAI* botAI) { return new ImmolateTrigger(botAI); }
    static Trigger* immolate_on_attacker(PlayerbotAI* ai) { return new ImmolateOnAttackerTrigger(ai); }
    static Trigger* unstable_affliction(PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); }
    static Trigger* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new UnstableAfflictionOnAttackerTrigger(ai); }
    static Trigger* haunt(PlayerbotAI* ai) { return new HauntTrigger(ai); }
    static Trigger* curse_of_the_elements(PlayerbotAI* ai) { return new CurseOfTheElementsTrigger(ai); }
    static Trigger* decimation(PlayerbotAI* ai) { return new DecimationTrigger(ai); }
    static Trigger* life_tap(PlayerbotAI* ai) { return new LifeTapTrigger(ai); }
    static Trigger* life_tap_glyph_buff(PlayerbotAI* ai) { return new LifeTapGlyphBuffTrigger(ai); }
    static Trigger* molten_core(PlayerbotAI* ai) { return new MoltenCoreTrigger(ai); }
    static Trigger* metamorphosis(PlayerbotAI* ai) { return new MetamorphosisTrigger(ai); }
    static Trigger* demonic_empowerment(PlayerbotAI* ai) { return new DemonicEmpowermentTrigger(ai); }
    static Trigger* immolation_aura_active(PlayerbotAI* ai) { return new ImmolationAuraActiveTrigger(ai); }
};

class WarlockAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    WarlockAiObjectContextInternal()
    {
        creators["fel armor"] = &WarlockAiObjectContextInternal::fel_armor;
        creators["demon armor"] = &WarlockAiObjectContextInternal::demon_armor;
        creators["demon skin"] = &WarlockAiObjectContextInternal::demon_skin;
        creators["soul link"] = &WarlockAiObjectContextInternal::soul_link;
        creators["create healthstone"] = &WarlockAiObjectContextInternal::create_healthstone;
        creators["create firestone"] = &WarlockAiObjectContextInternal::create_firestone;
        creators["create spellstone"] = &WarlockAiObjectContextInternal::create_spellstone;
        creators["create soulstone"] = &WarlockAiObjectContextInternal::create_soulstone;
        creators["firestone"] = &WarlockAiObjectContextInternal::firestone;
        creators["spellstone"] = &WarlockAiObjectContextInternal::spellstone;
        creators["soulstone self"] = &WarlockAiObjectContextInternal::soulstone_self;
        creators["soulstone master"] = &WarlockAiObjectContextInternal::soulstone_master;
        creators["soulstone tank"] = &WarlockAiObjectContextInternal::soulstone_tank;
        creators["soulstone healer"] = &WarlockAiObjectContextInternal::soulstone_healer;
        creators["summon voidwalker"] = &WarlockAiObjectContextInternal::summon_voidwalker;
        creators["summon felguard"] = &WarlockAiObjectContextInternal::summon_felguard;
        creators["summon felhunter"] = &WarlockAiObjectContextInternal::summon_felhunter;
        creators["summon succubus"] = &WarlockAiObjectContextInternal::summon_succubus;
        creators["summon imp"] = &WarlockAiObjectContextInternal::summon_imp;
        creators["fel domination"] = &WarlockAiObjectContextInternal::fel_domination;
        creators["immolate"] = &WarlockAiObjectContextInternal::immolate;
        creators["immolate on attacker"] = &WarlockAiObjectContextInternal::immolate_on_attacker;
        creators["corruption"] = &WarlockAiObjectContextInternal::corruption;
        creators["corruption on attacker"] = &WarlockAiObjectContextInternal::corruption_on_attacker;
        creators["curse of agony"] = &WarlockAiObjectContextInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockAiObjectContextInternal::curse_of_agony_on_attacker;
        creators["shadow bolt"] = &WarlockAiObjectContextInternal::shadow_bolt;
        creators["drain soul"] = &WarlockAiObjectContextInternal::drain_soul;
        creators["drain mana"] = &WarlockAiObjectContextInternal::drain_mana;
        creators["drain life"] = &WarlockAiObjectContextInternal::drain_life;
        creators["banish on cc"] = &WarlockAiObjectContextInternal::banish_on_cc;
        creators["fear on cc"] = &WarlockAiObjectContextInternal::fear_on_cc;
        creators["spell lock"] = &WarlockAiObjectContextInternal::spell_lock;
        creators["devour magic purge"] = &WarlockAiObjectContextInternal::devour_magic_purge;
        creators["devour magic cleanse"] = &WarlockAiObjectContextInternal::devour_magic_cleanse;
        creators["seed of corruption"] = &WarlockAiObjectContextInternal::seed_of_corruption;
        creators["seed of corruption on attacker"] = &WarlockAiObjectContextInternal::seed_of_corruption_on_attacker;
        creators["rain of fire"] = &WarlockAiObjectContextInternal::rain_of_fire;
        creators["hellfire"] = &WarlockAiObjectContextInternal::hellfire;
        creators["shadowfury"] = &WarlockAiObjectContextInternal::shadowfury;
        creators["life tap"] = &WarlockAiObjectContextInternal::life_tap;
        creators["incinerate"] = &WarlockAiObjectContextInternal::incinerate;
        creators["conflagrate"] = &WarlockAiObjectContextInternal::conflagrate;
        creators["unstable affliction"] = &WarlockAiObjectContextInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockAiObjectContextInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockAiObjectContextInternal::haunt;
        creators["curse of the elements"] = &WarlockAiObjectContextInternal::curse_of_the_elements;
        creators["demonic empowerment"] = &WarlockAiObjectContextInternal::demonic_empowerment;
        creators["metamorphosis"] = &WarlockAiObjectContextInternal::metamorphosis;
        creators["soul fire"] = &WarlockAiObjectContextInternal::soul_fire;
        creators["incinerate"] = &WarlockAiObjectContextInternal::incinerate;
        creators["demon charge"] = &WarlockAiObjectContextInternal::demon_charge;
        creators["shadow cleave"] = &WarlockAiObjectContextInternal::shadow_cleave;
        creators["shadowburn"] = &WarlockAiObjectContextInternal::shadowburn;
        creators["shadowflame"] = &WarlockAiObjectContextInternal::shadowflame;
        creators["immolation aura"] = &WarlockAiObjectContextInternal::immolation_aura;
        creators["chaos bolt"] = &WarlockAiObjectContextInternal::chaos_bolt;
        creators["soulshatter"] = &WarlockAiObjectContextInternal::soulshatter;
        creators["searing pain"] = WarlockAiObjectContextInternal::searing_pain;
        creators["shadow ward"] = &WarlockAiObjectContextInternal::shadow_ward;
 }

private:
    static Action* conflagrate(PlayerbotAI* botAI) { return new CastConflagrateAction(botAI); }
    static Action* incinerate(PlayerbotAI* botAI) { return new CastIncinerateAction(botAI); }
    static Action* immolate(PlayerbotAI* botAI) { return new CastImmolateAction(botAI); }
    static Action* immolate_on_attacker(PlayerbotAI* botAI) { return new CastImmolateOnAttackerAction(botAI); }
    static Action* fel_armor(PlayerbotAI* botAI) { return new CastFelArmorAction(botAI); }
    static Action* demon_armor(PlayerbotAI* botAI) { return new CastDemonArmorAction(botAI); }
    static Action* demon_skin(PlayerbotAI* botAI) { return new CastDemonSkinAction(botAI); }
    static Action* soul_link(PlayerbotAI* botAI) { return new CastSoulLinkAction(botAI); }
    static Action* create_healthstone(PlayerbotAI* botAI) { return new CastCreateHealthstoneAction(botAI); }
    static Action* create_firestone(PlayerbotAI* botAI) { return new CastCreateFirestoneAction(botAI); }
    static Action* create_spellstone(PlayerbotAI* botAI) { return new CastCreateSpellstoneAction(botAI); }
    static Action* create_soulstone(PlayerbotAI* botAI) { return new CastCreateSoulstoneAction(botAI); }
    static Action* firestone(PlayerbotAI* botAI) { return new UseSpellItemAction(botAI, "firestone", true); }
    static Action* spellstone(PlayerbotAI* botAI) { return new UseSpellItemAction(botAI, "spellstone", true); }
    static Action* soulstone_self(PlayerbotAI* botAI) { return new UseSoulstoneSelfAction(botAI); }
    static Action* soulstone_master(PlayerbotAI* botAI) { return new UseSoulstoneMasterAction(botAI); }
    static Action* soulstone_tank(PlayerbotAI* botAI) { return new UseSoulstoneTankAction(botAI); }
    static Action* soulstone_healer(PlayerbotAI* botAI) { return new UseSoulstoneHealerAction(botAI); }
    static Action* summon_voidwalker(PlayerbotAI* botAI) { return new CastSummonVoidwalkerAction(botAI); }
    static Action* summon_felguard(PlayerbotAI* botAI) { return new CastSummonFelguardAction(botAI); }
    static Action* summon_felhunter(PlayerbotAI* botAI) { return new CastSummonFelhunterAction(botAI); }
    static Action* summon_imp(PlayerbotAI* botAI) { return new CastSummonImpAction(botAI); }
    static Action* summon_succubus(PlayerbotAI* botAI) { return new CastSummonSuccubusAction(botAI); }
    static Action* fel_domination(PlayerbotAI* botAI) { return new CastFelDominationAction(botAI); }
    static Action* corruption(PlayerbotAI* botAI) { return new CastCorruptionAction(botAI); }
    static Action* corruption_on_attacker(PlayerbotAI* botAI) { return new CastCorruptionOnAttackerAction(botAI); }
    static Action* curse_of_agony(PlayerbotAI* botAI) { return new CastCurseOfAgonyAction(botAI); }
    static Action* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CastCurseOfAgonyOnAttackerAction(botAI); }
    static Action* shadow_bolt(PlayerbotAI* botAI) { return new CastShadowBoltAction(botAI); }
    static Action* drain_soul(PlayerbotAI* botAI) { return new CastDrainSoulAction(botAI); }
    static Action* drain_mana(PlayerbotAI* botAI) { return new CastDrainManaAction(botAI); }
    static Action* drain_life(PlayerbotAI* botAI) { return new CastDrainLifeAction(botAI); }
    static Action* banish_on_cc(PlayerbotAI* botAI) { return new CastBanishOnCcAction(botAI); }
    static Action* fear_on_cc(PlayerbotAI* botAI) { return new CastFearOnCcAction(botAI); }
    static Action* spell_lock(PlayerbotAI* botAI) { return new CastSpellLockAction(botAI); }
    static Action* devour_magic_purge(PlayerbotAI* botAI) { return new CastDevourMagicPurgeAction(botAI); }
    static Action* devour_magic_cleanse(PlayerbotAI* botAI) { return new CastDevourMagicCleanseAction(botAI); }
    static Action* seed_of_corruption(PlayerbotAI* botAI) { return new CastSeedOfCorruptionAction(botAI); }
    static Action* seed_of_corruption_on_attacker(PlayerbotAI* botAI) { return new CastSeedOfCorruptionOnAttackerAction(botAI); }
    static Action* rain_of_fire(PlayerbotAI* botAI) { return new CastRainOfFireAction(botAI); }
    static Action* hellfire(PlayerbotAI* botAI) { return new CastHellfireAction(botAI); }
    static Action* shadowfury(PlayerbotAI* botAI) { return new CastShadowfuryAction(botAI); }
    static Action* life_tap(PlayerbotAI* botAI) { return new CastLifeTapAction(botAI); }
    static Action* unstable_affliction(PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); }
    static Action* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new CastUnstableAfflictionOnAttackerAction(ai); }
    static Action* haunt(PlayerbotAI* ai) { return new CastHauntAction(ai); }
    static Action* curse_of_the_elements(PlayerbotAI* ai) { return new CastCurseOfTheElementsAction(ai); }
    static Action* demonic_empowerment(PlayerbotAI* ai) { return new CastDemonicEmpowermentAction(ai); }
    static Action* metamorphosis(PlayerbotAI* ai) { return new CastMetamorphosisAction(ai); }
    static Action* soul_fire(PlayerbotAI* ai) { return new CastSoulFireAction(ai); }
    static Action* demon_charge(PlayerbotAI* ai) { return new DemonChargeAction(ai); }
    static Action* shadow_cleave(PlayerbotAI* ai) { return new ShadowCleaveAction(ai); }
    static Action* shadowburn(PlayerbotAI* ai) { return new CastShadowburnAction(ai); }
    static Action* shadowflame(PlayerbotAI* botAI) { return new CastShadowflameAction(botAI); }
    static Action* immolation_aura(PlayerbotAI* botAI) { return new CastImmolationAuraAction(botAI); }
    static Action* chaos_bolt(PlayerbotAI* botAI) { return new CastChaosBoltAction(botAI); }
    static Action* soulshatter(PlayerbotAI* botAI) { return new CastSoulshatterAction(botAI);}
    static Action* searing_pain(PlayerbotAI* botAI) { return new CastSearingPainAction(botAI); }
    static Action* shadow_ward(PlayerbotAI* botAI) { return new CastShadowWardAction(botAI); }
};

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarlockStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCombatStrategyFactoryInternal());
    strategyContexts.Add(new NonCombatBuffStrategyFactoryInternal());
    actionContexts.Add(new WarlockAiObjectContextInternal());
    triggerContexts.Add(new WarlockTriggerFactoryInternal());
}
