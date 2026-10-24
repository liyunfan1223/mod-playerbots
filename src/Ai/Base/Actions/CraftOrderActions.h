/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTORDERACTIONS_H
#define PLAYERBOTS_CRAFTORDERACTIONS_H

#include "Action.h"

class PlayerbotAI;

class CraftOrderAction : public Action
{
public:
    CraftOrderAction(PlayerbotAI* botAI) : Action(botAI, "craft order") {}

    bool Execute(Event event) override;
    bool isPossible() override;
};

class RefundCraftOrderAction : public Action
{
public:
    RefundCraftOrderAction(PlayerbotAI* botAI) : Action(botAI, "refund craft order") {}

    bool Execute(Event event) override;
};

#endif
