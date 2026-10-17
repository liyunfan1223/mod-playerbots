/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOT_BOTSTATEACTIONS_H
#define _PLAYERBOT_BOTSTATEACTIONS_H

#include "Action.h"

class PlayerbotAI;

class WakeOnCombatStartAction : public Action
{
public:
    WakeOnCombatStartAction(PlayerbotAI* botAI) : Action(botAI, "wake on combat start") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
