// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
//  and/or modify it under version 2 of the License, or (at your option), any later version.
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
};

#endif
