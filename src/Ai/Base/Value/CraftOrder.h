/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTORDER_H
#define PLAYERBOTS_CRAFTORDER_H

#include "Common.h"
#include "ObjectGuid.h"
#include <map>
#include <string>

class CraftData;
class Item;
class Player;
class PlayerbotAI;
class SpellInfo;

struct ItemTemplate;

// Steps of a bot's crafting order (its "craft" value) shared by the craft command, the order actions and triggers,
// the trade handler, the item factory, and bot login and logout.
class CraftOrder
{
public:
    static void Touch(CraftData& data);
    // Tells the bot's master through the usual channel, anyone else by whisper.
    static void Tell(PlayerbotAI* botAI, Player* player, std::string const& text);
    // Empty slots in the backpack and in general-purpose bags; profession bags are not counted.
    static uint32 FreeBagSlots(Player* bot);
    // The most items one cast of the recipe can create.
    static uint32 GetMaxCreated(SpellInfo const* spellInfo, ItemTemplate const* proto);
    // Crafted items the customer is still owed and the bot actually holds.
    static uint32 GetOwedProducts(Player* bot, CraftData const& data);
    // Lowers each reagent's obtained count to what the bot holds above its own stock.
    static void SyncWithBags(Player* bot, CraftData& data);
    static uint32 GetFee(Player* bot, CraftData const& data, uint32 crafts);
    static void TellStatus(PlayerbotAI* botAI);
    static void OnItemCreated(PlayerbotAI* botAI, Item* item, uint32 itemCount);
    // A random bot hands an order's items only to its customer, only once crafted and only as many as it owes, and
    // buys them from no one while the order is open; false, after saying why, when the trade breaks that.
    static bool CheckTrade(PlayerbotAI* botAI, Player* trader);
    // Puts exactly what the customer is owed into the trade they opened; false when there is nothing to hand over.
    static bool OfferCraftedItems(PlayerbotAI* botAI, Player* trader);
    static void OnTradeCompleted(PlayerbotAI* botAI, Player* trader, std::map<uint32, uint32> const& given,
                                 std::map<uint32, uint32> const& taken);
    // Mails back what is left of the customer's reagents and anything crafted they have not collected, then clears
    // the order.
    static void Refund(PlayerbotAI* botAI);
    static void Save(PlayerbotAI* botAI);
    // Reads every saved order once at startup; Load hands a bot its order when it logs in.
    static void LoadAll();
    static void Load(PlayerbotAI* botAI);
    // A payment for one of the bot's refunds, or a refund sent back that fits in its bags, is in its mailbox.
    static bool HasCollectableMail(PlayerbotAI* botAI);
    // Takes the payments and returned refunds out of the mailbox the bot stands at.
    static void CollectMail(PlayerbotAI* botAI, ObjectGuid mailbox);
};

#endif
