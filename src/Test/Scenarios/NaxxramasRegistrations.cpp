/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "NaxxramasScenarios.h"
#include "RaidComps.h"
#include "TestRunner.h"

#include "SharedDefines.h"

void RegisterNaxxramasScenarios(IntegrationTestMgr& mgr)
{
    mgr.RegisterScenario(MakePatchwerk("patchwerk", ITEM_QUALITY_EPIC, Raid10CatComp(), 2));
    mgr.RegisterScenario(MakePatchwerk("patchwerk_bis200", { ITEM_QUALITY_EPIC, 200, true }, Raid25BearComp(), 3));
    mgr.RegisterScenario(MakePatchwerk("patchwerk_bis290", { ITEM_QUALITY_EPIC, 290, true }, Raid25BearComp(), 3));
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
