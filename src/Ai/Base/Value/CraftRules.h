/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTRULES_H
#define PLAYERBOTS_CRAFTRULES_H

#include "Define.h"
#include <map>
#include <string>
#include <vector>

// Pure bookkeeping for crafting orders and profession links. No core types, so it is unit-testable without a server.
namespace CraftRules
{
constexpr uint32 MAX_ORDER_COUNT = 100;

struct Reagent
{
    uint32 perCraft = 0;
    uint32 obtained = 0;  // traded in by the customer and not yet used by a craft
    uint32 own = 0;       // the bot's own stock when the order was placed, which is never the customer's
};

using Reagents = std::map<uint32, Reagent>;

// Above the bot's own stock; what can be the customer's.
inline uint32 HeldAbove(uint32 held, uint32 own) { return held > own ? held - own : 0; }

// An order's counts. What is the customer's is never more than they traded in, nor more than the bot holds above
// its own stock from before the order.
struct OrderState
{
    uint32 itemId = 0;
    uint32 spellId = 0;
    uint32 count = 0;           // crafts ordered
    uint32 crafted = 0;         // crafts completed
    uint32 made = 0;            // items those crafts created
    uint32 delivered = 0;       // items the customer has taken in trades
    uint32 ownProducts = 0;     // the bot's own stock of the product when the order was placed
    uint32 lastProgress = 0;    // getMSTime() of the last step forward
    bool castPending = false;   // the order action started the current cast
    Reagents reagents;

    bool IsEmpty() const { return itemId == 0; }
    bool IsRequired(uint32 item) const { return reagents.find(item) != reagents.end(); }
    bool IsReserved(uint32 item) const { return !IsEmpty() && (item == itemId || IsRequired(item)); }
    bool IsCrafted() const { return !IsEmpty() && crafted >= count; }
    // The fee is paid in the first trade that hands crafted items over.
    bool FeePaid() const { return delivered > 0; }

    bool HasReagentsForNextCraft() const;
    // False when the item is not a reagent of the order.
    bool AddObtained(uint32 item, uint32 itemCount);
    void OnCrafted(uint32 itemCount);
    // Lowers a reagent's obtained count to what the bot holds above its own stock.
    void LimitObtained(uint32 item, uint32 held);
    // Crafted items the customer is still owed, of the held ones above the bot's own stock.
    uint32 OwedProducts(uint32 held) const;
    // Counts the customer's take; never more than was made.
    void OnDelivered(uint32 itemCount);

    void Clear() { *this = OrderState(); }
};

// The order count after the item link ("craft [link] 3"): 1 when absent or not a number, at most MAX_ORDER_COUNT.
uint32 ParseOrderCount(std::string const& param);

// The fee for a number of crafts of an item.
uint32 CraftFee(uint32 itemLevel, uint32 requiredLevel, uint32 crafts);

// The cash-on-delivery amount for each mail, split by the items each carries; the last mail takes the remainder.
std::vector<uint32> SplitFee(uint32 fee, std::vector<uint32> const& mailItemCounts);

struct StackNeed
{
    uint32 perCraft;  // items per craft
    uint32 maxStack;
};

// Bag slots the crafts need at worst: every reagent traded in before the first craft and the products in new stacks.
uint32 SlotsNeeded(std::vector<StackNeed> const& needs, uint32 crafts);

// The most crafts, up to requested, whose worst case fits in freeSlots; 0 when not even one does.
uint32 CraftsThatFit(std::vector<StackNeed> const& needs, uint32 requested, uint32 freeSlots);

// The recipe field of a |Htrade: link: one bit per skill line entry, six per character, least significant bit first,
// in standard base64 without padding.
std::string EncodeRecipeBits(std::vector<bool> const& known);
}  // namespace CraftRules

#endif
