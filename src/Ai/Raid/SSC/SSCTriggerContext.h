/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCTRIGGERCONTEXT_H
#define PLAYERBOTS_SSCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SSCTriggers.h"

class RaidSscTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSscTriggerContext()
    {
        // Shared
        creators["ssc no encounter in progress"] =
            &RaidSscTriggerContext::ssc_no_encounter_in_progress;

        // Trash
        creators["underbog colossus in toxic pool"] =
            &RaidSscTriggerContext::underbog_colossus_in_toxic_pool;

        creators["greyheart tidecaller water elemental totem spawned"] =
            &RaidSscTriggerContext::greyheart_tidecaller_water_elemental_totem_spawned;

        // Hydross the Unstable <Duke of Currents>
        creators["hydross the unstable should be tanked by frost tank"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_be_tanked_by_frost_tank;

        creators["hydross the unstable should be tanked by nature tank"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_be_tanked_by_nature_tank;

        creators["hydross the unstable ranged should spread in frost phase"] =
            &RaidSscTriggerContext::hydross_the_unstable_ranged_should_spread_in_frost_phase;

        creators["hydross the unstable should misdirect upon phase change"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_misdirect_upon_phase_change;

        creators["hydross the unstable aggro resets upon phase change"] =
            &RaidSscTriggerContext::hydross_the_unstable_aggro_resets_upon_phase_change;

        creators["hydross the unstable off phase tank attacking"] =
            &RaidSscTriggerContext::hydross_the_unstable_off_phase_tank_attacking;

        creators["hydross the unstable should manage phase timers"] =
            &RaidSscTriggerContext::hydross_the_unstable_should_manage_phase_timers;

        // The Lurker Below
        creators["the lurker below spout is active"] =
            &RaidSscTriggerContext::the_lurker_below_spout_is_active;

        creators["the lurker below should be tanked"] =
            &RaidSscTriggerContext::the_lurker_below_should_be_tanked;

        creators["the lurker below ranged should spread"] =
            &RaidSscTriggerContext::the_lurker_below_ranged_should_spread;

        creators["the lurker below guardians should be tanked"] =
            &RaidSscTriggerContext::the_lurker_below_guardians_should_be_tanked;

        creators["the lurker below melee cannot reach target"] =
            &RaidSscTriggerContext::the_lurker_below_melee_cannot_reach_target;

        creators["the lurker below melee in water"] =
            &RaidSscTriggerContext::the_lurker_below_melee_in_water;

        // Leotheras the Blind
        creators["leotheras the blind ranged should spread upon pull"] =
            &RaidSscTriggerContext::leotheras_the_blind_ranged_should_spread_upon_pull;

        creators["leotheras the blind warlock should tank demon form"] =
            &RaidSscTriggerContext::leotheras_the_blind_warlock_should_tank_demon_form;

        creators["leotheras the blind tanks should auto-attack demon form"] =
            &RaidSscTriggerContext::leotheras_the_blind_tanks_should_auto_attack_demon_form;

        creators["leotheras the blind ranged should keep distance"] =
            &RaidSscTriggerContext::leotheras_the_blind_ranged_should_keep_distance;

        creators["leotheras the blind channeling whirlwind"] =
            &RaidSscTriggerContext::leotheras_the_blind_channeling_whirlwind;

        creators["leotheras the blind too many chaos blast stacks"] =
            &RaidSscTriggerContext::leotheras_the_blind_too_many_chaos_blast_stacks;

        creators["leotheras the blind inner demon has awakened"] =
            &RaidSscTriggerContext::leotheras_the_blind_inner_demon_has_awakened;

        creators["leotheras the blind in final phase"] =
            &RaidSscTriggerContext::leotheras_the_blind_in_final_phase;

        creators["leotheras the blind should separate boss from demon"] =
            &RaidSscTriggerContext::leotheras_the_blind_should_separate_boss_from_demon;

        creators["leotheras the blind hunter should misdirect demon form"] =
            &RaidSscTriggerContext::leotheras_the_blind_hunter_should_misdirect_demon_form;

        creators["leotheras the blind aggro resets"] =
            &RaidSscTriggerContext::leotheras_the_blind_aggro_resets;

        creators["leotheras the blind should manage dps wait timers"] =
            &RaidSscTriggerContext::leotheras_the_blind_should_manage_dps_wait_timers;

        // Fathom-Lord Karathress
        creators["fathom-lord karathress targets should be tanked"] =
            &RaidSscTriggerContext::fathom_lord_karathress_targets_should_be_tanked;

        creators["fathom-lord karathress should heal caribdis tank"] =
            &RaidSscTriggerContext::fathom_lord_karathress_should_heal_caribdis_tank;

        creators["fathom-lord karathress hunter should misdirect"] =
            &RaidSscTriggerContext::fathom_lord_karathress_hunter_should_misdirect;

        creators["fathom-lord karathress should assign dps priority"] =
            &RaidSscTriggerContext::fathom_lord_karathress_should_assign_dps_priority;

        creators["fathom-lord karathress should manage dps timer"] =
            &RaidSscTriggerContext::fathom_lord_karathress_should_manage_dps_timer;

        creators["fathom-lord karathress ranged should spread"] =
            &RaidSscTriggerContext::fathom_lord_karathress_ranged_should_spread;

        creators["fathom-lord karathress stuck midair after cyclone"] =
            &RaidSscTriggerContext::fathom_lord_karathress_stuck_midair_after_cyclone;

        // Morogrim Tidewalker
        creators["morogrim tidewalker should be tanked"] =
            &RaidSscTriggerContext::morogrim_tidewalker_should_be_tanked;

        creators["morogrim tidewalker ranged should stack"] =
            &RaidSscTriggerContext::morogrim_tidewalker_ranged_should_stack;

        creators["morogrim tidewalker too far from boss"] =
            &RaidSscTriggerContext::morogrim_tidewalker_too_far_from_boss;

        creators["morogrim tidewalker hunter should misdirect"] =
            &RaidSscTriggerContext::morogrim_tidewalker_hunter_should_misdirect;

        // Lady Vashj <Coilfang Matron>
        creators["lady vashj should be tanked"] =
            &RaidSscTriggerContext::lady_vashj_should_be_tanked;

        creators["lady vashj ranged should spread in phase 1"] =
            &RaidSscTriggerContext::lady_vashj_ranged_should_spread_in_phase_1;

        creators["lady vashj station slots need holders"] =
            &RaidSscTriggerContext::lady_vashj_station_slots_need_holders;

        creators["lady vashj should hold station in phase 2"] =
            &RaidSscTriggerContext::lady_vashj_should_hold_station_in_phase_2;

        creators["lady vashj ranged should position in phase 3"] =
            &RaidSscTriggerContext::lady_vashj_ranged_should_position_in_phase_3;

        creators["lady vashj out of sight in phase 3"] =
            &RaidSscTriggerContext::lady_vashj_out_of_sight_in_phase_3;

        creators["lady vashj main tank needs grounding shaman"] =
            &RaidSscTriggerContext::lady_vashj_main_tank_needs_grounding_shaman;

        creators["lady vashj shaman should ground shock blast"] =
            &RaidSscTriggerContext::lady_vashj_shaman_should_ground_shock_blast;

        creators["lady vashj static charge on group member"] =
            &RaidSscTriggerContext::lady_vashj_static_charge_on_group_member;

        creators["lady vashj hunter should misdirect"] =
            &RaidSscTriggerContext::lady_vashj_hunter_should_misdirect;

        creators["lady vashj should assign target priority"] =
            &RaidSscTriggerContext::lady_vashj_should_assign_target_priority;

        creators["lady vashj tank needs fear ward"] =
            &RaidSscTriggerContext::lady_vashj_tank_needs_fear_ward;

        creators["lady vashj coilfang strider should be tanked"] =
            &RaidSscTriggerContext::lady_vashj_coilfang_strider_should_be_tanked;

        creators["lady vashj coilfang elite should be tanked"] =
            &RaidSscTriggerContext::lady_vashj_coilfang_elite_should_be_tanked;

        creators["lady vashj tank is idle away from the middle"] =
            &RaidSscTriggerContext::lady_vashj_tank_is_idle_away_from_the_middle;

        creators["lady vashj tainted elemental needs looter"] =
            &RaidSscTriggerContext::lady_vashj_tainted_elemental_needs_looter;

        creators["lady vashj should attack tainted elemental"] =
            &RaidSscTriggerContext::lady_vashj_should_attack_tainted_elemental;

        creators["lady vashj tainted core looter"] =
            &RaidSscTriggerContext::lady_vashj_tainted_core_looter;

        creators["lady vashj core passing chain member"] =
            &RaidSscTriggerContext::lady_vashj_core_passing_chain_member;

        creators["lady vashj should destroy tainted core"] =
            &RaidSscTriggerContext::lady_vashj_should_destroy_tainted_core;

        creators["lady vashj pet should switch target"] =
            &RaidSscTriggerContext::lady_vashj_pet_should_switch_target;

        creators["lady vashj bot above the ground"] =
            &RaidSscTriggerContext::lady_vashj_bot_above_the_ground;

        creators["lady vashj bot in toxic spores"] =
            &RaidSscTriggerContext::lady_vashj_bot_in_toxic_spores;

        creators["lady vashj melee near toxic spores"] =
            &RaidSscTriggerContext::lady_vashj_melee_near_toxic_spores;

        creators["lady vashj ranged reach blocked by toxic spores"] =
            &RaidSscTriggerContext::lady_vashj_ranged_reach_blocked_by_toxic_spores;

        creators["lady vashj entangle on melee"] =
            &RaidSscTriggerContext::lady_vashj_entangle_on_melee;

        creators["lady vashj static charge on rogue"] =
            &RaidSscTriggerContext::lady_vashj_static_charge_on_rogue;
    }

private:
    // Shared
    static Trigger* ssc_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new SscNoEncounterInProgressTrigger(botAI);
    }

    // Trash
    static Trigger* underbog_colossus_in_toxic_pool(PlayerbotAI* botAI)
    {
        return new UnderbogColossusInToxicPoolTrigger(botAI);
    }
    static Trigger* greyheart_tidecaller_water_elemental_totem_spawned(PlayerbotAI* botAI)
    {
        return new GreyheartTidecallerWaterElementalTotemSpawnedTrigger(botAI);
    }

    // Hydross the Unstable <Duke of Currents>
    static Trigger* hydross_the_unstable_should_be_tanked_by_frost_tank(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableShouldBeTankedByFrostTankTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_should_be_tanked_by_nature_tank(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableShouldBeTankedByNatureTankTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_ranged_should_spread_in_frost_phase(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableRangedShouldSpreadInFrostPhaseTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_should_misdirect_upon_phase_change(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableShouldMisdirectUponPhaseChangeTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_aggro_resets_upon_phase_change(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_off_phase_tank_attacking(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableOffPhaseTankAttackingTrigger(botAI);
    }
    static Trigger* hydross_the_unstable_should_manage_phase_timers(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableShouldManagePhaseTimersTrigger(botAI);
    }

    // The Lurker Below
    static Trigger* the_lurker_below_spout_is_active(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowSpoutIsActiveTrigger(botAI);
    }
    static Trigger* the_lurker_below_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowShouldBeTankedTrigger(botAI);
    }
    static Trigger* the_lurker_below_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* the_lurker_below_guardians_should_be_tanked(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowGuardiansShouldBeTankedTrigger(botAI);
    }
    static Trigger* the_lurker_below_melee_cannot_reach_target(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowMeleeCannotReachTargetTrigger(botAI);
    }
    static Trigger* the_lurker_below_melee_in_water(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowMeleeInWaterTrigger(botAI);
    }

    // Leotheras the Blind
    static Trigger* leotheras_the_blind_ranged_should_spread_upon_pull(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindRangedShouldSpreadUponPullTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_warlock_should_tank_demon_form(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindWarlockShouldTankDemonFormTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_tanks_should_auto_attack_demon_form(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindTanksShouldAutoAttackDemonFormTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_ranged_should_keep_distance(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindRangedShouldKeepDistanceTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_channeling_whirlwind(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_too_many_chaos_blast_stacks(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindTooManyChaosBlastStacksTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_inner_demon_has_awakened(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindInnerDemonHasAwakenedTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_in_final_phase(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindInFinalPhaseTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_should_separate_boss_from_demon(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindShouldSeparateBossFromDemonTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_hunter_should_misdirect_demon_form(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindHunterShouldMisdirectDemonFormTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_aggro_resets(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindAggroResetsTrigger(botAI);
    }
    static Trigger* leotheras_the_blind_should_manage_dps_wait_timers(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindShouldManageDpsWaitTimersTrigger(botAI);
    }

    // Fathom-Lord Karathress
    static Trigger* fathom_lord_karathress_targets_should_be_tanked(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressTargetsShouldBeTankedTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_should_heal_caribdis_tank(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressShouldHealCaribdisTankTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SscHunterShouldMisdirectTrigger(
            botAI, "fathom-lord karathress hunter should misdirect", "fathom-guard tidalvess");
    }
    static Trigger* fathom_lord_karathress_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_should_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressShouldManageDpsTimerTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* fathom_lord_karathress_stuck_midair_after_cyclone(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressStuckMidairAfterCycloneTrigger(botAI);
    }

    // Morogrim Tidewalker
    static Trigger* morogrim_tidewalker_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerShouldBeTankedTrigger(botAI);
    }
    static Trigger* morogrim_tidewalker_ranged_should_stack(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerRangedShouldStackTrigger(botAI);
    }
    static Trigger* morogrim_tidewalker_too_far_from_boss(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerTooFarFromBossTrigger(botAI);
    }
    static Trigger* morogrim_tidewalker_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SscHunterShouldMisdirectTrigger(
            botAI, "morogrim tidewalker hunter should misdirect", "morogrim tidewalker");
    }

    // Lady Vashj <Coilfang Matron>
    static Trigger* lady_vashj_should_be_tanked(PlayerbotAI* botAI)
    {
        return new LadyVashjShouldBeTankedTrigger(botAI);
    }
    static Trigger* lady_vashj_ranged_should_spread_in_phase_1(PlayerbotAI* botAI)
    {
        return new LadyVashjRangedShouldSpreadInPhase1Trigger(botAI);
    }
    static Trigger* lady_vashj_station_slots_need_holders(PlayerbotAI* botAI)
    {
        return new LadyVashjStationSlotsNeedHoldersTrigger(botAI);
    }
    static Trigger* lady_vashj_should_hold_station_in_phase_2(PlayerbotAI* botAI)
    {
        return new LadyVashjShouldHoldStationInPhase2Trigger(botAI);
    }
    static Trigger* lady_vashj_ranged_should_position_in_phase_3(PlayerbotAI* botAI)
    {
        return new LadyVashjRangedShouldPositionInPhase3Trigger(botAI);
    }
    static Trigger* lady_vashj_out_of_sight_in_phase_3(PlayerbotAI* botAI)
    {
        return new LadyVashjOutOfSightInPhase3Trigger(botAI);
    }
    static Trigger* lady_vashj_main_tank_needs_grounding_shaman(PlayerbotAI* botAI)
    {
        return new LadyVashjMainTankNeedsGroundingShamanTrigger(botAI);
    }
    static Trigger* lady_vashj_shaman_should_ground_shock_blast(PlayerbotAI* botAI)
    {
        return new LadyVashjShamanShouldGroundShockBlastTrigger(botAI);
    }
    static Trigger* lady_vashj_static_charge_on_group_member(PlayerbotAI* botAI)
    {
        return new LadyVashjStaticChargeOnGroupMemberTrigger(botAI);
    }
    static Trigger* lady_vashj_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SscHunterShouldMisdirectTrigger(
            botAI, "lady vashj hunter should misdirect", "lady vashj");
    }
    static Trigger* lady_vashj_should_assign_target_priority(PlayerbotAI* botAI)
    {
        return new LadyVashjShouldAssignTargetPriorityTrigger(botAI);
    }
    static Trigger* lady_vashj_tank_needs_fear_ward(PlayerbotAI* botAI)
    {
        return new LadyVashjTankNeedsFearWardTrigger(botAI);
    }
    static Trigger* lady_vashj_coilfang_strider_should_be_tanked(PlayerbotAI* botAI)
    {
        return new LadyVashjCoilfangStriderShouldBeTankedTrigger(botAI);
    }
    static Trigger* lady_vashj_coilfang_elite_should_be_tanked(PlayerbotAI* botAI)
    {
        return new LadyVashjCoilfangEliteShouldBeTankedTrigger(botAI);
    }
    static Trigger* lady_vashj_tank_is_idle_away_from_the_middle(PlayerbotAI* botAI)
    {
        return new LadyVashjTankIsIdleAwayFromTheMiddleTrigger(botAI);
    }
    static Trigger* lady_vashj_tainted_elemental_needs_looter(PlayerbotAI* botAI)
    {
        return new LadyVashjTaintedElementalNeedsLooterTrigger(botAI);
    }
    static Trigger* lady_vashj_should_attack_tainted_elemental(PlayerbotAI* botAI)
    {
        return new LadyVashjShouldAttackTaintedElementalTrigger(botAI);
    }
    static Trigger* lady_vashj_tainted_core_looter(PlayerbotAI* botAI)
    {
        return new LadyVashjTaintedCoreLooterTrigger(botAI);
    }
    static Trigger* lady_vashj_core_passing_chain_member(PlayerbotAI* botAI)
    {
        return new LadyVashjCorePassingChainMemberTrigger(botAI);
    }
    static Trigger* lady_vashj_should_destroy_tainted_core(PlayerbotAI* botAI)
    {
        return new LadyVashjShouldDestroyTaintedCoreTrigger(botAI);
    }
    static Trigger* lady_vashj_pet_should_switch_target(PlayerbotAI* botAI)
    {
        return new LadyVashjPetShouldSwitchTargetTrigger(botAI);
    }
    static Trigger* lady_vashj_bot_above_the_ground(PlayerbotAI* botAI)
    {
        return new LadyVashjBotAboveTheGroundTrigger(botAI);
    }
    static Trigger* lady_vashj_bot_in_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjBotInToxicSporesTrigger(botAI);
    }
    static Trigger* lady_vashj_melee_near_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjMeleeNearToxicSporesTrigger(botAI);
    }
    static Trigger* lady_vashj_ranged_reach_blocked_by_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjRangedReachBlockedByToxicSporesTrigger(botAI);
    }
    static Trigger* lady_vashj_entangle_on_melee(PlayerbotAI* botAI)
    {
        return new LadyVashjEntangleOnMeleeTrigger(botAI);
    }
    static Trigger* lady_vashj_static_charge_on_rogue(PlayerbotAI* botAI)
    {
        return new LadyVashjStaticChargeOnRogueTrigger(botAI);
    }
};

#endif
