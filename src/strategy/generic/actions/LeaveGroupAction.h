/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LEAVEGROUPACTION_H
#define _PLAYERBOT_LEAVEGROUPACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

class LeaveGroupAction : public Action
{
public:
    LeaveGroupAction(PlayerbotAI* botAI, std::string const name = "leave") : Action(botAI, name) {}

    bool Execute(Event event) override;

    virtual bool Leave(Player* player);
};

class PartyCommandAction : public LeaveGroupAction
{
public:
    PartyCommandAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "party command") {}

    bool Execute(Event event) override;
};

class UninviteAction : public LeaveGroupAction
{
public:
    UninviteAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "uninvite") {}

    bool Execute(Event event) override;
};

class LeaveFarAwayAction : public LeaveGroupAction
{
public:
    LeaveFarAwayAction(PlayerbotAI* botAI) : LeaveGroupAction(botAI, "leave far away") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
