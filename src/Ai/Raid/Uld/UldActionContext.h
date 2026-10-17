/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONCONTEXT_H
#define PLAYERBOTS_ULDACTIONCONTEXT_H

#include "Action.h"
#include "BossAuraActions.h"
#include "NamedObjectContext.h"
#include "UldActions.h"

class RaidUlduarActionContext : public NamedObjectContext<Action>
{
public:
    RaidUlduarActionContext()
    {
        creators["flame leviathan vehicle"] = &RaidUlduarActionContext::flame_leviathan_vehicle;
        creators["flame leviathan enter vehicle"] = &RaidUlduarActionContext::flame_leviathan_enter_vehicle;
        creators["razorscale avoid devouring flames"] = &RaidUlduarActionContext::razorscale_avoid_devouring_flames;
        creators["razorscale avoid sentinel"] = &RaidUlduarActionContext::razorscale_avoid_sentinel;
        creators["razorscale ignore flying alone"] = &RaidUlduarActionContext::razorscale_ignore_flying_alone;
        creators["razorscale avoid whirlwind"] = &RaidUlduarActionContext::razorscale_avoid_whirlwind;
        creators["razorscale grounded"] = &RaidUlduarActionContext::razorscale_grounded;
        creators["razorscale harpoon action"] = &RaidUlduarActionContext::razorscale_harpoon_action;
        creators["razorscale fuse armor action"] = &RaidUlduarActionContext::razorscale_fuse_armor_action;
        creators["razorscale fire resistance action"] = &RaidUlduarActionContext::razorscale_fire_resistance_action;
        creators["ignis fire resistance action"] = &RaidUlduarActionContext::ignis_fire_resistance_action;
        creators["iron assembly lightning tendrils action"] = &RaidUlduarActionContext::iron_assembly_lightning_tendrils_action;
        creators["iron assembly overload action"] = &RaidUlduarActionContext::iron_assembly_overload_action;
        creators["iron assembly rune of power action"] = &RaidUlduarActionContext::iron_assembly_rune_of_power_action;
        creators["kologarn mark dps target action"] = &RaidUlduarActionContext::kologarn_mark_dps_target_action;
        creators["kologarn fall from floor action"] = &RaidUlduarActionContext::kologarn_fall_from_floor_action;
        creators["kologarn nature resistance action"] = &RaidUlduarActionContext::kologarn_nature_resistance_action;
        creators["kologarn rubble slowdown action"] = &RaidUlduarActionContext::kologarn_rubble_slowdown_action;
        creators["kologarn eyebeam action"] = &RaidUlduarActionContext::kologarn_eyebeam_action;
        creators["kologarn rti target action"] = &RaidUlduarActionContext::kologarn_rti_target_action;
        creators["kologarn crunch armor action"] = &RaidUlduarActionContext::kologarn_crunch_armor_action;
        creators["auriaya fall from floor action"] = &RaidUlduarActionContext::auriaya_fall_from_floor_action;
        creators["hodir move snowpacked icicle"] = &RaidUlduarActionContext::hodir_move_snowpacked_icicle;
        creators["hodir biting cold jump"] = &RaidUlduarActionContext::hodir_biting_cold_jump;
        creators["hodir frost resistance action"] = &RaidUlduarActionContext::hodir_frost_resistance_action;
        creators["freya move away nature bomb"] = &RaidUlduarActionContext::freya_move_away_nature_bomb;
        creators["freya fire resistance action"] = &RaidUlduarActionContext::freya_fire_resistance_action;
        creators["freya nature resistance action"] = &RaidUlduarActionContext::freya_nature_resistance_action;
        creators["freya mark dps target action"] = &RaidUlduarActionContext::freya_mark_dps_target_action;
        creators["freya move to healing spore action"] = &RaidUlduarActionContext::freya_move_to_healing_spore_action;
        creators["thorim frost resistance action"] = &RaidUlduarActionContext::thorim_frost_resistance_action;
        creators["thorim nature resistance action"] = &RaidUlduarActionContext::thorim_nature_resistance_action;
        creators["thorim unbalancing strike action"] = &RaidUlduarActionContext::thorim_unbalancing_strike_action;
        creators["thorim mark dps target action"] = &RaidUlduarActionContext::thorim_mark_dps_target_action;
        creators["thorim arena positioning action"] = &RaidUlduarActionContext::thorim_arena_positioning_action;
        creators["thorim gauntlet positioning action"] = &RaidUlduarActionContext::thorim_gauntlet_positioning_action;
        creators["thorim phase 2 positioning action"] = &RaidUlduarActionContext::thorim_phase2_positioning_action;
        creators["mimiron fire resistance action"] = &RaidUlduarActionContext::mimiron_fire_resistance_action;
        creators["mimiron shock blast action"] = &RaidUlduarActionContext::mimiron_shock_blast_action;
        creators["mimiron phase 1 positioning action"] = &RaidUlduarActionContext::mimiron_phase_1_positioning_action;
        creators["mimiron p3wx2 laser barrage action"] = &RaidUlduarActionContext::mimiron_p3wx2_laser_barrage_action;
        creators["mimiron rapid burst action"] = &RaidUlduarActionContext::mimiron_rapid_burst_action;
        creators["mimiron aerial command unit action"] = &RaidUlduarActionContext::mimiron_aerial_command_unit_action;
        creators["mimiron rocket strike action"] = &RaidUlduarActionContext::mimiron_rocket_strike_action;
        creators["mimiron phase 4 mark dps action"] = &RaidUlduarActionContext::mimiron_phase_4_mark_dps_action;
        creators["mimiron cheat action"] = &RaidUlduarActionContext::mimiron_cheat_action;
        creators["vezax cheat action"] = &RaidUlduarActionContext::vezax_cheat_action;
        creators["vezax shadow crash action"] = &RaidUlduarActionContext::vezax_shadow_crash_action;
        creators["vezax mark of the faceless action"] = &RaidUlduarActionContext::vezax_mark_of_the_faceless_action;
        creators["vezax shadow resistance action"] = &RaidUlduarActionContext::vezax_shadow_resistance_action;
        creators["sara shadow resistance action"] = &RaidUlduarActionContext::sara_shadow_resistance_action;
        creators["yogg-saron shadow resistance action"] = &RaidUlduarActionContext::yogg_saron_shadow_resistance_action;
        creators["yogg-saron ominous cloud cheat action"] = &RaidUlduarActionContext::yogg_saron_ominous_cloud_cheat_action;
        creators["yogg-saron guardian positioning action"] = &RaidUlduarActionContext::yogg_saron_guardian_positioning_action;
        creators["yogg-saron sanity action"] = &RaidUlduarActionContext::yogg_saron_sanity_action;
        creators["yogg-saron death orb action"] = &RaidUlduarActionContext::yogg_saron_death_orb_action;
        creators["yogg-saron malady of the mind action"] = &RaidUlduarActionContext::yogg_saron_malady_of_the_mind_action;
        creators["yogg-saron mark target action"] = &RaidUlduarActionContext::yogg_saron_mark_target_action;
        creators["yogg-saron brain link action"] = &RaidUlduarActionContext::yogg_saron_brain_link_action;
        creators["yogg-saron move to enter portal action"] = &RaidUlduarActionContext::yogg_saron_move_to_enter_portal_action;
        creators["yogg-saron use portal action"] = &RaidUlduarActionContext::yogg_saron_use_portal_action;
        creators["yogg-saron fall from floor action"] = &RaidUlduarActionContext::yogg_saron_fall_from_floor_action;
        creators["yogg-saron boss room movement cheat action"] = &RaidUlduarActionContext::yogg_saron_boss_room_movement_cheat_action;
        creators["yogg-saron illusion room action"] = &RaidUlduarActionContext::yogg_saron_illusion_room_action;
        creators["yogg-saron move to exit portal action"] = &RaidUlduarActionContext::yogg_saron_move_to_exit_portal_action;
        creators["yogg-saron lunatic gaze action"] = &RaidUlduarActionContext::yogg_saron_lunatic_gaze_action;
        creators["yogg-saron phase 3 positioning action"] = &RaidUlduarActionContext::yogg_saron_phase_3_positioning_action;
    }

private:
    static Action* flame_leviathan_vehicle(PlayerbotAI* botAI) { return new FlameLeviathanVehicleAction(botAI); }
    static Action* flame_leviathan_enter_vehicle(PlayerbotAI* botAI) { return new FlameLeviathanEnterVehicleAction(botAI); }
    static Action* razorscale_avoid_devouring_flames(PlayerbotAI* botAI) { return new RazorscaleAvoidDevouringFlameAction(botAI); }
    static Action* razorscale_avoid_sentinel(PlayerbotAI* botAI) { return new RazorscaleAvoidSentinelAction(botAI); }
    static Action* razorscale_ignore_flying_alone(PlayerbotAI* botAI) { return new RazorscaleIgnoreBossAction(botAI); }
    static Action* razorscale_avoid_whirlwind(PlayerbotAI* botAI) { return new RazorscaleAvoidWhirlwindAction(botAI); }
    static Action* razorscale_grounded(PlayerbotAI* botAI) { return new RazorscaleGroundedAction(botAI); }
    static Action* razorscale_harpoon_action(PlayerbotAI* botAI) { return new RazorscaleHarpoonAction(botAI); }
    static Action* razorscale_fuse_armor_action(PlayerbotAI* botAI) { return new RazorscaleFuseArmorAction(botAI); }
    static Action* razorscale_fire_resistance_action(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "razorscale"); }
    static Action* ignis_fire_resistance_action(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "ignis the furnace master"); }
    static Action* iron_assembly_lightning_tendrils_action(PlayerbotAI* botAI) { return new IronAssemblyLightningTendrilsAction(botAI); }
    static Action* iron_assembly_overload_action(PlayerbotAI* botAI) { return new IronAssemblyOverloadAction(botAI); }
    static Action* iron_assembly_rune_of_power_action(PlayerbotAI* botAI) { return new IronAssemblyRuneOfPowerAction(botAI); }
    static Action* kologarn_mark_dps_target_action(PlayerbotAI* botAI) { return new KologarnMarkDpsTargetAction(botAI); }
    static Action* kologarn_fall_from_floor_action(PlayerbotAI* botAI) { return new KologarnFallFromFloorAction(botAI); }
    static Action* kologarn_nature_resistance_action(PlayerbotAI* botAI) { return new BossNatureResistanceAction(botAI, "kologarn"); }
    static Action* kologarn_rubble_slowdown_action(PlayerbotAI* botAI) { return new KologarnRubbleSlowdownAction(botAI); }
    static Action* kologarn_eyebeam_action(PlayerbotAI* botAI) { return new KologarnEyebeamAction(botAI); }
    static Action* kologarn_rti_target_action(PlayerbotAI* botAI) { return new KologarnRtiTargetAction(botAI); }
    static Action* kologarn_crunch_armor_action(PlayerbotAI* botAI) { return new KologarnCrunchArmorAction(botAI); }
    static Action* auriaya_fall_from_floor_action(PlayerbotAI* botAI) { return new AuriayaFallFromFloorAction(botAI); }
    static Action* hodir_move_snowpacked_icicle(PlayerbotAI* botAI) { return new HodirMoveSnowpackedIcicleAction(botAI); }
    static Action* hodir_biting_cold_jump(PlayerbotAI* botAI) { return new HodirBitingColdJumpAction(botAI); }
    static Action* hodir_frost_resistance_action(PlayerbotAI* botAI) { return new BossFrostResistanceAction(botAI, "hodir"); }
    static Action* freya_move_away_nature_bomb(PlayerbotAI* botAI) { return new FreyaMoveAwayNatureBombAction(botAI); }
    static Action* freya_fire_resistance_action(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "freya"); }
    static Action* freya_nature_resistance_action(PlayerbotAI* botAI) { return new BossNatureResistanceAction(botAI, "freya"); }
    static Action* freya_mark_dps_target_action(PlayerbotAI* botAI) { return new FreyaMarkDpsTargetAction(botAI); }
    static Action* freya_move_to_healing_spore_action(PlayerbotAI* botAI) { return new FreyaMoveToHealingSporeAction(botAI); }
    static Action* thorim_frost_resistance_action(PlayerbotAI* botAI) { return new BossFrostResistanceAction(botAI, "thorim"); }
    static Action* thorim_nature_resistance_action(PlayerbotAI* botAI) { return new BossNatureResistanceAction(botAI, "thorim"); }
    static Action* thorim_unbalancing_strike_action(PlayerbotAI* botAI) { return new ThorimUnbalancingStrikeAction(botAI); }
    static Action* thorim_mark_dps_target_action(PlayerbotAI* botAI) { return new ThorimMarkDpsTargetAction(botAI); }
    static Action* thorim_arena_positioning_action(PlayerbotAI* botAI) { return new ThorimArenaPositioningAction(botAI); }
    static Action* thorim_gauntlet_positioning_action(PlayerbotAI* botAI) { return new ThorimGauntletPositioningAction(botAI); }
    static Action* thorim_phase2_positioning_action(PlayerbotAI* botAI) { return new ThorimPhase2PositioningAction(botAI); }
    static Action* mimiron_fire_resistance_action(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "mimiron"); }
    static Action* mimiron_shock_blast_action(PlayerbotAI* botAI) { return new MimironShockBlastAction(botAI); }
    static Action* mimiron_phase_1_positioning_action(PlayerbotAI* botAI) { return new MimironPhase1PositioningAction(botAI); }
    static Action* mimiron_p3wx2_laser_barrage_action(PlayerbotAI* botAI) { return new MimironP3Wx2LaserBarrageAction(botAI); }
    static Action* mimiron_rapid_burst_action(PlayerbotAI* botAI) { return new MimironRapidBurstAction(botAI); }
    static Action* mimiron_aerial_command_unit_action(PlayerbotAI* botAI) { return new MimironAerialCommandUnitAction(botAI); }
    static Action* mimiron_rocket_strike_action(PlayerbotAI* botAI) { return new MimironRocketStrikeAction(botAI); }
    static Action* mimiron_phase_4_mark_dps_action(PlayerbotAI* botAI) { return new MimironPhase4MarkDpsAction(botAI); }
    static Action* mimiron_cheat_action(PlayerbotAI* botAI) { return new MimironCheatAction(botAI); }
    static Action* vezax_cheat_action(PlayerbotAI* botAI) { return new VezaxCheatAction(botAI); }
    static Action* vezax_shadow_crash_action(PlayerbotAI* botAI) { return new VezaxShadowCrashAction(botAI); }
    static Action* vezax_mark_of_the_faceless_action(PlayerbotAI* botAI) { return new VezaxMarkOfTheFacelessAction(botAI); }
    static Action* vezax_shadow_resistance_action(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "general vezax"); }
    static Action* sara_shadow_resistance_action(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "sara"); }
    static Action* yogg_saron_shadow_resistance_action(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "yogg-saron"); }
    static Action* yogg_saron_ominous_cloud_cheat_action(PlayerbotAI* botAI) { return new YoggSaronOminousCloudCheatAction(botAI); }
    static Action* yogg_saron_guardian_positioning_action(PlayerbotAI* botAI) { return new YoggSaronGuardianPositioningAction(botAI); }
    static Action* yogg_saron_sanity_action(PlayerbotAI* botAI) { return new YoggSaronSanityAction(botAI); }
    static Action* yogg_saron_death_orb_action(PlayerbotAI* botAI) { return new YoggSaronDeathOrbAction(botAI); }
    static Action* yogg_saron_malady_of_the_mind_action(PlayerbotAI* botAI) { return new YoggSaronMaladyOfTheMindAction(botAI); }
    static Action* yogg_saron_mark_target_action(PlayerbotAI* botAI) { return new YoggSaronMarkTargetAction(botAI); }
    static Action* yogg_saron_brain_link_action(PlayerbotAI* botAI) { return new YoggSaronBrainLinkAction(botAI); }
    static Action* yogg_saron_move_to_enter_portal_action(PlayerbotAI* botAI) { return new YoggSaronMoveToEnterPortalAction(botAI); }
    static Action* yogg_saron_use_portal_action(PlayerbotAI* botAI) { return new YoggSaronUsePortalAction(botAI); }
    static Action* yogg_saron_fall_from_floor_action(PlayerbotAI* botAI) { return new YoggSaronFallFromFloorAction(botAI); }
    static Action* yogg_saron_boss_room_movement_cheat_action(PlayerbotAI* botAI) { return new YoggSaronBossRoomMovementCheatAction(botAI); }
    static Action* yogg_saron_illusion_room_action(PlayerbotAI* botAI) { return new YoggSaronIllusionRoomAction(botAI); }
    static Action* yogg_saron_move_to_exit_portal_action(PlayerbotAI* botAI) { return new YoggSaronMoveToExitPortalAction(botAI); }
    static Action* yogg_saron_lunatic_gaze_action(PlayerbotAI* botAI) { return new YoggSaronLunaticGazeAction(botAI); }
    static Action* yogg_saron_phase_3_positioning_action(PlayerbotAI* botAI) { return new YoggSaronPhase3PositioningAction(botAI); }
};

#endif
