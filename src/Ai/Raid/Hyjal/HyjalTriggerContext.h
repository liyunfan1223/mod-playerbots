/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALTRIGGERCONTEXT_H
#define PLAYERBOTS_HYJALTRIGGERCONTEXT_H

#include "EncounterHelpers.h"
#include "HyjalTriggers.h"
#include "NamedObjectContext.h"

class RaidHyjalTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidHyjalTriggerContext()
    {
        // General
        creators["hyjal no encounter in progress"] =
            &RaidHyjalTriggerContext::hyjal_no_encounter_in_progress;

        // Rage Winterchill
        creators["rage winterchill pulling boss"] =
            &RaidHyjalTriggerContext::rage_winterchill_pulling_boss;

        creators["rage winterchill should be tanked"] =
            &RaidHyjalTriggerContext::rage_winterchill_should_be_tanked;

        creators["rage winterchill ranged should spread"] =
            &RaidHyjalTriggerContext::rage_winterchill_ranged_should_spread;

        creators["rage winterchill melee near death and decay"] =
            &RaidHyjalTriggerContext::rage_winterchill_melee_near_death_and_decay;

        creators["rage winterchill ranged in death and decay"] =
            &RaidHyjalTriggerContext::rage_winterchill_ranged_in_death_and_decay;

        // Anetheron
        creators["anetheron pulling boss or infernal"] =
            &RaidHyjalTriggerContext::anetheron_pulling_boss_or_infernal;

        creators["anetheron should be tanked"] =
            &RaidHyjalTriggerContext::anetheron_should_be_tanked;

        creators["anetheron ranged should spread"] =
            &RaidHyjalTriggerContext::anetheron_ranged_should_spread;

        creators["anetheron near inferno target"] =
            &RaidHyjalTriggerContext::anetheron_near_inferno_target;

        creators["anetheron targeted by infernal"] =
            &RaidHyjalTriggerContext::anetheron_targeted_by_infernal;

        creators["anetheron infernals pulse immolation"] =
            &RaidHyjalTriggerContext::anetheron_infernals_pulse_immolation;

        creators["anetheron infernals should be tanked away"] =
            &RaidHyjalTriggerContext::anetheron_infernals_should_be_tanked_away;

        creators["anetheron should divide dps"] =
            &RaidHyjalTriggerContext::anetheron_should_divide_dps;

        // Kaz'rogal
        creators["kaz'rogal pulling boss"] = &RaidHyjalTriggerContext::kazrogal_pulling_boss;

        creators["kaz'rogal should be tanked"] =
            &RaidHyjalTriggerContext::kazrogal_should_be_tanked;

        creators["kaz'rogal can split malevolent cleave damage"] =
            &RaidHyjalTriggerContext::kazrogal_can_split_malevolent_cleave_damage;

        creators["kaz'rogal ranged should avoid war stomp"] =
            &RaidHyjalTriggerContext::kazrogal_ranged_should_avoid_war_stomp;

        creators["kaz'rogal low on mana"] = &RaidHyjalTriggerContext::kazrogal_low_on_mana;

        creators["kaz'rogal hunter should preserve mana"] =
            &RaidHyjalTriggerContext::kazrogal_hunter_should_preserve_mana;

        creators["kaz'rogal mark on mage or paladin"] =
            &RaidHyjalTriggerContext::kazrogal_mark_on_mage_or_paladin;

        creators["kaz'rogal immunity no longer needed"] =
            &RaidHyjalTriggerContext::kazrogal_immunity_no_longer_needed;

        creators["kaz'rogal warlock should manage mana"] =
            &RaidHyjalTriggerContext::kazrogal_warlock_should_manage_mana;

        // Azgalor
        creators["azgalor pulling boss"] = &RaidHyjalTriggerContext::azgalor_pulling_boss;

        creators["azgalor should be tanked"] = &RaidHyjalTriggerContext::azgalor_should_be_tanked;

        creators["azgalor ranged should spread"] =
            &RaidHyjalTriggerContext::azgalor_ranged_should_spread;

        creators["azgalor melee near rain of fire"] =
            &RaidHyjalTriggerContext::azgalor_melee_near_rain_of_fire;

        creators["azgalor ranged in rain of fire"] =
            &RaidHyjalTriggerContext::azgalor_ranged_in_rain_of_fire;

        creators["azgalor bot is doomed"] = &RaidHyjalTriggerContext::azgalor_bot_is_doomed;

        creators["azgalor should control doomguards"] =
            &RaidHyjalTriggerContext::azgalor_should_control_doomguards;

        creators["azgalor should divide dps"] = &RaidHyjalTriggerContext::azgalor_should_divide_dps;

        // Archimonde
        creators["archimonde pulling boss"] = &RaidHyjalTriggerContext::archimonde_pulling_boss;

        creators["archimonde should be tanked"] =
            &RaidHyjalTriggerContext::archimonde_should_be_tanked;

        creators["archimonde shaman should protect against fear"] =
            &RaidHyjalTriggerContext::archimonde_shaman_should_protect_against_fear;

        creators["archimonde casting air burst"] =
            &RaidHyjalTriggerContext::archimonde_casting_air_burst;

        creators["archimonde ranged should spread"] =
            &RaidHyjalTriggerContext::archimonde_ranged_should_spread;

        creators["archimonde near doomfire"] = &RaidHyjalTriggerContext::archimonde_near_doomfire;

        creators["archimonde stood in doomfire"] =
            &RaidHyjalTriggerContext::archimonde_stood_in_doomfire;
    }

private:
    // General
    static Trigger* hyjal_no_encounter_in_progress(PlayerbotAI* botAI) {
        return new HyjalNoEncounterInProgressTrigger(botAI);
    }

    // Rage Winterchill
    static Trigger* rage_winterchill_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(
            botAI, "rage winterchill pulling boss", "rage winterchill");
    }
    static Trigger* rage_winterchill_should_be_tanked(PlayerbotAI* botAI) {
        return new HyjalBossShouldBeTankedTrigger(
            botAI, "rage winterchill should be tanked", "rage winterchill", 0.0f, false);
    }
    static Trigger* rage_winterchill_ranged_should_spread(PlayerbotAI* botAI) {
        return new RageWinterchillRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* rage_winterchill_melee_near_death_and_decay(PlayerbotAI* botAI) {
        return new RageWinterchillMeleeNearDeathAndDecayTrigger(botAI);
    }
    static Trigger* rage_winterchill_ranged_in_death_and_decay(PlayerbotAI* botAI) {
        return new RageWinterchillRangedInDeathAndDecayTrigger(botAI);
    }

    // Anetheron
    static Trigger* anetheron_pulling_boss_or_infernal(PlayerbotAI* botAI) {
        return new AnetheronPullingBossOrInfernalTrigger(botAI);
    }
    static Trigger* anetheron_should_be_tanked(PlayerbotAI* botAI) {
        return new HyjalBossShouldBeTankedTrigger(
            botAI, "anetheron should be tanked", "anetheron");
    }
    static Trigger* anetheron_ranged_should_spread(PlayerbotAI* botAI) {
        return new AnetheronRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* anetheron_near_inferno_target(PlayerbotAI* botAI) {
        return new AnetheronNearInfernoTargetTrigger(botAI);
    }
    static Trigger* anetheron_targeted_by_infernal(PlayerbotAI* botAI) {
        return new AnetheronTargetedByInfernalTrigger(botAI);
    }
    static Trigger* anetheron_infernals_pulse_immolation(PlayerbotAI* botAI) {
        return new AnetheronInfernalsPulseImmolationTrigger(botAI);
    }
    static Trigger* anetheron_infernals_should_be_tanked_away(PlayerbotAI* botAI) {
        return new AnetheronInfernalsShouldBeTankedAwayTrigger(botAI);
    }
    static Trigger* anetheron_should_divide_dps(PlayerbotAI* botAI) {
        return new AnetheronShouldDivideDpsTrigger(botAI);
    }

    // Kaz'rogal
    static Trigger* kazrogal_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "kaz'rogal pulling boss", "kaz'rogal");
    }
    static Trigger* kazrogal_should_be_tanked(PlayerbotAI* botAI) {
        return new HyjalBossShouldBeTankedTrigger(
            botAI, "kaz'rogal should be tanked", "kaz'rogal");
    }
    static Trigger* kazrogal_can_split_malevolent_cleave_damage(PlayerbotAI* botAI) {
        return new KazrogalCanSplitMalevolentCleaveDamageTrigger(botAI);
    }
    static Trigger* kazrogal_ranged_should_avoid_war_stomp(PlayerbotAI* botAI) {
        return new KazrogalRangedShouldAvoidWarStompTrigger(botAI);
    }
    static Trigger* kazrogal_low_on_mana(PlayerbotAI* botAI) {
        return new KazrogalLowOnManaTrigger(botAI);
    }
    static Trigger* kazrogal_hunter_should_preserve_mana(PlayerbotAI* botAI) {
        return new KazrogalHunterShouldPreserveManaTrigger(botAI);
    }
    static Trigger* kazrogal_mark_on_mage_or_paladin(PlayerbotAI* botAI) {
        return new KazrogalMarkOnMageOrPaladinTrigger(botAI);
    }
    static Trigger* kazrogal_immunity_no_longer_needed(PlayerbotAI* botAI) {
        return new KazrogalImmunityNoLongerNeededTrigger(botAI);
    }
    static Trigger* kazrogal_warlock_should_manage_mana(PlayerbotAI* botAI) {
        return new KazrogalWarlockShouldManageManaTrigger(botAI);
    }

    // Azgalor
    static Trigger* azgalor_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "azgalor pulling boss", "azgalor");
    }
    static Trigger* azgalor_should_be_tanked(PlayerbotAI* botAI) {
        return new HyjalBossShouldBeTankedTrigger(
            botAI, "azgalor should be tanked", "azgalor");
    }
    static Trigger* azgalor_ranged_should_spread(PlayerbotAI* botAI) {
        return new AzgalorRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* azgalor_melee_near_rain_of_fire(PlayerbotAI* botAI) {
        return new AzgalorMeleeNearRainOfFireTrigger(botAI);
    }
    static Trigger* azgalor_ranged_in_rain_of_fire(PlayerbotAI* botAI) {
        return new AzgalorRangedInRainOfFireTrigger(botAI);
    }
    static Trigger* azgalor_bot_is_doomed(PlayerbotAI* botAI) {
        return new AzgalorBotIsDoomedTrigger(botAI);
    }
    static Trigger* azgalor_should_control_doomguards(PlayerbotAI* botAI) {
        return new AzgalorShouldControlDoomguardsTrigger(botAI);
    }
    static Trigger* azgalor_should_divide_dps(PlayerbotAI* botAI) {
        return new AzgalorShouldDivideDpsTrigger(botAI);
    }

    // Archimonde
    static Trigger* archimonde_pulling_boss(PlayerbotAI* botAI) {
        return new HyjalPullingBossTrigger(botAI, "archimonde pulling boss", "archimonde");
    }
    static Trigger* archimonde_should_be_tanked(PlayerbotAI* botAI) {
        return new HyjalBossShouldBeTankedTrigger(
            botAI, "archimonde should be tanked", "archimonde",
            EncounterHelpers::BOSS_ENGAGED_HEALTH_PCT, false);
    }
    static Trigger* archimonde_shaman_should_protect_against_fear(PlayerbotAI* botAI) {
        return new ArchimondeShamanShouldProtectAgainstFearTrigger(botAI);
    }
    static Trigger* archimonde_casting_air_burst(PlayerbotAI* botAI) {
        return new ArchimondeCastingAirBurstTrigger(botAI);
    }
    static Trigger* archimonde_ranged_should_spread(PlayerbotAI* botAI) {
        return new ArchimondeRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* archimonde_near_doomfire(PlayerbotAI* botAI) {
        return new ArchimondeNearDoomfireTrigger(botAI);
    }
    static Trigger* archimonde_stood_in_doomfire(PlayerbotAI* botAI) {
        return new ArchimondeStoodInDoomfireTrigger(botAI);
    }
};

#endif
