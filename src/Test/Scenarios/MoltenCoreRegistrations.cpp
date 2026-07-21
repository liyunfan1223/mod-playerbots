/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "MoltenCoreDefs.h"
#include "MoltenCoreScenarios.h"
#include "RaidComps.h"
#include "TestRunner.h"

#include "SharedDefines.h"

#include <memory>

namespace
{
    // Geddon uses Garr's coords + Relocate.
    McBoss const MC_BOSSES[] = {
        { "mc_lucifron", 12118, 1037.02f, -986.34f, -181.5f, 5.93f, { 12119 } },
        { "mc_magmadar", 11982, 1144.6f, -1020.5f, -185.7f, 2.93f, {} },
        { "mc_gehennas", 12259, 898.1f, -545.9f, -203.4f, 4.82f, { 11661 } },
        { "mc_garr", 12057, 691.0f, -498.4f, -214.3f, 6.02f, { 12099 } },
        { "mc_geddon", 12056, 691.0f, -498.4f, -214.3f, 6.02f, {}, 45.0f, PullMode::Relocate },
        { "mc_shazzrah", 12264, 587.3f, -802.0f, -205.2f, 0.24f, {} },
        { "mc_sulfuron", 12098, 601.1f, -1179.1f, -195.9f, 1.57f, { 11662 } },
        { "mc_golemagg", 11988, 793.2f, -998.3f, -206.8f, 5.911f, { 11672 }, 30.0f },
    };
}

void RegisterMoltenCoreScenarios(IntegrationTestMgr& mgr)
{
    GearDef const bis66{ ITEM_QUALITY_EPIC, 66, true };
    GearDef const bis76{ ITEM_QUALITY_EPIC, 76, true };
    for (McBoss const& b : MC_BOSSES)
    {
        mgr.RegisterScenario(MakeMoltenCoreBoss(b, "", AllSpecsComp(), 2, ITEM_QUALITY_RARE));
        mgr.RegisterScenario(MakeMoltenCoreBoss(b, "_bis66", AllSpecsComp(), 2, bis66));
        mgr.RegisterScenario(MakeMoltenCoreBoss(b, "_bis76", AllSpecsComp(), 2, bis76));
    }

    mgr.RegisterScenario(MakeMCMajordomoScenario("mc_majordomo", AllSpecsComp(), ITEM_QUALITY_RARE));
    mgr.RegisterScenario(MakeMCMajordomoScenario("mc_majordomo_bis66", AllSpecsComp(), bis66));
    mgr.RegisterScenario(MakeMCMajordomoScenario("mc_majordomo_bis76", AllSpecsComp(), bis76));
    mgr.RegisterScenario(MakeMCRagnarosScenario("mc_ragnaros", AllSpecsComp(), ITEM_QUALITY_RARE));
    mgr.RegisterScenario(MakeMCRagnarosScenario("mc_ragnaros_bis66", AllSpecsComp(), bis66));
    mgr.RegisterScenario(MakeMCRagnarosScenario("mc_ragnaros_bis76", AllSpecsComp(), bis76));

    // Fast pipeline smoke test: first MC boss with a level-80 raid, quick kill.
    McBoss const lucifron80 = { "raid_boss", 12118, 1037.02f, -986.34f, -181.5f, 5.93f, { 12119 } };
    mgr.RegisterScenario(std::make_unique<RaidBossScenario>(
        "raid_boss", lucifron80.entry, MoltenCoreTest::MAP_ID,
        lucifron80.x, lucifron80.y, lucifron80.z, lucifron80.o,
        Raid10Comp(), 2, 80, ITEM_QUALITY_EPIC, 10 * MINUTE * IN_MILLISECONDS,
        lucifron80.adds, PROGRESSION_WOTLK_RAIDS));
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
