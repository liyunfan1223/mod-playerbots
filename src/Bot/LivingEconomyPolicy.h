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
