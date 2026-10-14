/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHANGECHATACTION_H
#define PLAYERBOTS_CHANGECHATACTION_H

#include "Action.h"

class PlayerbotAI;

class ChangeChatAction : public Action
{
public:
    ChangeChatAction(PlayerbotAI* botAI) : Action(botAI, "chat") {}

    bool Execute(Event event) override;
    bool isUsefulWithoutControl() override { return true; }
};

#endif
