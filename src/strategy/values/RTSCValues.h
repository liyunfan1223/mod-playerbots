/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTSCVALUE_H
#define _PLAYERBOT_RTSCVALUE_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class PlayerbotAI;

class SeeSpellLocationValue : public LogCalculatedValue<WorldPosition>
{
public:
    SeeSpellLocationValue(PlayerbotAI* botAI, std::string const name = "see spell location")
        : LogCalculatedValue(botAI, name){};

    bool EqualToLast(WorldPosition value) override;
    WorldPosition Calculate() override;
};

class RTSCSelectedValue : public ManualSetValue<bool>
{
public:
    RTSCSelectedValue(PlayerbotAI* botAI, bool defaultvalue = false, std::string const name = "RTSC selected")
        : ManualSetValue(botAI, defaultvalue, name){};
};

class RTSCNextSpellActionValue : public ManualSetValue<std::string>
{
public:
    RTSCNextSpellActionValue(PlayerbotAI* botAI, std::string const defaultvalue = "",
                             std::string const name = "RTSC next spell action")
        : ManualSetValue(botAI, defaultvalue, name){};
};

class RTSCSavedLocationValue : public ManualSetValue<WorldPosition>, public Qualified
{
public:
    RTSCSavedLocationValue(PlayerbotAI* botAI, WorldPosition defaultvalue = WorldPosition(),
                           std::string const name = "RTSC saved location")
        : ManualSetValue(botAI, defaultvalue, name){};

    std::string const Save() override;
    bool Load(std::string const text) override;
};

#endif
