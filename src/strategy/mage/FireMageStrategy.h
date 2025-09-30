/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FIREMAGESTRATEGY_H
#define _PLAYERBOT_FIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FireMageStrategy : public GenericMageStrategy
{
public:
    FireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire"; }
    NextAction** getDefaultActions() override;
};

class FirestarterStrategy : public CombatStrategy
{
public:
    FirestarterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "firestarter"; }
};
#endif
