/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGMULTIPLIERS_H
#define PLAYERBOTS_MAGMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "Multiplier.h"
#include <string>

class MagtheridonEncounterMultiplier : public Multiplier
{
public:
    MagtheridonEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, MagHelpers::MAG_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class MagtheridonUseManticronCubeMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonUseManticronCubeMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon use manticron cube") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonHoldDpsMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonHoldDpsMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon hold dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonControlTankActionsMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonControlTankActionsMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon control tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MagtheridonAvoidDebrisDangerMultiplier : public MagtheridonEncounterMultiplier
{
public:
    MagtheridonAvoidDebrisDangerMultiplier(PlayerbotAI* botAI)
        : MagtheridonEncounterMultiplier(botAI, "magtheridon avoid debris danger") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
