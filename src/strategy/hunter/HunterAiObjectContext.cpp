/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HunterAiObjectContext.h"

#include "BeastMasteryHunterStrategy.h"
#include "GenericHunterNonCombatStrategy.h"
#include "GenericHunterStrategy.h"
#include "HunterActions.h"
#include "HunterBuffStrategies.h"
#include "HunterTriggers.h"
#include "MarksmanshipHunterStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "SurvivalHunterStrategy.h"

class HunterStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    HunterStrategyFactoryInternal()
    {
        creators["nc"] = &HunterStrategyFactoryInternal::nc;
        creators["boost"] = &HunterStrategyFactoryInternal::boost;
        creators["pet"] = &HunterStrategyFactoryInternal::pet;
        creators["cc"] = &HunterStrategyFactoryInternal::cc;
        creators["trap weave"] = &HunterStrategyFactoryInternal::trap_weave;
        creators["bm"] = &HunterStrategyFactoryInternal::beast_mastery;
        creators["mm"] = &HunterStrategyFactoryInternal::marksmanship;
        creators["surv"] = &HunterStrategyFactoryInternal::survival;
        creators["aoe"] = &HunterStrategyFactoryInternal::aoe;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericHunterNonCombatStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new HunterBoostStrategy(botAI); }
    static Strategy* pet(PlayerbotAI* botAI) { return new HunterPetStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new HunterCcStrategy(botAI); }
    static Strategy* trap_weave(PlayerbotAI* botAI) { return new HunterTrapWeaveStrategy(botAI); }
    static Strategy* beast_mastery(PlayerbotAI* botAI) { return new BeastMasteryHunterStrategy(botAI); }
    static Strategy* marksmanship(PlayerbotAI* botAI) { return new MarksmanshipHunterStrategy(botAI); }
    static Strategy* survival(PlayerbotAI* botAI) { return new SurvivalHunterStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new AoEHunterStrategy(botAI); }
};

class HunterBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    HunterBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bspeed"] = &HunterBuffStrategyFactoryInternal::bspeed;
        creators["bdps"] = &HunterBuffStrategyFactoryInternal::bdps;
        creators["bmana"] = &HunterBuffStrategyFactoryInternal::bmana;
        creators["rnature"] = &HunterBuffStrategyFactoryInternal::rnature;
    }

private:
    static Strategy* bspeed(PlayerbotAI* botAI) { return new HunterBuffSpeedStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new HunterBuffDpsStrategy(botAI); }
    static Strategy* bmana(PlayerbotAI* botAI) { return new HunterBuffManaStrategy(botAI); }
    static Strategy* rnature(PlayerbotAI* botAI) { return new HunterNatureResistanceStrategy(botAI); }
};

class HunterTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    HunterTriggerFactoryInternal()
    {
        creators["aspect of the viper"] = &HunterTriggerFactoryInternal::aspect_of_the_viper;
        creators["black arrow"] = &HunterTriggerFactoryInternal::black_arrow;
        creators["no stings"] = &HunterTriggerFactoryInternal::NoStings;
        creators["hunters pet dead"] = &HunterTriggerFactoryInternal::hunters_pet_dead;
        creators["hunters pet low health"] = &HunterTriggerFactoryInternal::hunters_pet_low_health;
        creators["hunters pet medium health"] = &HunterTriggerFactoryInternal::hunters_pet_medium_health;
        creators["hunter's mark"] = &HunterTriggerFactoryInternal::hunters_mark;
        creators["freezing trap"] = &HunterTriggerFactoryInternal::freezing_trap;
        creators["aspect of the pack"] = &HunterTriggerFactoryInternal::aspect_of_the_pack;
        creators["rapid fire"] = &HunterTriggerFactoryInternal::rapid_fire;
        creators["aspect of the hawk"] = &HunterTriggerFactoryInternal::aspect_of_the_hawk;
        creators["aspect of the monkey"] = &HunterTriggerFactoryInternal::aspect_of_the_monkey;
        creators["aspect of the wild"] = &HunterTriggerFactoryInternal::aspect_of_the_wild;
        creators["aspect of the viper"] = &HunterTriggerFactoryInternal::aspect_of_the_viper;
        creators["trueshot aura"] = &HunterTriggerFactoryInternal::trueshot_aura;
        creators["no track"] = &HunterTriggerFactoryInternal::no_track;
        creators["serpent sting on attacker"] = &HunterTriggerFactoryInternal::serpent_sting_on_attacker;
        creators["pet not happy"] = &HunterTriggerFactoryInternal::pet_not_happy;
        creators["concussive shot on snare target"] = &HunterTriggerFactoryInternal::concussive_shot_on_snare_target;
        creators["scare beast"] = &HunterTriggerFactoryInternal::scare_beast;
        creators["low ammo"] = &HunterTriggerFactoryInternal::low_ammo;
        creators["no ammo"] = &HunterTriggerFactoryInternal::no_ammo;
        creators["has ammo"] = &HunterTriggerFactoryInternal::has_ammo;
        creators["switch to melee"] = &HunterTriggerFactoryInternal::switch_to_melee;
        creators["switch to ranged"] = &HunterTriggerFactoryInternal::switch_to_ranged;
        creators["misdirection on main tank"] = &HunterTriggerFactoryInternal::misdirection_on_main_tank;
        creators["tranquilizing shot enrage"] = &HunterTriggerFactoryInternal::remove_enrage;
        creators["tranquilizing shot magic"] = &HunterTriggerFactoryInternal::remove_magic;
        creators["immolation trap no cd"] = &HunterTriggerFactoryInternal::immolation_trap_no_cd;
        creators["kill command"] = &HunterTriggerFactoryInternal::kill_command;
        creators["explosive shot"] = &HunterTriggerFactoryInternal::explosive_shot;
        creators["lock and load"] = &HunterTriggerFactoryInternal::lock_and_load;
        creators["silencing shot"] = &HunterTriggerFactoryInternal::silencing_shot;
        creators["intimidation"] = &HunterTriggerFactoryInternal::intimidation;
        creators["volley channel check"] = &HunterTriggerFactoryInternal::volley_channel_check;
    }

private:
    static Trigger* auto_shot(PlayerbotAI* botAI) { return new AutoShotTrigger(botAI); }
    static Trigger* scare_beast(PlayerbotAI* botAI) { return new ScareBeastTrigger(botAI); }
    static Trigger* concussive_shot_on_snare_target(PlayerbotAI* botAI)
    {
        return new ConsussiveShotSnareTrigger(botAI);
    }
    static Trigger* pet_not_happy(PlayerbotAI* botAI) { return new HunterPetNotHappy(botAI); }
    static Trigger* serpent_sting_on_attacker(PlayerbotAI* botAI) { return new SerpentStingOnAttackerTrigger(botAI); }
    static Trigger* trueshot_aura(PlayerbotAI* botAI) { return new TrueshotAuraTrigger(botAI); }
    static Trigger* no_track(PlayerbotAI* botAI) { return new NoTrackTrigger(botAI); }
    static Trigger* aspect_of_the_viper(PlayerbotAI* botAI) { return new HunterAspectOfTheViperTrigger(botAI); }
    static Trigger* black_arrow(PlayerbotAI* botAI) { return new BlackArrowTrigger(botAI); }
    static Trigger* NoStings(PlayerbotAI* botAI) { return new HunterNoStingsActiveTrigger(botAI); }
    static Trigger* hunters_pet_dead(PlayerbotAI* botAI) { return new HuntersPetDeadTrigger(botAI); }
    static Trigger* hunters_pet_low_health(PlayerbotAI* botAI) { return new HuntersPetLowHealthTrigger(botAI); }
    static Trigger* hunters_pet_medium_health(PlayerbotAI* botAI) { return new HuntersPetMediumHealthTrigger(botAI); }
    static Trigger* hunters_mark(PlayerbotAI* botAI) { return new HuntersMarkTrigger(botAI); }
    static Trigger* freezing_trap(PlayerbotAI* botAI) { return new FreezingTrapTrigger(botAI); }
    static Trigger* aspect_of_the_pack(PlayerbotAI* botAI) { return new HunterAspectOfThePackTrigger(botAI); }
    static Trigger* rapid_fire(PlayerbotAI* botAI) { return new RapidFireTrigger(botAI); }
    static Trigger* aspect_of_the_hawk(PlayerbotAI* botAI) { return new HunterAspectOfTheHawkTrigger(botAI); }
    static Trigger* aspect_of_the_monkey(PlayerbotAI* botAI) { return new HunterAspectOfTheMonkeyTrigger(botAI); }
    static Trigger* aspect_of_the_wild(PlayerbotAI* botAI) { return new HunterAspectOfTheWildTrigger(botAI); }
    static Trigger* low_ammo(PlayerbotAI* botAI) { return new HunterLowAmmoTrigger(botAI); }
    static Trigger* no_ammo(PlayerbotAI* botAI) { return new HunterNoAmmoTrigger(botAI); }
    static Trigger* has_ammo(PlayerbotAI* botAI) { return new HunterHasAmmoTrigger(botAI); }
    static Trigger* switch_to_melee(PlayerbotAI* botAI) { return new SwitchToMeleeTrigger(botAI); }
    static Trigger* switch_to_ranged(PlayerbotAI* botAI) { return new SwitchToRangedTrigger(botAI); }
    static Trigger* misdirection_on_main_tank(PlayerbotAI* ai) { return new MisdirectionOnMainTankTrigger(ai); }
    static Trigger* remove_enrage(PlayerbotAI* ai) { return new TargetRemoveEnrageTrigger(ai); }
    static Trigger* remove_magic(PlayerbotAI* ai) { return new TargetRemoveMagicTrigger(ai); }
    static Trigger* immolation_trap_no_cd(PlayerbotAI* ai) { return new ImmolationTrapNoCdTrigger(ai); }
    static Trigger* kill_command(PlayerbotAI* botAI) { return new KillCommandTrigger(botAI); }
    static Trigger* explosive_shot(PlayerbotAI* botAI) { return new ExplosiveShotTrigger(botAI); }
    static Trigger* lock_and_load(PlayerbotAI* botAI) { return new LockAndLoadTrigger(botAI); }
    static Trigger* silencing_shot(PlayerbotAI* botAI) { return new SilencingShotTrigger(botAI); }
    static Trigger* intimidation(PlayerbotAI* botAI) { return new IntimidationTrigger(botAI); }
    static Trigger* volley_channel_check(PlayerbotAI* botAI) { return new VolleyChannelCheckTrigger(botAI); }
};

class HunterAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    HunterAiObjectContextInternal()
    {
        creators["auto shot"] = &HunterAiObjectContextInternal::auto_shot;
        creators["aimed shot"] = &HunterAiObjectContextInternal::aimed_shot;
        creators["chimera shot"] = &HunterAiObjectContextInternal::chimera_shot;
        creators["explosive shot"] = &HunterAiObjectContextInternal::explosive_shot;
        creators["arcane shot"] = &HunterAiObjectContextInternal::arcane_shot;
        creators["concussive shot"] = &HunterAiObjectContextInternal::concussive_shot;
        creators["distracting shot"] = &HunterAiObjectContextInternal::distracting_shot;
        creators["multi-shot"] = &HunterAiObjectContextInternal::multi_shot;
        creators["volley"] = &HunterAiObjectContextInternal::volley;
        creators["serpent sting"] = &HunterAiObjectContextInternal::serpent_sting;
        creators["serpent sting on attacker"] = &HunterAiObjectContextInternal::serpent_sting_on_attacker;
        creators["wyvern sting"] = &HunterAiObjectContextInternal::wyvern_sting;
        creators["viper sting"] = &HunterAiObjectContextInternal::viper_sting;
        creators["scorpid sting"] = &HunterAiObjectContextInternal::scorpid_sting;
        creators["hunter's mark"] = &HunterAiObjectContextInternal::hunters_mark;
        creators["mend pet"] = &HunterAiObjectContextInternal::mend_pet;
        creators["kill command"] = &HunterAiObjectContextInternal::kill_command;
        creators["revive pet"] = &HunterAiObjectContextInternal::revive_pet;
        creators["call pet"] = &HunterAiObjectContextInternal::call_pet;
        creators["black arrow"] = &HunterAiObjectContextInternal::black_arrow;
        creators["freezing trap"] = &HunterAiObjectContextInternal::freezing_trap;
        creators["rapid fire"] = &HunterAiObjectContextInternal::rapid_fire;
        creators["boost"] = &HunterAiObjectContextInternal::rapid_fire;
        creators["deterrence"] = &HunterAiObjectContextInternal::deterrence;
        creators["readiness"] = &HunterAiObjectContextInternal::readiness;
        creators["aspect of the hawk"] = &HunterAiObjectContextInternal::aspect_of_the_hawk;
        creators["aspect of the monkey"] = &HunterAiObjectContextInternal::aspect_of_the_monkey;
        creators["aspect of the wild"] = &HunterAiObjectContextInternal::aspect_of_the_wild;
        creators["aspect of the viper"] = &HunterAiObjectContextInternal::aspect_of_the_viper;
        creators["aspect of the pack"] = &HunterAiObjectContextInternal::aspect_of_the_pack;
        creators["aspect of the cheetah"] = &HunterAiObjectContextInternal::aspect_of_the_cheetah;
        creators["trueshot aura"] = &HunterAiObjectContextInternal::trueshot_aura;
        creators["track humanoids"] = &HunterAiObjectContextInternal::track_humanoids;
        creators["feign death"] = &HunterAiObjectContextInternal::feign_death;
        creators["wing clip"] = &HunterAiObjectContextInternal::wing_clip;
        creators["raptor strike"] = &HunterAiObjectContextInternal::raptor_strike;
        creators["mongoose bite"] = &HunterAiObjectContextInternal::mongoose_bite;
        creators["feed pet"] = &HunterAiObjectContextInternal::feed_pet;
        creators["bestial wrath"] = &HunterAiObjectContextInternal::bestial_wrath;
        creators["scare beast"] = &HunterAiObjectContextInternal::scare_beast;
        creators["scare beast on cc"] = &HunterAiObjectContextInternal::scare_beast_on_cc;
        creators["aspect of the dragonhawk"] = &HunterAiObjectContextInternal::aspect_of_the_dragonhawk;
        creators["tranquilizing shot"] = &HunterAiObjectContextInternal::tranquilizing_shot;
        creators["steady shot"] = &HunterAiObjectContextInternal::steady_shot;
        creators["kill shot"] = &HunterAiObjectContextInternal::kill_shot;
        creators["misdirection on main tank"] = &HunterAiObjectContextInternal::misdirection_on_main_tank;
        creators["silencing shot"] = &HunterAiObjectContextInternal::silencing_shot;
        creators["disengage"] = &HunterAiObjectContextInternal::disengage;
        creators["immolation trap"] = &HunterAiObjectContextInternal::immolation_trap;
        creators["explosive trap"] = &HunterAiObjectContextInternal::explosive_trap;
        creators["explosive shot rank 4"] = &HunterAiObjectContextInternal::explosive_shot_rank_4;
        creators["explosive shot rank 3"] = &HunterAiObjectContextInternal::explosive_shot_rank_3;
        creators["explosive shot rank 2"] = &HunterAiObjectContextInternal::explosive_shot_rank_2;
        creators["explosive shot rank 1"] = &HunterAiObjectContextInternal::explosive_shot_rank_1;
        creators["intimidation"] = &HunterAiObjectContextInternal::intimidation;
    }

private:
    static Action* scare_beast(PlayerbotAI* botAI) { return new CastScareBeastAction(botAI); }
    static Action* scare_beast_on_cc(PlayerbotAI* botAI) { return new CastScareBeastCcAction(botAI); }
    static Action* bestial_wrath(PlayerbotAI* botAI) { return new CastBestialWrathAction(botAI); }
    static Action* feed_pet(PlayerbotAI* botAI) { return new FeedPetAction(botAI); }
    static Action* feign_death(PlayerbotAI* botAI) { return new CastFeignDeathAction(botAI); }
    static Action* trueshot_aura(PlayerbotAI* botAI) { return new CastTrueshotAuraAction(botAI); }
    static Action* track_humanoids(PlayerbotAI* botAI) { return new CastBuffSpellAction(botAI, "track humanoids"); }
    static Action* auto_shot(PlayerbotAI* botAI) { return new CastAutoShotAction(botAI); }
    static Action* aimed_shot(PlayerbotAI* botAI) { return new CastAimedShotAction(botAI); }
    static Action* chimera_shot(PlayerbotAI* botAI) { return new CastChimeraShotAction(botAI); }
    static Action* explosive_shot(PlayerbotAI* botAI) { return new CastExplosiveShotAction(botAI); }
    static Action* arcane_shot(PlayerbotAI* botAI) { return new CastArcaneShotAction(botAI); }
    static Action* concussive_shot(PlayerbotAI* botAI) { return new CastConcussiveShotAction(botAI); }
    static Action* distracting_shot(PlayerbotAI* botAI) { return new CastDistractingShotAction(botAI); }
    static Action* multi_shot(PlayerbotAI* botAI) { return new CastMultiShotAction(botAI); }
    static Action* volley(PlayerbotAI* botAI) { return new CastVolleyAction(botAI); }
    static Action* serpent_sting(PlayerbotAI* botAI) { return new CastSerpentStingAction(botAI); }
    static Action* serpent_sting_on_attacker(PlayerbotAI* botAI) { return new CastSerpentStingOnAttackerAction(botAI); }
    static Action* wyvern_sting(PlayerbotAI* botAI) { return new CastWyvernStingAction(botAI); }
    static Action* viper_sting(PlayerbotAI* botAI) { return new CastViperStingAction(botAI); }
    static Action* scorpid_sting(PlayerbotAI* botAI) { return new CastScorpidStingAction(botAI); }
    static Action* hunters_mark(PlayerbotAI* botAI) { return new CastHuntersMarkAction(botAI); }
    static Action* mend_pet(PlayerbotAI* botAI) { return new CastMendPetAction(botAI); }
    static Action* kill_command(PlayerbotAI* botAI) { return new CastKillCommandAction(botAI); }
    static Action* revive_pet(PlayerbotAI* botAI) { return new CastRevivePetAction(botAI); }
    static Action* call_pet(PlayerbotAI* botAI) { return new CastCallPetAction(botAI); }
    static Action* black_arrow(PlayerbotAI* botAI) { return new CastBlackArrow(botAI); }
    static Action* freezing_trap(PlayerbotAI* botAI) { return new CastFreezingTrap(botAI); }
    static Action* rapid_fire(PlayerbotAI* botAI) { return new CastRapidFireAction(botAI); }
    static Action* deterrence(PlayerbotAI* botAI) { return new CastDeterrenceAction(botAI); }
    static Action* readiness(PlayerbotAI* botAI) { return new CastReadinessAction(botAI); }
    static Action* aspect_of_the_hawk(PlayerbotAI* botAI) { return new CastAspectOfTheHawkAction(botAI); }
    static Action* aspect_of_the_monkey(PlayerbotAI* botAI) { return new CastAspectOfTheMonkeyAction(botAI); }
    static Action* aspect_of_the_wild(PlayerbotAI* botAI) { return new CastAspectOfTheWildAction(botAI); }
    static Action* aspect_of_the_viper(PlayerbotAI* botAI) { return new CastAspectOfTheViperAction(botAI); }
    static Action* aspect_of_the_pack(PlayerbotAI* botAI) { return new CastAspectOfThePackAction(botAI); }
    static Action* aspect_of_the_cheetah(PlayerbotAI* botAI) { return new CastAspectOfTheCheetahAction(botAI); }
    static Action* wing_clip(PlayerbotAI* botAI) { return new CastWingClipAction(botAI); }
    static Action* raptor_strike(PlayerbotAI* botAI) { return new CastRaptorStrikeAction(botAI); }
    static Action* mongoose_bite(PlayerbotAI* botAI) { return new CastMongooseBiteAction(botAI); }
    static Action* aspect_of_the_dragonhawk(PlayerbotAI* ai) { return new CastAspectOfTheDragonhawkAction(ai); }
    static Action* tranquilizing_shot(PlayerbotAI* ai) { return new CastTranquilizingShotAction(ai); }
    static Action* steady_shot(PlayerbotAI* ai) { return new CastSteadyShotAction(ai); }
    static Action* kill_shot(PlayerbotAI* ai) { return new CastKillShotAction(ai); }
    static Action* misdirection_on_main_tank(PlayerbotAI* ai) { return new CastMisdirectionOnMainTankAction(ai); }
    static Action* silencing_shot(PlayerbotAI* ai) { return new CastSilencingShotAction(ai); }
    static Action* disengage(PlayerbotAI* ai) { return new CastDisengageAction(ai); }
    static Action* immolation_trap(PlayerbotAI* ai) { return new CastImmolationTrapAction(ai); }
    static Action* explosive_trap(PlayerbotAI* ai) { return new CastExplosiveTrapAction(ai); }
    static Action* explosive_shot_rank_4(PlayerbotAI* ai) { return new CastExplosiveShotRank4Action(ai); }
    static Action* explosive_shot_rank_3(PlayerbotAI* ai) { return new CastExplosiveShotRank3Action(ai); }
    static Action* explosive_shot_rank_2(PlayerbotAI* ai) { return new CastExplosiveShotRank2Action(ai); }
    static Action* explosive_shot_rank_1(PlayerbotAI* ai) { return new CastExplosiveShotRank1Action(ai); }
    static Action* intimidation(PlayerbotAI* ai) { return new CastIntimidationAction(ai); }
};

SharedNamedObjectContextList<Strategy> HunterAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Action> HunterAiObjectContext::sharedActionContexts;
SharedNamedObjectContextList<Trigger> HunterAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> HunterAiObjectContext::sharedValueContexts;

HunterAiObjectContext::HunterAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedActionContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void HunterAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedActionContexts(sharedActionContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void HunterAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new HunterStrategyFactoryInternal());
    strategyContexts.Add(new HunterBuffStrategyFactoryInternal());
}

void HunterAiObjectContext::BuildSharedActionContexts(SharedNamedObjectContextList<Action>& actionContexts)
{
    AiObjectContext::BuildSharedActionContexts(actionContexts);
    actionContexts.Add(new HunterAiObjectContextInternal());
}

void HunterAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new HunterTriggerFactoryInternal());
}

void HunterAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
