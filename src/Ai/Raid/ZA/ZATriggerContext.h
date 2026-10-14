/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZATRIGGERCONTEXT_H
#define PLAYERBOTS_ZATRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ZATriggers.h"

class RaidZulAmanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZulAmanTriggerContext()
    {
        // General
        creators["zul'aman no encounter in progress"] =
            &RaidZulAmanTriggerContext::zulaman_no_encounter_in_progress;

        // Trash
        creators["amani'shi medicine man summoned ward"] =
            &RaidZulAmanTriggerContext::amanishi_medicine_man_summoned_ward;

        // Akil'zon <Eagle Avatar>
        creators["akil'zon pulling boss"] = &RaidZulAmanTriggerContext::akilzon_pulling_boss;

        creators["akil'zon should be tanked"] =
            &RaidZulAmanTriggerContext::akilzon_should_be_tanked;

        creators["akil'zon spread for static disruption"] =
            &RaidZulAmanTriggerContext::akilzon_spread_for_static_disruption;

        creators["akil'zon electrical storm incoming"] =
            &RaidZulAmanTriggerContext::akilzon_electrical_storm_incoming;

        creators["akil'zon should track electrical storm"] =
            &RaidZulAmanTriggerContext::akilzon_should_track_electrical_storm;

        // Nalorakk <Bear Avatar>
        creators["nalorakk pulling boss"] = &RaidZulAmanTriggerContext::nalorakk_pulling_boss;

        creators["nalorakk spread for surge"] =
            &RaidZulAmanTriggerContext::nalorakk_spread_for_surge;

        creators["nalorakk both forms should be tanked"] =
            &RaidZulAmanTriggerContext::nalorakk_both_forms_should_be_tanked;

        // Jan'alai <Dragonhawk Avatar>
        creators["jan'alai pulling boss"] = &RaidZulAmanTriggerContext::janalai_pulling_boss;

        creators["jan'alai should be tanked"] =
            &RaidZulAmanTriggerContext::janalai_should_be_tanked;

        creators["jan'alai spread for flame breath"] =
            &RaidZulAmanTriggerContext::janalai_spread_for_flame_breath;

        creators["jan'alai is fire bombing"] = &RaidZulAmanTriggerContext::janalai_is_fire_bombing;

        creators["jan'alai amani'shi hatchers spawned"] =
            &RaidZulAmanTriggerContext::janalai_amanishi_hatchers_spawned;

        // Halazzi <Lynx Avatar>
        creators["halazzi pulling boss"] = &RaidZulAmanTriggerContext::halazzi_pulling_boss;

        creators["halazzi should be tanked"] = &RaidZulAmanTriggerContext::halazzi_should_be_tanked;

        creators["halazzi spirit lynx has appeared"] =
            &RaidZulAmanTriggerContext::halazzi_spirit_lynx_has_appeared;

        creators["halazzi should focus dps"] = &RaidZulAmanTriggerContext::halazzi_should_focus_dps;

        // Hex Lord Malacrass
        creators["hex lord malacrass pulling boss"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_pulling_boss;

        creators["hex lord malacrass should prioritize adds"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_should_prioritize_adds;

        creators["hex lord malacrass channeling whirlwind"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_channeling_whirlwind;

        creators["hex lord malacrass freezing trap placed"] =
            &RaidZulAmanTriggerContext::hex_lord_malacrass_freezing_trap_placed;

        // Zul'jin
        creators["zul'jin pulling boss"] = &RaidZulAmanTriggerContext::zuljin_pulling_boss;

        creators["zul'jin should be tanked"] = &RaidZulAmanTriggerContext::zuljin_should_be_tanked;

        creators["zul'jin channeling whirlwind in troll form"] =
            &RaidZulAmanTriggerContext::zuljin_channeling_whirlwind_in_troll_form;

        creators["zul'jin creeping paralysis in bear form"] =
            &RaidZulAmanTriggerContext::zuljin_creeping_paralysis_in_bear_form;

        creators["zul'jin summoning cyclones in eagle form"] =
            &RaidZulAmanTriggerContext::zuljin_summoning_cyclones_in_eagle_form;

        creators["zul'jin spread for dragonhawk aoe"] =
            &RaidZulAmanTriggerContext::zuljin_spread_for_dragonhawk_aoe;
    }

private:
    // General
    static Trigger* zulaman_no_encounter_in_progress(PlayerbotAI* botAI) {
        return new ZulAmanNoEncounterInProgressTrigger(botAI);
    }

    // Trash
    static Trigger* amanishi_medicine_man_summoned_ward(PlayerbotAI* botAI) {
        return new AmanishiMedicineManSummonedWardTrigger(botAI);
    }

    // Akil'zon <Eagle Avatar>
    static Trigger* akilzon_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(botAI, "akil'zon pulling boss", "akil'zon");
    }
    static Trigger* akilzon_should_be_tanked(PlayerbotAI* botAI) {
        return new AkilzonShouldBeTankedTrigger(botAI);
    }
    static Trigger* akilzon_spread_for_static_disruption(PlayerbotAI* botAI) {
        return new AkilzonSpreadForStaticDisruptionTrigger(botAI);
    }
    static Trigger* akilzon_electrical_storm_incoming(PlayerbotAI* botAI) {
        return new AkilzonElectricalStormIncomingTrigger(botAI);
    }
    static Trigger* akilzon_should_track_electrical_storm(PlayerbotAI* botAI) {
        return new AkilzonShouldTrackElectricalStormTrigger(botAI);
    }

    // Nalorakk <Bear Avatar>
    static Trigger* nalorakk_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(botAI, "nalorakk pulling boss", "nalorakk");
    }
    static Trigger* nalorakk_spread_for_surge(PlayerbotAI* botAI) {
        return new NalorakkSpreadForSurgeTrigger(botAI);
    }
    static Trigger* nalorakk_both_forms_should_be_tanked(PlayerbotAI* botAI) {
        return new NalorakkBothFormsShouldBeTankedTrigger(botAI);
    }

    // Jan'alai <Dragonhawk Avatar>
    static Trigger* janalai_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(botAI, "jan'alai pulling boss", "jan'alai");
    }
    static Trigger* janalai_should_be_tanked(PlayerbotAI* botAI) {
        return new JanalaiShouldBeTankedTrigger(botAI);
    }
    static Trigger* janalai_spread_for_flame_breath(PlayerbotAI* botAI) {
        return new JanalaiSpreadForFlameBreathTrigger(botAI);
    }
    static Trigger* janalai_is_fire_bombing(PlayerbotAI* botAI) {
        return new JanalaiIsFireBombingTrigger(botAI);
    }
    static Trigger* janalai_amanishi_hatchers_spawned(PlayerbotAI* botAI) {
        return new JanalaiAmanishiHatchersSpawnedTrigger(botAI);
    }

    // Halazzi <Lynx Avatar>
    static Trigger* halazzi_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(botAI, "halazzi pulling boss", "halazzi");
    }
    static Trigger* halazzi_should_be_tanked(PlayerbotAI* botAI) {
        return new HalazziShouldBeTankedTrigger(botAI);
    }
    static Trigger* halazzi_spirit_lynx_has_appeared(PlayerbotAI* botAI) {
        return new HalazziSpiritLynxHasAppearedTrigger(botAI);
    }
    static Trigger* halazzi_should_focus_dps(PlayerbotAI* botAI) {
        return new HalazziShouldFocusDpsTrigger(botAI);
    }

    // Hex Lord Malacrass
    static Trigger* hex_lord_malacrass_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(
            botAI, "hex lord malacrass pulling boss", "hex lord malacrass");
    }
    static Trigger* hex_lord_malacrass_should_prioritize_adds(PlayerbotAI* botAI) {
        return new HexLordMalacrassShouldPrioritizeAddsTrigger(botAI);
    }
    static Trigger* hex_lord_malacrass_channeling_whirlwind(PlayerbotAI* botAI) {
        return new HexLordMalacrassChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* hex_lord_malacrass_freezing_trap_placed(PlayerbotAI* botAI) {
        return new HexLordMalacrassFreezingTrapPlacedTrigger(botAI);
    }

    // Zul'jin
    static Trigger* zuljin_pulling_boss(PlayerbotAI* botAI) {
        return new ZulAmanPullingBossTrigger(botAI, "zul'jin pulling boss", "zul'jin");
    }
    static Trigger* zuljin_should_be_tanked(PlayerbotAI* botAI) {
        return new ZuljinShouldBeTankedTrigger(botAI);
    }
    static Trigger* zuljin_channeling_whirlwind_in_troll_form(PlayerbotAI* botAI) {
        return new ZuljinChannelingWhirlwindInTrollFormTrigger(botAI);
    }
    static Trigger* zuljin_creeping_paralysis_in_bear_form(PlayerbotAI* botAI) {
        return new ZuljinCreepingParalysisInBearFormTrigger(botAI);
    }
    static Trigger* zuljin_summoning_cyclones_in_eagle_form(PlayerbotAI* botAI) {
        return new ZuljinSummoningCyclonesInEagleFormTrigger(botAI);
    }
    static Trigger* zuljin_spread_for_dragonhawk_aoe(PlayerbotAI* botAI) {
        return new ZuljinSpreadForDragonhawkAoeTrigger(botAI);
    }
};

#endif
