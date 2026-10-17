/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPTRIGGERCONTEXT_H
#define PLAYERBOTS_SWPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SWPTriggers.h"

class RaidSwpTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidSwpTriggerContext()
    {
        // Shared
        creators["sunwell no encounter in progress"] =
            &RaidSwpTriggerContext::sunwell_no_encounter_in_progress;

        creators["sunwell aura to remove"] = &RaidSwpTriggerContext::sunwell_aura_to_remove;

        // Trash
        creators["volatile fiend self destructs when near"] =
            &RaidSwpTriggerContext::volatile_fiend_self_destructs_when_near;

        creators["apocalypse guard protected by infernal defense"] =
            &RaidSwpTriggerContext::apocalypse_guard_protected_by_infernal_defense;

        // Kalecgos
        creators["kalecgos should communicate boss health"] =
            &RaidSwpTriggerContext::kalecgos_should_communicate_boss_health;

        creators["kalecgos hunter should misdirect"] =
            &RaidSwpTriggerContext::kalecgos_hunter_should_misdirect;

        creators["kalecgos requires tank rotation"] =
            &RaidSwpTriggerContext::kalecgos_requires_tank_rotation;

        creators["kalecgos spectral rift is open"] =
            &RaidSwpTriggerContext::kalecgos_spectral_rift_is_open;

        creators["kalecgos ranged should spread"] =
            &RaidSwpTriggerContext::kalecgos_ranged_should_spread;

        creators["kalecgos too many arcane buffet stacks"] =
            &RaidSwpTriggerContext::kalecgos_too_many_arcane_buffet_stacks;

        creators["kalecgos humanoid kalec tanks sathrovarr"] =
            &RaidSwpTriggerContext::kalecgos_humanoid_kalec_tanks_sathrovarr;

        creators["kalecgos bots don't observe gravity"] =
            &RaidSwpTriggerContext::kalecgos_bots_dont_observe_gravity;

        // Brutallus
        creators["brutallus hunter should misdirect"] =
            &RaidSwpTriggerContext::brutallus_hunter_should_misdirect;

        creators["brutallus requires two tanks"] =
            &RaidSwpTriggerContext::brutallus_requires_two_tanks;

        creators["brutallus melee should stand in place"] =
            &RaidSwpTriggerContext::brutallus_melee_should_stand_in_place;

        creators["brutallus ranged should soak meteor slash"] =
            &RaidSwpTriggerContext::brutallus_ranged_should_soak_meteor_slash;

        creators["brutallus burn on non-tank"] = &RaidSwpTriggerContext::brutallus_burn_on_non_tank;

        // Felmyst
        creators["felmyst hunter should misdirect"] =
            &RaidSwpTriggerContext::felmyst_hunter_should_misdirect;

        creators["felmyst ground phase should be tanked"] =
            &RaidSwpTriggerContext::felmyst_ground_phase_should_be_tanked;

        creators["felmyst ranged should position to dispel and flee"] =
            &RaidSwpTriggerContext::felmyst_ranged_should_position_to_dispel_and_flee;

        creators["felmyst melee should stay together"] =
            &RaidSwpTriggerContext::felmyst_melee_should_stay_together;

        creators["felmyst encapsulate on mage or paladin"] =
            &RaidSwpTriggerContext::felmyst_encapsulate_on_mage_or_paladin;

        creators["felmyst near encapsulated player"] =
            &RaidSwpTriggerContext::felmyst_near_encapsulated_player;

        creators["felmyst player has gas nova"] =
            &RaidSwpTriggerContext::felmyst_player_has_gas_nova;

        creators["felmyst should avoid demonic vapor trails"] =
            &RaidSwpTriggerContext::felmyst_should_avoid_demonic_vapor_trails;

        creators["felmyst targeted by demonic vapor"] =
            &RaidSwpTriggerContext::felmyst_targeted_by_demonic_vapor;

        creators["felmyst fog of corruption is active"] =
            &RaidSwpTriggerContext::felmyst_fog_of_corruption_is_active;

        creators["felmyst melee cannot reach flying boss"] =
            &RaidSwpTriggerContext::felmyst_melee_cannot_reach_flying_boss;

        creators["felmyst player is charmed by fog"] =
            &RaidSwpTriggerContext::felmyst_player_is_charmed_by_fog;

        creators["felmyst should hold dps while landing"] =
            &RaidSwpTriggerContext::felmyst_should_hold_dps_while_landing;

        // Eredar Twins
        creators["eredar twins melee is at balcony"] =
            &RaidSwpTriggerContext::eredar_twins_melee_is_at_balcony;

        creators["eredar twins should announce alythess tank"] =
            &RaidSwpTriggerContext::eredar_twins_should_announce_alythess_tank;

        creators["eredar twins hunter should misdirect"] =
            &RaidSwpTriggerContext::eredar_twins_hunter_should_misdirect;

        creators["eredar twins sacrolash requires two tanks"] =
            &RaidSwpTriggerContext::eredar_twins_sacrolash_requires_two_tanks;

        creators["eredar twins alythess casts blaze on tank"] =
            &RaidSwpTriggerContext::eredar_twins_alythess_casts_blaze_on_tank;

        creators["eredar twins ranged needs los"] =
            &RaidSwpTriggerContext::eredar_twins_ranged_needs_los;

        creators["eredar twins only alythess remains"] =
            &RaidSwpTriggerContext::eredar_twins_only_alythess_remains;

        creators["eredar twins too many flame touched stacks"] =
            &RaidSwpTriggerContext::eredar_twins_too_many_flame_touched_stacks;

        creators["eredar twins should focus dps"] =
            &RaidSwpTriggerContext::eredar_twins_should_focus_dps;

        creators["eredar twins active conflagration target"] =
            &RaidSwpTriggerContext::eredar_twins_active_conflagration_target;

        creators["eredar twins sacrolash victim has conflagration"] =
            &RaidSwpTriggerContext::eredar_twins_sacrolash_victim_has_conflagration;

        // M'uru
        creators["m'uru hunter should misdirect new enemy"] =
            &RaidSwpTriggerContext::muru_hunter_should_misdirect_new_enemy;

        creators["m'uru transformed into entropius"] =
            &RaidSwpTriggerContext::muru_transformed_into_entropius;

        creators["m'uru ranged should stack or spread"] =
            &RaidSwpTriggerContext::muru_ranged_should_stack_or_spread;

        creators["m'uru should assign dps priority"] =
            &RaidSwpTriggerContext::muru_should_assign_dps_priority;

        creators["m'uru m'uru void sentinel should be tanked"] =
            &RaidSwpTriggerContext::muru_void_sentinel_should_be_tanked;

        creators["m'uru adds spawn at entrance"] =
            &RaidSwpTriggerContext::muru_adds_spawn_at_entrance;

        creators["m'uru dark fiends spawned"] = &RaidSwpTriggerContext::muru_dark_fiends_spawned;

        creators["m'uru darkness is coming"] = &RaidSwpTriggerContext::muru_darkness_is_coming;

        creators["m'uru berserker has flurry"] =
            &RaidSwpTriggerContext::muru_berserker_has_flurry;

        creators["m'uru fury mage casting fel fireball"] =
            &RaidSwpTriggerContext::muru_fury_mage_casting_fel_fireball;

        creators["m'uru fury mage has spell fury"] =
            &RaidSwpTriggerContext::muru_fury_mage_has_spell_fury;

        creators["m'uru void spawn available for enslave"] =
            &RaidSwpTriggerContext::muru_void_spawn_available_for_enslave;

        creators["m'uru warlock has enslaved void spawn"] =
            &RaidSwpTriggerContext::muru_warlock_has_enslaved_void_spawn;

        creators["m'uru entropius summons void zones"] =
            &RaidSwpTriggerContext::muru_entropius_summons_void_zones;

        creators["m'uru the singularity is near"] =
            &RaidSwpTriggerContext::muru_the_singularity_is_near;

        // Kil'jaeden <The Deceiver>
        creators["kil'jaeden should coordinate orb use"] =
            &RaidSwpTriggerContext::kiljaeden_should_coordinate_orb_use;

        creators["kil'jaeden hands of the deceiver are active"] =
            &RaidSwpTriggerContext::kiljaeden_hands_of_the_deceiver_are_active;

        creators["kil'jaeden tanks should hold boss and reflections"] =
            &RaidSwpTriggerContext::kiljaeden_tanks_should_hold_boss_and_reflections;

        creators["kil'jaeden melee should split into two groups"] =
            &RaidSwpTriggerContext::kiljaeden_melee_should_split_into_two_groups;

        creators["kil'jaeden ranged should spread in two arcs"] =
            &RaidSwpTriggerContext::kiljaeden_ranged_should_spread_in_two_arcs;

        creators["kil'jaeden fire bloom on immunity class"] =
            &RaidSwpTriggerContext::kiljaeden_fire_bloom_on_immunity_class;

        creators["kil'jaeden says: Chaos! Destruction! Oblivion!"] =
            &RaidSwpTriggerContext::kiljaeden_says_chaos_destruction_oblivion;

        creators["kil'jaeden dragon orb is active"] =
            &RaidSwpTriggerContext::kiljaeden_dragon_orb_is_active;

        creators["kil'jaeden bot controls dragon"] =
            &RaidSwpTriggerContext::kiljaeden_bot_controls_dragon;

        creators["kil'jaeden stale root after dragon"] =
            &RaidSwpTriggerContext::kiljaeden_stale_root_after_dragon;
    }

private:
    // Shared
    static Trigger* sunwell_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new SunwellNoEncounterInProgressTrigger(botAI);
    }
    static Trigger* sunwell_aura_to_remove(PlayerbotAI* botAI)
    {
        return new SunwellAuraToRemoveTrigger(botAI);
    }

    // Trash
    static Trigger* volatile_fiend_self_destructs_when_near(PlayerbotAI* botAI)
    {
        return new VolatileFiendSelfDestructsWhenNearTrigger(botAI);
    }
    static Trigger* apocalypse_guard_protected_by_infernal_defense(PlayerbotAI* botAI)
    {
        return new ApocalypseGuardProtectedByInfernalDefenseTrigger(botAI);
    }

    // Kalecgos
    static Trigger* kalecgos_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SunwellHunterShouldMisdirectTrigger(
            botAI, "kalecgos hunter should misdirect", "kalecgos");
    }
    static Trigger* kalecgos_should_communicate_boss_health(PlayerbotAI* botAI)
    {
        return new KalecgosShouldCommunicateBossHealthTrigger(botAI);
    }
    static Trigger* kalecgos_requires_tank_rotation(PlayerbotAI* botAI)
    {
        return new KalecgosRequiresTankRotationTrigger(botAI);
    }
    static Trigger* kalecgos_spectral_rift_is_open(PlayerbotAI* botAI)
    {
        return new KalecgosSpectralRiftIsOpenTrigger(botAI);
    }
    static Trigger* kalecgos_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new KalecgosRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* kalecgos_humanoid_kalec_tanks_sathrovarr(PlayerbotAI* botAI)
    {
        return new KalecgosHumanoidKalecTanksSathrovarrTrigger(botAI);
    }
    static Trigger* kalecgos_too_many_arcane_buffet_stacks(PlayerbotAI* botAI)
    {
        return new KalecgosTooManyArcaneBuffetStacksTrigger(botAI);
    }
    static Trigger* kalecgos_bots_dont_observe_gravity(PlayerbotAI* botAI)
    {
        return new KalecgosBotsDontObserveGravityTrigger(botAI);
    }

    // Brutallus
    static Trigger* brutallus_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SunwellHunterShouldMisdirectTrigger(
            botAI, "brutallus hunter should misdirect", "brutallus");
    }
    static Trigger* brutallus_requires_two_tanks(PlayerbotAI* botAI)
    {
        return new BrutallusRequiresTwoTanksTrigger(botAI);
    }
    static Trigger* brutallus_melee_should_stand_in_place(PlayerbotAI* botAI)
    {
        return new BrutallusMeleeShouldStandInPlaceTrigger(botAI);
    }
    static Trigger* brutallus_ranged_should_soak_meteor_slash(PlayerbotAI* botAI)
    {
        return new BrutallusRangedShouldSoakMeteorSlashTrigger(botAI);
    }
    static Trigger* brutallus_burn_on_non_tank(PlayerbotAI* botAI)
    {
        return new BrutallusBurnOnNonTankTrigger(botAI);
    }

    // Felmyst
    static Trigger* felmyst_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new FelmystHunterShouldMisdirectTrigger(botAI);
    }
    static Trigger* felmyst_ground_phase_should_be_tanked(PlayerbotAI* botAI)
    {
        return new FelmystGroundPhaseShouldBeTankedTrigger(botAI);
    }
    static Trigger* felmyst_ranged_should_position_to_dispel_and_flee(PlayerbotAI* botAI)
    {
        return new FelmystRangedShouldPositionToDispelAndFleeTrigger(botAI);
    }
    static Trigger* felmyst_melee_should_stay_together(PlayerbotAI* botAI)
    {
        return new FelmystMeleeShouldStayTogetherTrigger(botAI);
    }
    static Trigger* felmyst_encapsulate_on_mage_or_paladin(PlayerbotAI* botAI)
    {
        return new FelmystEncapsulateOnMageOrPaladinTrigger(botAI);
    }
    static Trigger* felmyst_near_encapsulated_player(PlayerbotAI* botAI)
    {
        return new FelmystNearEncapsulatedPlayerTrigger(botAI);
    }
    static Trigger* felmyst_player_has_gas_nova(PlayerbotAI* botAI)
    {
        return new FelmystPlayerHasGasNovaTrigger(botAI);
    }
    static Trigger* felmyst_should_avoid_demonic_vapor_trails(PlayerbotAI* botAI)
    {
        return new FelmystShouldAvoidDemonicVaporTrailsTrigger(botAI);
    }
    static Trigger* felmyst_targeted_by_demonic_vapor(PlayerbotAI* botAI)
    {
        return new FelmystTargetedByDemonicVaporTrigger(botAI);
    }
    static Trigger* felmyst_fog_of_corruption_is_active(PlayerbotAI* botAI)
    {
        return new FelmystFogOfCorruptionIsActiveTrigger(botAI);
    }
    static Trigger* felmyst_melee_cannot_reach_flying_boss(PlayerbotAI* botAI)
    {
        return new FelmystMeleeCannotReachFlyingBossTrigger(botAI);
    }
    static Trigger* felmyst_player_is_charmed_by_fog(PlayerbotAI* botAI)
    {
        return new FelmystPlayerIsCharmedByFogTrigger(botAI);
    }
    static Trigger* felmyst_should_hold_dps_while_landing(PlayerbotAI* botAI)
    {
        return new FelmystShouldHoldDpsWhileLandingTrigger(botAI);
    }

    // Eredar Twins
    static Trigger* eredar_twins_melee_is_at_balcony(PlayerbotAI* botAI)
    {
        return new EredarTwinsMeleeIsAtBalconyTrigger(botAI);
    }
    static Trigger* eredar_twins_should_announce_alythess_tank(PlayerbotAI* botAI)
    {
        return new EredarTwinsShouldAnnounceAlythessTankTrigger(botAI);
    }
    static Trigger* eredar_twins_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new SunwellHunterShouldMisdirectTrigger(
            botAI, "eredar twins hunter should misdirect", "grand warlock alythess");
    }
    static Trigger* eredar_twins_sacrolash_requires_two_tanks(PlayerbotAI* botAI)
    {
        return new EredarTwinsSacrolashRequiresTwoTanksTrigger(botAI);
    }
    static Trigger* eredar_twins_alythess_casts_blaze_on_tank(PlayerbotAI* botAI)
    {
        return new EredarTwinsAlythessCastsBlazeOnTankTrigger(botAI);
    }
    static Trigger* eredar_twins_ranged_needs_los(PlayerbotAI* botAI)
    {
        return new EredarTwinsRangedNeedsLosTrigger(botAI);
    }
    static Trigger* eredar_twins_only_alythess_remains(PlayerbotAI* botAI)
    {
        return new EredarTwinsOnlyAlythessRemainsTrigger(botAI);
    }
    static Trigger* eredar_twins_too_many_flame_touched_stacks(PlayerbotAI* botAI)
    {
        return new EredarTwinsTooManyFlameTouchedStacksTrigger(botAI);
    }
    static Trigger* eredar_twins_should_focus_dps(PlayerbotAI* botAI)
    {
        return new EredarTwinsShouldFocusDpsTrigger(botAI);
    }
    static Trigger* eredar_twins_active_conflagration_target(PlayerbotAI* botAI)
    {
        return new EredarTwinsActiveConflagrationTargetTrigger(botAI);
    }
    static Trigger* eredar_twins_sacrolash_victim_has_conflagration(PlayerbotAI* botAI)
    {
        return new EredarTwinsSacrolashVictimHasConflagrationTrigger(botAI);
    }

    // M'uru
    static Trigger* muru_hunter_should_misdirect_new_enemy(PlayerbotAI* botAI)
    {
        return new MuruHunterShouldMisdirectNewEnemyTrigger(botAI);
    }
    static Trigger* muru_transformed_into_entropius(PlayerbotAI* botAI)
    {
        return new MuruTransformedIntoEntropiusTrigger(botAI);
    }
    static Trigger* muru_ranged_should_stack_or_spread(PlayerbotAI* botAI)
    {
        return new MuruRangedShouldStackOrSpreadTrigger(botAI);
    }
    static Trigger* muru_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new MuruShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* muru_void_sentinel_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MuruVoidSentinelShouldBeTankedTrigger(botAI);
    }
    static Trigger* muru_adds_spawn_at_entrance(PlayerbotAI* botAI)
    {
        return new MuruAddsSpawnAtEntranceTrigger(botAI);
    }
    static Trigger* muru_dark_fiends_spawned(PlayerbotAI* botAI)
    {
        return new MuruDarkFiendsSpawnedTrigger(botAI);
    }
    static Trigger* muru_darkness_is_coming(PlayerbotAI* botAI)
    {
        return new MuruDarknessIsComingTrigger(botAI);
    }
    static Trigger* muru_berserker_has_flurry(PlayerbotAI* botAI)
    {
        return new MuruBerserkerHasFlurryTrigger(botAI);
    }
    static Trigger* muru_fury_mage_casting_fel_fireball(PlayerbotAI* botAI)
    {
        return new MuruFuryMageCastingFelFireballTrigger(botAI);
    }
    static Trigger* muru_fury_mage_has_spell_fury(PlayerbotAI* botAI)
    {
        return new MuruFuryMageHasSpellFuryTrigger(botAI);
    }
    static Trigger* muru_void_spawn_available_for_enslave(PlayerbotAI* botAI)
    {
        return new MuruVoidSpawnAvailableForEnslaveTrigger(botAI);
    }
    static Trigger* muru_warlock_has_enslaved_void_spawn(PlayerbotAI* botAI)
    {
        return new MuruWarlockHasEnslavedVoidSpawnTrigger(botAI);
    }
    static Trigger* muru_entropius_summons_void_zones(PlayerbotAI* botAI)
    {
        return new MuruEntropiusSummonsVoidZonesTrigger(botAI);
    }
    static Trigger* muru_the_singularity_is_near(PlayerbotAI* botAI)
    {
        return new MuruTheSingularityIsNearTrigger(botAI);
    }

    // Kil'jaeden <The Deceiver>
    static Trigger* kiljaeden_should_coordinate_orb_use(PlayerbotAI* botAI)
    {
        return new KiljaedenShouldCoordinateOrbUseTrigger(botAI);
    }
    static Trigger* kiljaeden_hands_of_the_deceiver_are_active(PlayerbotAI* botAI)
    {
        return new KiljaedenHandsOfTheDeceiverAreActiveTrigger(botAI);
    }
    static Trigger* kiljaeden_tanks_should_hold_boss_and_reflections(PlayerbotAI* botAI)
    {
        return new KiljaedenTanksShouldHoldBossAndReflectionsTrigger(botAI);
    }
    static Trigger* kiljaeden_melee_should_split_into_two_groups(PlayerbotAI* botAI)
    {
        return new KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger(botAI);
    }
    static Trigger* kiljaeden_ranged_should_spread_in_two_arcs(PlayerbotAI* botAI)
    {
        return new KiljaedenRangedShouldSpreadInTwoArcsTrigger(botAI);
    }
    static Trigger* kiljaeden_fire_bloom_on_immunity_class(PlayerbotAI* botAI)
    {
        return new KiljaedenFireBloomOnImmunityClassTrigger(botAI);
    }
    static Trigger* kiljaeden_says_chaos_destruction_oblivion(PlayerbotAI* botAI)
    {
        return new KiljaedenSaysChaosDestructionOblivionTrigger(botAI);
    }
    static Trigger* kiljaeden_dragon_orb_is_active(PlayerbotAI* botAI)
    {
        return new KiljaedenDragonOrbIsActiveTrigger(botAI);
    }
    static Trigger* kiljaeden_bot_controls_dragon(PlayerbotAI* botAI)
    {
        return new KiljaedenBotControlsDragonTrigger(botAI);
    }
    static Trigger* kiljaeden_stale_root_after_dragon(PlayerbotAI* botAI)
    {
        return new KiljaedenStaleRootAfterDragonTrigger(botAI);
    }
};

#endif
