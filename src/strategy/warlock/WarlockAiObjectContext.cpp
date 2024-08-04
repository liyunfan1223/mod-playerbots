/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarlockAiObjectContext.h"

#include "DpsWarlockStrategy.h"
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
        creators["aoe"] = &WarlockStrategyFactoryInternal::aoe;
        creators["dps debuff"] = &WarlockStrategyFactoryInternal::dps_debuff;
        creators["boost"] = &WarlockStrategyFactoryInternal::boost;
        creators["cc"] = &WarlockStrategyFactoryInternal::cc;
        creators["pet"] = &WarlockStrategyFactoryInternal::pet;
    }

private:
    static Strategy* pet(PlayerbotAI* botAI) { return new WarlockPetStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarlockNonCombatStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new DpsAoeWarlockStrategy(botAI); }
    static Strategy* dps_debuff(PlayerbotAI* botAI) { return new DpsWarlockDebuffStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* boost(PlayerbotAI* botAI) { return new WarlockBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new WarlockCcStrategy(botAI); }
};

class WarlockCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["dps"] = &WarlockCombatStrategyFactoryInternal::dps;
        creators["tank"] = &WarlockCombatStrategyFactoryInternal::tank;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarlockStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsWarlockStrategy(botAI); }
};

class NonCombatBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    NonCombatBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bdps"] = &NonCombatBuffStrategyFactoryInternal::felguard;
        creators["bmana"] = &NonCombatBuffStrategyFactoryInternal::felhunter;
        creators["bhealth"] = &NonCombatBuffStrategyFactoryInternal::imp;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardStrategy(ai); }
};

class WarlockTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarlockTriggerFactoryInternal()
    {
        creators["shadow trance"] = &WarlockTriggerFactoryInternal::shadow_trance;
        creators["demon armor"] = &WarlockTriggerFactoryInternal::demon_armor;
        creators["no healthstone"] = &WarlockTriggerFactoryInternal::HasHealthstone;
        creators["no firestone"] = &WarlockTriggerFactoryInternal::HasFirestone;
        creators["no spellstone"] = &WarlockTriggerFactoryInternal::HasSpellstone;
        creators["corruption"] = &WarlockTriggerFactoryInternal::corruption;
        creators["corruption on attacker"] = &WarlockTriggerFactoryInternal::corruption_on_attacker;
        creators["curse of agony"] = &WarlockTriggerFactoryInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockTriggerFactoryInternal::curse_of_agony_on_attacker;
        creators["banish"] = &WarlockTriggerFactoryInternal::banish;
        creators["spellstone"] = &WarlockTriggerFactoryInternal::spellstone;
        creators["backlash"] = &WarlockTriggerFactoryInternal::backlash;
        creators["fear"] = &WarlockTriggerFactoryInternal::fear;
        creators["immolate"] = &WarlockTriggerFactoryInternal::immolate;
        creators["amplify curse"] = &WarlockTriggerFactoryInternal::amplify_curse;
        creators["siphon life"] = &WarlockTriggerFactoryInternal::siphon_life;
        creators["siphon life on attacker"] = &WarlockTriggerFactoryInternal::siphon_life_on_attacker;

        creators["immolate on attacker"] = &WarlockTriggerFactoryInternal::immolate_on_attacker;
        creators["unstable affliction"] = &WarlockTriggerFactoryInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockTriggerFactoryInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockTriggerFactoryInternal::haunt;
        creators["decimation"] = &WarlockTriggerFactoryInternal::decimation;
        creators["molten core"] = &WarlockTriggerFactoryInternal::molten_core;
        creators["metamorphosis"] = &WarlockTriggerFactoryInternal::metamorphosis;
    }

private:
    static Trigger* amplify_curse(PlayerbotAI* botAI) { return new AmplifyCurseTrigger(botAI); }
    static Trigger* shadow_trance(PlayerbotAI* botAI) { return new ShadowTranceTrigger(botAI); }
    static Trigger* demon_armor(PlayerbotAI* botAI) { return new DemonArmorTrigger(botAI); }
    static Trigger* HasHealthstone(PlayerbotAI* botAI) { return new HasHealthstoneTrigger(botAI); }
    static Trigger* HasFirestone(PlayerbotAI* botAI) { return new HasFirestoneTrigger(botAI); }
    static Trigger* HasSpellstone(PlayerbotAI* botAI) { return new HasSpellstoneTrigger(botAI); }
    static Trigger* corruption(PlayerbotAI* botAI) { return new CorruptionTrigger(botAI); }
    static Trigger* corruption_on_attacker(PlayerbotAI* botAI) { return new CorruptionOnAttackerTrigger(botAI); }
    static Trigger* siphon_life(PlayerbotAI* botAI) { return new SiphonLifeTrigger(botAI); }
    static Trigger* siphon_life_on_attacker(PlayerbotAI* botAI) { return new SiphonLifeOnAttackerTrigger(botAI); }
    static Trigger* curse_of_agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* curse_of_agony_on_attacker(PlayerbotAI* botAI)
    {
        return new CastCurseOfAgonyOnAttackerTrigger(botAI);
    }
    static Trigger* banish(PlayerbotAI* botAI) { return new BanishTrigger(botAI); }
    static Trigger* spellstone(PlayerbotAI* botAI) { return new SpellstoneTrigger(botAI); }
    static Trigger* backlash(PlayerbotAI* botAI) { return new BacklashTrigger(botAI); }
    static Trigger* fear(PlayerbotAI* botAI) { return new FearTrigger(botAI); }
    static Trigger* immolate(PlayerbotAI* botAI) { return new ImmolateTrigger(botAI); }
    static Trigger* immolate_on_attacker(PlayerbotAI* ai) { return new ImmolateOnAttackerTrigger(ai); }
    static Trigger* unstable_affliction(PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); }
    static Trigger* unstable_affliction_on_attacker(PlayerbotAI* ai)
    {
        return new UnstableAfflictionOnAttackerTrigger(ai);
    }
    static Trigger* haunt(PlayerbotAI* ai) { return new HauntTrigger(ai); }
    static Trigger* decimation(PlayerbotAI* ai) { return new DecimationTrigger(ai); }
    static Trigger* molten_core(PlayerbotAI* ai) { return new MoltenCoreTrigger(ai); }
    static Trigger* metamorphosis(PlayerbotAI* ai) { return new MetamorphosisTrigger(ai); }
};

class WarlockAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    WarlockAiObjectContextInternal()
    {
        creators["fel armor"] = &WarlockAiObjectContextInternal::fel_armor;
        creators["demon armor"] = &WarlockAiObjectContextInternal::demon_armor;
        creators["demon skin"] = &WarlockAiObjectContextInternal::demon_skin;
        creators["create healthstone"] = &WarlockAiObjectContextInternal::create_healthstone;
        creators["create firestone"] = &WarlockAiObjectContextInternal::create_firestone;
        creators["create spellstone"] = &WarlockAiObjectContextInternal::create_spellstone;
        creators["spellstone"] = &WarlockAiObjectContextInternal::spellstone;
        creators["summon voidwalker"] = &WarlockAiObjectContextInternal::summon_voidwalker;
        creators["summon felguard"] = &WarlockAiObjectContextInternal::summon_felguard;
        creators["summon felhunter"] = &WarlockAiObjectContextInternal::summon_felhunter;
        creators["summon succubus"] = &WarlockAiObjectContextInternal::summon_succubus;
        creators["summon imp"] = &WarlockAiObjectContextInternal::summon_imp;
        creators["immolate"] = &WarlockAiObjectContextInternal::immolate;
        creators["corruption"] = &WarlockAiObjectContextInternal::corruption;
        creators["corruption on attacker"] = &WarlockAiObjectContextInternal::corruption_on_attacker;
        creators["siphon life"] = &WarlockAiObjectContextInternal::siphon_life;
        creators["siphon life on attacker"] = &WarlockAiObjectContextInternal::siphon_life_on_attacker;
        creators["curse of agony"] = &WarlockAiObjectContextInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockAiObjectContextInternal::curse_of_agony_on_attacker;
        creators["shadow bolt"] = &WarlockAiObjectContextInternal::shadow_bolt;
        creators["drain soul"] = &WarlockAiObjectContextInternal::drain_soul;
        creators["drain mana"] = &WarlockAiObjectContextInternal::drain_mana;
        creators["drain life"] = &WarlockAiObjectContextInternal::drain_life;
        creators["banish"] = &WarlockAiObjectContextInternal::banish;
        creators["banish on cc"] = &WarlockAiObjectContextInternal::banish_on_cc;
        creators["seed of corruption"] = &WarlockAiObjectContextInternal::seed_of_corruption;
        creators["seed of corruption on attacker"] = &WarlockAiObjectContextInternal::seed_of_corruption_on_attacker;
        creators["rain of fire"] = &WarlockAiObjectContextInternal::rain_of_fire;
        creators["shadowfury"] = &WarlockAiObjectContextInternal::shadowfury;
        creators["life tap"] = &WarlockAiObjectContextInternal::life_tap;
        creators["fear"] = &WarlockAiObjectContextInternal::fear;
        creators["fear on cc"] = &WarlockAiObjectContextInternal::fear_on_cc;
        creators["incinirate"] = &WarlockAiObjectContextInternal::incinirate;
        creators["conflagrate"] = &WarlockAiObjectContextInternal::conflagrate;
        creators["amplify curse"] = &WarlockAiObjectContextInternal::amplify_curse;

        creators["immolate on attacker"] = &WarlockAiObjectContextInternal::immolate_on_attacker;
        creators["unstable affliction"] = &WarlockAiObjectContextInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockAiObjectContextInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockAiObjectContextInternal::haunt;
        creators["demonic empowerment"] = &WarlockAiObjectContextInternal::demonic_empowerment;
        creators["metamorphosis"] = &WarlockAiObjectContextInternal::metamorphosis;
        creators["soul fire"] = &WarlockAiObjectContextInternal::soul_fire;
        creators["incinerate"] = &WarlockAiObjectContextInternal::incinerate;
    }

private:
    static Action* amplify_curse(PlayerbotAI* botAI) { return new CastAmplifyCurseAction(botAI); }
    static Action* conflagrate(PlayerbotAI* botAI) { return new CastConflagrateAction(botAI); }
    static Action* incinirate(PlayerbotAI* botAI) { return new CastIncinirateAction(botAI); }
    static Action* fear_on_cc(PlayerbotAI* botAI) { return new CastFearOnCcAction(botAI); }
    static Action* fear(PlayerbotAI* botAI) { return new CastFearAction(botAI); }
    static Action* immolate(PlayerbotAI* botAI) { return new CastImmolateAction(botAI); }
    static Action* summon_imp(PlayerbotAI* botAI) { return new CastSummonImpAction(botAI); }
    static Action* summon_succubus(PlayerbotAI* botAI) { return new CastSummonSuccubusAction(botAI); }
    static Action* fel_armor(PlayerbotAI* botAI) { return new CastFelArmorAction(botAI); }
    static Action* demon_armor(PlayerbotAI* botAI) { return new CastDemonArmorAction(botAI); }
    static Action* demon_skin(PlayerbotAI* botAI) { return new CastDemonSkinAction(botAI); }
    static Action* create_healthstone(PlayerbotAI* botAI) { return new CastCreateHealthstoneAction(botAI); }
    static Action* create_firestone(PlayerbotAI* botAI) { return new CastCreateFirestoneAction(botAI); }
    static Action* create_spellstone(PlayerbotAI* botAI) { return new CastCreateSpellstoneAction(botAI); }
    static Action* spellstone(PlayerbotAI* botAI) { return new UseSpellItemAction(botAI, "spellstone", true); }
    static Action* summon_voidwalker(PlayerbotAI* botAI) { return new CastSummonVoidwalkerAction(botAI); }
    static Action* summon_felguard(PlayerbotAI* botAI) { return new CastSummonFelguardAction(botAI); }
    static Action* summon_felhunter(PlayerbotAI* botAI) { return new CastSummonFelhunterAction(botAI); }
    static Action* corruption(PlayerbotAI* botAI) { return new CastCorruptionAction(botAI); }
    static Action* corruption_on_attacker(PlayerbotAI* botAI) { return new CastCorruptionOnAttackerAction(botAI); }
    static Action* siphon_life(PlayerbotAI* botAI) { return new CastSiphonLifeAction(botAI); }
    static Action* siphon_life_on_attacker(PlayerbotAI* botAI) { return new CastSiphonLifeOnAttackerAction(botAI); }
    static Action* curse_of_agony(PlayerbotAI* botAI) { return new CastCurseOfAgonyAction(botAI); }
    static Action* curse_of_agony_on_attacker(PlayerbotAI* botAI)
    {
        return new CastCurseOfAgonyOnAttackerAction(botAI);
    }
    static Action* shadow_bolt(PlayerbotAI* botAI) { return new CastShadowBoltAction(botAI); }
    static Action* drain_soul(PlayerbotAI* botAI) { return new CastDrainSoulAction(botAI); }
    static Action* drain_mana(PlayerbotAI* botAI) { return new CastDrainManaAction(botAI); }
    static Action* drain_life(PlayerbotAI* botAI) { return new CastDrainLifeAction(botAI); }
    static Action* banish(PlayerbotAI* botAI) { return new CastBanishAction(botAI); }
    static Action* banish_on_cc(PlayerbotAI* botAI) { return new CastBanishAction(botAI); }
    static Action* seed_of_corruption(PlayerbotAI* botAI) { return new CastSeedOfCorruptionAction(botAI); }
    static Action* seed_of_corruption_on_attacker(PlayerbotAI* botAI)
    {
        return new CastSeedOfCorruptionOnAttackerAction(botAI);
    }
    static Action* rain_of_fire(PlayerbotAI* botAI) { return new CastRainOfFireAction(botAI); }
    static Action* shadowfury(PlayerbotAI* botAI) { return new CastShadowfuryAction(botAI); }
    static Action* life_tap(PlayerbotAI* botAI) { return new CastLifeTapAction(botAI); }
    static Action* immolate_on_attacker(PlayerbotAI* ai) { return new CastImmolateOnAttackerAction(ai); }
    static Action* unstable_affliction(PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); }
    static Action* unstable_affliction_on_attacker(PlayerbotAI* ai)
    {
        return new CastUnstableAfflictionOnAttackerAction(ai);
    }
    static Action* haunt(PlayerbotAI* ai) { return new CastHauntAction(ai); }
    static Action* demonic_empowerment(PlayerbotAI* ai) { return new CastDemonicEmpowermentAction(ai); }
    static Action* metamorphosis(PlayerbotAI* ai) { return new CastMetamorphosisAction(ai); }
    static Action* soul_fire(PlayerbotAI* ai) { return new CastSoulFireAction(ai); }
    static Action* incinerate(PlayerbotAI* ai) { return new CastIncinerateAction(ai); }
};

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarlockStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCombatStrategyFactoryInternal());
    strategyContexts.Add(new NonCombatBuffStrategyFactoryInternal());
    actionContexts.Add(new WarlockAiObjectContextInternal());
    triggerContexts.Add(new WarlockTriggerFactoryInternal());
}
