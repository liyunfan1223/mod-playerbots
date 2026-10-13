/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGTRIGGERS_H
#define PLAYERBOTS_MAGTRIGGERS_H

#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Trigger.h"
#include <string>

class MagtheridonEncounterTrigger : public Trigger
{
public:
    MagtheridonEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, MagHelpers::MAG_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class MagtheridonNoEncounterInProgressTrigger : public Trigger
{
public:
    MagtheridonNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "magtheridon no encounter in progress", 1000) {};
    bool IsActive() override;
};

class MagtheridonMainTankShouldTankChannelersTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonMainTankShouldTankChannelersTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon main tank should tank channelers") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonAssistTanksShouldTankChannelersTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonAssistTanksShouldTankChannelersTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon assist tanks should tank channelers") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonHunterShouldMisdirectChannelersTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonHunterShouldMisdirectChannelersTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon hunter should misdirect channelers") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonShouldAssignDpsPriorityTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonBurningAbyssalSpawnedTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonBurningAbyssalSpawnedTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon burning abyssal spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonShouldBeTankedTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonShouldBeTankedTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonRangedShouldSpreadTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};
class MagtheridonStandingInDebrisTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonStandingInDebrisTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon standing in debris") {};

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonIncomingBlastNovaTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonIncomingBlastNovaTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon incoming blast nova") {}

protected:
    bool IsActiveInEncounter() override;
};

class MagtheridonShouldManageTimersAndAssignmentsTrigger : public MagtheridonEncounterTrigger
{
public:
    MagtheridonShouldManageTimersAndAssignmentsTrigger(PlayerbotAI* botAI)
        : MagtheridonEncounterTrigger(botAI, "magtheridon should manage timers and assignments") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
