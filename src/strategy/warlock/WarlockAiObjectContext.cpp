/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarlockAiObjectContext.h"

#include "AfflictionWarlockStrategy.h"
#include "DemonologyWarlockStrategy.h"
#include "DestructionWarlockStrategy.h"
#include "GenericTriggers.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"
#include "Strategy.h"
#include "TankWarlockStrategy.h"
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
        creators["meta melee"] = &WarlockStrategyFactoryInternal::meta_melee_aoe;
        creators["tank"] = &WarlockStrategyFactoryInternal::tank;
        creators["aoe"] = &WarlockStrategyFactoryInternal::aoe;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarlockNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* boost(PlayerbotAI* botAI) { return new WarlockBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new WarlockCcStrategy(botAI); }
    static Strategy* pet(PlayerbotAI* botAI) { return new WarlockPetStrategy(botAI); }
    static Strategy* meta_melee_aoe(PlayerbotAI* botAI) { return new MetaMeleeAoeStrategy(botAI); }
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarlockStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new AoEWarlockStrategy(botAI); }
};

class WarlockCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["affli"] = &WarlockCombatStrategyFactoryInternal::affliction;
        creators["demo"] = &WarlockCombatStrategyFactoryInternal::demonology;
        creators["destro"] = &WarlockCombatStrategyFactoryInternal::destruction;
    }

private:
    static Strategy* affliction(PlayerbotAI* botAI) { return new AfflictionWarlockStrategy(botAI); }
    static Strategy* demonology(PlayerbotAI* botAI) { return new DemonologyWarlockStrategy(botAI); }
    static Strategy* destruction(PlayerbotAI* botAI) { return new DestructionWarlockStrategy(botAI); }
};

class WarlockPetStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockPetStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["imp"] = &WarlockPetStrategyFactoryInternal::imp;
        creators["voidwalker"] = &WarlockPetStrategyFactoryInternal::voidwalker;
        creators["succubus"] = &WarlockPetStrategyFactoryInternal::succubus;
        creators["felhunter"] = &WarlockPetStrategyFactoryInternal::felhunter;
        creators["felguard"] = &WarlockPetStrategyFactoryInternal::felguard;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpStrategy(ai); }
    static Strategy* voidwalker(PlayerbotAI* ai) { return new SummonVoidwalkerStrategy(ai); }
    static Strategy* succubus(PlayerbotAI* ai) { return new SummonSuccubusStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardStrategy(ai); }
};

class WarlockSoulstoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockSoulstoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["ss self"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_self;
        creators["ss master"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_master;
        creators["ss tank"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_tank;
        creators["ss healer"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_healer;
    }

private:
    static Strategy* soulstone_self(PlayerbotAI* ai) { return new SoulstoneSelfStrategy(ai); }
    static Strategy* soulstone_master(PlayerbotAI* ai) { return new SoulstoneMasterStrategy(ai); }
    static Strategy* soulstone_tank(PlayerbotAI* ai) { return new SoulstoneTankStrategy(ai); }
    static Strategy* soulstone_healer(PlayerbotAI* ai) { return new SoulstoneHealerStrategy(ai); }
};

class WarlockCurseStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCurseStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["curse of agony"] = &WarlockCurseStrategyFactoryInternal::curse_of_agony;
        creators["curse of elements"] = &WarlockCurseStrategyFactoryInternal::curse_of_elements;
        creators["curse of doom"] = &WarlockCurseStrategyFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockCurseStrategyFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockCurseStrategyFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockCurseStrategyFactoryInternal::curse_of_weakness;
    }

private:
    static Strategy* curse_of_agony(PlayerbotAI* botAI) { return new WarlockCurseOfAgonyStrategy(botAI); }
    static Strategy* curse_of_elements(PlayerbotAI* botAI) { return new WarlockCurseOfTheElementsStrategy(botAI); }
    static Strategy* curse_of_doom(PlayerbotAI* botAI) { return new WarlockCurseOfDoomStrategy(botAI); }
    static Strategy* curse_of_exhaustion(PlayerbotAI* botAI) { return new WarlockCurseOfExhaustionStrategy(botAI); }
    static Strategy* curse_of_tongues(PlayerbotAI* botAI) { return new WarlockCurseOfTonguesStrategy(botAI); }
    static Strategy* curse_of_weakness(PlayerbotAI* botAI) { return new WarlockCurseOfWeaknessStrategy(botAI); }
};

class WarlockWeaponStoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockWeaponStoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["firestone"] = &WarlockWeaponStoneStrategyFactoryInternal::firestone;
        creators["spellstone"] = &WarlockWeaponStoneStrategyFactoryInternal::spellstone;
    }

private:
    static Strategy* firestone(PlayerbotAI* ai) { return new UseFirestoneStrategy(ai); }
    static Strategy* spellstone(PlayerbotAI* ai) { return new UseSpellstoneStrategy(ai); }
};

class WarlockTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarlockTriggerFactoryInternal()
    {
        creators["shadow trance"] = &WarlockTriggerFactoryInternal::shadow_trance;
        creators["demon armor"] = &WarlockTriggerFactoryInternal::demon_armor;
        creators["soul link"] = &WarlockTriggerFactoryInternal::soul_link;
        creators["no soul shard"] = &WarlockTriggerFactoryInternal::no_soul_shard;
        creators["too many soul shards"] = &WarlockTriggerFactoryInternal::too_many_soul_shards;
        creators["no healthstone"] = &WarlockTriggerFactoryInternal::HasHealthstone;
        creators["no firestone"] = &WarlockTriggerFactoryInternal::HasFirestone;
        creators["no spellstone"] = &WarlockTriggerFactoryInternal::HasSpellstone;
        creators["no soulstone"] = &WarlockTriggerFactoryInternal::OutOfSoulstone;
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
        creators["immolate"] = &WarlockTriggerFactoryInternal::immolate;
        creators["immolate on attacker"] = &WarlockTriggerFactoryInternal::immolate_on_attacker;
        creators["unstable affliction"] = &WarlockTriggerFactoryInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockTriggerFactoryInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockTriggerFactoryInternal::haunt;
        creators["decimation"] = &WarlockTriggerFactoryInternal::decimation;
        creators["life tap"] = &WarlockTriggerFactoryInternal::life_tap;
        creators["life tap glyph buff"] = &WarlockTriggerFactoryInternal::life_tap_glyph_buff;
        creators["molten core"] = &WarlockTriggerFactoryInternal::molten_core;
        creators["metamorphosis"] = &WarlockTriggerFactoryInternal::metamorphosis;
        creators["demonic empowerment"] = &WarlockTriggerFactoryInternal::demonic_empowerment;
        creators["immolation aura active"] = &WarlockTriggerFactoryInternal::immolation_aura_active;
        creators["metamorphosis not active"] = &WarlockTriggerFactoryInternal::metamorphosis_not_active;
        creators["meta melee flee check"] = &WarlockTriggerFactoryInternal::meta_melee_flee_check;
        creators["curse of agony"] = &WarlockTriggerFactoryInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockTriggerFactoryInternal::curse_of_agony_on_attacker;
        creators["curse of the elements"] = &WarlockTriggerFactoryInternal::curse_of_the_elements;
        creators["curse of doom"] = &WarlockTriggerFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockTriggerFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockTriggerFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockTriggerFactoryInternal::curse_of_weakness;
        creators["wrong pet"] = &WarlockTriggerFactoryInternal::wrong_pet;
        creators["rain of fire channel check"] = &WarlockTriggerFactoryInternal::rain_of_fire_channel_check;
    }

private:
    static Trigger* shadow_trance(PlayerbotAI* botAI) { return new ShadowTranceTrigger(botAI); }
    static Trigger* demon_armor(PlayerbotAI* botAI) { return new DemonArmorTrigger(botAI); }
    static Trigger* soul_link(PlayerbotAI* botAI) { return new SoulLinkTrigger(botAI); }
    static Trigger* no_soul_shard(PlayerbotAI* botAI) { return new OutOfSoulShardsTrigger(botAI); }
    static Trigger* too_many_soul_shards(PlayerbotAI* botAI) { return new TooManySoulShardsTrigger(botAI); }
    static Trigger* HasHealthstone(PlayerbotAI* botAI) { return new HasHealthstoneTrigger(botAI); }
    static Trigger* HasFirestone(PlayerbotAI* botAI) { return new HasFirestoneTrigger(botAI); }
    static Trigger* HasSpellstone(PlayerbotAI* botAI) { return new HasSpellstoneTrigger(botAI); }
    static Trigger* OutOfSoulstone(PlayerbotAI* botAI) { return new OutOfSoulstoneTrigger(botAI); }
    static Trigger* firestone(PlayerbotAI* botAI) { return new FirestoneTrigger(botAI); }
    static Trigger* spellstone(PlayerbotAI* botAI) { return new SpellstoneTrigger(botAI); }
    static Trigger* soulstone(PlayerbotAI* botAI) { return new SoulstoneTrigger(botAI); }
    static Trigger* corruption(PlayerbotAI* botAI) { return new CorruptionTrigger(botAI); }
    static Trigger* corruption_on_attacker(PlayerbotAI* botAI) { return new CorruptionOnAttackerTrigger(botAI); }
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
    static Trigger* decimation(PlayerbotAI* ai) { return new DecimationTrigger(ai); }
    static Trigger* life_tap(PlayerbotAI* ai) { return new LifeTapTrigger(ai); }
    static Trigger* life_tap_glyph_buff(PlayerbotAI* ai) { return new LifeTapGlyphBuffTrigger(ai); }
    static Trigger* molten_core(PlayerbotAI* ai) { return new MoltenCoreTrigger(ai); }
    static Trigger* metamorphosis(PlayerbotAI* ai) { return new MetamorphosisTrigger(ai); }
    static Trigger* demonic_empowerment(PlayerbotAI* ai) { return new DemonicEmpowermentTrigger(ai); }
    static Trigger* immolation_aura_active(PlayerbotAI* ai) { return new ImmolationAuraActiveTrigger(ai); }
    static Trigger* metamorphosis_not_active(PlayerbotAI* ai) { return new MetamorphosisNotActiveTrigger(ai); }
    static Trigger* meta_melee_flee_check(PlayerbotAI* ai) { return new MetaMeleeEnemyTooCloseForSpellTrigger(ai); }
    static Trigger* curse_of_agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CurseOfAgonyOnAttackerTrigger(botAI); }
    static Trigger* curse_of_the_elements(PlayerbotAI* ai) { return new CurseOfTheElementsTrigger(ai); }
    static Trigger* curse_of_doom(PlayerbotAI* ai) { return new CurseOfDoomTrigger(ai); }
    static Trigger* curse_of_exhaustion(PlayerbotAI* ai) { return new CurseOfExhaustionTrigger(ai); }
    static Trigger* curse_of_tongues(PlayerbotAI* ai) { return new CurseOfTonguesTrigger(ai); }
    static Trigger* curse_of_weakness(PlayerbotAI* ai) { return new CurseOfWeaknessTrigger(ai); }
    static Trigger* wrong_pet(PlayerbotAI* ai) { return new WrongPetTrigger(ai); }
    static Trigger* rain_of_fire_channel_check(PlayerbotAI* ai) { return new RainOfFireChannelCheckTrigger(ai); }
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
        creators["create soul shard"] = &WarlockAiObjectContextInternal::create_soul_shard;
        creators["destroy soul shard"] = &WarlockAiObjectContextInternal::destroy_soul_shard;
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
        creators["curse of agony"] = &WarlockAiObjectContextInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockAiObjectContextInternal::curse_of_agony_on_attacker;
        creators["curse of the elements"] = &WarlockAiObjectContextInternal::curse_of_the_elements;
        creators["curse of doom"] = &WarlockAiObjectContextInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockAiObjectContextInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockAiObjectContextInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockAiObjectContextInternal::curse_of_weakness;
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
    static Action* create_soul_shard(PlayerbotAI* botAI) { return new CreateSoulShardAction(botAI); }
    static Action* destroy_soul_shard(PlayerbotAI* botAI) { return new DestroySoulShardAction(botAI); }
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
    static Action* demonic_empowerment(PlayerbotAI* ai) { return new CastDemonicEmpowermentAction(ai); }
    static Action* metamorphosis(PlayerbotAI* ai) { return new CastMetamorphosisAction(ai); }
    static Action* soul_fire(PlayerbotAI* ai) { return new CastSoulFireAction(ai); }
    static Action* demon_charge(PlayerbotAI* ai) { return new DemonChargeAction(ai); }
    static Action* shadow_cleave(PlayerbotAI* ai) { return new ShadowCleaveAction(ai); }
    static Action* shadowburn(PlayerbotAI* ai) { return new CastShadowburnAction(ai); }
    static Action* shadowflame(PlayerbotAI* botAI) { return new CastShadowflameAction(botAI); }
    static Action* immolation_aura(PlayerbotAI* botAI) { return new CastImmolationAuraAction(botAI); }
    static Action* chaos_bolt(PlayerbotAI* botAI) { return new CastChaosBoltAction(botAI); }
    static Action* soulshatter(PlayerbotAI* botAI) { return new CastSoulshatterAction(botAI); }
    static Action* searing_pain(PlayerbotAI* botAI) { return new CastSearingPainAction(botAI); }
    static Action* shadow_ward(PlayerbotAI* botAI) { return new CastShadowWardAction(botAI); }
    static Action* curse_of_agony(PlayerbotAI* botAI) { return new CastCurseOfAgonyAction(botAI); }
    static Action* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CastCurseOfAgonyOnAttackerAction(botAI); }
    static Action* curse_of_the_elements(PlayerbotAI* ai) { return new CastCurseOfTheElementsAction(ai); }
    static Action* curse_of_doom(PlayerbotAI* ai) { return new CastCurseOfDoomAction(ai); }
    static Action* curse_of_exhaustion(PlayerbotAI* ai) { return new CastCurseOfExhaustionAction(ai); }
    static Action* curse_of_tongues(PlayerbotAI* ai) { return new CastCurseOfTonguesAction(ai); }
    static Action* curse_of_weakness(PlayerbotAI* ai) { return new CastCurseOfWeaknessAction(ai); }
};

SharedNamedObjectContextList<Strategy> WarlockAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Action> WarlockAiObjectContext::sharedActionContexts;
SharedNamedObjectContextList<Trigger> WarlockAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> WarlockAiObjectContext::sharedValueContexts;

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedActionContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void WarlockAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedActionContexts(sharedActionContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void WarlockAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new WarlockStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCombatStrategyFactoryInternal());
    strategyContexts.Add(new WarlockPetStrategyFactoryInternal());
    strategyContexts.Add(new WarlockSoulstoneStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCurseStrategyFactoryInternal());
    strategyContexts.Add(new WarlockWeaponStoneStrategyFactoryInternal());
}

void WarlockAiObjectContext::BuildSharedActionContexts(SharedNamedObjectContextList<Action>& actionContexts)
{
    AiObjectContext::BuildSharedActionContexts(actionContexts);
    actionContexts.Add(new WarlockAiObjectContextInternal());
}

void WarlockAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new WarlockTriggerFactoryInternal());
}

void WarlockAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
