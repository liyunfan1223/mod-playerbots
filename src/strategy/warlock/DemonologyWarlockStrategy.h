/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DEMONOLOGYWARLOCKSTRATEGY_H
#define _PLAYERBOT_DEMONOLOGYWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "CombatStrategy.h"

class PlayerbotAI;

class DemonologyWarlockStrategy : public GenericWarlockStrategy
{
public:
    DemonologyWarlockStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "demo"; }
    NextAction** getDefaultActions() override;
};

class MetaMeleeAoeStrategy : public CombatStrategy
{
public:
    MetaMeleeAoeStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "meta melee"; }
};
#endif
