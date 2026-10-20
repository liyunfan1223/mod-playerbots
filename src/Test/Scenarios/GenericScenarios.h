/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_GENERICSCENARIOS_H
#define _PLAYERBOT_GENERICSCENARIOS_H

#include "TestScenario.h"

#include <memory>

std::unique_ptr<TestScenario> MakeSelfTestScenario();
std::unique_ptr<TestScenario> MakeBotSmokeScenario();
std::unique_ptr<TestScenario> MakeReadyCheckScenario();

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
