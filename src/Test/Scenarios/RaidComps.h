/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_RAIDCOMPS_H
#define _PLAYERBOT_RAIDCOMPS_H

#include "SharedDefines.h"
#include "TestScenario.h"

// Raid comps for test scenarios. Comps with DKs are level-80 only.

inline std::vector<BotDef> AllSpecsComp()
{
    // One bot per talent tree — 9 classes x 3, no DK (27).
    return {
        // group 1: tanks + warriors
        { CLASS_WARRIOR, "prot pve" },    // MT
        { CLASS_PALADIN, "prot pve" },    // OT
        { CLASS_WARRIOR, "arms pve" },
        { CLASS_WARRIOR, "fury pve" },
        { CLASS_PALADIN, "holy pve" },
        // group 2: healers
        { CLASS_PRIEST, "disc pve" },
        { CLASS_PRIEST, "holy pve" },
        { CLASS_DRUID, "resto pve" },
        { CLASS_SHAMAN, "resto pve" },
        { CLASS_PRIEST, "shadow pve" },
        // group 3: casters
        { CLASS_DRUID, "balance pve" },
        { CLASS_SHAMAN, "ele pve" },
        { CLASS_MAGE, "arcane pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_MAGE, "frost pve" },
        // group 4: locks + hunters
        { CLASS_WARLOCK, "affli pve" },
        { CLASS_WARLOCK, "demo pve" },
        { CLASS_WARLOCK, "destro pve" },
        { CLASS_HUNTER, "bm pve" },
        { CLASS_HUNTER, "mm pve" },
        // group 5: melee
        { CLASS_PALADIN, "ret pve" },
        { CLASS_SHAMAN, "enh pve" },
        { CLASS_DRUID, "cat pve" },
        { CLASS_ROGUE, "as pve" },
        { CLASS_ROGUE, "combat pve" },
        // group 6: overflow
        { CLASS_ROGUE, "subtlety pve" },
        { CLASS_HUNTER, "surv pve" },
    };
}

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
