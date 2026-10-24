/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTORDERTRIGGERS_H
#define PLAYERBOTS_CRAFTORDERTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class CraftOrderReadyTrigger : public Trigger
{
public:
    CraftOrderReadyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "craft order ready", 2) {}

    bool IsActive() override;
};

// Fires when an order has made no progress for the timeout, or when the bot now answers to someone else.
class CraftOrderStaleTrigger : public Trigger
{
public:
    CraftOrderStaleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "craft order stale", 5) {}

    bool IsActive() override;
};

#endif
