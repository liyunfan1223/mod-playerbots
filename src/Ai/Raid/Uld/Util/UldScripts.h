/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDSCRIPTS_H
#define PLAYERBOTS_ULDSCRIPTS_H

// NPC entries ported from ulduar.h in AC. ulduar.h is in ACs /scripts directory,
// so the entries are redefined here to avoid including ulduar.h as a relative path.
enum UlduarNPCs
{
    // Razorscale
    NPC_RAZORSCALE                   = 33186,

    // Iron Assembly
    NPC_BRUNDIR                      = 32857,
    NPC_STEELBREAKER                 = 32867,
    NPC_MOLGEIM                      = 32927,

    // Kologarn
    NPC_KOLOGARN                     = 32930,

    // Auriaya
    NPC_AURIAYA                      = 33515,

    // XT-002 Deconstructor
    NPC_XT002                        = 33293,
    NPC_HEART_OF_DECONSTRUCTOR       = 33329,
    NPC_XT_TOY_PILE                  = 33337,
    NPC_XS013_SCRAPBOT               = 33343,

    // Thorim
    NPC_THORIM                       = 32865,

    // Freya
    NPC_FREYA                        = 32906,

    // Hodir
    NPC_HODIR                        = 32845,

    // Mimiron
    NPC_MIMIRON                      = 33350,

    // Flame Leviathan
    NPC_FLAME_LEVIATHAN              = 33113,

    // Flame Leviathan vehicles
    NPC_SALVAGED_SIEGE_ENGINE        = 33060,
    NPC_VEHICLE_CHOPPER              = 33062,
    NPC_SALVAGED_SIEGE_ENGINE_TURRET = 33067,
    NPC_SALVAGED_DEMOLISHER          = 33109,
    NPC_SALVAGED_DEMOLISHER_TURRET   = 33167
};

// Boss ids for InstanceScript::GetCreature, mirrored from the core's ulduar.h for the same reason as
// the entries above. Index into that file's BossIds enum, not an NPC entry.
enum UlduarBossData
{
    ULD_DATA_VEZAX = 11
};

enum UlduarPersistentData
{
    PERSISTENT_DATA_WATCHERS_MASK = 0
};

// Bit index into PERSISTENT_DATA_WATCHERS_MASK, one per keeper left up for Yogg-Saron.
enum UlduarKeepers
{
    KEEPER_FREYA   = 0,
    KEEPER_HODIR   = 1,
    KEEPER_MIMIRON = 2,
    KEEPER_THORIM  = 3
};

#endif
