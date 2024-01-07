/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorAiObjectContext.h"
#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"
#include "GenericWarriorNonCombatStrategy.h"
#include "TankWarriorStrategy.h"
#include "WarriorActions.h"
#include "WarriorTriggers.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"
#include "Playerbots.h"

class WarriorStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        WarriorStrategyFactoryInternal()
        {
            creators["nc"] = &WarriorStrategyFactoryInternal::nc;
            creators["pull"] = &WarriorStrategyFactoryInternal::pull;
            creators["aoe"] = &WarriorStrategyFactoryInternal::warrior_aoe;
        }

    private:
        static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarriorNonCombatStrategy(botAI); }
        static Strategy* warrior_aoe(PlayerbotAI* botAI) { return new WarrirorAoeStrategy(botAI); }
        static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
};

class WarriorCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
    public:
        WarriorCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
        {
            creators["tank"] = &WarriorCombatStrategyFactoryInternal::tank;
            creators["arms"] = &WarriorCombatStrategyFactoryInternal::arms;
            creators["fury"] = &WarriorCombatStrategyFactoryInternal::fury;
        }

    private:
        static Strategy* tank(PlayerbotAI* botAI) { return new TankWarriorStrategy(botAI); }
        static Strategy* arms(PlayerbotAI* botAI) { return new ArmsWarriorStrategy(botAI); }
        static Strategy* fury(PlayerbotAI* botAI) { return new FuryWarriorStrategy(botAI); }
};

class WarriorTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
    public:
        WarriorTriggerFactoryInternal()
        {
            creators["hamstring"] = &WarriorTriggerFactoryInternal::hamstring;
            creators["victory rush"] = &WarriorTriggerFactoryInternal::victory_rush;
            creators["death wish"] = &WarriorTriggerFactoryInternal::death_wish;
            creators["battle shout"] = &WarriorTriggerFactoryInternal::battle_shout;
            creators["rend"] = &WarriorTriggerFactoryInternal::rend;
            creators["rend on attacker"] = &WarriorTriggerFactoryInternal::rend_on_attacker;
            creators["bloodrage"] = &WarriorTriggerFactoryInternal::bloodrage;
            creators["shield bash"] = &WarriorTriggerFactoryInternal::shield_bash;
            creators["disarm"] = &WarriorTriggerFactoryInternal::disarm;
            creators["concussion blow"] = &WarriorTriggerFactoryInternal::concussion_blow;
            creators["sword and board"] = &WarriorTriggerFactoryInternal::SwordAndBoard;
            creators["shield bash on enemy healer"] = &WarriorTriggerFactoryInternal::shield_bash_on_enemy_healer;
            creators["battle stance"] = &WarriorTriggerFactoryInternal::battle_stance;
            creators["defensive stance"] = &WarriorTriggerFactoryInternal::defensive_stance;
            creators["berserker stance"] = &WarriorTriggerFactoryInternal::berserker_stance;
            creators["shield block"] = &WarriorTriggerFactoryInternal::shield_block;
            creators["sunder armor"] = &WarriorTriggerFactoryInternal::sunder_armor;
            creators["revenge"] = &WarriorTriggerFactoryInternal::revenge;
            creators["overpower"] = &WarriorTriggerFactoryInternal::overpower;
            creators["mocking blow"] = &WarriorTriggerFactoryInternal::mocking_blow;
            creators["rampage"] = &WarriorTriggerFactoryInternal::rampage;
            creators["mortal strike"] = &WarriorTriggerFactoryInternal::mortal_strike;
            creators["thunder clap on snare target"] = &WarriorTriggerFactoryInternal::thunder_clap_on_snare_target;
            creators["thunder clap"] = &WarriorTriggerFactoryInternal::thunder_clap;
            creators["bloodthirst"] = &WarriorTriggerFactoryInternal::bloodthirst;
            creators["berserker rage"] = &WarriorTriggerFactoryInternal::berserker_rage;
            creators["pummel on enemy healer"] = &WarriorTriggerFactoryInternal::pummel_on_enemy_healer;
            creators["pummel"] = &WarriorTriggerFactoryInternal::pummel;
            creators["intercept on enemy healer"] = &WarriorTriggerFactoryInternal::intercept_on_enemy_healer;
            creators["intercept"] = &WarriorTriggerFactoryInternal::intercept;
            creators["taunt on snare target"] = &WarriorTriggerFactoryInternal::taunt_on_snare_target;
            creators["commanding shout"] = &WarriorTriggerFactoryInternal::commanding_shout;
            creators["intercept on snare target"] = &WarriorTriggerFactoryInternal::intercept_on_snare_target;
            creators["spell reflection"] = &WarriorTriggerFactoryInternal::spell_reflection;
            creators["sudden death"] = &WarriorTriggerFactoryInternal::sudden_death;
            creators["instant slam"] = &WarriorTriggerFactoryInternal::instant_slam;
            creators["shockwave"] = &WarriorTriggerFactoryInternal::shockwave;
            creators["shockwave on snare target"] = &WarriorTriggerFactoryInternal::shockwave_on_snare_target;
            creators["taste for blood"] = &WarriorTriggerFactoryInternal::taste_for_blood;

            creators["thunder clap and rage"] = &WarriorTriggerFactoryInternal::thunderclap_and_rage;
            creators["intercept can cast"] = &WarriorTriggerFactoryInternal::intercept_can_cast;
            creators["intercept and far enemy"] = &WarriorTriggerFactoryInternal::intercept_and_far_enemy;
            creators["intercept and rage"] = &WarriorTriggerFactoryInternal::intercept_and_rage;
            // creators["slam"] = &WarriorTriggerFactoryInternal::slam;
        }

    private:
        static Trigger* shield_block(PlayerbotAI* botAI) { return new ShieldBlockTrigger(botAI); }
        static Trigger* defensive_stance(PlayerbotAI* botAI) { return new DefensiveStanceTrigger(botAI); }
        static Trigger* berserker_stance(PlayerbotAI* botAI) { return new BerserkerStanceTrigger(botAI); }
        static Trigger* battle_stance(PlayerbotAI* botAI) { return new BattleStanceTrigger(botAI); }
        static Trigger* hamstring(PlayerbotAI* botAI) { return new HamstringTrigger(botAI); }
        static Trigger* victory_rush(PlayerbotAI* botAI) { return new VictoryRushTrigger(botAI); }
        static Trigger* death_wish(PlayerbotAI* botAI) { return new DeathWishTrigger(botAI); }
        static Trigger* battle_shout(PlayerbotAI* botAI) { return new BattleShoutTrigger(botAI); }
        static Trigger* rend(PlayerbotAI* botAI) { return new RendDebuffTrigger(botAI); }
        static Trigger* rend_on_attacker(PlayerbotAI* botAI) { return new RendDebuffOnAttackerTrigger(botAI); }
        static Trigger* bloodrage(PlayerbotAI* botAI) { return new BloodrageBuffTrigger(botAI); }
        static Trigger* shield_bash(PlayerbotAI* botAI) { return new ShieldBashInterruptSpellTrigger(botAI); }
        static Trigger* disarm(PlayerbotAI* botAI) { return new DisarmDebuffTrigger(botAI); }
        static Trigger* concussion_blow(PlayerbotAI* botAI) { return new ConcussionBlowTrigger(botAI); }
        static Trigger* SwordAndBoard(PlayerbotAI* botAI) { return new SwordAndBoardTrigger(botAI); }
        static Trigger* shield_bash_on_enemy_healer(PlayerbotAI* botAI) { return new ShieldBashInterruptEnemyHealerSpellTrigger(botAI); }

        static Trigger* thunderclap_and_rage(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "thunderclap", "light rage available"); }
        static Trigger* intercept_can_cast(PlayerbotAI* botAI) { return new InterceptCanCastTrigger(botAI); }
        static Trigger* intercept_and_far_enemy(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "enemy is out of melee", "intercept can cast"); }
        static Trigger* intercept_and_rage(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "intercept and far enemy", "light rage available"); }

        static Trigger* intercept_on_snare_target(PlayerbotAI* botAI) { return new InterceptSnareTrigger(botAI); }
        static Trigger* spell_reflection(PlayerbotAI* botAI) { return new SpellReflectionTrigger(botAI); }
        static Trigger* taste_for_blood(PlayerbotAI* botAI) { return new TasteForBloodTrigger(botAI); }
        static Trigger* shockwave_on_snare_target(PlayerbotAI* botAI) { return new ShockwaveSnareTrigger(botAI); }
        static Trigger* shockwave(PlayerbotAI* botAI) { return new ShockwaveTrigger(botAI); }
        static Trigger* instant_slam(PlayerbotAI* botAI) { return new SlamInstantTrigger(botAI); }
        static Trigger* sudden_death(PlayerbotAI* botAI) { return new SuddenDeathTrigger(botAI); }
        static Trigger* commanding_shout(PlayerbotAI* botAI) { return new CommandingShoutTrigger(botAI); }
        static Trigger* taunt_on_snare_target(PlayerbotAI* botAI) { return new TauntSnareTrigger(botAI); }
        static Trigger* intercept(PlayerbotAI* botAI) { return new InterceptInterruptSpellTrigger(botAI); }
        static Trigger* intercept_on_enemy_healer(PlayerbotAI* botAI) { return new InterceptInterruptEnemyHealerSpellTrigger(botAI); }
        static Trigger* pummel(PlayerbotAI* botAI) { return new PummelInterruptSpellTrigger(botAI); }
        static Trigger* pummel_on_enemy_healer(PlayerbotAI* botAI) { return new PummelInterruptEnemyHealerSpellTrigger(botAI); }
        static Trigger* berserker_rage(PlayerbotAI* botAI) { return new BerserkerRageBuffTrigger(botAI); }
        static Trigger* bloodthirst(PlayerbotAI* botAI) { return new BloodthirstBuffTrigger(botAI); }
        static Trigger* thunder_clap_on_snare_target(PlayerbotAI* botAI) { return new ThunderClapSnareTrigger(botAI); }
        static Trigger* thunder_clap(PlayerbotAI* botAI) { return new ThunderClapTrigger(botAI); }
        static Trigger* mortal_strike(PlayerbotAI* botAI) { return new MortalStrikeDebuffTrigger(botAI); }
        static Trigger* rampage(PlayerbotAI* botAI) { return new RampageAvailableTrigger(botAI); }
        static Trigger* mocking_blow(PlayerbotAI* botAI) { return new MockingBlowTrigger(botAI); }
        static Trigger* overpower(PlayerbotAI* botAI) { return new OverpowerAvailableTrigger(botAI); }
        static Trigger* revenge(PlayerbotAI* botAI) { return new RevengeAvailableTrigger(botAI); }
        static Trigger* sunder_armor(PlayerbotAI* botAI) { return new SunderArmorDebuffTrigger(botAI); }
        // static Trigger* slam(PlayerbotAI* ai) { return new SlamTrigger(ai); }
};


class WarriorAiObjectContextInternal : public NamedObjectContext<Action>
{
    public:
        WarriorAiObjectContextInternal()
        {
            creators["devastate"] = &WarriorAiObjectContextInternal::devastate;
            creators["overpower"] = &WarriorAiObjectContextInternal::overpower;
            creators["charge"] = &WarriorAiObjectContextInternal::charge;
            creators["bloodthirst"] = &WarriorAiObjectContextInternal::bloodthirst;
            creators["rend"] = &WarriorAiObjectContextInternal::rend;
            creators["rend on attacker"] = &WarriorAiObjectContextInternal::rend_on_attacker;
            creators["mocking blow"] = &WarriorAiObjectContextInternal::mocking_blow;
            creators["death wish"] = &WarriorAiObjectContextInternal::death_wish;
            creators["berserker rage"] = &WarriorAiObjectContextInternal::berserker_rage;
            creators["victory rush"] = &WarriorAiObjectContextInternal::victory_rush;
            creators["execute"] = &WarriorAiObjectContextInternal::execute;
            creators["defensive stance"] = &WarriorAiObjectContextInternal::defensive_stance;
            creators["hamstring"] = &WarriorAiObjectContextInternal::hamstring;
            creators["shield bash"] = &WarriorAiObjectContextInternal::shield_bash;
            creators["shield block"] = &WarriorAiObjectContextInternal::shield_block;
            creators["bloodrage"] = &WarriorAiObjectContextInternal::bloodrage;
            creators["battle stance"] = &WarriorAiObjectContextInternal::battle_stance;
            creators["heroic strike"] = &WarriorAiObjectContextInternal::heroic_strike;
            creators["intimidating shout"] = &WarriorAiObjectContextInternal::intimidating_shout;
            creators["demoralizing shout"] = &WarriorAiObjectContextInternal::demoralizing_shout;
            creators["demoralizing shout without life time check"] = &WarriorAiObjectContextInternal::demoralizing_shout_without_life_time_check;
            creators["challenging shout"] = &WarriorAiObjectContextInternal::challenging_shout;
            creators["shield wall"] = &WarriorAiObjectContextInternal::shield_wall;
            creators["battle shout"] = &WarriorAiObjectContextInternal::battle_shout;
            creators["battle shout taunt"] = &WarriorAiObjectContextInternal::battle_shout_taunt;
            creators["thunder clap"] = &WarriorAiObjectContextInternal::thunder_clap;
            creators["taunt"] = &WarriorAiObjectContextInternal::taunt;
            creators["revenge"] = &WarriorAiObjectContextInternal::revenge;
            creators["slam"] = &WarriorAiObjectContextInternal::slam;
            creators["shield slam"] = &WarriorAiObjectContextInternal::shield_slam;
            creators["disarm"] = &WarriorAiObjectContextInternal::disarm;
            creators["sunder armor"] = &WarriorAiObjectContextInternal::sunder_armor;
            creators["last stand"] = &WarriorAiObjectContextInternal::last_stand;
            creators["shockwave on snare target"] = &WarriorAiObjectContextInternal::shockwave_on_snare_target;
            creators["shockwave"] = &WarriorAiObjectContextInternal::shockwave;
            creators["cleave"] = &WarriorAiObjectContextInternal::cleave;
            creators["concussion blow"] = &WarriorAiObjectContextInternal::concussion_blow;
            creators["shield bash on enemy healer"] = &WarriorAiObjectContextInternal::shield_bash_on_enemy_healer;
            creators["berserker stance"] = &WarriorAiObjectContextInternal::berserker_stance;
            creators["commanding shout"] = &WarriorAiObjectContextInternal::commanding_shout;
            creators["retaliation"] = &WarriorAiObjectContextInternal::retaliation;
            creators["mortal strike"] = &WarriorAiObjectContextInternal::mortal_strike;
            creators["sweeping strikes"] = &WarriorAiObjectContextInternal::sweeping_strikes;
            creators["intercept"] = &WarriorAiObjectContextInternal::intercept;
            creators["whirlwind"] = &WarriorAiObjectContextInternal::whirlwind;
            creators["pummel"] = &WarriorAiObjectContextInternal::pummel;
            creators["pummel on enemy healer"] = &WarriorAiObjectContextInternal::pummel_on_enemy_healer;
            creators["recklessness"] = &WarriorAiObjectContextInternal::recklessness;
            creators["piercing howl"] = &WarriorAiObjectContextInternal::piercing_howl;
            creators["rampage"] = &WarriorAiObjectContextInternal::rampage;
            creators["intervene"] = &WarriorAiObjectContextInternal::intervene;
            creators["spell reflection"] = &WarriorAiObjectContextInternal::spell_reflection;
            creators["thunder clap on snare target"] = &WarriorAiObjectContextInternal::thunder_clap_on_snare_target;
            creators["taunt on snare target"] = &WarriorAiObjectContextInternal::taunt_on_snare_target;
            creators["intercept on enemy healer"] = &WarriorAiObjectContextInternal::intercept_on_enemy_healer;
            creators["intercept on snare target"] = &WarriorAiObjectContextInternal::intercept_on_snare_target;
            creators["bladestorm"] = &WarriorAiObjectContextInternal::bladestorm;
            creators["heroic throw"] = &WarriorAiObjectContextInternal::heroic_throw;
            creators["heroic throw on snare target"] = &WarriorAiObjectContextInternal::heroic_throw_on_snare_target;
            creators["shattering throw"] = &WarriorAiObjectContextInternal::shattering_throw;
        }

    private:
        static Action* devastate(PlayerbotAI* botAI) { return new CastDevastateAction(botAI); }
        static Action* last_stand(PlayerbotAI* botAI) { return new CastLastStandAction(botAI); }
        static Action* shockwave(PlayerbotAI* botAI) { return new CastShockwaveAction(botAI); }
        static Action* shockwave_on_snare_target(PlayerbotAI* botAI) { return new CastShockwaveSnareAction(botAI); }
        static Action* cleave(PlayerbotAI* botAI) { return new CastCleaveAction(botAI); }
        static Action* concussion_blow(PlayerbotAI* botAI) { return new CastConcussionBlowAction(botAI); }
        static Action* taunt(PlayerbotAI* botAI) { return new CastTauntAction(botAI); }
        static Action* revenge(PlayerbotAI* botAI) { return new CastRevengeAction(botAI); }
        static Action* slam(PlayerbotAI* botAI) { return new CastSlamAction(botAI); }
        static Action* shield_slam(PlayerbotAI* botAI) { return new CastShieldSlamAction(botAI); }
        static Action* disarm(PlayerbotAI* botAI) { return new CastDisarmAction(botAI); }
        static Action* sunder_armor(PlayerbotAI* botAI) { return new CastSunderArmorAction(botAI); }
        static Action* overpower(PlayerbotAI* botAI) { return new CastOverpowerAction(botAI); }
        static Action* charge(PlayerbotAI* botAI) { return new CastChargeAction(botAI); }
        static Action* bloodthirst(PlayerbotAI* botAI) { return new CastBloodthirstAction(botAI); }
        static Action* rend(PlayerbotAI* botAI) { return new CastRendAction(botAI); }
        static Action* rend_on_attacker(PlayerbotAI* botAI) { return new CastRendOnAttackerAction(botAI); }
        static Action* mocking_blow(PlayerbotAI* botAI) { return new CastMockingBlowAction(botAI); }
        static Action* death_wish(PlayerbotAI* botAI) { return new CastDeathWishAction(botAI); }
        static Action* berserker_rage(PlayerbotAI* botAI) { return new CastBerserkerRageAction(botAI); }
        static Action* victory_rush(PlayerbotAI* botAI) { return new CastVictoryRushAction(botAI); }
        static Action* execute(PlayerbotAI* botAI) { return new CastExecuteAction(botAI); }
        static Action* defensive_stance(PlayerbotAI* botAI) { return new CastDefensiveStanceAction(botAI); }
        static Action* hamstring(PlayerbotAI* botAI) { return new CastHamstringAction(botAI); }
        static Action* shield_bash(PlayerbotAI* botAI) { return new CastShieldBashAction(botAI); }
        static Action* shield_block(PlayerbotAI* botAI) { return new CastShieldBlockAction(botAI); }
        static Action* bloodrage(PlayerbotAI* botAI) { return new CastBloodrageAction(botAI); }
        static Action* battle_stance(PlayerbotAI* botAI) { return new CastBattleStanceAction(botAI); }
        static Action* heroic_strike(PlayerbotAI* botAI) { return new CastHeroicStrikeAction(botAI); }
        static Action* intimidating_shout(PlayerbotAI* botAI) { return new CastIntimidatingShoutAction(botAI); }
        static Action* demoralizing_shout(PlayerbotAI* botAI) { return new CastDemoralizingShoutAction(botAI); }
        static Action* demoralizing_shout_without_life_time_check(PlayerbotAI* botAI) { return new CastDemoralizingShoutWithoutLifeTimeCheckAction(botAI); }
        static Action* challenging_shout(PlayerbotAI* botAI) { return new CastChallengingShoutAction(botAI); }
        static Action* shield_wall(PlayerbotAI* botAI) { return new CastShieldWallAction(botAI); }
        static Action* battle_shout(PlayerbotAI* botAI) { return new CastBattleShoutAction(botAI); }
        static Action* battle_shout_taunt(PlayerbotAI* botAI) { return new CastBattleShoutTauntAction(botAI); }
        static Action* thunder_clap(PlayerbotAI* botAI) { return new CastThunderClapAction(botAI); }
        static Action* shield_bash_on_enemy_healer(PlayerbotAI* botAI) { return new CastShieldBashOnEnemyHealerAction(botAI); }
        static Action* intercept_on_snare_target(PlayerbotAI* botAI) { return new CastInterceptOnSnareTargetAction(botAI); }
        static Action* intercept_on_enemy_healer(PlayerbotAI* botAI) { return new CastInterceptOnEnemyHealerAction(botAI); }
        static Action* taunt_on_snare_target(PlayerbotAI* botAI) { return new CastTauntOnSnareTargetAction(botAI); }
        static Action* thunder_clap_on_snare_target(PlayerbotAI* botAI) { return new CastThunderClapSnareAction(botAI); }
        static Action* berserker_stance(PlayerbotAI* botAI) { return new CastBerserkerStanceAction(botAI); }
        static Action* commanding_shout(PlayerbotAI* botAI) { return new CastCommandingShoutAction(botAI); }
        static Action* retaliation(PlayerbotAI* botAI) { return new CastRetaliationAction(botAI); }
        static Action* mortal_strike(PlayerbotAI* botAI) { return new CastMortalStrikeAction(botAI); }
        static Action* sweeping_strikes(PlayerbotAI* botAI) { return new CastSweepingStrikesAction(botAI); }
        static Action* intercept(PlayerbotAI* botAI) { return new CastInterceptAction(botAI); }
        static Action* whirlwind(PlayerbotAI* botAI) { return new CastWhirlwindAction(botAI); }
        static Action* pummel(PlayerbotAI* botAI) { return new CastPummelAction (botAI); }
        static Action* pummel_on_enemy_healer(PlayerbotAI* botAI) { return new CastPummelOnEnemyHealerAction(botAI); }
        static Action* recklessness(PlayerbotAI* botAI) { return new CastRecklessnessAction(botAI); }
        static Action* piercing_howl(PlayerbotAI* botAI) { return new CastPiercingHowlAction(botAI); }
        static Action* rampage(PlayerbotAI* botAI) { return new CastRampageAction(botAI); }
        static Action* intervene(PlayerbotAI* botAI) { return new CastInterveneAction(botAI); }
        static Action* spell_reflection(PlayerbotAI* botAI) { return new CastSpellReflectionAction(botAI); }
        static Action* shattering_throw(PlayerbotAI* botAI) { return new CastShatteringThrowAction(botAI); }
        static Action* heroic_throw_on_snare_target(PlayerbotAI* botAI) { return new CastHeroicThrowSnareAction(botAI); }
        static Action* heroic_throw(PlayerbotAI* botAI) { return new CastHeroicThrowAction(botAI); }
        static Action* bladestorm(PlayerbotAI* botAI) { return new CastBladestormAction(botAI); }
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarriorStrategyFactoryInternal());
    strategyContexts.Add(new WarriorCombatStrategyFactoryInternal());
    actionContexts.Add(new WarriorAiObjectContextInternal());
    triggerContexts.Add(new WarriorTriggerFactoryInternal());
}
