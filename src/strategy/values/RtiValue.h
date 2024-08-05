/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTIVALUE_H
#define _PLAYERBOT_RTIVALUE_H

#include "Value.h"

class PlayerbotAI;

class RtiValue : public ManualSetValue<std::string>
{
public:
    RtiValue(PlayerbotAI* botAI);

    std::string const Save() override;
    bool Load(std::string const text) override;
};

class RtiCcValue : public ManualSetValue<std::string>
{
public:
    RtiCcValue(PlayerbotAI* botAI);

    std::string const Save() override;
    bool Load(std::string const text) override;
};

#endif
