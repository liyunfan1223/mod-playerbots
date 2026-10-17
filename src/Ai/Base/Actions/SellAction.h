/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SELLACTION_H
#define PLAYERBOTS_SELLACTION_H

#include "InventoryAction.h"

class FindItemVisitor;
class Item;
class PlayerbotAI;

class SellAction : public InventoryAction
{
public:
    SellAction(PlayerbotAI* botAI, std::string const name = "sell") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;
    void Sell(FindItemVisitor* visitor);
    void Sell(Item* item, bool force = false);
    uint32 GetQuestItemRequirement(uint32 itemId);
};

#endif
