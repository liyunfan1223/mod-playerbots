// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
//  and/or modify it under version 2 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDULDUARTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDULDUARTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidUlduarTriggers.h"

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
        creators["iron assembly lightning tendrils trigger"] = &RaidUlduarTriggerContext::iron_assembly_lightning_tendrils_trigger;
        creators["iron assembly overload trigger"] = &RaidUlduarTriggerContext::iron_assembly_overload_trigger;
        creators["kologarn mark dps target trigger"] = &RaidUlduarTriggerContext::kologarn_mark_dps_target_trigger;
        creators["kologarn fall from floor trigger"] = &RaidUlduarTriggerContext::kologarn_fall_from_floor_trigger;
        creators["kologarn nature resistance trigger"] = &RaidUlduarTriggerContext::kologarn_nature_resistance_trigger;
        creators["hodir biting cold"] = &RaidUlduarTriggerContext::hodir_biting_cold;
        creators["hodir near snowpacked icicle"] = &RaidUlduarTriggerContext::hodir_near_snowpacked_icicle;
        creators["freya near nature bomb"] = &RaidUlduarTriggerContext::freya_near_nature_bomb;
        creators["freya tank near eonars gift"] = &RaidUlduarTriggerContext::freya_tank_near_eonars_gift;
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
    static Trigger* iron_assembly_lightning_tendrils_trigger(PlayerbotAI* ai) { return new IronAssemblyLightningTendrilsTrigger(ai); }
    static Trigger* iron_assembly_overload_trigger(PlayerbotAI* ai) { return new IronAssemblyOverloadTrigger(ai); }
    static Trigger* kologarn_mark_dps_target_trigger(PlayerbotAI* ai) { return new KologarnMarkDpsTargetTrigger(ai); }
    static Trigger* kologarn_fall_from_floor_trigger(PlayerbotAI* ai) { return new KologarnFallFromFloorTrigger(ai); }
    static Trigger* kologarn_nature_resistance_trigger(PlayerbotAI* ai) { return new KologarnNatureResistanceTrigger(ai); }
    static Trigger* hodir_biting_cold(PlayerbotAI* ai) { return new HodirBitingColdTrigger(ai); }
    static Trigger* hodir_near_snowpacked_icicle(PlayerbotAI* ai) { return new HodirNearSnowpackedIcicleTrigger(ai); }
    static Trigger* freya_near_nature_bomb(PlayerbotAI* ai) { return new FreyaNearNatureBombTrigger(ai); }
    static Trigger* freya_tank_near_eonars_gift(PlayerbotAI* ai) { return new FreyaTankNearEonarsGiftTrigger(ai); }
};

#endif
