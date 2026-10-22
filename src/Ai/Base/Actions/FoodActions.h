/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOODACTIONS_H
#define PLAYERBOTS_FOODACTIONS_H

#include "UseItemAction.h"

class PlayerbotAI;

class DrinkAction : public UseItemAction
{
public:
    DrinkAction(PlayerbotAI* botAI) : UseItemAction(botAI, "drink") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class EatAction : public UseItemAction
{
public:
    // "food" is the inventory category; the action and chat command are named "eat".
    EatAction(PlayerbotAI* botAI) : UseItemAction(botAI, "eat", false, "food") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

#endif
