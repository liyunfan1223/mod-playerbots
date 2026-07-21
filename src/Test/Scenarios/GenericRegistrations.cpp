/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "GenericScenarios.h"
#include "TestRunner.h"

void RegisterGenericScenarios(IntegrationTestMgr& mgr)
{
    mgr.RegisterScenario(MakeSelfTestScenario());
    mgr.RegisterScenario(MakeBotSmokeScenario());
    mgr.RegisterScenario(MakeReadyCheckScenario());
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
