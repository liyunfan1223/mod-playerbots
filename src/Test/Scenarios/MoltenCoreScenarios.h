/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_MOLTENCORESCENARIOS_H
#define _PLAYERBOT_MOLTENCORESCENARIOS_H

#include "RaidBossScenario.h"
#include "TestScenario.h"

#include <memory>
#include <string>
#include <vector>

// One Molten Core boss: spawn location, encounter adds and how to pull him.
struct McBoss
{
    char const* name;
    uint32 entry;
    float x, y, z, o;
    std::vector<uint32> adds;
    float arenaOffset = 45.0f;
    PullMode pullMode = PullMode::WalkIn;
};

std::unique_ptr<TestScenario> MakeMoltenCoreBoss(McBoss const& b, std::string nameSuffix,
                                                 std::vector<BotDef> comp, uint32 tankCount, GearDef gear);
std::unique_ptr<TestScenario> MakeMCMajordomoScenario(std::string name, std::vector<BotDef> comp, GearDef gear);
std::unique_ptr<TestScenario> MakeMCRagnarosScenario(std::string name, std::vector<BotDef> comp, GearDef gear);

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
