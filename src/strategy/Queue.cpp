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
    if (!action)
    {
        return;
    }

    for (ActionBasket* basket : actions)
    {
        if (action->getAction()->getName() == basket->getAction()->getName())
        {
            updateExistingBasket(basket, action);
            return;
        }
    }

    actions.push_back(action);
}

ActionNode* Queue::Pop()
{
    ActionBasket* highestRelevanceBasket = findHighestRelevanceBasket();
    if (!highestRelevanceBasket)
    {
        return nullptr;
    }

    return extractAndDeleteBasket(highestRelevanceBasket);
}

ActionBasket* Queue::Peek()
{
    return findHighestRelevanceBasket();
}

uint32 Queue::Size()
{
    return actions.size();
}

void Queue::RemoveExpired()
{
    if (!sPlayerbotAIConfig->expireActionTime)
    {
        return;
    }

    std::list<ActionBasket*> expiredBaskets;
    collectExpiredBaskets(expiredBaskets);
    removeAndDeleteBaskets(expiredBaskets);
}

// Private helper methods
void Queue::updateExistingBasket(ActionBasket* existing, ActionBasket* newBasket)
{
    if (existing->getRelevance() < newBasket->getRelevance())
    {
        existing->setRelevance(newBasket->getRelevance());
    }

    if (ActionNode* actionNode = newBasket->getAction())
    {
        delete actionNode;
    }

    delete newBasket;
}

ActionBasket* Queue::findHighestRelevanceBasket() const
{
    if (actions.empty())
    {
        return nullptr;
    }

    float maxRelevance = -1.0f;
    ActionBasket* selection = nullptr;

    for (ActionBasket* basket : actions)
    {
        if (!basket)
        {
            continue;
        }

        if (basket->getRelevance() > maxRelevance)
        {
            maxRelevance = basket->getRelevance();
            selection = basket;
        }
    }

    return selection;
}

ActionNode* Queue::extractAndDeleteBasket(ActionBasket* basket)
{
    ActionNode* action = basket->getAction();
    actions.remove(basket);
    delete basket;
    return action;
}

void Queue::collectExpiredBaskets(std::list<ActionBasket*>& expiredBaskets)
{
    uint32 expiryTime = sPlayerbotAIConfig->expireActionTime;
    for (ActionBasket* basket : actions)
    {
        if (basket->isExpired(expiryTime))
        {
            expiredBaskets.push_back(basket);
        }
    }
}

void Queue::removeAndDeleteBaskets(std::list<ActionBasket*>& basketsToRemove)
{
    for (ActionBasket* basket : basketsToRemove)
    {
        actions.remove(basket);

        if (ActionNode* action = basket->getAction())
        {
            delete action;
        }

        delete basket;
    }
}
