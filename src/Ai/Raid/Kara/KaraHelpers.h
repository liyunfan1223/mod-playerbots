/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAHELPERS_H
#define PLAYERBOTS_KARAHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <ctime>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

class Player;
class Unit;

namespace KaraHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class KaraSpells : uint32
{
    // Maiden of Virtue
    SPELL_REPENTANCE              = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD  = 30756,

    // The Curator
    SPELL_CURATOR_EVOCATION       = 30254,

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST       = 30004,
    SPELL_FLAME_WREATH_AURA       = 29946,
    SPELL_BLIZZARD                = 29951,
    SPELL_ARCANE_EXPLOSION        = 29973,

    // Netherspite
    SPELL_RED_BEAM_DEBUFF         = 30421, // "Nether Portal - Perseverance" (player aura)
    SPELL_GREEN_BEAM_DEBUFF       = 30422, // "Nether Portal - Serenity" (player aura)
    SPELL_BLUE_BEAM_DEBUFF        = 30423, // "Nether Portal - Dominance" (player aura)
    SPELL_NETHER_EXHAUSTION_RED   = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE  = 38639,
    SPELL_NETHERSPITE_BANISHED    = 39833, // "Vortex Shade Black"

    // Prince Malchezaar
    SPELL_ENFEEBLE                = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH           = 30129,
    SPELL_RAIN_OF_BONES           = 37091,

    // Priest
    SPELL_FEAR_WARD               = 6346,

    // Shaman
    SPELL_TREMOR_TOTEM            = 8143,
    SPELL_GROUNDING_TOTEM         = 8177,
};

enum class KaraNpcs : uint32
{
    // Trash
    NPC_MANA_WARP                 = 16530,

    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN      = 16152, // ID for mounted version

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL        = 17167,

    // Netherspite
    NPC_VOID_ZONE                 = 16697,
    NPC_GREEN_PORTAL              = 17367, // "Nether Portal - Serenity <Healing Portal>"
    NPC_BLUE_PORTAL               = 17368, // "Nether Portal - Dominance <Damage Portal>"
    NPC_RED_PORTAL                = 17369, // "Nether Portal - Perseverance <Tanking Portal>"

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL      = 17646,
};

// General
inline constexpr uint32 KARA_MAP_ID = 532;
bool IsSafePosition (float x, float y, std::vector<Unit*> const& hazards, float hazardRadius);

// Attumen the Huntsman
inline Position const ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };
extern std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
Unit* GetAttumenMounted(Player* bot);

// Maiden of Virtue
inline Position const MAIDEN_OF_VIRTUE_TANK_POSITION = { -10945.881f, -2103.782f, 92.712f };
inline std::array const MAIDEN_OF_VIRTUE_RANGED_POSITIONS = {
    Position{ -10959.242f, -2119.617f, 92.180f }, // SE (opposite entrance)
    Position{ -10944.495f, -2123.857f, 92.180f }, // E
    Position{ -10966.017f, -2105.288f, 92.175f }, // S
    Position{ -10931.178f, -2116.580f, 92.179f }, // NE
    Position{ -10960.912f, -2090.437f, 92.179f }, // SW
    Position{ -10925.828f, -2102.425f, 92.180f }, // N
    Position{ -10947.590f, -2082.815f, 92.180f }, // W
    Position{ -10933.089f, -2088.502f, 92.180f }, // NW (entrance)
};

// The Big Bad Wolf
inline Position const BIG_BAD_WOLF_TANK_POSITION = { -10913.391f, -1773.508f, 90.477f };
inline std::array const BIG_BAD_WOLF_RUN_POSITIONS = {
    Position{ -10875.456f, -1779.036f, 90.477f },
    Position{ -10872.281f, -1751.638f, 90.477f },
    Position{ -10910.492f, -1747.401f, 90.477f },
    Position{ -10913.391f, -1773.508f, 90.477f },
};

// Wizard of Oz
inline std::array const OZ_TARGETS = {
    "dorothee",
    "tito",
    "roar",
    "strawman",
    "tinhead",
};

// The Curator
inline Position const THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };

// Shade of Aran
bool IsAranCastingArcaneExplosion(Unit* aran);
bool IsFlameWreathActive(Player* bot);

// Netherspite
extern std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
extern std::unordered_map<uint32, ObjectGuid> currentRedBlocker;
extern std::unordered_map<uint32, ObjectGuid> currentGreenBlocker;
extern std::unordered_map<uint32, ObjectGuid> currentBlueBlocker;
bool IsBanishPhase(Unit* netherspite);
std::vector<Player*> GetRedBlockers(Player* bot);
std::vector<Player*> GetBlueBlockers(Player* bot);
std::vector<Player*> GetGreenBlockers(Player* bot);
std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot);
std::vector<Unit*> GetAllVoidZones(Player* bot);
bool FindBeamPosition(
    Unit* netherspite, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos);

// Prince Malchezaar
std::vector<Unit*> GetSpawnedInfernals(Player* bot);
bool IsStraightPathSafe(
    float startX, float startY, float targetX, float targetY,
    std::vector<Unit*> const& hazards, float hazardRadius);
bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY);

// Nightbane
inline constexpr float NIGHTBANE_FLIGHT_Z = 95.000f;
inline constexpr float NIGHTBANE_GROUND_Z = 91.473f;
inline Position const TERRACE_DOME_CENTER = { -11126.015f, -1925.271f, 91.473f };
inline Position const TERRACE_EAST_END    = { -11115.958f, -1972.058f, 91.457f };
inline Position const TERRACE_WEST_END    = { -11077.521f, -1913.315f, 91.471f };
inline std::array const NIGHTBANE_FLIGHT_STACK_POSITIONS = {
    Position{ -11156.233f, -1888.353f, 91.473f },  // primary
    Position{ -11149.115f, -1897.154f, 91.473f },  // backup in case of charred earth
};
inline std::array const NIGHTBANE_RAIN_OF_BONES_POSITIONS = {
    Position{ -11166.516f, -1901.405f, 91.473f },  // primary
    Position{ -11158.752f, -1909.394f, 91.473f },  // backup in case of charred earth
};
inline Position const NIGHTBANE_TELEPORT_POSITION = { -11159.555f, -1893.526f, 91.473f };
extern std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
extern std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

}

#endif
