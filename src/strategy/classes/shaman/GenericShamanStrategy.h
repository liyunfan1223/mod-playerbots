/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICSHAMANSTRATEGY_H
#define _PLAYERBOT_GENERICSHAMANSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericShamanStrategy : public CombatStrategy
{
public:
    GenericShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class ShamanBuffDpsStrategy : public Strategy
{
public:
    ShamanBuffDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bdps"; }
};

class ShamanBuffManaStrategy : public Strategy
{
public:
    ShamanBuffManaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bmana"; }
};

class ShamanCureStrategy : public Strategy
{
public:
    ShamanCureStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cure"; }
};

class ShamanHealerDpsStrategy : public Strategy
{
public:
    ShamanHealerDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healer dps"; }
};

#endif
