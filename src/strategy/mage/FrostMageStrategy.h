/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FROSTMAGESTRATEGY_H
#define _PLAYERBOT_FROSTMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostMageStrategy : public GenericMageStrategy
{
public:
    FrostMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost"; }
    NextAction** getDefaultActions() override;
};

class FrostMageAoeStrategy : public CombatStrategy
{
public:
    FrostMageAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost aoe"; }
};

#endif
