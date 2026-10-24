/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERGATE_H
#define PLAYERBOTS_ULDENCOUNTERGATE_H

#include "RaidEncounter.h"

// Boss ids from AC's ulduar.h (under scripts/, so not includable), as InstanceScript::GetBossState keys them.
// AC #28079 renumbered them to DungeonEncounter.dbc order: a stale copy keeps that boss's gate shut all fight.
enum UlduarEncounterId
{
    ULD_BOSS_LEVIATHAN = 0,
    ULD_BOSS_IGNIS = 1,
    ULD_BOSS_RAZORSCALE = 2,
    ULD_BOSS_XT002 = 3,
    ULD_BOSS_ASSEMBLY = 4,
    ULD_BOSS_KOLOGARN = 5,
    ULD_BOSS_AURIAYA = 6,
    ULD_BOSS_HODIR = 7,
    ULD_BOSS_THORIM = 8,
    ULD_BOSS_FREYA = 9,
    ULD_BOSS_MIMIRON = 10,
    ULD_BOSS_VEZAX = 11,
    ULD_BOSS_YOGGSARON = 12,
    ULD_BOSS_ALGALON = 13
};

// Ported the same way. The types InstanceScript::GetCreature is keyed by; a boss's own is its id above.
enum UlduarObjectData
{
    ULD_DATA_SARA = 760,
    ULD_DATA_BRAIN_OF_YOGG_SARON = 761
};

#endif
