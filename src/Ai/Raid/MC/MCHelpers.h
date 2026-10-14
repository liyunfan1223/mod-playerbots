/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MCHELPERS_H
#define PLAYERBOTS_MCHELPERS_H

namespace MoltenCoreHelpers
{
enum MoltenCoreNPCs
{
    // Golemagg
    NPC_CORE_RAGER = 11672,

    // Majordomo Executus
    NPC_MAJORDOMO_EXECUTUS = 12018,

    // Core Hound (trash)
    NPC_CORE_HOUND = 11671,
};
enum MoltenCoreSpells
{
    // Baron Geddon
    SPELL_INFERNO = 19695,
    SPELL_LIVING_BOMB = 20475,

    // Golemagg
    SPELL_GOLEMAGGS_TRUST = 20553,
    SPELL_MAGMA_SPLASH = 13880,
};

constexpr uint32 MAGMA_SPLASH_BACK_OFF_STACKS = 20;
constexpr float MAGMA_SPLASH_BACK_OFF_DISTANCE = 12.0f;

// Shazzrah's Arcane Explosion radius
constexpr float ARCANE_EXPLOSION_DISTANCE = 26.0f;
}

#endif
