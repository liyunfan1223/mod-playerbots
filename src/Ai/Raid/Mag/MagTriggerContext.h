/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGTRIGGERCONTEXT_H
#define PLAYERBOTS_MAGTRIGGERCONTEXT_H

#include "MagTriggers.h"
#include "NamedObjectContext.h"

class RaidMagtheridonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMagtheridonTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["magtheridon no encounter in progress"] =
            &RaidMagtheridonTriggerContext::magtheridon_no_encounter_in_progress;

        creators["magtheridon main tank should tank channelers"] =
            &RaidMagtheridonTriggerContext::magtheridon_main_tank_should_tank_channelers;

        creators["magtheridon assist tanks should tank channelers"] =
            &RaidMagtheridonTriggerContext::magtheridon_assist_tanks_should_tank_channelers;

        creators["magtheridon hunter should misdirect channelers"] =
            &RaidMagtheridonTriggerContext::magtheridon_hunter_should_misdirect_channelers;

        creators["magtheridon should assign dps priority"] =
            &RaidMagtheridonTriggerContext::magtheridon_should_assign_dps_priority;

        creators["magtheridon burning abyssal spawned"] =
            &RaidMagtheridonTriggerContext::magtheridon_burning_abyssal_spawned;

        creators["magtheridon should be tanked"] =
            &RaidMagtheridonTriggerContext::magtheridon_should_be_tanked;

        creators["magtheridon ranged should spread"] =
            &RaidMagtheridonTriggerContext::magtheridon_ranged_should_spread;

        creators["magtheridon standing in debris"] =
            &RaidMagtheridonTriggerContext::magtheridon_standing_in_debris;

        creators["magtheridon incoming blast nova"] =
            &RaidMagtheridonTriggerContext::magtheridon_incoming_blast_nova;

        creators["magtheridon should manage timers and assignments"] =
            &RaidMagtheridonTriggerContext::magtheridon_should_manage_timers_and_assignments;
    }

private:
    static Trigger* magtheridon_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new MagtheridonNoEncounterInProgressTrigger(botAI);
    }
    static Trigger* magtheridon_main_tank_should_tank_channelers(PlayerbotAI* botAI)
    {
        return new MagtheridonMainTankShouldTankChannelersTrigger(botAI);
    }
    static Trigger* magtheridon_assist_tanks_should_tank_channelers(PlayerbotAI* botAI)
    {
        return new MagtheridonAssistTanksShouldTankChannelersTrigger(botAI);
    }
    static Trigger* magtheridon_hunter_should_misdirect_channelers(PlayerbotAI* botAI)
    {
        return new MagtheridonHunterShouldMisdirectChannelersTrigger(botAI);
    }
    static Trigger* magtheridon_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new MagtheridonShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* magtheridon_burning_abyssal_spawned(PlayerbotAI* botAI)
    {
        return new MagtheridonBurningAbyssalSpawnedTrigger(botAI);
    }
    static Trigger* magtheridon_should_be_tanked(PlayerbotAI* botAI)
    {
        return new MagtheridonShouldBeTankedTrigger(botAI);
    }
    static Trigger* magtheridon_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new MagtheridonRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* magtheridon_standing_in_debris(PlayerbotAI* botAI)
    {
        return new MagtheridonStandingInDebrisTrigger(botAI);
    }
    static Trigger* magtheridon_incoming_blast_nova(PlayerbotAI* botAI)
    {
        return new MagtheridonIncomingBlastNovaTrigger(botAI);
    }
    static Trigger* magtheridon_should_manage_timers_and_assignments(PlayerbotAI* botAI)
    {
        return new MagtheridonShouldManageTimersAndAssignmentsTrigger(botAI);
    }
};

#endif
