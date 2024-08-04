/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUEUE_H
#define _PLAYERBOT_QUEUE_H

#include "Action.h"
#include "Common.h"

class Queue
{
public:
    Queue(void) {}
    ~Queue(void) {}

    void Push(ActionBasket* action);
    ActionNode* Pop();
    ActionBasket* Peek();
    uint32 Size();
    void RemoveExpired();

private:
    std::list<ActionBasket*> actions;
};

#endif
