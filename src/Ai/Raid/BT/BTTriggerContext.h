/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERCONTEXT_H
#define PLAYERBOTS_BTTRIGGERCONTEXT_H

#include "BTTriggers.h"
#include "NamedObjectContext.h"

class RaidBlackTempleTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBlackTempleTriggerContext()
    {
        // Shared
        creators["black temple bot is not in combat"] =
            &RaidBlackTempleTriggerContext::black_temple_bot_is_not_in_combat;

        // High Warlord Naj'entus
        creators["high warlord naj'entus pulling boss"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_pulling_boss;

        creators["high warlord naj'entus boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_engaged_by_tanks;

        creators["high warlord naj'entus casts needle spines"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_casts_needle_spines;

        creators["high warlord naj'entus player is impaled"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_player_is_impaled;

        creators["high warlord naj'entus boss has tidal shield"] =
            &RaidBlackTempleTriggerContext::high_warlord_najentus_boss_has_tidal_shield;

        // Supremus
        creators["supremus pulling boss or changing phase"] =
            &RaidBlackTempleTriggerContext::supremus_pulling_boss_or_changing_phase;

        creators["supremus boss engaged by ranged"] =
            &RaidBlackTempleTriggerContext::supremus_boss_engaged_by_ranged;

        creators["supremus boss is fixated on bot"] =
            &RaidBlackTempleTriggerContext::supremus_boss_is_fixated_on_bot;

        creators["supremus volcano is nearby"] =
            &RaidBlackTempleTriggerContext::supremus_volcano_is_nearby;

        creators["supremus need to manage phase timer"] =
            &RaidBlackTempleTriggerContext::supremus_need_to_manage_phase_timer;

        // Shade of Akama
        creators["shade of akama killing channelers starts phase 2"] =
            &RaidBlackTempleTriggerContext::shade_of_akama_killing_channelers_starts_phase_2;

        // Teron Gorefiend
        creators["teron gorefiend pulling boss"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_pulling_boss;

        creators["teron gorefiend boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_boss_engaged_by_tanks;

        creators["teron gorefiend boss engaged by ranged"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_boss_engaged_by_ranged;

        creators["teron gorefiend boss is casting shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_boss_is_casting_shadow_of_death;

        creators["teron gorefiend bot has shadow of death"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_bot_has_shadow_of_death;

        creators["teron gorefiend bot transformed into vengeful spirit"] =
            &RaidBlackTempleTriggerContext::teron_gorefiend_bot_transformed_into_vengeful_spirit;

        // Gurtogg Bloodboil
        creators["gurtogg bloodboil pulling boss"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_pulling_boss;

        creators["gurtogg bloodboil boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_boss_engaged_by_tanks;

        creators["gurtogg bloodboil boss casts bloodboil"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_boss_casts_bloodboil;

        creators["gurtogg bloodboil bot has fel rage"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_bot_has_fel_rage;

        creators["gurtogg bloodboil need to manage phase timer"] =
            &RaidBlackTempleTriggerContext::gurtogg_bloodboil_need_to_manage_phase_timer;

        // Reliquary of Souls
        creators["reliquary of souls aggro resets upon phase change"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_aggro_resets_upon_phase_change;

        creators["reliquary of souls essence of suffering fixates on closest target"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_fixates_on_closest_target;

        creators["reliquary of souls essence of suffering disables healing"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_suffering_disables_healing;

        creators["reliquary of souls essence of desire has rune shield"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_has_rune_shield;

        creators["reliquary of souls essence of desire casting deaden"] =
            &RaidBlackTempleTriggerContext::reliquary_of_souls_essence_of_desire_casting_deaden;

        // Mother Shahraz
        creators["mother shahraz pulling boss"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_pulling_boss;

        creators["mother shahraz boss engaged by tanks"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_boss_engaged_by_tanks;

        creators["mother shahraz tanks are positioning boss"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_tanks_are_positioning_boss;

        creators["mother shahraz sinister beam knocks back players"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_sinister_beam_knocks_back_players;

        creators["mother shahraz bots are linked by fatal attraction"] =
            &RaidBlackTempleTriggerContext::mother_shahraz_bots_are_linked_by_fatal_attraction;

        // Illidari Council
        creators["illidari council pulling bosses"] =
            &RaidBlackTempleTriggerContext::illidari_council_pulling_bosses;

        creators["illidari council gathios engaged by main tank"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_engaged_by_main_tank;

        creators["illidari council gathios casting judgement of command"] =
            &RaidBlackTempleTriggerContext::illidari_council_gathios_casting_judgement_of_command;

        creators["illidari council malande engaged by first assist tank"] =
            &RaidBlackTempleTriggerContext::illidari_council_malande_engaged_by_first_assist_tank;

        creators["illidari council darkshadow engaged by second assist tank"] =
            &RaidBlackTempleTriggerContext::illidari_council_darkshadow_engaged_by_second_assist_tank;

        creators["illidari council zerevor engaged by mage tank"] =
            &RaidBlackTempleTriggerContext::illidari_council_zerevor_engaged_by_mage_tank;

        creators["illidari council mage tank needs dedicated healer"] =
            &RaidBlackTempleTriggerContext::illidari_council_mage_tank_needs_dedicated_healer;

        creators["illidari council zerevor casts dangerous aoes"] =
            &RaidBlackTempleTriggerContext::illidari_council_zerevor_casts_dangerous_aoes;

        creators["illidari council pets screw up the pull"] =
            &RaidBlackTempleTriggerContext::illidari_council_pets_screw_up_the_pull;

        creators["illidari council determining dps assignments"] =
            &RaidBlackTempleTriggerContext::illidari_council_determining_dps_assignments;

        creators["illidari council need to manage dps timer"] =
            &RaidBlackTempleTriggerContext::illidari_council_need_to_manage_dps_timer;

        // Illidan Stormrage <The Betrayer>
        creators["illidan stormrage tank needs aggro"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_tank_needs_aggro;

        creators["illidan stormrage boss casts flame crash in front of main tank"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_casts_flame_crash_in_front_of_main_tank;

        creators["illidan stormrage bot has parasitic shadowfiend"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_bot_has_parasitic_shadowfiend;

        creators["illidan stormrage parasitic shadowfiends run wild"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_parasitic_shadowfiends_run_wild;

        creators["illidan stormrage boss summoned flames of azzinoth"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_summoned_flames_of_azzinoth;

        creators["illidan stormrage pets die to fire"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_pets_die_to_fire;

        creators["illidan stormrage grate is safe from flames"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_grate_is_safe_from_flames;

        creators["illidan stormrage bot struck by dark barrage"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_bot_struck_by_dark_barrage;

        creators["illidan stormrage boss is preparing to land"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_is_preparing_to_land;

        creators["illidan stormrage boss deals splash damage"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_deals_splash_damage;

        creators["illidan stormrage this expansion hates melee"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_this_expansion_hates_melee;

        creators["illidan stormrage boss transforms into demon"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_transforms_into_demon;

        creators["illidan stormrage boss spawns adds"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_boss_spawns_adds;

        creators["illidan stormrage maiev placed shadow trap"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_maiev_placed_shadow_trap;

        creators["illidan stormrage need to manage dps timer and rti"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_need_to_manage_dps_timer_and_rti;

        creators["illidan stormrage need to clear hazards between phases"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_need_to_clear_hazards_between_phases;

        creators["illidan stormrage cheat"] =
            &RaidBlackTempleTriggerContext::illidan_stormrage_cheat;
    }

private:
    // Shared
    static Trigger* black_temple_bot_is_not_in_combat(PlayerbotAI* botAI)
    {
        return new BlackTempleBotIsNotInCombatTrigger(botAI);
    }

    // High Warlord Naj'entus
    static Trigger* high_warlord_najentus_pulling_boss(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusPullingBossTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_casts_needle_spines(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusCastsNeedleSpinesTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_player_is_impaled(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusPlayerIsImpaledTrigger(botAI);
    }
    static Trigger* high_warlord_najentus_boss_has_tidal_shield(PlayerbotAI* botAI)
    {
        return new HighWarlordNajentusBossHasTidalShieldTrigger(botAI);
    }

    // Supremus
    static Trigger* supremus_pulling_boss_or_changing_phase(PlayerbotAI* botAI)
    {
        return new SupremusPullingBossOrChangingPhaseTrigger(botAI);
    }
    static Trigger* supremus_boss_engaged_by_ranged(PlayerbotAI* botAI)
    {
        return new SupremusBossEngagedByRangedTrigger(botAI);
    }
    static Trigger* supremus_boss_is_fixated_on_bot(PlayerbotAI* botAI)
    {
        return new SupremusBossIsFixatedOnBotTrigger(botAI);
    }
    static Trigger* supremus_volcano_is_nearby(PlayerbotAI* botAI)
    {
        return new SupremusVolcanoIsNearbyTrigger(botAI);
    }
    static Trigger* supremus_need_to_manage_phase_timer(PlayerbotAI* botAI)
    {
        return new SupremusNeedToManagePhaseTimerTrigger(botAI);
    }

    // Shade of Akama
    static Trigger* shade_of_akama_killing_channelers_starts_phase_2(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaKillingChannelersStartsPhase2Trigger(botAI);
    }

    // Teron Gorefiend
    static Trigger* teron_gorefiend_pulling_boss(PlayerbotAI* botAI)
    {
        return new TeronGorefiendPullingBossTrigger(botAI);
    }
    static Trigger* teron_gorefiend_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new TeronGorefiendBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* teron_gorefiend_boss_engaged_by_ranged(PlayerbotAI* botAI)
    {
        return new TeronGorefiendBossEngagedByRangedTrigger(botAI);
    }
    static Trigger* teron_gorefiend_boss_is_casting_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendBossIsCastingShadowOfDeathTrigger(botAI);
    }
    static Trigger* teron_gorefiend_bot_has_shadow_of_death(PlayerbotAI* botAI)
    {
        return new TeronGorefiendBotHasShadowOfDeathTrigger(botAI);
    }
    static Trigger* teron_gorefiend_bot_transformed_into_vengeful_spirit(PlayerbotAI* botAI)
    {
        return new TeronGorefiendBotTransformedIntoVengefulSpiritTrigger(botAI);
    }

    // Gurtogg Bloodboil
    static Trigger* gurtogg_bloodboil_pulling_boss(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilPullingBossTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_boss_casts_bloodboil(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilBossCastsBloodboilTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_bot_has_fel_rage(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilBotHasFelRageTrigger(botAI);
    }
    static Trigger* gurtogg_bloodboil_need_to_manage_phase_timer(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilNeedToManagePhaseTimerTrigger(botAI);
    }

    // Reliquary of Souls
    static Trigger* reliquary_of_souls_aggro_resets_upon_phase_change(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_suffering_fixates_on_closest_target(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_suffering_disables_healing(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_has_rune_shield(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(botAI);
    }
    static Trigger* reliquary_of_souls_essence_of_desire_casting_deaden(PlayerbotAI* botAI)
    {
        return new ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(botAI);
    }

    // Mother Shahraz
    static Trigger* mother_shahraz_pulling_boss(PlayerbotAI* botAI)
    {
        return new MotherShahrazPullingBossTrigger(botAI);
    }
    static Trigger* mother_shahraz_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new MotherShahrazBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* mother_shahraz_tanks_are_positioning_boss(PlayerbotAI* botAI)
    {
        return new MotherShahrazTanksArePositioningBossTrigger(botAI);
    }
    static Trigger* mother_shahraz_sinister_beam_knocks_back_players(PlayerbotAI* botAI)
    {
        return new MotherShahrazSinisterBeamKnocksBackPlayersTrigger(botAI);
    }
    static Trigger* mother_shahraz_bots_are_linked_by_fatal_attraction(PlayerbotAI* botAI)
    {
        return new MotherShahrazBotsAreLinkedByFatalAttractionTrigger(botAI);
    }

    // Illidari Council
    static Trigger* illidari_council_pulling_bosses(PlayerbotAI* botAI)
    {
        return new IllidariCouncilPullingBossesTrigger(botAI);
    }
    static Trigger* illidari_council_gathios_engaged_by_main_tank(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosEngagedByMainTankTrigger(botAI);
    }
    static Trigger* illidari_council_gathios_casting_judgement_of_command(PlayerbotAI* botAI)
    {
        return new IllidariCouncilGathiosCastingJudgementOfCommandTrigger(botAI);
    }
    static Trigger* illidari_council_malande_engaged_by_first_assist_tank(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMalandeEngagedByFirstAssistTankTrigger(botAI);
    }
    static Trigger* illidari_council_darkshadow_engaged_by_second_assist_tank(PlayerbotAI* botAI)
    {
        return new IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger(botAI);
    }
    static Trigger* illidari_council_zerevor_engaged_by_mage_tank(PlayerbotAI* botAI)
    {
        return new IllidariCouncilZerevorEngagedByMageTankTrigger(botAI);
    }
    static Trigger* illidari_council_mage_tank_needs_dedicated_healer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilMageTankNeedsDedicatedHealerTrigger(botAI);
    }
    static Trigger* illidari_council_zerevor_casts_dangerous_aoes(PlayerbotAI* botAI)
    {
        return new IllidariCouncilZerevorCastsDangerousAoesTrigger(botAI);
    }
    static Trigger* illidari_council_pets_screw_up_the_pull(PlayerbotAI* botAI)
    {
        return new IllidariCouncilPetsScrewUpThePullTrigger(botAI);
    }
    static Trigger* illidari_council_determining_dps_assignments(PlayerbotAI* botAI)
    {
        return new IllidariCouncilDeterminingDpsAssignmentsTrigger(botAI);
    }
    static Trigger* illidari_council_need_to_manage_dps_timer(PlayerbotAI* botAI)
    {
        return new IllidariCouncilNeedToManageDpsTimerTrigger(botAI);
    }

    // Illidan Stormrage <The Betrayer>
    static Trigger* illidan_stormrage_tank_needs_aggro(PlayerbotAI* botAI)
    {
        return new IllidanStormrageTankNeedsAggroTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_casts_flame_crash_in_front_of_main_tank(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger(botAI);
    }
    static Trigger* illidan_stormrage_bot_has_parasitic_shadowfiend(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBotHasParasiticShadowfiendTrigger(botAI);
    }
    static Trigger* illidan_stormrage_parasitic_shadowfiends_run_wild(PlayerbotAI* botAI)
    {
        return new IllidanStormrageParasiticShadowfiendsRunWildTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_summoned_flames_of_azzinoth(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossSummonedFlamesOfAzzinothTrigger(botAI);
    }
    static Trigger* illidan_stormrage_pets_die_to_fire(PlayerbotAI* botAI)
    {
        return new IllidanStormragePetsDieToFireTrigger(botAI);
    }
    static Trigger* illidan_stormrage_grate_is_safe_from_flames(PlayerbotAI* botAI)
    {
        return new IllidanStormrageGrateIsSafeFromFlamesTrigger(botAI);
    }
    static Trigger* illidan_stormrage_bot_struck_by_dark_barrage(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBotStruckByDarkBarrageTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_is_preparing_to_land(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossIsPreparingToLandTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_deals_splash_damage(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossDealsSplashDamageTrigger(botAI);
    }
    static Trigger* illidan_stormrage_this_expansion_hates_melee(PlayerbotAI* botAI)
    {
        return new IllidanStormrageThisExpansionHatesMeleeTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_transforms_into_demon(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossTransformsIntoDemonTrigger(botAI);
    }
    static Trigger* illidan_stormrage_boss_spawns_adds(PlayerbotAI* botAI)
    {
        return new IllidanStormrageBossSpawnsAddsTrigger(botAI);
    }
    static Trigger* illidan_stormrage_maiev_placed_shadow_trap(PlayerbotAI* botAI)
    {
        return new IllidanStormrageMaievPlacedShadowTrapTrigger(botAI);
    }
    static Trigger* illidan_stormrage_need_to_manage_dps_timer_and_rti(PlayerbotAI* botAI)
    {
        return new IllidanStormrageNeedToManageDpsTimerAndRtiTrigger(botAI);
    }
    static Trigger* illidan_stormrage_need_to_clear_hazards_between_phases(PlayerbotAI* botAI)
    {
        return new IllidanStormrageNeedToClearHazardsBetweenPhasesTrigger(botAI);
    }
    static Trigger* illidan_stormrage_cheat(PlayerbotAI* botAI)
    {
        return new IllidanStormrageCheatTrigger(botAI);
    }
};

#endif
