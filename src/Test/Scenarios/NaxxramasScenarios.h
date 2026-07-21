/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_NAXXRAMASSCENARIOS_H
#define _PLAYERBOT_NAXXRAMASSCENARIOS_H

#include "TestScenario.h"

#include <memory>
#include <string>
#include <vector>

std::unique_ptr<TestScenario> MakePatchwerk(std::string name, GearDef gear, std::vector<BotDef> comp,
                                            uint32 tankCount);

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
