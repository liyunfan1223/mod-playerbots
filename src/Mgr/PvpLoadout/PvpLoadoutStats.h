/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTSTATS_H
#define PLAYERBOTS_PVPLOADOUTSTATS_H

#include "PvpLoadoutEp.h"

class Player;
struct ItemTemplate;

// Reads gear and bots into EP stat vectors. Ratings are scaled to their level-80 equivalent at the bot's level, since
// the profiles' targets are level-80 values.
class PvpLoadoutStats
{
public:
    // The bot's PvP role, from its talents.
    static PvpLoadout::Role RoleOf(Player* bot);

    static PvpLoadout::StatVector ItemStats(Player const* bot, PvpLoadout::Role role, ItemTemplate const* proto);
    static PvpLoadout::StatVector EnchantStats(Player const* bot, PvpLoadout::Role role, uint32 enchantId);

    // The bot's current hit (for its role), spell penetration and resilience, ratings scaled like the items'. Only
    // capped stats need a measured base; the others are linear, so their totals do not change what an item is worth.
    static PvpLoadout::StatVector MeasureCappedTotals(Player* bot, PvpLoadout::Role role);
};

#endif
