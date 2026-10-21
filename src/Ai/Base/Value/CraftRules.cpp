/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"

#include <algorithm>
#include <charconv>
#include <sstream>

namespace CraftRules
{
bool OrderState::HasReagentsForNextCraft() const
{
    if (IsEmpty() || IsCrafted())
        return false;

    for (auto const& [item, reagent] : reagents)
    {
        if (reagent.obtained < reagent.perCraft)
            return false;
    }

    return true;
}

bool OrderState::AddObtained(uint32 item, uint32 itemCount)
{
    auto const itr = reagents.find(item);
    if (itr == reagents.end())
        return false;

    itr->second.obtained += itemCount;
    return true;
}

void OrderState::OnCrafted(uint32 itemCount)
{
    for (auto& [item, reagent] : reagents)
        reagent.obtained -= std::min(reagent.obtained, reagent.perCraft);

    ++crafted;
    made += itemCount;
}

void OrderState::LimitObtained(uint32 item, uint32 held)
{
    auto const itr = reagents.find(item);
    if (itr != reagents.end())
        itr->second.obtained = std::min(itr->second.obtained, HeldAbove(held, itr->second.own));
}

uint32 OrderState::OwedProducts(uint32 held) const
{
    if (IsEmpty() || made <= delivered)
        return 0;

    return std::min(made - delivered, HeldAbove(held, ownProducts));
}

void OrderState::OnDelivered(uint32 itemCount) { delivered = std::min(made, delivered + itemCount); }

uint32 ParseOrderCount(std::string const& param)
{
    size_t const lastSpace = param.rfind(' ');
    if (lastSpace == std::string::npos)
        return 1;

    char const* first = param.data() + lastSpace + 1;
    char const* last = param.data() + param.size();
    if (first == last || !std::all_of(first, last, [](char c) { return c >= '0' && c <= '9'; }))
        return 1;

    uint32 count = 0;
    if (std::from_chars(first, last, count).ec != std::errc())
        return MAX_ORDER_COUNT;  // all digits, so only too large

    return std::clamp<uint32>(count, 1, MAX_ORDER_COUNT);
}

uint32 CraftFee(uint32 itemLevel, uint32 requiredLevel, uint32 crafts)
{
    uint32 const level = std::max(itemLevel, requiredLevel);
    return crafts * (level * level / 40);
}

std::vector<uint32> SplitFee(uint32 fee, std::vector<uint32> const& mailItemCounts)
{
    uint64 total = 0;
    for (uint32 itemCount : mailItemCounts)
        total += itemCount;

    std::vector<uint32> cods;
    uint32 feeLeft = fee;
    for (size_t i = 0; i < mailItemCounts.size(); ++i)
    {
        uint32 const cod = i + 1 == mailItemCounts.size() || !total
                               ? feeLeft
                               : std::min(feeLeft, uint32(uint64(fee) * mailItemCounts[i] / total));
        feeLeft -= cod;
        cods.push_back(cod);
    }

    return cods;
}

uint32 SlotsNeeded(std::vector<StackNeed> const& needs, uint32 crafts)
{
    uint32 slots = 0;
    for (StackNeed const& need : needs)
    {
        uint32 const stack = std::max<uint32>(need.maxStack, 1);
        slots += (need.perCraft * crafts + stack - 1) / stack;
    }

    return slots;
}

uint32 CraftsThatFit(std::vector<StackNeed> const& needs, uint32 requested, uint32 freeSlots)
{
    uint32 crafts = requested;
    while (crafts && SlotsNeeded(needs, crafts) > freeSlots)
        --crafts;

    return crafts;
}

std::string FormatReagents(Reagents const& reagents)
{
    std::ostringstream out;
    for (auto const& [item, reagent] : reagents)
        out << item << ":" << reagent.perCraft << ":" << reagent.obtained << ":" << reagent.own << " ";

    return out.str();
}

bool ParseReagents(std::string const& text, Reagents& reagents)
{
    reagents.clear();
    std::istringstream in(text);
    std::string entry;
    while (in >> entry)
    {
        uint32 values[4] = {};
        char const* pos = entry.data();
        char const* const end = entry.data() + entry.size();
        for (uint32 i = 0; i < 4; ++i)
        {
            auto const [next, ec] = std::from_chars(pos, end, values[i]);
            bool const separated = i < 3 ? next != end && *next == ':' : next == end;
            if (ec != std::errc() || !separated)
            {
                reagents.clear();
                return false;
            }

            pos = next + 1;
        }

        reagents[values[0]] = {values[1], values[2], values[3]};
    }

    return true;
}

std::string EncodeRecipeBits(std::vector<bool> const& known)
{
    static constexpr char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::vector<uint8> values((known.size() + 5) / 6, 0);
    for (size_t i = 0; i < known.size(); ++i)
    {
        if (known[i])
            values[i / 6] |= uint8(1 << (i % 6));
    }

    std::string out;
    out.reserve(values.size());
    for (uint8 value : values)
        out += base64[value];

    return out;
}
}  // namespace CraftRules
