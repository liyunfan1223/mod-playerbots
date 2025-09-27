/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOT_QUEUE_H
#define PLAYERBOT_QUEUE_H

#include "Action.h"
#include "Common.h"

/**
 * @class Queue
 * @brief Manages a priority queue of actions for the playerbot system
 *
 * This queue maintains a list of ActionBasket objects, each containing an action
 * and its relevance score. Actions with higher relevance scores are prioritized.
 */
class Queue
{
public:
    Queue() = default;
    ~Queue() = default;

    /**
     * @brief Adds an action to the queue or updates existing action's relevance
     * @param action Pointer to the ActionBasket to be added
     *
     * If an action with the same name exists, updates its relevance if the new
     * relevance is higher, then deletes the new action. Otherwise, adds the new
     * action to the queue.
     */
    void Push(ActionBasket* action);

    /**
     * @brief Removes and returns the action with highest relevance
     * @return Pointer to the highest relevance ActionNode, or nullptr if queue is empty
     *
     * Ownership of the returned ActionNode is transferred to the caller.
     * The associated ActionBasket is deleted.
     */
    ActionNode* Pop();

    /**
     * @brief Returns the action with highest relevance without removing it
     * @return Pointer to the ActionBasket with highest relevance, or nullptr if queue is empty
     */
    ActionBasket* Peek();

    /**
     * @brief Returns the current size of the queue
     * @return Number of actions in the queue
     */
    uint32 Size();

    /**
     * @brief Removes and deletes expired actions from the queue
     *
     * Uses sPlayerbotAIConfig->expireActionTime to determine if actions have expired.
     * Both the ActionNode and ActionBasket are deleted for expired actions.
     */
    void RemoveExpired();

private:
    /**
     * @brief Updates existing basket with new relevance and cleans up new basket
     */
    void updateExistingBasket(ActionBasket* existing, ActionBasket* newBasket);

    /**
     * @brief Finds the basket with the highest relevance score
     * @return Pointer to the highest relevance basket, or nullptr if queue is empty
     */
    ActionBasket* findHighestRelevanceBasket() const;

    /**
     * @brief Extracts action from basket and handles basket cleanup
     */
    ActionNode* extractAndDeleteBasket(ActionBasket* basket);

    /**
     * @brief Collects all expired baskets into the provided list
     */
    void collectExpiredBaskets(std::list<ActionBasket*>& expiredBaskets);

    /**
     * @brief Removes and deletes all baskets in the provided list
     */
    void removeAndDeleteBaskets(std::list<ActionBasket*>& basketsToRemove);

    std::list<ActionBasket*> actions; /**< Container for action baskets */
};

#endif
