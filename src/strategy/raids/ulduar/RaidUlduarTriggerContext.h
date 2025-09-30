// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
//  and/or modify it under version 3 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDULDUARTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDULDUARTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidUlduarTriggers.h"
#include "BossAuraTriggers.h"

class RaidUlduarTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidUlduarTriggerContext()
    {
        creators["flame leviathan on vehicle"] = &RaidUlduarTriggerContext::flame_leviathan_on_vehicle;
        creators["flame leviathan vehicle near"] = &RaidUlduarTriggerContext::flame_leviathan_vehicle_near;
        creators["razorscale flying alone"] = &RaidUlduarTriggerContext::razorscale_flying_alone;
        creators["razorscale avoid devouring flames"] = &RaidUlduarTriggerContext::razorscale_avoid_devouring_flames;
        creators["razorscale avoid sentinel"] = &RaidUlduarTriggerContext::razorscale_avoid_sentinel;
        creators["razorscale avoid whirlwind"] = &RaidUlduarTriggerContext::razorscale_avoid_whirlwind;
        creators["razorscale grounded"] = &RaidUlduarTriggerContext::razorscale_grounded;
        creators["razorscale harpoon trigger"] = &RaidUlduarTriggerContext::razorscale_harpoon_trigger;
        creators["razorscale fuse armor trigger"] = &RaidUlduarTriggerContext::razorscale_fuse_armor_trigger;
        creators["razorscale fire resistance trigger"] = &RaidUlduarTriggerContext::razorscale_fire_resistance_trigger;
        creators["ignis fire resistance trigger"] = &RaidUlduarTriggerContext::ignis_fire_resistance_trigger;
        creators["iron assembly lightning tendrils trigger"] = &RaidUlduarTriggerContext::iron_assembly_lightning_tendrils_trigger;
        creators["iron assembly overload trigger"] = &RaidUlduarTriggerContext::iron_assembly_overload_trigger;
        creators["iron assembly rune of power trigger"] = &RaidUlduarTriggerContext::iron_assembly_rune_of_power_trigger;
        creators["kologarn mark dps target trigger"] = &RaidUlduarTriggerContext::kologarn_mark_dps_target_trigger;
        creators["kologarn fall from floor trigger"] = &RaidUlduarTriggerContext::kologarn_fall_from_floor_trigger;
        creators["kologarn nature resistance trigger"] = &RaidUlduarTriggerContext::kologarn_nature_resistance_trigger;
        creators["kologarn rubble slowdown trigger"] = &RaidUlduarTriggerContext::kologarn_rubble_slowdown_trigger;
        creators["kologarn eyebeam trigger"] = &RaidUlduarTriggerContext::kologarn_eyebeam_trigger;
        creators["kologarn rti target trigger"] = &RaidUlduarTriggerContext::kologarn_rti_target_trigger;
        creators["kologarn crunch armor trigger"] = &RaidUlduarTriggerContext::kologarn_crunch_armor_trigger;
        creators["kologarn attack dps target trigger"] = &RaidUlduarTriggerContext::kologarn_attack_dps_target_trigger;
        creators["auriaya fall from floor trigger"] = &RaidUlduarTriggerContext::auriaya_fall_from_floor_trigger;
        creators["hodir biting cold"] = &RaidUlduarTriggerContext::hodir_biting_cold;
        creators["hodir near snowpacked icicle"] = &RaidUlduarTriggerContext::hodir_near_snowpacked_icicle;
        creators["hodir frost resistance trigger"] = &RaidUlduarTriggerContext::hodir_frost_resistance_trigger;
        creators["freya near nature bomb"] = &RaidUlduarTriggerContext::freya_near_nature_bomb;
        creators["freya fire resistance trigger"] = &RaidUlduarTriggerContext::freya_fire_resistance_trigger;
        creators["freya nature resistance trigger"] = &RaidUlduarTriggerContext::freya_nature_resistance_trigger;
        creators["freya mark dps target trigger"] = &RaidUlduarTriggerContext::freya_mark_dps_target_trigger;
        creators["freya move to healing spore trigger"] = &RaidUlduarTriggerContext::freya_move_to_healing_spore_trigger;
        creators["thorim frost resistance trigger"] = &RaidUlduarTriggerContext::thorim_frost_resistance_trigger;
        creators["thorim nature resistance trigger"] = &RaidUlduarTriggerContext::thorim_nature_resistance_trigger;
        creators["thorim unbalancing strike trigger"] = &RaidUlduarTriggerContext::thorim_unbalancing_strike_trigger;
        creators["thorim mark dps target trigger"] = &RaidUlduarTriggerContext::thorim_mark_dps_target_trigger;
        creators["thorim arena positioning trigger"] = &RaidUlduarTriggerContext::thorim_arena_positioning_trigger;
        creators["thorim gauntlet positioning trigger"] = &RaidUlduarTriggerContext::thorim_gauntlet_positioning_trigger;
        creators["thorim fall from floor trigger"] = &RaidUlduarTriggerContext::thorim_fall_from_floor_trigger;
        creators["thorim phase 2 positioning trigger"] = &RaidUlduarTriggerContext::thorim_phase2_positioning_trigger;
        creators["mimiron fire resistance trigger"] = &RaidUlduarTriggerContext::mimiron_fire_resistance_trigger;
        creators["mimiron shock blast trigger"] = &RaidUlduarTriggerContext::mimiron_shock_blast_trigger;
        creators["mimiron phase 1 positioning trigger"] = &RaidUlduarTriggerContext::mimiron_phase_1_positioning_trigger;
        creators["mimiron p3wx2 laser barrage trigger"] = &RaidUlduarTriggerContext::mimiron_p3wx2_laser_barrage_trigger;
        creators["mimiron rapid burst trigger"] = &RaidUlduarTriggerContext::mimiron_rapid_burst_trigger;
        creators["mimiron aerial command unit trigger"] = &RaidUlduarTriggerContext::mimiron_aerial_command_unit_trigger;
        creators["mimiron rocket strike trigger"] = &RaidUlduarTriggerContext::mimiron_rocket_strike_trigger;
        creators["mimiron phase 4 mark dps trigger"] = &RaidUlduarTriggerContext::mimiron_phase_4_mark_dps_trigger;
        creators["mimiron cheat trigger"] = &RaidUlduarTriggerContext::mimiron_cheat_trigger;
        creators["vezax cheat trigger"] = &RaidUlduarTriggerContext::vezax_cheat_trigger;
        creators["vezax shadow crash trigger"] = &RaidUlduarTriggerContext::vezax_shadow_crash_trigger;
        creators["vezax mark of the faceless trigger"] = &RaidUlduarTriggerContext::vezax_mark_of_the_faceless_trigger;
        creators["vezax shadow resistance trigger"] = &RaidUlduarTriggerContext::vezax_shadow_resistance_trigger;
        creators["sara shadow resistance trigger"] = &RaidUlduarTriggerContext::sara_shadow_resistance_trigger;
        creators["yogg-saron shadow resistance triggerr"] = &RaidUlduarTriggerContext::yogg_saron_shadow_resistance_trigger;
        creators["yogg-saron ominous cloud cheat trigger"] = &RaidUlduarTriggerContext::yogg_saron_ominous_cloud_cheat_trigger;
        creators["yogg-saron guardian positioning trigger"] = &RaidUlduarTriggerContext::yogg_saron_guardian_positioning_trigger;
        creators["yogg-saron sanity trigger"] = &RaidUlduarTriggerContext::yogg_saron_sanity_trigger;
        creators["yogg-saron death orb trigger"] = &RaidUlduarTriggerContext::yogg_saron_death_orb_trigger;
        creators["yogg-saron malady of the mind trigger"] = &RaidUlduarTriggerContext::yogg_saron_malady_of_the_mind_trigger;
        creators["yogg-saron mark target trigger"] = &RaidUlduarTriggerContext::yogg_saron_mark_target_trigger;
        creators["yogg-saron brain link trigger"] = &RaidUlduarTriggerContext::yogg_saron_brain_link_trigger;
        creators["yogg-saron move to enter portal trigger"] = &RaidUlduarTriggerContext::yogg_saron_move_to_enter_portal_trigger;
        creators["yogg-saron use portal trigger"] = &RaidUlduarTriggerContext::yogg_saron_use_portal_trigger;
        creators["yogg-saron fall from floor trigger"] = &RaidUlduarTriggerContext::yogg_saron_fall_from_floor_trigger;
        creators["yogg-saron boss room movement cheat trigger"] = &RaidUlduarTriggerContext::yogg_saron_boss_room_movement_cheat_trigger;
        creators["yogg-saron illusion room trigger"] = &RaidUlduarTriggerContext::yogg_saron_illusion_room_trigger;
        creators["yogg-saron move to exit portal trigger"] = &RaidUlduarTriggerContext::yogg_saron_move_to_exit_portal_trigger;
        creators["yogg-saron lunatic gaze trigger"] = &RaidUlduarTriggerContext::yogg_saron_lunatic_gaze_trigger;
        creators["yogg-saron phase 3 positioning trigger"] = &RaidUlduarTriggerContext::yogg_saron_phase_3_positioning_trigger;
    }

private:
    static Trigger* flame_leviathan_on_vehicle(PlayerbotAI* ai) { return new FlameLeviathanOnVehicleTrigger(ai); }
    static Trigger* flame_leviathan_vehicle_near(PlayerbotAI* ai) { return new FlameLeviathanVehicleNearTrigger(ai); }
    static Trigger* razorscale_flying_alone(PlayerbotAI* ai) { return new RazorscaleFlyingAloneTrigger(ai); }
    static Trigger* razorscale_avoid_devouring_flames(PlayerbotAI* ai) { return new RazorscaleDevouringFlamesTrigger(ai); }
    static Trigger* razorscale_avoid_sentinel(PlayerbotAI* ai) { return new RazorscaleAvoidSentinelTrigger(ai); }
    static Trigger* razorscale_avoid_whirlwind(PlayerbotAI* ai) { return new RazorscaleAvoidWhirlwindTrigger(ai); }
    static Trigger* razorscale_grounded(PlayerbotAI* ai) { return new RazorscaleGroundedTrigger(ai); }
    static Trigger* razorscale_harpoon_trigger(PlayerbotAI* ai) { return new RazorscaleHarpoonAvailableTrigger(ai); }
    static Trigger* razorscale_fuse_armor_trigger(PlayerbotAI* ai) { return new RazorscaleFuseArmorTrigger(ai); }
    static Trigger* razorscale_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "razorscale"); }
    static Trigger* ignis_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "ignis the furnace master"); }
    static Trigger* iron_assembly_lightning_tendrils_trigger(PlayerbotAI* ai) { return new IronAssemblyLightningTendrilsTrigger(ai); }
    static Trigger* iron_assembly_overload_trigger(PlayerbotAI* ai) { return new IronAssemblyOverloadTrigger(ai); }
    static Trigger* iron_assembly_rune_of_power_trigger(PlayerbotAI* ai) { return new IronAssemblyRuneOfPowerTrigger(ai); }
    static Trigger* kologarn_mark_dps_target_trigger(PlayerbotAI* ai) { return new KologarnMarkDpsTargetTrigger(ai); }
    static Trigger* kologarn_fall_from_floor_trigger(PlayerbotAI* ai) { return new KologarnFallFromFloorTrigger(ai); }
    static Trigger* kologarn_nature_resistance_trigger(PlayerbotAI* ai) { return new BossNatureResistanceTrigger(ai, "kologarn"); }
    static Trigger* kologarn_rubble_slowdown_trigger(PlayerbotAI* ai) { return new KologarnRubbleSlowdownTrigger(ai); }
    static Trigger* kologarn_eyebeam_trigger(PlayerbotAI* ai) { return new KologarnEyebeamTrigger(ai); }
    static Trigger* kologarn_rti_target_trigger(PlayerbotAI* ai) { return new KologarnRtiTargetTrigger(ai); }
    static Trigger* kologarn_crunch_armor_trigger(PlayerbotAI* ai) { return new KologarnCrunchArmorTrigger(ai); }
    static Trigger* kologarn_attack_dps_target_trigger(PlayerbotAI* ai) { return new KologarnAttackDpsTargetTrigger(ai); }
    static Trigger* auriaya_fall_from_floor_trigger(PlayerbotAI* ai) { return new AuriayaFallFromFloorTrigger(ai); }
    static Trigger* hodir_biting_cold(PlayerbotAI* ai) { return new HodirBitingColdTrigger(ai); }
    static Trigger* hodir_near_snowpacked_icicle(PlayerbotAI* ai) { return new HodirNearSnowpackedIcicleTrigger(ai); }
    static Trigger* hodir_frost_resistance_trigger(PlayerbotAI* ai) { return new BossFrostResistanceTrigger(ai, "hodir"); }
    static Trigger* freya_near_nature_bomb(PlayerbotAI* ai) { return new FreyaNearNatureBombTrigger(ai); }
    static Trigger* freya_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "freya"); }
    static Trigger* freya_nature_resistance_trigger(PlayerbotAI* ai) { return new BossNatureResistanceTrigger(ai, "freya"); }
    static Trigger* freya_mark_dps_target_trigger(PlayerbotAI* ai) { return new FreyaMarkDpsTargetTrigger(ai); }
    static Trigger* freya_move_to_healing_spore_trigger(PlayerbotAI* ai) { return new FreyaMoveToHealingSporeTrigger(ai); }
    static Trigger* thorim_frost_resistance_trigger(PlayerbotAI* ai) { return new BossFrostResistanceTrigger(ai, "thorim"); }
    static Trigger* thorim_nature_resistance_trigger(PlayerbotAI* ai) { return new BossNatureResistanceTrigger(ai, "thorim"); }
    static Trigger* thorim_unbalancing_strike_trigger(PlayerbotAI* ai) { return new ThorimUnbalancingStrikeTrigger(ai); }
    static Trigger* thorim_mark_dps_target_trigger(PlayerbotAI* ai) { return new ThorimMarkDpsTargetTrigger(ai); }
    static Trigger* thorim_arena_positioning_trigger(PlayerbotAI* ai) { return new ThorimArenaPositioningTrigger(ai); }
    static Trigger* thorim_gauntlet_positioning_trigger(PlayerbotAI* ai) { return new ThorimGauntletPositioningTrigger(ai); }
    static Trigger* thorim_fall_from_floor_trigger(PlayerbotAI* ai) { return new ThorimFallFromFloorTrigger(ai); }
    static Trigger* thorim_phase2_positioning_trigger(PlayerbotAI* ai) { return new ThorimPhase2PositioningTrigger(ai); }
    static Trigger* mimiron_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "mimiron"); }
    static Trigger* mimiron_shock_blast_trigger(PlayerbotAI* ai) { return new MimironShockBlastTrigger(ai); }
    static Trigger* mimiron_phase_1_positioning_trigger(PlayerbotAI* ai) { return new MimironPhase1PositioningTrigger(ai); }
    static Trigger* mimiron_p3wx2_laser_barrage_trigger(PlayerbotAI* ai) { return new MimironP3Wx2LaserBarrageTrigger(ai); }
    static Trigger* mimiron_rapid_burst_trigger(PlayerbotAI* ai) { return new MimironRapidBurstTrigger(ai); }
    static Trigger* mimiron_aerial_command_unit_trigger(PlayerbotAI* ai) { return new MimironAerialCommandUnitTrigger(ai); }
    static Trigger* mimiron_rocket_strike_trigger(PlayerbotAI* ai) { return new MimironRocketStrikeTrigger(ai); }
    static Trigger* mimiron_phase_4_mark_dps_trigger(PlayerbotAI* ai) { return new MimironPhase4MarkDpsTrigger(ai); }
    static Trigger* mimiron_cheat_trigger(PlayerbotAI* ai) { return new MimironCheatTrigger(ai); }
    static Trigger* vezax_cheat_trigger(PlayerbotAI* ai) { return new VezaxCheatTrigger(ai); }
    static Trigger* vezax_shadow_crash_trigger(PlayerbotAI* ai) { return new VezaxShadowCrashTrigger(ai); }
    static Trigger* vezax_shadow_resistance_trigger(PlayerbotAI* ai) { return new BossShadowResistanceTrigger(ai, "general vezax"); }
    static Trigger* sara_shadow_resistance_trigger(PlayerbotAI* ai) { return new BossShadowResistanceTrigger(ai, "sara"); }
    static Trigger* yogg_saron_shadow_resistance_trigger(PlayerbotAI* ai) { return new BossShadowResistanceTrigger(ai, "yogg-saron"); }
    static Trigger* vezax_mark_of_the_faceless_trigger(PlayerbotAI* ai) { return new VezaxMarkOfTheFacelessTrigger(ai); }
    static Trigger* yogg_saron_ominous_cloud_cheat_trigger(PlayerbotAI* ai) { return new YoggSaronOminousCloudCheatTrigger(ai); }
    static Trigger* yogg_saron_guardian_positioning_trigger(PlayerbotAI* ai) { return new YoggSaronGuardianPositioningTrigger(ai); }
    static Trigger* yogg_saron_sanity_trigger(PlayerbotAI* ai) { return new YoggSaronSanityTrigger(ai); }
    static Trigger* yogg_saron_death_orb_trigger(PlayerbotAI* ai) { return new YoggSaronDeathOrbTrigger(ai); }
    static Trigger* yogg_saron_malady_of_the_mind_trigger(PlayerbotAI* ai) { return new YoggSaronMaladyOfTheMindTrigger(ai); }
    static Trigger* yogg_saron_mark_target_trigger(PlayerbotAI* ai) { return new YoggSaronMarkTargetTrigger(ai); }
    static Trigger* yogg_saron_brain_link_trigger(PlayerbotAI* ai) { return new YoggSaronBrainLinkTrigger(ai); }
    static Trigger* yogg_saron_move_to_enter_portal_trigger(PlayerbotAI* ai) { return new YoggSaronMoveToEnterPortalTrigger(ai); }
    static Trigger* yogg_saron_use_portal_trigger(PlayerbotAI* ai) { return new YoggSaronUsePortalTrigger(ai); }
    static Trigger* yogg_saron_fall_from_floor_trigger(PlayerbotAI* ai) { return new YoggSaronFallFromFloorTrigger(ai); }
    static Trigger* yogg_saron_boss_room_movement_cheat_trigger(PlayerbotAI* ai) { return new YoggSaronBossRoomMovementCheatTrigger(ai); }
    static Trigger* yogg_saron_illusion_room_trigger(PlayerbotAI* ai) { return new YoggSaronIllusionRoomTrigger(ai); }
    static Trigger* yogg_saron_move_to_exit_portal_trigger(PlayerbotAI* ai) { return new YoggSaronMoveToExitPortalTrigger(ai); }
    static Trigger* yogg_saron_lunatic_gaze_trigger(PlayerbotAI* ai) { return new YoggSaronLunaticGazeTrigger(ai); }
    static Trigger* yogg_saron_phase_3_positioning_trigger(PlayerbotAI* ai) { return new YoggSaronPhase3PositioningTrigger(ai); }
};

#endif
