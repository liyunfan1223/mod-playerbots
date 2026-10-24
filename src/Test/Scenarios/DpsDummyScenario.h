/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_DPSDUMMYSCENARIO_H
#define _PLAYERBOT_DPSDUMMYSCENARIO_H

#include "TestScenario.h"

#include <memory>
#include <string>
#include <vector>

std::unique_ptr<TestScenario> MakeDpsDummyScenario(std::string name, uint32 level, std::vector<BotDef> comp,
                                                   GearDef gear, uint32 dummyEntry, uint32 durationSec,
                                                   uint8 progression);

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
