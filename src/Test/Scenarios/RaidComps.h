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

inline std::vector<BotDef> Raid10Comp()
{
    // Three healers behind a warrior tank pair.
    return {
        { CLASS_WARRIOR, "prot pve" },   // MT
        { CLASS_WARRIOR, "prot pve" },   // OT
        { CLASS_PALADIN, "holy pve" },
        { CLASS_PRIEST, "holy pve" },
        { CLASS_DRUID, "resto pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_ROGUE, "combat pve" },
        { CLASS_HUNTER, "mm pve" },
        { CLASS_PRIEST, "shadow pve" },
    };
}

inline std::vector<BotDef> Raid10CatComp()
{
    // Raid10Comp with the resto druid swapped for a cat
    return {
        { CLASS_WARRIOR, "prot pve" },   // MT
        { CLASS_WARRIOR, "prot pve" },   // OT
        { CLASS_PALADIN, "holy pve" },
        { CLASS_PRIEST, "holy pve" },
        { CLASS_DRUID, "cat pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_ROGUE, "combat pve" },
        { CLASS_HUNTER, "mm pve" },
        { CLASS_PRIEST, "shadow pve" },
    };
}

inline std::vector<BotDef> Raid25Comp()
{
    // All specs minus arms/prot warrior, subtlety, arcane and destro — 25
    // exactly, tanked by a paladin and a DK.
    return {
        // group 1: tanks + melee
        { CLASS_PALADIN, "prot pve" },        // MT
        { CLASS_DEATH_KNIGHT, "blood pve" },  // OT
        { CLASS_WARRIOR, "fury pve" },
        { CLASS_DEATH_KNIGHT, "frost pve" },
        { CLASS_DEATH_KNIGHT, "unholy pve" },
        // group 2: healers
        { CLASS_PALADIN, "holy pve" },
        { CLASS_PRIEST, "disc pve" },
        { CLASS_PRIEST, "holy pve" },
        { CLASS_DRUID, "resto pve" },
        { CLASS_SHAMAN, "resto pve" },
        // group 3: casters
        { CLASS_PRIEST, "shadow pve" },
        { CLASS_DRUID, "balance pve" },
        { CLASS_SHAMAN, "ele pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_MAGE, "frost pve" },
        // group 4: locks + hunters
        { CLASS_WARLOCK, "affli pve" },
        { CLASS_WARLOCK, "demo pve" },
        { CLASS_HUNTER, "bm pve" },
        { CLASS_HUNTER, "mm pve" },
        { CLASS_HUNTER, "surv pve" },
        // group 5: melee
        { CLASS_PALADIN, "ret pve" },
        { CLASS_SHAMAN, "enh pve" },
        { CLASS_DRUID, "cat pve" },
        { CLASS_ROGUE, "as pve" },
        { CLASS_ROGUE, "combat pve" },
    };
}

inline std::vector<BotDef> Raid25BearComp()
{
    // Raid25Comp with cat traded for a bear third tank
    return {
        // group 1: tanks
        { CLASS_PALADIN, "prot pve" },        // MT
        { CLASS_DEATH_KNIGHT, "blood pve" },  // OT / soak
        { CLASS_DRUID, "bear pve" },          // OT / soak
        { CLASS_WARRIOR, "fury pve" },
        { CLASS_DEATH_KNIGHT, "frost pve" },
        // group 2: healers
        { CLASS_PALADIN, "holy pve" },
        { CLASS_PRIEST, "disc pve" },
        { CLASS_PRIEST, "holy pve" },
        { CLASS_DRUID, "resto pve" },
        { CLASS_SHAMAN, "resto pve" },
        // group 3: casters
        { CLASS_PRIEST, "shadow pve" },
        { CLASS_DRUID, "balance pve" },
        { CLASS_SHAMAN, "ele pve" },
        { CLASS_MAGE, "fire pve" },
        { CLASS_MAGE, "frost pve" },
        // group 4: locks + hunters
        { CLASS_WARLOCK, "affli pve" },
        { CLASS_WARLOCK, "demo pve" },
        { CLASS_HUNTER, "bm pve" },
        { CLASS_HUNTER, "mm pve" },
        { CLASS_HUNTER, "surv pve" },
        // group 5: melee
        { CLASS_PALADIN, "ret pve" },
        { CLASS_SHAMAN, "enh pve" },
        { CLASS_DEATH_KNIGHT, "unholy pve" },
        { CLASS_ROGUE, "as pve" },
        { CLASS_ROGUE, "combat pve" },
    };
}

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
