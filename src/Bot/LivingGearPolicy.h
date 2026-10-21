/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_GEAR_POLICY_H
#define PLAYERBOTS_LIVING_GEAR_POLICY_H

#include <algorithm>
#include <cstdint>

// Gear hunting: which open-world drops a gear-motivated bot may farm, and where.
// Pure rules (no core types) so they can be tested standalone.
namespace LivingWorld
{
// ItemClass / ItemQualities values from the core's SharedDefines/ItemTemplate.
inline constexpr uint32_t ItemClassWeapon = 2;
inline constexpr uint32_t ItemClassArmor = 4;
inline constexpr uint32_t QualityUncommon = 2;
inline constexpr uint32_t QualityEpic = 4;

// Bounded work per wishlist refresh: creature types considered, and hunt targets kept.
inline constexpr uint32_t MaxGearCandidates = 60;
inline constexpr uint32_t MaxGearTargets = 3;
// How far (same map) a gear hunter will travel to a hunting ground.
inline constexpr float GearHuntDistance = 2500.0f;
// Percent of grind trips a gear hunter spends at a wishlist ground (the rest stay varied).
inline constexpr uint32_t GearHuntChance = 70;

// Static part of "could this drop be worth hunting": green-to-epic weapons and armor, at or
// below the era's generated item-level cap (0 = no cap). Whether it is an upgrade for this
// bot is decided separately by the bot's own item-usage evaluation.
constexpr bool IsGearDrop(uint32_t itemClass, uint32_t quality, uint32_t itemLevel, int32_t itemLevelCap)
{
    return (itemClass == ItemClassWeapon || itemClass == ItemClassArmor) && quality >= QualityUncommon &&
           quality <= QualityEpic && (itemLevelCap <= 0 || itemLevel <= uint32_t(itemLevelCap));
}

// A drop is a reason to go somewhere only if few creature types have it. Shared world-drop
// pools reach hundreds of types and drop everywhere anyway.
inline constexpr uint32_t MaxSignatureSources = 8;
constexpr bool IsSignatureDrop(uint32_t creatureTypesDropping)
{
    return creatureTypesDropping >= 1 && creatureTypesDropping <= MaxSignatureSources;
}

// The bot must be able to wear it now (no hoarding future-level gear).
constexpr bool CanWearSoon(uint32_t requiredLevel, uint32_t botLevel) { return requiredLevel <= botLevel; }

// Hunting grounds: mobs the bot can kill efficiently, from a little below to slightly above it.
constexpr bool HuntableLevel(uint32_t creatureLevel, uint32_t botLevel)
{
    return creatureLevel + 4 >= botLevel && creatureLevel <= botLevel + 2;
}
}  // namespace LivingWorld

#endif
