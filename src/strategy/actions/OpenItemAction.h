/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license,
 * you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OPENITEMACTION_H
#define _PLAYERBOT_OPENITEMACTION_H

#include "Action.h"

class Player;
class Item;
class Event;

class OpenItemAction : public Action
{
public:
    OpenItemAction(PlayerbotAI* botAI) : Action(botAI, "open item") { }

    // The main function that is executed when the action is triggered
    bool Execute(Event event) override;

private:
    // Checks if the given item can be opened (i.e., has the openable flag)
    bool CanOpenItem(Item* item);

    // Performs the action of opening the item
    void OpenItem(Item* item, uint8 bag, uint8 slot);
};

#endif
