/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_GEAR_ACTIONS_H
#define PLAYERBOTS_LIVING_GEAR_ACTIONS_H

#include <set>

#include "ItemUsageValue.h"

class WorldPosition;

// Gear-motivated bots keep a short wishlist of nearby open-world creature types whose signature
// drops would be upgrades, and prefer those hunting grounds when they go grinding. Looted
// upgrades are equipped by the existing loot-upgrade logic.
struct LivingGearHunt
{
    std::vector<uint32> creatures;  // best first
    uint32 upgrades = 0;            // wanted items across those creatures
};

class LivingGearHuntValue : public CalculatedValue<LivingGearHunt>
{
public:
    // Refreshed every 10 minutes: gear changes slowly and the scan is bounded but not free.
    LivingGearHuntValue(PlayerbotAI* ai) : CalculatedValue(ai, "living gear hunt", 600000) {}
    LivingGearHunt Calculate() override;
};

namespace LivingGear
{
// A grind position at one of the bot's wishlist hunting grounds, or false if it has none.
bool SelectHuntPos(Player* bot, WorldPosition& out);
// Narrows a gear hunter's Dungeon Finder list (LFG dungeon ids) to dungeons whose bosses drop
// upgrades for it. Leaves the list alone for other bots or when no dungeon has one.
void FilterDungeons(PlayerbotAI* ai, std::set<uint32>& dungeons);
}  // namespace LivingGear

#endif
