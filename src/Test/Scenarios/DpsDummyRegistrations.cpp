/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "DpsDummyScenario.h"
#include "RaidComps.h"
#include "TestRunner.h"

#include "SharedDefines.h"

namespace
{
    constexpr uint32 DUMMY_80 = 31146;  // Heroic Training Dummy
    constexpr uint32 DUMMY_60 = 32666;  // Expert's Training Dummy
    constexpr uint32 WINDOW_SEC = 180;
}

void RegisterDpsDummyScenarios(IntegrationTestMgr& mgr)
{
    mgr.RegisterScenario(MakeDpsDummyScenario(
        "dps_dummy", 80, Raid25Comp(), ITEM_QUALITY_EPIC, DUMMY_80, WINDOW_SEC, PROGRESSION_WOTLK_RAIDS));
    mgr.RegisterScenario(MakeDpsDummyScenario(
        "dps_dummy_bis200", 80, Raid25Comp(), { ITEM_QUALITY_EPIC, 200, true }, DUMMY_80, WINDOW_SEC, PROGRESSION_WOTLK_RAIDS));
    mgr.RegisterScenario(MakeDpsDummyScenario(
        "dps_dummy_bis290", 80, Raid25Comp(), { ITEM_QUALITY_EPIC, 290, true }, DUMMY_80, WINDOW_SEC, PROGRESSION_WOTLK_RAIDS));
    mgr.RegisterScenario(MakeDpsDummyScenario(
        "dps_dummy_60", 60, AllSpecsComp(), ITEM_QUALITY_RARE, DUMMY_60, WINDOW_SEC, PROGRESSION_VANILLA_START));
    mgr.RegisterScenario(MakeDpsDummyScenario(
        "dps_dummy_60_bis66", 60, AllSpecsComp(), { ITEM_QUALITY_EPIC, 66, true }, DUMMY_60, WINDOW_SEC, PROGRESSION_VANILLA_START));
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
