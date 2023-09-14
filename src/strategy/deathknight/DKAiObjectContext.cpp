/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DKAiObjectContext.h"
#include "BloodDKStrategy.h"
#include "DKActions.h"
#include "DKTriggers.h"
#include "FrostDKStrategy.h"
#include "GenericDKNonCombatStrategy.h"
#include "UnholyDKStrategy.h"
#include "Playerbots.h"
#include "PullStrategy.h"

class DeathKnightStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        DeathKnightStrategyFactoryInternal()
        {
            creators["nc"] = &DeathKnightStrategyFactoryInternal::nc;
            creators["pull"] = &DeathKnightStrategyFactoryInternal::pull;
		    creators["frost aoe"] = &DeathKnightStrategyFactoryInternal::frost_aoe;
		    creators["unholy aoe"] = &DeathKnightStrategyFactoryInternal::unholy_aoe;
        }

    private:
        static Strategy* nc(PlayerbotAI* botAI) { return new GenericDKNonCombatStrategy(botAI); }
        static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "icy touch"); }
	    static Strategy* frost_aoe(PlayerbotAI* botAI) { return new FrostDKAoeStrategy(botAI); }
	    static Strategy* unholy_aoe(PlayerbotAI* botAI) { return new UnholyDKAoeStrategy(botAI); }
};

class DeathKnightCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
		DeathKnightCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
        {
            creators["tank"] = &DeathKnightCombatStrategyFactoryInternal::blood;
			creators["blood"] = &DeathKnightCombatStrategyFactoryInternal::blood;
            creators["frost"] = &DeathKnightCombatStrategyFactoryInternal::frost_dps;
			creators["unholy"] = &DeathKnightCombatStrategyFactoryInternal::unholy_dps;
        }

    private:
        static Strategy* frost_dps(PlayerbotAI* botAI) { return new FrostDKStrategy(botAI); }
        static Strategy* unholy_dps(PlayerbotAI* botAI) { return new UnholyDKStrategy(botAI); }
        static Strategy* tank(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
		static Strategy* blood(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
};

class DeathKnightDKBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        DeathKnightDKBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
        {

            creators["bdps"] = &DeathKnightDKBuffStrategyFactoryInternal::bdps;
        }

    private:
        static Strategy* bdps(PlayerbotAI* botAI) { return new DKBuffDpsStrategy(botAI); }
};

class DeathKnightTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
    public:
        DeathKnightTriggerFactoryInternal()
        {
            creators["bone shield"] = &DeathKnightTriggerFactoryInternal::bone_shield;
            creators["pestilence"] = &DeathKnightTriggerFactoryInternal::pestilence;
            creators["blood strike"] = &DeathKnightTriggerFactoryInternal::blood_strike;
			creators["plague strike"] = &DeathKnightTriggerFactoryInternal::plague_strike;
			creators["plague strike on attacker"] = &DeathKnightTriggerFactoryInternal::plague_strike_on_attacker;
            creators["icy touch"] = &DeathKnightTriggerFactoryInternal::icy_touch;
			creators["death coil"] = &DeathKnightTriggerFactoryInternal::death_coil;
			creators["icy touch on attacker"] = &DeathKnightTriggerFactoryInternal::icy_touch_on_attacker;
            creators["improved icy talons"] = &DeathKnightTriggerFactoryInternal::improved_icy_talons;
            creators["plague strike"] = &DeathKnightTriggerFactoryInternal::plague_strike;
            creators["horn of winter"] = &DeathKnightTriggerFactoryInternal::horn_of_winter;
            creators["mind freeze"] = &DeathKnightTriggerFactoryInternal::mind_freeze;
            creators["mind freeze on enemy healer"] = &DeathKnightTriggerFactoryInternal::mind_freeze_on_enemy_healer;
			creators["strangulate"] = &DeathKnightTriggerFactoryInternal::strangulate;
			creators["strangulate on enemy healer"] = &DeathKnightTriggerFactoryInternal::strangulate_on_enemy_healer;
			creators["blood tap"] = &DeathKnightTriggerFactoryInternal::blood_tap;
			creators["raise dead"] = &DeathKnightTriggerFactoryInternal::raise_dead;
			creators["chains of ice"] = &DeathKnightTriggerFactoryInternal::chains_of_ice;
			creators["unbreakable armor"] = &DeathKnightTriggerFactoryInternal::unbreakable_armor;
			creators["high blood rune"] = &DeathKnightTriggerFactoryInternal::high_blood_rune;
			creators["freezing fog"] = &DeathKnightTriggerFactoryInternal::freezing_fog;
        }

    private:
        static Trigger* bone_shield(PlayerbotAI* botAI) { return new BoneShieldTrigger(botAI); }
        static Trigger* pestilence(PlayerbotAI* botAI) { return new PestilenceTrigger(botAI); }
        static Trigger* blood_strike(PlayerbotAI* botAI) { return new BloodStrikeTrigger(botAI); }
		static Trigger* plague_strike(PlayerbotAI* botAI) { return new PlagueStrikeDebuffTrigger(botAI); }
		static Trigger* plague_strike_on_attacker(PlayerbotAI* botAI) { return new PlagueStrikeDebuffOnAttackerTrigger(botAI); }
        static Trigger* icy_touch(PlayerbotAI* botAI) { return new IcyTouchDebuffTrigger(botAI); }
		static Trigger* death_coil(PlayerbotAI* botAI) { return new DeathCoilTrigger(botAI); }
		static Trigger* icy_touch_on_attacker(PlayerbotAI* botAI) { return new IcyTouchDebuffOnAttackerTrigger(botAI); }
        static Trigger* improved_icy_talons(PlayerbotAI* botAI) { return new ImprovedIcyTalonsTrigger(botAI); }
        static Trigger* horn_of_winter(PlayerbotAI* botAI) { return new HornOfWinterTrigger(botAI); }
        static Trigger* mind_freeze(PlayerbotAI* botAI) { return new MindFreezeInterruptSpellTrigger(botAI); }
        static Trigger* mind_freeze_on_enemy_healer(PlayerbotAI* botAI) { return new MindFreezeOnEnemyHealerTrigger(botAI); }
		static Trigger* strangulate(PlayerbotAI* botAI) { return new StrangulateInterruptSpellTrigger(botAI); }
		static Trigger* strangulate_on_enemy_healer(PlayerbotAI* botAI) { return new StrangulateOnEnemyHealerTrigger(botAI); }
		static Trigger* blood_tap(PlayerbotAI* botAI) { return new BloodTapTrigger(botAI); }
		static Trigger* raise_dead(PlayerbotAI* botAI) { return new RaiseDeadTrigger(botAI); }
		static Trigger* chains_of_ice(PlayerbotAI* botAI) { return new ChainsOfIceSnareTrigger(botAI); }
		static Trigger* unbreakable_armor(PlayerbotAI* botAI) { return new UnbreakableArmorTrigger(botAI); }
		static Trigger* high_blood_rune(PlayerbotAI* botAI) { return new HighBloodRuneTrigger(botAI); }
		static Trigger* freezing_fog(PlayerbotAI* botAI) { return new FreezingFogTrigger(botAI); }
		
};

class DeathKnightAiObjectContextInternal : public NamedObjectContext<Action>
{
    public:
        DeathKnightAiObjectContextInternal()
        {
			// Unholy
			creators["bone shield"] = &DeathKnightAiObjectContextInternal::bone_shield;
			creators["plague strike"] = &DeathKnightAiObjectContextInternal::plague_strike;
			creators["plague strike on attacker"] = &DeathKnightAiObjectContextInternal::plague_strike_on_attacker;
			creators["death grip"] = &DeathKnightAiObjectContextInternal::death_grip;
			creators["death coil"] = &DeathKnightAiObjectContextInternal::death_coil;
			creators["death strike"] = &DeathKnightAiObjectContextInternal::death_strike;
			creators["unholy blight"] = &DeathKnightAiObjectContextInternal::unholy_blight;
			creators["scourge strike"] = &DeathKnightAiObjectContextInternal::scourge_strike;
			creators["death and decay"] = &DeathKnightAiObjectContextInternal::death_and_decay;
			creators["unholy pressence"] = &DeathKnightAiObjectContextInternal::unholy_pressence;
			creators["raise dead"] = &DeathKnightAiObjectContextInternal::raise_dead;
			creators["army of the dead"] = &DeathKnightAiObjectContextInternal::army_of_the_dead;
			creators["summon gargoyle"] = &DeathKnightAiObjectContextInternal::summon_gargoyle;
			creators["anti magic shell"] = &DeathKnightAiObjectContextInternal::anti_magic_shell;
			creators["anti magic zone"] = &DeathKnightAiObjectContextInternal::anti_magic_zone;
			creators["ghoul frenzy"] = &DeathKnightAiObjectContextInternal::ghoul_frenzy;
			creators["corpse explosion"] = &DeathKnightAiObjectContextInternal::corpse_explosion;
			// Frost
			creators["icy touch"] = &DeathKnightAiObjectContextInternal::icy_touch;
			creators["icy touch on attacker"] = &DeathKnightAiObjectContextInternal::icy_touch_on_attacker;
			creators["obliterate"] = &DeathKnightAiObjectContextInternal::obliterate;
			creators["howling blast"] = &DeathKnightAiObjectContextInternal::howling_blast;
			creators["frost strike"] = &DeathKnightAiObjectContextInternal::frost_strike;
			creators["chains of ice"] = &DeathKnightAiObjectContextInternal::chains_of_ice;
			creators["rune strike"] = &DeathKnightAiObjectContextInternal::rune_strike;
			//creators["icy clutch"] = &DeathKnightAiObjectContextInternal::icy_clutch;
			creators["horn of winter"] = &DeathKnightAiObjectContextInternal::horn_of_winter;
			creators["killing machine"] = &DeathKnightAiObjectContextInternal::killing_machine;
			creators["frost presence"] = &DeathKnightAiObjectContextInternal::frost_presence;
			creators["deathchill"] = &DeathKnightAiObjectContextInternal::deathchill;
			creators["icebound fortitude"] = &DeathKnightAiObjectContextInternal::icebound_fortitude;
			creators["mind freeze"] = &DeathKnightAiObjectContextInternal::mind_freeze;
			creators["empower rune weapon"] = &DeathKnightAiObjectContextInternal::empower_rune_weapon;
			creators["hungering cold"] = &DeathKnightAiObjectContextInternal::hungering_cold;
			creators["unbreakable armor"] = &DeathKnightAiObjectContextInternal::unbreakable_armor;
			creators["improved icy talons"] = &DeathKnightAiObjectContextInternal::improved_icy_talons;
			// blood
			creators["blood strike"] = &DeathKnightAiObjectContextInternal::blood_strike;
			creators["blood tap"] = &DeathKnightAiObjectContextInternal::blood_tap;
			creators["pestilence"] = &DeathKnightAiObjectContextInternal::pestilence;
			creators["strangulate"] = &DeathKnightAiObjectContextInternal::strangulate;
			creators["blood boil"] = &DeathKnightAiObjectContextInternal::blood_boil;
			creators["heart strike"] = &DeathKnightAiObjectContextInternal::heart_strike;
			creators["mark of_blood"] = &DeathKnightAiObjectContextInternal::mark_of_blood;
			creators["blood presence"] = &DeathKnightAiObjectContextInternal::blood_presence;
			creators["rune tap"] = &DeathKnightAiObjectContextInternal::rune_tap;
			creators["vampiric blood"] = &DeathKnightAiObjectContextInternal::vampiric_blood;
			creators["death pact"] = &DeathKnightAiObjectContextInternal::death_pact;
			creators["death rune_mastery"] = &DeathKnightAiObjectContextInternal::death_rune_mastery;
			//creators["hysteria"] = &DeathKnightAiObjectContextInternal::hysteria;
			creators["dancing rune weapon"] = &DeathKnightAiObjectContextInternal::dancing_rune_weapon;
			creators["dark command"] = &DeathKnightAiObjectContextInternal::dark_command;
        }

    private:

		// Unholy
		static Action* bone_shield(PlayerbotAI* botAI) { return new CastBoneShieldAction(botAI); }
		static Action* plague_strike(PlayerbotAI* botAI) { return new CastPlagueStrikeAction(botAI); }
		static Action* plague_strike_on_attacker(PlayerbotAI* botAI) { return new CastPlagueStrikeOnAttackerAction(botAI); }
		static Action* death_grip(PlayerbotAI* botAI) { return new CastDeathGripAction(botAI); }
		static Action* death_coil(PlayerbotAI* botAI) { return new CastDeathCoilAction(botAI); }
		static Action* death_strike(PlayerbotAI* botAI) { return new CastDeathStrikeAction(botAI); }
		static Action* unholy_blight(PlayerbotAI* botAI) { return new CastUnholyBlightAction(botAI); }
		static Action* scourge_strike(PlayerbotAI* botAI) { return new CastScourgeStrikeAction(botAI); }
		static Action* death_and_decay(PlayerbotAI* botAI) { return new CastDeathAndDecayAction(botAI); }
		static Action* unholy_pressence(PlayerbotAI* botAI) { return new CastUnholyPresenceAction(botAI); }
		static Action* raise_dead(PlayerbotAI* botAI) { return new CastRaiseDeadAction(botAI); }
		static Action* army_of_the_dead(PlayerbotAI* botAI) { return new CastArmyOfTheDeadAction(botAI); }
		static Action* summon_gargoyle(PlayerbotAI* botAI) { return new CastSummonGargoyleAction(botAI); }
		static Action* anti_magic_shell(PlayerbotAI* botAI) { return new CastAntiMagicShellAction(botAI); }
		static Action* anti_magic_zone(PlayerbotAI* botAI) { return new CastAntiMagicZoneAction(botAI); }
		static Action* ghoul_frenzy(PlayerbotAI* botAI) { return new CastGhoulFrenzyAction(botAI); }
		static Action* corpse_explosion(PlayerbotAI* botAI) { return new CastCorpseExplosionAction(botAI); }
		// Frost
		static Action* icy_touch(PlayerbotAI* botAI) { return new CastIcyTouchAction(botAI); }
		static Action* icy_touch_on_attacker(PlayerbotAI* botAI) { return new CastIcyTouchOnAttackerAction(botAI); }
		static Action* obliterate(PlayerbotAI* botAI) { return new CastObliterateAction(botAI); }
		static Action* howling_blast(PlayerbotAI* botAI) { return new CastHowlingBlastAction(botAI); }
		static Action* frost_strike(PlayerbotAI* botAI) { return new CastFrostStrikeAction(botAI); }
		static Action* chains_of_ice(PlayerbotAI* botAI) { return new CastChainsOfIceAction(botAI); }
		static Action* rune_strike(PlayerbotAI* botAI) { return new CastRuneStrikeAction(botAI); }
		//static Action* icy_clutch(PlayerbotAI* botAI) { return new CastIcyClutchAction(botAI); }
		static Action* horn_of_winter(PlayerbotAI* botAI) { return new CastHornOfWinterAction(botAI); }
		static Action* killing_machine(PlayerbotAI* botAI) { return new CastKillingMachineAction(botAI); }
		static Action* frost_presence(PlayerbotAI* botAI) { return new CastFrostPresenceAction(botAI); }
		static Action* deathchill(PlayerbotAI* botAI) { return new CastDeathchillAction(botAI); }
		static Action* icebound_fortitude(PlayerbotAI* botAI) { return new CastIceboundFortitudeAction(botAI); }
		static Action* mind_freeze(PlayerbotAI* botAI) { return new CastMindFreezeAction(botAI); }
		static Action* empower_rune_weapon(PlayerbotAI* botAI) { return new CastEmpowerRuneWeaponAction(botAI); }
		static Action* hungering_cold(PlayerbotAI* botAI) { return new CastHungeringColdAction(botAI); }
		static Action* unbreakable_armor(PlayerbotAI* botAI) { return new CastUnbreakableArmorAction(botAI); }
		static Action* improved_icy_talons(PlayerbotAI* botAI) { return new CastImprovedIcyTalonsAction(botAI); }
		// blood
		static Action* blood_strike(PlayerbotAI* botAI) { return new CastBloodStrikeAction(botAI); }
		static Action* blood_tap(PlayerbotAI* botAI) { return new CastBloodTapAction(botAI); }
		static Action* pestilence(PlayerbotAI* botAI) { return new CastPestilenceAction(botAI); }
		static Action* strangulate(PlayerbotAI* botAI) { return new CastStrangulateAction(botAI); }
		static Action* blood_boil(PlayerbotAI* botAI) { return new CastBloodBoilAction(botAI); }
		static Action* heart_strike(PlayerbotAI* botAI) { return new CastHeartStrikeAction(botAI); }
		static Action* mark_of_blood(PlayerbotAI* botAI) { return new CastMarkOfBloodAction(botAI); }
		static Action* blood_presence(PlayerbotAI* botAI) { return new CastBloodPresenceAction(botAI); }
		static Action* rune_tap(PlayerbotAI* botAI) { return new CastRuneTapAction(botAI); }
		static Action* vampiric_blood(PlayerbotAI* botAI) { return new CastVampiricBloodAction(botAI); }
		static Action* death_pact(PlayerbotAI* botAI) { return new CastDeathPactAction(botAI); }
		static Action* death_rune_mastery(PlayerbotAI* botAI) { return new CastDeathRuneMasteryAction(botAI); }
		//static Action* hysteria(PlayerbotAI* botAI) { return new CastHysteriaAction(botAI); }
		static Action* dancing_rune_weapon(PlayerbotAI* botAI) { return new CastDancingRuneWeaponAction(botAI); }
		static Action* dark_command(PlayerbotAI* botAI) { return new CastDarkCommandAction(botAI); }
        static Action* mind_freeze_on_enemy_healer(PlayerbotAI* botAI) { return new CastMindFreezeOnEnemyHealerAction(botAI); }
};

DKAiObjectContext::DKAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new DeathKnightStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightCombatStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightDKBuffStrategyFactoryInternal());
    actionContexts.Add(new DeathKnightAiObjectContextInternal());
    triggerContexts.Add(new DeathKnightTriggerFactoryInternal());
}
