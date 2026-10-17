/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCACTIONCONTEXT_H
#define PLAYERBOTS_SSCACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SSCActions.h"

class RaidSscActionContext : public NamedObjectContext<Action>
{
public:
    RaidSscActionContext()
    {
        // Shared
        creators["ssc reset encounter states"] =
            &RaidSscActionContext::ssc_reset_encounter_states;

        // Trash
        creators["underbog colossus escape toxic pool"] =
            &RaidSscActionContext::underbog_colossus_escape_toxic_pool;

        creators["greyheart tidecaller mark water elemental totem"] =
            &RaidSscActionContext::greyheart_tidecaller_mark_water_elemental_totem;

        // Hydross the Unstable <Duke of Currents>
        creators["hydross the unstable position frost tank"] =
            &RaidSscActionContext::hydross_the_unstable_position_frost_tank;

        creators["hydross the unstable position nature tank"] =
            &RaidSscActionContext::hydross_the_unstable_position_nature_tank;

        creators["hydross the unstable frost phase spread ranged"] =
            &RaidSscActionContext::hydross_the_unstable_frost_phase_spread_ranged;

        creators["hydross the unstable misdirect to tank"] =
            &RaidSscActionContext::hydross_the_unstable_misdirect_to_tank;

        creators["hydross the unstable stop attacking upon phase change"] =
            &RaidSscActionContext::hydross_the_unstable_stop_attacking_upon_phase_change;

        creators["hydross the unstable manage phase timers"] =
            &RaidSscActionContext::hydross_the_unstable_manage_phase_timers;

        // The Lurker Below
        creators["the lurker below run around behind boss"] =
            &RaidSscActionContext::the_lurker_below_run_around_behind_boss;

        creators["the lurker below position main tank"] =
            &RaidSscActionContext::the_lurker_below_position_main_tank;

        creators["the lurker below spread ranged in arc"] =
            &RaidSscActionContext::the_lurker_below_spread_ranged_in_arc;

        creators["the lurker below tanks pick up guardians"] =
            &RaidSscActionContext::the_lurker_below_tanks_pick_up_guardians;

        creators["the lurker below melee move directly to target"] =
            &RaidSscActionContext::the_lurker_below_melee_move_directly_to_target;

        creators["the lurker below melee get out of water"] =
            &RaidSscActionContext::the_lurker_below_melee_get_out_of_water;

        // Leotheras the Blind
        creators["leotheras the blind spread ranged upon pull"] =
            &RaidSscActionContext::leotheras_the_blind_spread_ranged_upon_pull;

        creators["leotheras the blind warlock tank attack demon form"] =
            &RaidSscActionContext::leotheras_the_blind_warlock_tank_attack_demon_form;

        creators["leotheras the blind tanks build rage on demon form"] =
            &RaidSscActionContext::leotheras_the_blind_tanks_build_rage_on_demon_form;

        creators["leotheras the blind ranged keep distance"] =
            &RaidSscActionContext::leotheras_the_blind_ranged_keep_distance;

        creators["leotheras the blind run away from whirlwind"] =
            &RaidSscActionContext::leotheras_the_blind_run_away_from_whirlwind;

        creators["leotheras the blind melee run from chaos blast"] =
            &RaidSscActionContext::leotheras_the_blind_melee_run_from_chaos_blast;

        creators["leotheras the blind destroy inner demon"] =
            &RaidSscActionContext::leotheras_the_blind_destroy_inner_demon;

        creators["leotheras the blind final phase attack boss"] =
            &RaidSscActionContext::leotheras_the_blind_final_phase_attack_boss;

        creators["leotheras the blind final phase separate boss from demon"] =
            &RaidSscActionContext::leotheras_the_blind_final_phase_separate_boss_from_demon;

        creators["leotheras the blind misdirect demon form to tank"] =
            &RaidSscActionContext::leotheras_the_blind_misdirect_demon_form_to_tank;

        creators["leotheras the blind melee stop attacking"] =
            &RaidSscActionContext::leotheras_the_blind_melee_stop_attacking;

        creators["leotheras the blind manage dps wait timers"] =
            &RaidSscActionContext::leotheras_the_blind_manage_dps_wait_timers;

        // Fathom-Lord Karathress
        creators["fathom-lord karathress tanks position targets"] =
            &RaidSscActionContext::fathom_lord_karathress_tanks_position_targets;

        creators["fathom-lord karathress position caribdis tank healer"] =
            &RaidSscActionContext::fathom_lord_karathress_position_caribdis_tank_healer;

        creators["fathom-lord karathress misdirect to tanks"] =
            &RaidSscActionContext::fathom_lord_karathress_misdirect_to_tanks;

        creators["fathom-lord karathress assign dps priority"] =
            &RaidSscActionContext::fathom_lord_karathress_assign_dps_priority;

        creators["fathom-lord karathress manage dps timer"] =
            &RaidSscActionContext::fathom_lord_karathress_manage_dps_timer;

        creators["fathom-lord karathress spread ranged"] =
            &RaidSscActionContext::fathom_lord_karathress_spread_ranged;

        creators["fathom-lord karathress drop to ground after cyclone"] =
            &RaidSscActionContext::fathom_lord_karathress_drop_to_ground_after_cyclone;

        // Morogrim Tidewalker
        creators["morogrim tidewalker position main tank"] =
            &RaidSscActionContext::morogrim_tidewalker_position_main_tank;

        creators["morogrim tidewalker stack ranged behind boss"] =
            &RaidSscActionContext::morogrim_tidewalker_stack_ranged_behind_boss;

        creators["morogrim tidewalker return to boss"] =
            &RaidSscActionContext::morogrim_tidewalker_return_to_boss;

        creators["morogrim tidewalker misdirect to main tank"] =
            &RaidSscActionContext::morogrim_tidewalker_misdirect_to_main_tank;

        // Lady Vashj <Coilfang Matron>
        creators["lady vashj main tank position boss"] =
            &RaidSscActionContext::lady_vashj_main_tank_position_boss;

        creators["lady vashj phase 1 spread ranged in arc"] =
            &RaidSscActionContext::lady_vashj_phase_1_spread_ranged_in_arc;

        creators["lady vashj assign station slots"] =
            &RaidSscActionContext::lady_vashj_assign_station_slots;

        creators["lady vashj phase 2 position at station"] =
            &RaidSscActionContext::lady_vashj_phase_2_position_at_station;

        creators["lady vashj phase 3 position ranged"] =
            &RaidSscActionContext::lady_vashj_phase_3_position_ranged;

        creators["lady vashj phase 3 move into sight"] =
            &RaidSscActionContext::lady_vashj_phase_3_move_into_sight;

        creators["lady vashj assign grounding shaman"] =
            &RaidSscActionContext::lady_vashj_assign_grounding_shaman;

        creators["lady vashj set grounding totem in main tank group"] =
            &RaidSscActionContext::lady_vashj_set_grounding_totem_in_main_tank_group;

        creators["lady vashj static charge move away from group"] =
            &RaidSscActionContext::lady_vashj_static_charge_move_away_from_group;

        creators["lady vashj misdirect to main tank"] =
            &RaidSscActionContext::lady_vashj_misdirect_to_main_tank;

        creators["lady vashj assign target priority"] =
            &RaidSscActionContext::lady_vashj_assign_target_priority;

        creators["lady vashj tank apply fear ward"] =
            &RaidSscActionContext::lady_vashj_tank_apply_fear_ward;

        creators["lady vashj position coilfang strider"] =
            &RaidSscActionContext::lady_vashj_position_coilfang_strider;

        creators["lady vashj position coilfang elite"] =
            &RaidSscActionContext::lady_vashj_position_coilfang_elite;

        creators["lady vashj tank wait in the middle"] =
            &RaidSscActionContext::lady_vashj_tank_wait_in_the_middle;

        creators["lady vashj assign tainted core looter"] =
            &RaidSscActionContext::lady_vashj_assign_tainted_core_looter;

        creators["lady vashj attack tainted elemental"] =
            &RaidSscActionContext::lady_vashj_attack_tainted_elemental;

        creators["lady vashj loot tainted core"] =
            &RaidSscActionContext::lady_vashj_loot_tainted_core;

        creators["lady vashj pass the tainted core"] =
            &RaidSscActionContext::lady_vashj_pass_the_tainted_core;

        creators["lady vashj destroy tainted core"] =
            &RaidSscActionContext::lady_vashj_destroy_tainted_core;

        creators["lady vashj command pet target"] =
            &RaidSscActionContext::lady_vashj_command_pet_target;

        creators["lady vashj return to the ground"] =
            &RaidSscActionContext::lady_vashj_return_to_the_ground;

        creators["lady vashj avoid toxic spores"] =
            &RaidSscActionContext::lady_vashj_avoid_toxic_spores;

        creators["lady vashj melee move around toxic spores"] =
            &RaidSscActionContext::lady_vashj_melee_move_around_toxic_spores;

        creators["lady vashj ranged reach around toxic spores"] =
            &RaidSscActionContext::lady_vashj_ranged_reach_around_toxic_spores;

        creators["lady vashj paladin use hand of freedom"] =
            &RaidSscActionContext::lady_vashj_paladin_use_hand_of_freedom;

        creators["lady vashj rogue use cloak of shadows"] =
            &RaidSscActionContext::lady_vashj_rogue_use_cloak_of_shadows;
    }

private:
    // Shared
    static Action* ssc_reset_encounter_states(PlayerbotAI* botAI)
    {
        return new SscResetEncounterStatesAction(botAI);
    }

    // Trash
    static Action* underbog_colossus_escape_toxic_pool(PlayerbotAI* botAI)
    {
        return new UnderbogColossusEscapeToxicPoolAction(botAI);
    }
    static Action* greyheart_tidecaller_mark_water_elemental_totem(PlayerbotAI* botAI)
    {
        return new GreyheartTidecallerMarkWaterElementalTotemAction(botAI);
    }

    // Hydross the Unstable <Duke of Currents>
    static Action* hydross_the_unstable_position_frost_tank(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstablePositionAndSwapTanksAction(
            botAI, "hydross the unstable position frost tank", true);
    }
    static Action* hydross_the_unstable_position_nature_tank(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstablePositionAndSwapTanksAction(
            botAI, "hydross the unstable position nature tank", false);
    }
    static Action* hydross_the_unstable_frost_phase_spread_ranged(PlayerbotAI* botAI)
    {
        return new SscSpreadRangedAction(
            botAI, "hydross the unstable frost phase spread ranged",
            SscHelpers::HYDROSS_FROST_RANGED_SPREAD_DISTANCE);
    }
    static Action* hydross_the_unstable_misdirect_to_tank(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableMisdirectToTankAction(botAI);
    }
    static Action* hydross_the_unstable_stop_attacking_upon_phase_change(PlayerbotAI* botAI)
    {
        return new SscStopAttackingAction(
            botAI, "hydross the unstable stop attacking upon phase change");
    }
    static Action* hydross_the_unstable_manage_phase_timers(PlayerbotAI* botAI)
    {
        return new HydrossTheUnstableManagePhaseTimersAction(botAI);
    }

    // The Lurker Below
    static Action* the_lurker_below_run_around_behind_boss(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowRunAroundBehindBossAction(botAI);
    }
    static Action* the_lurker_below_position_main_tank(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowPositionMainTankAction(botAI);
    }
    static Action* the_lurker_below_spread_ranged_in_arc(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowSpreadRangedInArcAction(botAI);
    }
    static Action* the_lurker_below_tanks_pick_up_guardians(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowTanksPickUpGuardiansAction(botAI);
    }
    static Action* the_lurker_below_melee_move_directly_to_target(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowMeleeMoveDirectlyToTargetAction(botAI);
    }
    static Action* the_lurker_below_melee_get_out_of_water(PlayerbotAI* botAI)
    {
        return new TheLurkerBelowMeleeGetOutOfWaterAction(botAI);
    }

    // Leotheras the Blind
    static Action* leotheras_the_blind_spread_ranged_upon_pull(PlayerbotAI* botAI)
    {
        return new SscSpreadRangedAction(
            botAI, "leotheras the blind spread ranged upon pull",
            SscHelpers::LEOTHERAS_RANGED_SPREAD_DISTANCE);
    }
    static Action* leotheras_the_blind_warlock_tank_attack_demon_form(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindWarlockTankAttackDemonFormAction(botAI);
    }
    static Action* leotheras_the_blind_tanks_build_rage_on_demon_form(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindTanksBuildRageOnDemonFormAction(botAI);
    }
    static Action* leotheras_the_blind_ranged_keep_distance(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindRangedKeepDistanceAction(botAI);
    }
    static Action* leotheras_the_blind_run_away_from_whirlwind(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindRunAwayFromWhirlwindAction(botAI);
    }
    static Action* leotheras_the_blind_melee_run_from_chaos_blast(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindMeleeRunFromChaosBlastAction(botAI);
    }
    static Action* leotheras_the_blind_destroy_inner_demon(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindDestroyInnerDemonAction(botAI);
    }
    static Action* leotheras_the_blind_final_phase_attack_boss(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindFinalPhaseAttackBossAction(botAI);
    }
    static Action* leotheras_the_blind_final_phase_separate_boss_from_demon(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindFinalPhaseSeparateBossFromDemonAction(botAI);
    }
    static Action* leotheras_the_blind_misdirect_demon_form_to_tank(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindMisdirectDemonFormToTankAction(botAI);
    }
    static Action* leotheras_the_blind_melee_stop_attacking(PlayerbotAI* botAI)
    {
        return new SscStopAttackingAction(botAI, "leotheras the blind melee stop attacking");
    }
    static Action* leotheras_the_blind_manage_dps_wait_timers(PlayerbotAI* botAI)
    {
        return new LeotherasTheBlindManageDpsWaitTimersAction(botAI);
    }

    // Fathom-Lord Karathress
    static Action* fathom_lord_karathress_tanks_position_targets(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressTanksPositionTargetsAction(botAI);
    }
    static Action* fathom_lord_karathress_position_caribdis_tank_healer(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressPositionCaribdisTankHealerAction(botAI);
    }
    static Action* fathom_lord_karathress_misdirect_to_tanks(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressMisdirectToTanksAction(botAI);
    }
    static Action* fathom_lord_karathress_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressAssignDpsPriorityAction(botAI);
    }
    static Action* fathom_lord_karathress_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressManageDpsTimerAction(botAI);
    }
    static Action* fathom_lord_karathress_spread_ranged(PlayerbotAI* botAI)
    {
        return new SscSpreadRangedAction(
            botAI, "fathom-lord karathress spread ranged",
            SscHelpers::CARIBDIS_RANGED_SPREAD_DISTANCE);
    }
    static Action* fathom_lord_karathress_drop_to_ground_after_cyclone(PlayerbotAI* botAI)
    {
        return new FathomLordKarathressDropToGroundAfterCycloneAction(botAI);
    }

    // Morogrim Tidewalker
    static Action* morogrim_tidewalker_position_main_tank(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerPositionMainTankAction(botAI);
    }
    static Action* morogrim_tidewalker_stack_ranged_behind_boss(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerStackRangedBehindBossAction(botAI);
    }
    static Action* morogrim_tidewalker_return_to_boss(PlayerbotAI* botAI)
    {
        return new MorogrimTidewalkerReturnToBossAction(botAI);
    }
    static Action* morogrim_tidewalker_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new SscMisdirectToMainTankAction(
            botAI, "morogrim tidewalker misdirect to main tank", "morogrim tidewalker");
    }

    // Lady Vashj <Coilfang Matron>
    static Action* lady_vashj_main_tank_position_boss(PlayerbotAI* botAI)
    {
        return new LadyVashjMainTankPositionBossAction(botAI);
    }
    static Action* lady_vashj_phase_1_spread_ranged_in_arc(PlayerbotAI* botAI)
    {
        return new LadyVashjPhase1SpreadRangedInArcAction(botAI);
    }
    static Action* lady_vashj_assign_station_slots(PlayerbotAI* botAI)
    {
        return new LadyVashjAssignStationSlotsAction(botAI);
    }
    static Action* lady_vashj_phase_2_position_at_station(PlayerbotAI* botAI)
    {
        return new LadyVashjPhase2PositionAtStationAction(botAI);
    }
    static Action* lady_vashj_phase_3_position_ranged(PlayerbotAI* botAI)
    {
        return new LadyVashjPhase3PositionRangedAction(botAI);
    }
    static Action* lady_vashj_phase_3_move_into_sight(PlayerbotAI* botAI)
    {
        return new LadyVashjPhase3MoveIntoSightAction(botAI);
    }
    static Action* lady_vashj_assign_grounding_shaman(PlayerbotAI* botAI)
    {
        return new LadyVashjAssignGroundingShamanAction(botAI);
    }
    static Action* lady_vashj_set_grounding_totem_in_main_tank_group(PlayerbotAI* botAI)
    {
        return new LadyVashjSetGroundingTotemInMainTankGroupAction(botAI);
    }
    static Action* lady_vashj_static_charge_move_away_from_group(PlayerbotAI* botAI)
    {
        return new LadyVashjStaticChargeMoveAwayFromGroupAction(botAI);
    }
    static Action* lady_vashj_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new SscMisdirectToMainTankAction(
            botAI, "lady vashj misdirect to main tank", "lady vashj");
    }
    static Action* lady_vashj_assign_target_priority(PlayerbotAI* botAI)
    {
        return new LadyVashjAssignTargetPriorityAction(botAI);
    }
    static Action* lady_vashj_tank_apply_fear_ward(PlayerbotAI* botAI)
    {
        return new LadyVashjTankApplyFearWardAction(botAI);
    }
    static Action* lady_vashj_position_coilfang_strider(PlayerbotAI* botAI)
    {
        return new LadyVashjPositionCoilfangStriderAction(botAI);
    }
    static Action* lady_vashj_position_coilfang_elite(PlayerbotAI* botAI)
    {
        return new LadyVashjPositionCoilfangEliteAction(botAI);
    }
    static Action* lady_vashj_tank_wait_in_the_middle(PlayerbotAI* botAI)
    {
        return new LadyVashjTankWaitInTheMiddleAction(botAI);
    }
    static Action* lady_vashj_assign_tainted_core_looter(PlayerbotAI* botAI)
    {
        return new LadyVashjAssignTaintedCoreLooterAction(botAI);
    }
    static Action* lady_vashj_attack_tainted_elemental(PlayerbotAI* botAI)
    {
        return new LadyVashjAttackTaintedElementalAction(botAI);
    }
    static Action* lady_vashj_loot_tainted_core(PlayerbotAI* botAI)
    {
        return new LadyVashjLootTaintedCoreAction(botAI);
    }
    static Action* lady_vashj_pass_the_tainted_core(PlayerbotAI* botAI)
    {
        return new LadyVashjPassTheTaintedCoreAction(botAI);
    }
    static Action* lady_vashj_destroy_tainted_core(PlayerbotAI* botAI)
    {
        return new LadyVashjDestroyTaintedCoreAction(botAI);
    }
    static Action* lady_vashj_command_pet_target(PlayerbotAI* botAI)
    {
        return new LadyVashjCommandPetTargetAction(botAI);
    }
    static Action* lady_vashj_return_to_the_ground(PlayerbotAI* botAI)
    {
        return new LadyVashjReturnToTheGroundAction(botAI);
    }
    static Action* lady_vashj_avoid_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjAvoidToxicSporesAction(botAI);
    }
    static Action* lady_vashj_melee_move_around_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjMeleeMoveAroundToxicSporesAction(botAI);
    }
    static Action* lady_vashj_ranged_reach_around_toxic_spores(PlayerbotAI* botAI)
    {
        return new LadyVashjRangedReachAroundToxicSporesAction(botAI);
    }
    static Action* lady_vashj_paladin_use_hand_of_freedom(PlayerbotAI* botAI)
    {
        return new LadyVashjPaladinUseHandOfFreedomAction(botAI);
    }
    static Action* lady_vashj_rogue_use_cloak_of_shadows(PlayerbotAI* botAI)
    {
        return new LadyVashjRogueUseCloakOfShadowsAction(botAI);
    }
};

#endif
