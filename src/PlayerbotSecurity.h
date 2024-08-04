/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTSECURITY_H
#define _PLAYERBOT_PLAYERBOTSECURITY_H

#include <map>

#include "Common.h"
#include "ObjectGuid.h"

class Player;

enum PlayerbotSecurityLevel : uint32
{
    PLAYERBOT_SECURITY_DENY_ALL = 0,
    PLAYERBOT_SECURITY_TALK = 1,
    PLAYERBOT_SECURITY_INVITE = 2,
    PLAYERBOT_SECURITY_ALLOW_ALL = 3
};

enum DenyReason
{
    PLAYERBOT_DENY_NONE,
    PLAYERBOT_DENY_LOW_LEVEL,
    PLAYERBOT_DENY_GEARSCORE,
    PLAYERBOT_DENY_NOT_YOURS,
    PLAYERBOT_DENY_IS_BOT,
    PLAYERBOT_DENY_OPPOSING,
    PLAYERBOT_DENY_DEAD,
    PLAYERBOT_DENY_FAR,
    PLAYERBOT_DENY_INVITE,
    PLAYERBOT_DENY_FULL_GROUP,
    PLAYERBOT_DENY_NOT_LEADER,
    PLAYERBOT_DENY_IS_LEADER,
    PLAYERBOT_DENY_BG,
    PLAYERBOT_DENY_LFG
};

class PlayerbotSecurity
{
public:
    PlayerbotSecurity(Player* const bot);

    PlayerbotSecurityLevel LevelFor(Player* from, DenyReason* reason = nullptr, bool ignoreGroup = false);
    bool CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup = false);

private:
    Player* const bot;
    uint32 account;
    std::map<ObjectGuid, std::map<std::string, time_t> > whispers;
};

#endif
