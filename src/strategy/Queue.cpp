/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Queue.h"

#include "AiObjectContext.h"
#include "Log.h"
#include "PlayerbotAIConfig.h"

void Queue::Push(ActionBasket* action)
{
    if (action)
    {
        for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
        {
            ActionBasket* basket = *iter;
            if (action->getAction()->getName() == basket->getAction()->getName())
            {
                if (basket->getRelevance() < action->getRelevance())
                    basket->setRelevance(action->getRelevance());

                if (ActionNode* actionNode = action->getAction())
                    delete actionNode;

                delete action;

                return;
            }
        }

        actions.push_back(action);
    }
}

ActionNode* Queue::Pop()
{
    float max = -1;
    ActionBasket* selection = nullptr;

    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }

    if (selection != nullptr)
    {
        ActionNode* action = selection->getAction();
        actions.remove(selection);
        delete selection;
        return action;
    }

    return nullptr;
}

ActionBasket* Queue::Peek()
{
    float max = -1;
    ActionBasket* selection = nullptr;
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }

    return selection;
}

uint32 Queue::Size() { return actions.size(); }

void Queue::RemoveExpired()
{
    std::list<ActionBasket*> expired;
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (sPlayerbotAIConfig->expireActionTime && basket->isExpired(sPlayerbotAIConfig->expireActionTime))
            expired.push_back(basket);
    }

    for (std::list<ActionBasket*>::iterator iter = expired.begin(); iter != expired.end(); iter++)
    {
        ActionBasket* basket = *iter;
        actions.remove(basket);

        if (ActionNode* action = basket->getAction())
        {
            delete action;
        }

        delete basket;
    }
}
