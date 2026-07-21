/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_MOLTENCOREDEFS_H
#define _PLAYERBOT_MOLTENCOREDEFS_H

#include "Common.h"
#include "Position.h"

namespace MoltenCoreTest
{
    constexpr uint32 MAP_ID = 409;
    inline Position const ENTRANCE = { 1091.89f, -466.99f, -105.08f, 3.14f };
    constexpr uint32 NPC_MAJORDOMO = 12018;
    constexpr uint32 NPC_RAGNAROS = 11502;
    constexpr uint32 NPC_FLAMEWALKER_HEALER = 11663;
    constexpr uint32 NPC_FLAMEWALKER_ELITE = 11664;
    constexpr uint8 DATA_MAGMADAR = 1;
    constexpr uint8 DATA_MAJORDOMO = 8;
    constexpr int32 ACTION_START_RAGNAROS_INTRO = -1;

    constexpr uint32 FIGHT_TIMEOUT_MS = 15 * MINUTE * IN_MILLISECONDS;
    constexpr uint32 SPAWN_WAIT_TIMEOUT_MS = 30 * IN_MILLISECONDS;
    constexpr uint32 WALK_IN_TIMEOUT_MS = 60 * IN_MILLISECONDS;
    constexpr uint32 EMERGE_TIMEOUT_MS = 180 * IN_MILLISECONDS;

    // Majordomo geometry: adds spawn in a 50y arc around the summon point; the arena
    // keeps 26y+ to every spawn on flat dry ground, and the fight starts on tank walk-in.
    inline Position const MAJORDOMO_SUMMON_POS = { 759.542f, -1173.43f, -118.974f, 3.3048f };
    inline Position const MAJORDOMO_ARENA = { 760.0f, -1225.0f, -120.1f, 1.571f };
    inline Position const MAJORDOMO_WALK_IN = { 749.6f, -1175.0f, -118.9f, 0.0f };

    // Ragnaros lair is mostly lava; Majordomo's own spot is proven floor, Ragnaros emerges 21y away.
    inline Position const RAGNAROS_ARENA = { 848.9f, -812.9f, -229.6f, 4.0f };
    inline Position const MAJORDOMO_LAIR_POS = { 848.933f, -812.875f, -229.601f, 4.046f };
}

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
