/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_ECONOMY_POLICY_H
#define PLAYERBOTS_LIVING_ECONOMY_POLICY_H

#include "LivingProfessionPolicy.h"

namespace LivingWorld
{
inline constexpr uint32_t MaxLivingAuctions = 20;
inline constexpr uint32_t MaxLivingItemAuctions = 2;
inline constexpr uint32_t MoneyLimit = 2147483647;
inline constexpr uint32_t PostmasterEntry = 34337;

// Traders travel to sell only when the auctioneer is out of reach (selling starts within 40
// yards) but on the same continent and not absurdly far (MoveFarTo handles long walks).
inline constexpr float MarketReach = 40.0f;
inline constexpr float MaxMarketTrip = 3000.0f;
constexpr bool WorthMarketTrip(float distance) { return distance > MarketReach && distance <= MaxMarketTrip; }

// Auction flipping: a trader with nothing of its own to sell may buy one clearly underpriced
// listing (at most 75% of the median of at least 3 other listings, above vendor price) and
// resell it at cost +15% or just under the going rate, whichever is higher.
inline constexpr uint32_t FlipMinListings = 3;
inline constexpr uint32_t FlipBuyPercent = 75;
inline constexpr uint32_t FlipCooldownSeconds = 600;
// A trader with nothing to sell travels to the auction house to hunt bargains only when it could
// spend at least 1 gold on one under the purchase budget rule (a purse of 5 gold or more).
inline constexpr uint32_t FlipTripBudget = 10000;
constexpr bool IsFlipBargain(uint32_t unitPrice, uint32_t median, uint32_t vendorPrice, uint32_t otherListings)
{
    return otherListings >= FlipMinListings && median && unitPrice > vendorPrice &&
           uint64_t(unitPrice) * 100 <= uint64_t(median) * FlipBuyPercent;
}
constexpr uint32_t FlipResalePrice(uint32_t paidUnit, uint32_t median)
{
    uint64_t const markup = uint64_t(paidUnit) * 115 / 100 + 1;
    uint64_t const going = uint64_t(median) * 97 / 100;
    uint64_t const price = markup > going ? markup : going;
    return price > MoneyLimit ? 0 : static_cast<uint32_t>(price);
}

inline uint32_t Surplus(uint32_t owned, uint32_t reserve, uint32_t stack)
{
    return owned > reserve ? std::min(owned - reserve, stack) : 0;
}

inline uint32_t PurchaseBundles(uint32_t owned, uint32_t needed, uint32_t bundleSize)
{
    if (!bundleSize || owned >= needed)
        return 0;
    return static_cast<uint32_t>(std::min<uint64_t>(255, (uint64_t(needed - owned) + bundleSize - 1) / bundleSize));
}

inline uint32_t AuctionUnitPrice(uint32_t vendor, uint32_t materialCost, uint32_t market, uint32_t count)
{
    if (!count)
        return 0;
    uint64_t const floor = std::max<uint64_t>({1, uint64_t(vendor) * 2, uint64_t(materialCost) * 2});
    uint64_t const desired = market ? std::clamp<uint64_t>(uint64_t(market) * 99 / 100, floor, floor * 10) : floor;
    return desired <= MoneyLimit / count ? static_cast<uint32_t>(desired) : 0;
}

inline bool CanListAuction(uint32_t total, uint32_t sameItem, uint32_t money, uint32_t deposit, uint32_t level)
{
    return total < MaxLivingAuctions && sameItem < MaxLivingItemAuctions && CanAffordTraining(money, deposit, level);
}

inline bool CanProcessBatch(uint32_t owned, uint32_t reserve, uint32_t stack, uint32_t batch)
{
    return batch && Surplus(owned, reserve, stack) >= batch;
}

inline bool CanDeleteLivingMail(bool eligible, uint32_t money, size_t attachments, uint32_t cod)
{
    return eligible && money == 0 && attachments == 0 && cod == 0;
}
}  // namespace LivingWorld

#endif
