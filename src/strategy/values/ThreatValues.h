/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_THREATVALUES_H
#define _PLAYERBOT_THREATVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class ThreatValue : public Uint8CalculatedValue, public Qualified
{
public:
    ThreatValue(PlayerbotAI* botAI, std::string const name = "threat") : Uint8CalculatedValue(botAI, name) {}

    uint8 Calculate() override;

protected:
    uint8 Calculate(Unit* target);
};

class NeglectThreatResetValue : public ManualSetValue<bool>
{
public:
    NeglectThreatResetValue(PlayerbotAI* ai, bool defaultValue = false, std::string name = "neglect threat")
        : ManualSetValue<bool>(ai, defaultValue, name)
    {
    }
    virtual bool Get()
    {
        bool ret = value;
        Reset();
        return ret;
    }
};

#endif
