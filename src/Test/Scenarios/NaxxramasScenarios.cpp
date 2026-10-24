/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "NaxxramasScenarios.h"

#include "RaidBossScenario.h"

#include <memory>

namespace
{
    constexpr uint32 MAP_ID = 533;
    constexpr uint32 NPC_PATCHWERK = 16028;
    // Pull spot 51y SE of Patchwerk's spawn
    Position const PATCHWERK_ARENA = { 3283.0f, -3268.0f, 293.4f, 2.4f };
    constexpr uint32 FIGHT_TIMEOUT_MS = 10 * MINUTE * IN_MILLISECONDS;
}

std::unique_ptr<TestScenario> MakePatchwerk(std::string name, GearDef gear, std::vector<BotDef> comp,
                                            uint32 tankCount)
{
    return std::make_unique<RaidBossScenario>(
        std::move(name), NPC_PATCHWERK, MAP_ID,
        PATCHWERK_ARENA.GetPositionX(), PATCHWERK_ARENA.GetPositionY(),
        PATCHWERK_ARENA.GetPositionZ(), PATCHWERK_ARENA.GetOrientation(),
        std::move(comp),
        tankCount, 80, gear, FIGHT_TIMEOUT_MS,
        std::vector<uint32>{}, PROGRESSION_WOTLK_RAIDS, RaidBossScenario::Staging{}, PullMode::Relocate);
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
