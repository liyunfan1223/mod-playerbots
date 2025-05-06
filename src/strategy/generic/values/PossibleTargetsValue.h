/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_POSSIBLETARGETSVALUE_H
#define _PLAYERBOT_POSSIBLETARGETSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class PossibleTargetsValue : public NearestUnitsValue
{
public:
    PossibleTargetsValue(PlayerbotAI* botAI, std::string const name = "possible targets",
                         float range = sPlayerbotAIConfig->sightDistance, bool ignoreLos = false)
        : NearestUnitsValue(botAI, name, range, ignoreLos)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class AllTargetsValue : public PossibleTargetsValue
{
public:
    AllTargetsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance)
        : PossibleTargetsValue(botAI, "all targets", range, true)
    {
    }
};

class PossibleTriggersValue : public NearestUnitsValue
{
public:
    PossibleTriggersValue(PlayerbotAI* botAI, std::string const name = "possible triggers", float range = 15.0f,
                          bool ignoreLos = true)
        : NearestUnitsValue(botAI, name, range, ignoreLos, 1 * 1000)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};
#endif
