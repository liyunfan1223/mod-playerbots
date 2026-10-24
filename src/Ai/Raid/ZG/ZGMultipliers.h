/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZGMULTIPLIERS_H
#define PLAYERBOTS_ZGMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "ZGHelpers.h"

class ZulGurubEncounterMultiplier : public Multiplier
{
public:
    ZulGurubEncounterMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, ZulGurubHelpers::ZG_MAP_ID) ? GetValueInEncounter(action)
                                                                                        : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class HakkarKeepPoisonousBloodMultiplier : public ZulGurubEncounterMultiplier
{
public:
    HakkarKeepPoisonousBloodMultiplier(PlayerbotAI* botAI)
        : ZulGurubEncounterMultiplier(botAI, "hakkar keep poisonous blood")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
