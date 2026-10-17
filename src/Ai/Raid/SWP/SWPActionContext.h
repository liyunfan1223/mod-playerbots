/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPACTIONCONTEXT_H
#define PLAYERBOTS_SWPACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPActions.h"

class RaidSwpActionContext : public NamedObjectContext<Action>
{
public:
    RaidSwpActionContext()
    {
        // Shared
        creators["sunwell reset encounter states"] =
            &RaidSwpActionContext::sunwell_reset_encounter_states;

        creators["sunwell remove aura"] =
            &RaidSwpActionContext::sunwell_remove_aura;

        // Trash
        creators["volatile fiend keep enemy away from group"] =
            &RaidSwpActionContext::volatile_fiend_keep_enemy_away_from_group;

        creators["apocalypse guard attack with holy magic"] =
            &RaidSwpActionContext::apocalypse_guard_attack_with_holy_magic;

        // Kalecgos
        creators["kalecgos announce boss health"] =
            &RaidSwpActionContext::kalecgos_announce_boss_health;

        creators["kalecgos misdirect to main tank"] =
            &RaidSwpActionContext::kalecgos_misdirect_to_main_tank;

        creators["kalecgos surface tank position dragon"] =
            &RaidSwpActionContext::kalecgos_surface_tank_position_dragon;

        creators["kalecgos enter spectral rift"] =
            &RaidSwpActionContext::kalecgos_enter_spectral_rift;

        creators["kalecgos disperse ranged"] =
            &RaidSwpActionContext::kalecgos_disperse_ranged;

        creators["kalecgos remove arcane buffet"] =
            &RaidSwpActionContext::kalecgos_remove_arcane_buffet;

        creators["kalecgos sathrovarr tank stand with kalec"] =
            &RaidSwpActionContext::kalecgos_sathrovarr_tank_stand_with_kalec;

        creators["kalecgos return to spectral realm ground"] =
            &RaidSwpActionContext::kalecgos_return_to_spectral_realm_ground;

        // Brutallus
        creators["brutallus misdirect to main tank"] =
            &RaidSwpActionContext::brutallus_misdirect_to_main_tank;

        creators["brutallus tanks position and swap"] =
            &RaidSwpActionContext::brutallus_tanks_position_and_swap;

        creators["brutallus position melee at rear center"] =
            &RaidSwpActionContext::brutallus_position_melee_at_rear_center;

        creators["brutallus position ranged in two groups"] =
            &RaidSwpActionContext::brutallus_position_ranged_in_two_groups;

        creators["brutallus isolate burn"] =
            &RaidSwpActionContext::brutallus_isolate_burn;

        // Felmyst
        creators["felmyst misdirect to main tank"] =
            &RaidSwpActionContext::felmyst_misdirect_to_main_tank;

        creators["felmyst main tank position boss on ground"] =
            &RaidSwpActionContext::felmyst_main_tank_position_boss_on_ground;

        creators["felmyst ranged stack in three groups"] =
            &RaidSwpActionContext::felmyst_ranged_stack_in_three_groups;

        creators["felmyst melee stack behind boss"] =
            &RaidSwpActionContext::felmyst_melee_stack_behind_boss;

        creators["felmyst remove encapsulate"] =
            &RaidSwpActionContext::felmyst_remove_encapsulate;

        creators["felmyst run away from encapsulated player"] =
            &RaidSwpActionContext::felmyst_run_away_from_encapsulated_player;

        creators["felmyst mass dispel gas nova"] =
            &RaidSwpActionContext::felmyst_mass_dispel_gas_nova;

        creators["felmyst avoid demonic vapor"] =
            &RaidSwpActionContext::felmyst_avoid_demonic_vapor;

        creators["felmyst kite demonic vapor"] =
            &RaidSwpActionContext::felmyst_kite_demonic_vapor;

        creators["felmyst move to safe fog lane"] =
            &RaidSwpActionContext::felmyst_move_to_safe_fog_lane;

        creators["felmyst melee clear target"] =
            &RaidSwpActionContext::felmyst_melee_clear_target;

        creators["felmyst kill charmed player"] =
            &RaidSwpActionContext::felmyst_kill_charmed_player;

        creators["felmyst manage landing dps timer"] =
            &RaidSwpActionContext::felmyst_manage_landing_dps_timer;

        // Eredar Twins
        creators["eredar twins melee jump from balcony"] =
            &RaidSwpActionContext::eredar_twins_melee_jump_from_balcony;

        creators["eredar twins announce alythess tank"] =
            &RaidSwpActionContext::eredar_twins_announce_alythess_tank;

        creators["eredar twins misdirect to tanks"] =
            &RaidSwpActionContext::eredar_twins_misdirect_to_tanks;

        creators["eredar twins position sacrolash tanks"] =
            &RaidSwpActionContext::eredar_twins_position_sacrolash_tanks;

        creators["eredar twins alythess tank move out of blaze"] =
            &RaidSwpActionContext::eredar_twins_alythess_tank_move_out_of_blaze;

        creators["eredar twins ranged stack at balcony edge"] =
            &RaidSwpActionContext::eredar_twins_ranged_stack_at_balcony_edge;

        creators["eredar twins stack in room center"] =
            &RaidSwpActionContext::eredar_twins_stack_in_room_center;

        creators["eredar twins remove flame sear"] =
            &RaidSwpActionContext::eredar_twins_remove_flame_sear;

        creators["eredar twins dps prioritize sacrolash"] =
            &RaidSwpActionContext::eredar_twins_dps_prioritize_sacrolash;

        creators["eredar twins conflagration target move from group"] =
            &RaidSwpActionContext::eredar_twins_conflagration_target_move_from_group;

        creators["eredar twins move away from sacrolash victim"] =
            &RaidSwpActionContext::eredar_twins_move_away_from_sacrolash_victim;

        // M'uru
        creators["m'uru misdirect enemy to tank"] =
            &RaidSwpActionContext::muru_misdirect_enemy_to_tank;

        creators["m'uru main tank pick up entropius"] =
            &RaidSwpActionContext::muru_main_tank_pick_up_entropius;

        creators["m'uru position ranged by phase"] =
            &RaidSwpActionContext::muru_position_ranged_by_phase;

        creators["m'uru assign dps priority"] =
            &RaidSwpActionContext::muru_assign_dps_priority;

        creators["m'uru kill dark fiends with dispel"] =
            &RaidSwpActionContext::muru_kill_dark_fiends_with_dispel;

        creators["m'uru tanks move sentinel to safe position"] =
            &RaidSwpActionContext::muru_tanks_move_sentinel_to_safe_position;

        creators["m'uru second assist tank guard ranged"] =
            &RaidSwpActionContext::muru_second_assist_tank_guard_ranged;

        creators["m'uru melee flee the darkness"] =
            &RaidSwpActionContext::muru_melee_flee_the_darkness;

        creators["m'uru cast stun on berserker"] =
            &RaidSwpActionContext::muru_cast_stun_on_berserker;

        creators["m'uru interrupt fel fireball"] =
            &RaidSwpActionContext::muru_interrupt_fel_fireball;

        creators["m'uru cast spellsteal on spell fury"] =
            &RaidSwpActionContext::muru_cast_spellsteal_on_spell_fury;

        creators["m'uru warlock enslave void spawn"] =
            &RaidSwpActionContext::muru_warlock_enslave_void_spawn;

        creators["m'uru void spawn cast shadow bolt volley"] =
            &RaidSwpActionContext::muru_void_spawn_cast_shadow_bolt_volley;

        creators["m'uru keep distance from dark fiends"] =
            &RaidSwpActionContext::muru_keep_distance_from_dark_fiends;

        creators["m'uru escape the singularity"] =
            &RaidSwpActionContext::muru_escape_the_singularity;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden announce dragon orb user"] =
            &RaidSwpActionContext::kiljaeden_announce_dragon_orb_user;

        creators["kil'jaeden control hands of the deceiver"] =
            &RaidSwpActionContext::kiljaeden_control_hands_of_the_deceiver;

        creators["kil'jaeden mark hand of the deceiver"] =
            &RaidSwpActionContext::kiljaeden_mark_hand_of_the_deceiver;

        creators["kil'jaeden move holy paladin into stun range"] =
            &RaidSwpActionContext::kiljaeden_move_holy_paladin_into_stun_range;

        creators["kil'jaeden position and move tanks"] =
            &RaidSwpActionContext::kiljaeden_position_and_move_tanks;

        creators["kil'jaeden position melee and avoid armageddons"] =
            &RaidSwpActionContext::kiljaeden_position_melee_and_avoid_armageddons;

        creators["kil'jaeden position ranged and avoid armageddons"] =
            &RaidSwpActionContext::kiljaeden_position_ranged_and_avoid_armageddons;

        creators["kil'jaeden remove fire bloom"] =
            &RaidSwpActionContext::kiljaeden_remove_fire_bloom;

        creators["kil'jaeden stack for shield of the blue"] =
            &RaidSwpActionContext::kiljaeden_stack_for_shield_of_the_blue;

        creators["kil'jaeden use dragon orb"] =
            &RaidSwpActionContext::kiljaeden_use_dragon_orb;

        creators["kil'jaeden dragon buff and protect raid"] =
            &RaidSwpActionContext::kiljaeden_dragon_buff_and_protect_raid;

        creators["kil'jaeden release stale root"] =
            &RaidSwpActionContext::kiljaeden_release_stale_root;
    }

private:
    // Shared
    static Action* sunwell_reset_encounter_states(PlayerbotAI* botAI)
    {
        return new SunwellResetEncounterStatesAction(botAI);
    }
    static Action* sunwell_remove_aura(PlayerbotAI* botAI)
    {
        return new SunwellRemoveAuraAction(botAI);
    }

    // Trash
    static Action* volatile_fiend_keep_enemy_away_from_group(PlayerbotAI* botAI)
    {
        return new VolatileFiendKeepEnemyAwayFromGroupAction(botAI);
    }
    static Action* apocalypse_guard_attack_with_holy_magic(PlayerbotAI* botAI)
    {
        return new ApocalypseGuardAttackWithHolyMagicAction(botAI);
    }

    // Kalecgos
    static Action* kalecgos_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new SunwellMisdirectToMainTankAction(
            botAI, "kalecgos misdirect to main tank", "kalecgos");
    }
    static Action* kalecgos_announce_boss_health(PlayerbotAI* botAI)
    {
        return new KalecgosAnnounceBossHealthAction(botAI);
    }
    static Action* kalecgos_surface_tank_position_dragon(PlayerbotAI* botAI)
    {
        return new KalecgosSurfaceTankPositionDragonAction(botAI);
    }
    static Action* kalecgos_enter_spectral_rift(PlayerbotAI* botAI)
    {
        return new KalecgosEnterSpectralRiftAction(botAI);
    }
    static Action* kalecgos_disperse_ranged(PlayerbotAI* botAI)
    {
        return new KalecgosDisperseRangedAction(botAI);
    }
    static Action* kalecgos_remove_arcane_buffet(PlayerbotAI* botAI)
    {
        return new SunwellRemoveDebuffWithImmunityAction(
            botAI, "kalecgos remove arcane buffet");
    }
    static Action* kalecgos_sathrovarr_tank_stand_with_kalec(PlayerbotAI* botAI)
    {
        return new KalecgosSathrovarrTankStandWithKalecAction(botAI);
    }
    static Action* kalecgos_return_to_spectral_realm_ground(PlayerbotAI* botAI)
    {
        return new KalecgosReturnToSpectralRealmGroundAction(botAI);
    }

    // Brutallus
    static Action* brutallus_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new SunwellMisdirectToMainTankAction(
            botAI, "brutallus misdirect to main tank", "brutallus");
    }
    static Action* brutallus_tanks_position_and_swap(PlayerbotAI* botAI)
    {
        return new BrutallusTanksPositionAndSwapAction(botAI);
    }
    static Action* brutallus_position_melee_at_rear_center(PlayerbotAI* botAI)
    {
        return new BrutallusPositionMeleeAtRearCenterAction(botAI);
    }
    static Action* brutallus_position_ranged_in_two_groups(PlayerbotAI* botAI)
    {
        return new BrutallusPositionRangedInTwoGroupsAction(botAI);
    }
    static Action* brutallus_isolate_burn(PlayerbotAI* botAI)
    {
        return new BrutallusIsolateBurnAction(botAI);
    }

    // Felmyst
    static Action* felmyst_misdirect_to_main_tank(PlayerbotAI* botAI)
    {
        return new SunwellMisdirectToMainTankAction(
            botAI, "felmyst misdirect to main tank", "felmyst");
    }
    static Action* felmyst_main_tank_position_boss_on_ground(PlayerbotAI* botAI)
    {
        return new FelmystMainTankPositionBossOnGroundAction(botAI);
    }
    static Action* felmyst_ranged_stack_in_three_groups(PlayerbotAI* botAI)
    {
        return new FelmystRangedStackInThreeGroupsAction(botAI);
    }
    static Action* felmyst_melee_stack_behind_boss(PlayerbotAI* botAI)
    {
        return new FelmystMeleeStackBehindBossAction(botAI);
    }
    static Action* felmyst_remove_encapsulate(PlayerbotAI* botAI)
    {
        return new FelmystRemoveEncapsulateAction(botAI);
    }
    static Action* felmyst_run_away_from_encapsulated_player(PlayerbotAI* botAI)
    {
        return new FelmystRunAwayFromEncapsulatedPlayerAction(botAI);
    }
    static Action* felmyst_mass_dispel_gas_nova(PlayerbotAI* botAI)
    {
        return new FelmystMassDispelGasNovaAction(botAI);
    }
    static Action* felmyst_avoid_demonic_vapor(PlayerbotAI* botAI)
    {
        return new FelmystAvoidDemonicVaporAction(botAI);
    }
    static Action* felmyst_kite_demonic_vapor(PlayerbotAI* botAI)
    {
        return new FelmystKiteDemonicVaporAction(botAI);
    }
    static Action* felmyst_move_to_safe_fog_lane(PlayerbotAI* botAI)
    {
        return new FelmystMoveToSafeFogLaneAction(botAI);
    }
    static Action* felmyst_melee_clear_target(PlayerbotAI* botAI)
    {
        return new FelmystMeleeClearTargetAction(botAI);
    }
    static Action* felmyst_kill_charmed_player(PlayerbotAI* botAI)
    {
        return new FelmystKillCharmedPlayerAction(botAI);
    }
    static Action* felmyst_manage_landing_dps_timer(PlayerbotAI* botAI)
    {
        return new FelmystManageLandingDpsTimerAction(botAI);
    }

    // Eredar Twins
    static Action* eredar_twins_melee_jump_from_balcony(PlayerbotAI* botAI)
    {
        return new EredarTwinsMeleeJumpFromBalconyAction(botAI);
    }
    static Action* eredar_twins_announce_alythess_tank(PlayerbotAI* botAI)
    {
        return new EredarTwinsAnnounceAlythessTankAction(botAI);
    }
    static Action* eredar_twins_misdirect_to_tanks(PlayerbotAI* botAI)
    {
        return new EredarTwinsMisdirectToTanksAction(botAI);
    }
    static Action* eredar_twins_position_sacrolash_tanks(PlayerbotAI* botAI)
    {
        return new EredarTwinsPositionSacrolashTanksAction(botAI);
    }
    static Action* eredar_twins_alythess_tank_move_out_of_blaze(PlayerbotAI* botAI)
    {
        return new EredarTwinsAlythessTankMoveOutOfBlazeAction(botAI);
    }
    static Action* eredar_twins_ranged_stack_at_balcony_edge(PlayerbotAI* botAI)
    {
        return new EredarTwinsRangedStackAtBalconyEdgeAction(botAI);
    }
    static Action* eredar_twins_stack_in_room_center(PlayerbotAI* botAI)
    {
        return new EredarTwinsStackInRoomCenterAction(botAI);
    }
    static Action* eredar_twins_remove_flame_sear(PlayerbotAI* botAI)
    {
        return new SunwellRemoveDebuffWithImmunityAction(
            botAI, "eredar twins remove flame sear");
    }
    static Action* eredar_twins_dps_prioritize_sacrolash(PlayerbotAI* botAI)
    {
        return new EredarTwinsDpsPrioritizeSacrolashAction(botAI);
    }
    static Action* eredar_twins_conflagration_target_move_from_group(PlayerbotAI* botAI)
    {
        return new EredarTwinsConflagrationTargetMoveFromGroupAction(botAI);
    }
    static Action* eredar_twins_move_away_from_sacrolash_victim(PlayerbotAI* botAI)
    {
        return new EredarTwinsMoveAwayFromSacrolashVictimAction(botAI);
    }

    // M'uru
    static Action* muru_misdirect_enemy_to_tank(PlayerbotAI* botAI)
    {
        return new MuruMisdirectEnemyToTankAction(botAI);
    }
    static Action* muru_main_tank_pick_up_entropius(PlayerbotAI* botAI)
    {
        return new MuruMainTankPickUpEntropiusAction(botAI);
    }
    static Action* muru_position_ranged_by_phase(PlayerbotAI* botAI)
    {
        return new MuruPositionRangedByPhaseAction(botAI);
    }
    static Action* muru_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new MuruAssignDpsPriorityAction(botAI);
    }
    static Action* muru_kill_dark_fiends_with_dispel(PlayerbotAI* botAI)
    {
        return new MuruKillDarkFiendsWithDispelAction(botAI);
    }
    static Action* muru_tanks_move_sentinel_to_safe_position(PlayerbotAI* botAI)
    {
        return new MuruTanksMoveSentinelToSafePositionAction(botAI);
    }
    static Action* muru_second_assist_tank_guard_ranged(PlayerbotAI* botAI)
    {
        return new MuruSecondAssistTankGuardRangedAction(botAI);
    }
    static Action* muru_melee_flee_the_darkness(PlayerbotAI* botAI)
    {
        return new MuruMeleeFleeTheDarknessAction(botAI);
    }
    static Action* muru_cast_stun_on_berserker(PlayerbotAI* botAI)
    {
        return new MuruCastStunOnBerserkerAction(botAI);
    }
    static Action* muru_interrupt_fel_fireball(PlayerbotAI* botAI)
    {
        return new MuruInterruptFelFireballAction(botAI);
    }
    static Action* muru_cast_spellsteal_on_spell_fury(PlayerbotAI* botAI)
    {
        return new MuruCastSpellStealOnSpellFuryAction(botAI);
    }
    static Action* muru_warlock_enslave_void_spawn(PlayerbotAI* botAI)
    {
        return new MuruWarlockEnslaveVoidSpawnAction(botAI);
    }
    static Action* muru_void_spawn_cast_shadow_bolt_volley(PlayerbotAI* botAI)
    {
        return new MuruVoidSpawnCastShadowBoltVolleyAction(botAI);
    }
    static Action* muru_keep_distance_from_dark_fiends(PlayerbotAI* botAI)
    {
        return new MuruKeepDistanceFromDarkFiendsAction(botAI);
    }
    static Action* muru_escape_the_singularity(PlayerbotAI* botAI)
    {
        return new MuruEscapeTheSingularityAction(botAI);
    }

    // Kil'jaeden <The Deceiver>
    static Action* kiljaeden_announce_dragon_orb_user(PlayerbotAI* botAI)
    {
        return new KiljaedenAnnounceDragonOrbUserAction(botAI);
    }
    static Action* kiljaeden_control_hands_of_the_deceiver(PlayerbotAI* botAI)
    {
        return new KiljaedenControlHandsOfTheDeceiverAction(botAI);
    }
    static Action* kiljaeden_mark_hand_of_the_deceiver(PlayerbotAI* botAI)
    {
        return new KiljaedenMarkHandOfTheDeceiverAction(botAI);
    }
    static Action* kiljaeden_move_holy_paladin_into_stun_range(PlayerbotAI* botAI)
    {
        return new KiljaedenMoveHolyPaladinIntoStunRangeAction(botAI);
    }
    static Action* kiljaeden_position_and_move_tanks(PlayerbotAI* botAI)
    {
        return new KiljaedenPositionAndMoveTanksAction(botAI);
    }
    static Action* kiljaeden_position_melee_and_avoid_armageddons(PlayerbotAI* botAI)
    {
        return new KiljaedenPositionMeleeAndAvoidArmageddonsAction(botAI);
    }
    static Action* kiljaeden_position_ranged_and_avoid_armageddons(PlayerbotAI* botAI)
    {
        return new KiljaedenPositionRangedAndAvoidArmageddonsAction(botAI);
    }
    static Action* kiljaeden_remove_fire_bloom(PlayerbotAI* botAI)
    {
        return new SunwellRemoveDebuffWithImmunityAction(
            botAI, "kil'jaeden remove fire bloom");
    }
    static Action* kiljaeden_stack_for_shield_of_the_blue(PlayerbotAI* botAI)
    {
        return new KiljaedenStackForShieldOfTheBlueAction(botAI);
    }
    static Action* kiljaeden_use_dragon_orb(PlayerbotAI* botAI)
    {
        return new KiljaedenUseDragonOrbAction(botAI);
    }
    static Action* kiljaeden_dragon_buff_and_protect_raid(PlayerbotAI* botAI)
    {
        return new KiljaedenDragonBuffAndProtectRaidAction(botAI);
    }
    static Action* kiljaeden_release_stale_root(PlayerbotAI* botAI)
    {
        return new KiljaedenReleaseStaleRootAction(botAI);
    }
};

#endif
