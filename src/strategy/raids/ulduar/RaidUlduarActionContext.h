// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
//  and/or modify it under version 3 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDULDUARACTIONCONTEXT_H
#define _PLAYERBOT_RAIDULDUARACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidUlduarActions.h"
#include "BossAuraActions.h"

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
    static Action* flame_leviathan_vehicle(PlayerbotAI* ai) { return new FlameLeviathanVehicleAction(ai); }
    static Action* flame_leviathan_enter_vehicle(PlayerbotAI* ai) { return new FlameLeviathanEnterVehicleAction(ai); }
    static Action* razorscale_avoid_devouring_flames(PlayerbotAI* ai) { return new RazorscaleAvoidDevouringFlameAction(ai); }
    static Action* razorscale_avoid_sentinel(PlayerbotAI* ai) { return new RazorscaleAvoidSentinelAction(ai); }
    static Action* razorscale_ignore_flying_alone(PlayerbotAI* ai) { return new RazorscaleIgnoreBossAction(ai); }
    static Action* razorscale_avoid_whirlwind(PlayerbotAI* ai) { return new RazorscaleAvoidWhirlwindAction(ai); }
    static Action* razorscale_grounded(PlayerbotAI* ai) { return new RazorscaleGroundedAction(ai); }
    static Action* razorscale_harpoon_action(PlayerbotAI* ai) { return new RazorscaleHarpoonAction(ai); }
    static Action* razorscale_fuse_armor_action(PlayerbotAI* ai) { return new RazorscaleFuseArmorAction(ai); }
    static Action* razorscale_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "razorscale"); }
    static Action* ignis_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "ignis the furnace master"); }
    static Action* iron_assembly_lightning_tendrils_action(PlayerbotAI* ai) { return new IronAssemblyLightningTendrilsAction(ai); }
    static Action* iron_assembly_overload_action(PlayerbotAI* ai) { return new IronAssemblyOverloadAction(ai); }
    static Action* iron_assembly_rune_of_power_action(PlayerbotAI* ai) { return new IronAssemblyRuneOfPowerAction(ai); }
    static Action* kologarn_mark_dps_target_action(PlayerbotAI* ai) { return new KologarnMarkDpsTargetAction(ai); }
    static Action* kologarn_fall_from_floor_action(PlayerbotAI* ai) { return new KologarnFallFromFloorAction(ai); }
    static Action* kologarn_nature_resistance_action(PlayerbotAI* ai) { return new BossNatureResistanceAction(ai, "kologarn"); }
    static Action* kologarn_rubble_slowdown_action(PlayerbotAI* ai) { return new KologarnRubbleSlowdownAction(ai); }
    static Action* kologarn_eyebeam_action(PlayerbotAI* ai) { return new KologarnEyebeamAction(ai); }
    static Action* kologarn_rti_target_action(PlayerbotAI* ai) { return new KologarnRtiTargetAction(ai); }
    static Action* kologarn_crunch_armor_action(PlayerbotAI* ai) { return new KologarnCrunchArmorAction(ai); }
    static Action* auriaya_fall_from_floor_action(PlayerbotAI* ai) { return new AuriayaFallFromFloorAction(ai); }
    static Action* hodir_move_snowpacked_icicle(PlayerbotAI* ai) { return new HodirMoveSnowpackedIcicleAction(ai); }
    static Action* hodir_biting_cold_jump(PlayerbotAI* ai) { return new HodirBitingColdJumpAction(ai); }
    static Action* hodir_frost_resistance_action(PlayerbotAI* ai) { return new BossFrostResistanceAction(ai, "hodir"); }
    static Action* freya_move_away_nature_bomb(PlayerbotAI* ai) { return new FreyaMoveAwayNatureBombAction(ai); }
    static Action* freya_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "freya"); }
    static Action* freya_nature_resistance_action(PlayerbotAI* ai) { return new BossNatureResistanceAction(ai, "freya"); }
    static Action* freya_mark_dps_target_action(PlayerbotAI* ai) { return new FreyaMarkDpsTargetAction(ai); }
    static Action* freya_move_to_healing_spore_action(PlayerbotAI* ai) { return new FreyaMoveToHealingSporeAction(ai); }
    static Action* thorim_frost_resistance_action(PlayerbotAI* ai) { return new BossFrostResistanceAction(ai, "thorim"); }
    static Action* thorim_nature_resistance_action(PlayerbotAI* ai) { return new BossNatureResistanceAction(ai, "thorim"); }
    static Action* thorim_unbalancing_strike_action(PlayerbotAI* ai) { return new ThorimUnbalancingStrikeAction(ai); }
    static Action* thorim_mark_dps_target_action(PlayerbotAI* ai) { return new ThorimMarkDpsTargetAction(ai); }
    static Action* thorim_arena_positioning_action(PlayerbotAI* ai) { return new ThorimArenaPositioningAction(ai); }
    static Action* thorim_gauntlet_positioning_action(PlayerbotAI* ai) { return new ThorimGauntletPositioningAction(ai); }
    static Action* thorim_phase2_positioning_action(PlayerbotAI* ai) { return new ThorimPhase2PositioningAction(ai); }
    static Action* mimiron_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "mimiron"); }
    static Action* mimiron_shock_blast_action(PlayerbotAI* ai) { return new MimironShockBlastAction(ai); }
    static Action* mimiron_phase_1_positioning_action(PlayerbotAI* ai) { return new MimironPhase1PositioningAction(ai); }
    static Action* mimiron_p3wx2_laser_barrage_action(PlayerbotAI* ai) { return new MimironP3Wx2LaserBarrageAction(ai); }
    static Action* mimiron_rapid_burst_action(PlayerbotAI* ai) { return new MimironRapidBurstAction(ai); }
    static Action* mimiron_aerial_command_unit_action(PlayerbotAI* ai) { return new MimironAerialCommandUnitAction(ai); }
    static Action* mimiron_rocket_strike_action(PlayerbotAI* ai) { return new MimironRocketStrikeAction(ai); }
    static Action* mimiron_phase_4_mark_dps_action(PlayerbotAI* ai) { return new MimironPhase4MarkDpsAction(ai); }
    static Action* mimiron_cheat_action(PlayerbotAI* ai) { return new MimironCheatAction(ai); }
    static Action* vezax_cheat_action(PlayerbotAI* ai) { return new VezaxCheatAction(ai); }
    static Action* vezax_shadow_crash_action(PlayerbotAI* ai) { return new VezaxShadowCrashAction(ai); }
    static Action* vezax_mark_of_the_faceless_action(PlayerbotAI* ai) { return new VezaxMarkOfTheFacelessAction(ai); }
    static Action* vezax_shadow_resistance_action(PlayerbotAI* ai) { return new BossShadowResistanceAction(ai, "general vezax"); }
    static Action* sara_shadow_resistance_action(PlayerbotAI* ai) { return new BossShadowResistanceAction(ai, "sara"); }
    static Action* yogg_saron_shadow_resistance_action(PlayerbotAI* ai) { return new BossShadowResistanceAction(ai, "yogg-saron"); }
    static Action* yogg_saron_ominous_cloud_cheat_action(PlayerbotAI* ai) { return new YoggSaronOminousCloudCheatAction(ai); }
    static Action* yogg_saron_guardian_positioning_action(PlayerbotAI* ai) { return new YoggSaronGuardianPositioningAction(ai); }
    static Action* yogg_saron_sanity_action(PlayerbotAI* ai) { return new YoggSaronSanityAction(ai); }
    static Action* yogg_saron_death_orb_action(PlayerbotAI* ai) { return new YoggSaronDeathOrbAction(ai); }
    static Action* yogg_saron_malady_of_the_mind_action(PlayerbotAI* ai) { return new YoggSaronMaladyOfTheMindAction(ai); }
    static Action* yogg_saron_mark_target_action(PlayerbotAI* ai) { return new YoggSaronMarkTargetAction(ai); }
    static Action* yogg_saron_brain_link_action(PlayerbotAI* ai) { return new YoggSaronBrainLinkAction(ai); }
    static Action* yogg_saron_move_to_enter_portal_action(PlayerbotAI* ai) { return new YoggSaronMoveToEnterPortalAction(ai); }
    static Action* yogg_saron_use_portal_action(PlayerbotAI* ai) { return new YoggSaronUsePortalAction(ai); }
    static Action* yogg_saron_fall_from_floor_action(PlayerbotAI* ai) { return new YoggSaronFallFromFloorAction(ai); }
    static Action* yogg_saron_boss_room_movement_cheat_action(PlayerbotAI* ai) { return new YoggSaronBossRoomMovementCheatAction(ai); }
    static Action* yogg_saron_illusion_room_action(PlayerbotAI* ai) { return new YoggSaronIllusionRoomAction(ai); }
    static Action* yogg_saron_move_to_exit_portal_action(PlayerbotAI* ai) { return new YoggSaronMoveToExitPortalAction(ai); }
    static Action* yogg_saron_lunatic_gaze_action(PlayerbotAI* ai) { return new YoggSaronLunaticGazeAction(ai); }
    static Action* yogg_saron_phase_3_positioning_action(PlayerbotAI* ai) { return new YoggSaronPhase3PositioningAction(ai); }
};

#endif
