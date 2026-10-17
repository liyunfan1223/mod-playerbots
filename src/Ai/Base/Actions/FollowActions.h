/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOLLOWACTIONS_H
#define PLAYERBOTS_FOLLOWACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;

class FollowAction : public MovementAction
{
public:
    FollowAction(PlayerbotAI* botAI, std::string const name = "follow") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool CanDeadFollow(Unit* target);
};

#endif
