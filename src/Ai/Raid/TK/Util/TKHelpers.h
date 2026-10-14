/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKHELPERS_H
#define PLAYERBOTS_TKHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

class Creature;
class Item;
class Player;
class PlayerbotAI;
class Unit;

namespace TkHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class TkSpells : uint32
{
    // Crimson Hand Centurion
    SPELL_ARCANE_FLURRY             = 37268,

    // Al'ar
    SPELL_MODEL_INVISIBILITY        = 24401, // "Test Pet Passive" (AC hack? used for Al'ar)
    SPELL_REBIRTH_PHASE2            = 34342,
    SPELL_REBIRTH_DIVE              = 35369,
    SPELL_MELT_ARMOR                = 35410,

    // Void Reaver
    SPELL_ARCANE_ORB                = 34172,

    // High Astromancer Solarian
    SPELL_SOLARIAN_TRANSFORM        = 39117,
    SPELL_WRATH_OF_THE_ASTROMANCER  = 42783,

    // Kael'thas Sunstrider
    SPELL_PERMANENT_FEIGN_DEATH     = 29266,
    SPELL_GRAVITY_LAPSE_AURA        = 39432, // AC uses this name (SPELL_GRAVITY_LAPSE is 35941)
    SPELL_MENTAL_PROTECTION_FIELD   = 36480, // Staff of Disintegration
    SPELL_ARCANE_BARRIER            = 36481, // Phaseshift Bulwark
    SPELL_KAELTHAS_MIND_CONTROL     = 36797,
    SPELL_SHOCK_BARRIER             = 36815,

    // Hunter
    SPELL_MISDIRECTION              = 35079,
};

enum class TkNpcs : uint32
{
    // Trash
    NPC_CRIMSON_HAND_CENTURION      = 20048,

    // Al'ar
    NPC_EMBER_OF_ALAR               = 19551,
    NPC_FLAME_PATCH                 = 20602,

    // High Astromancer Solarian
    NPC_SOLARIUM_PRIEST             = 18806,

    // Kael'thas Sunstrider
    NPC_KAELTHAS_SUNSTRIDER         = 19622,
    NPC_NETHERSTRAND_LONGBOW        = 21268,
    NPC_DEVASTATION                 = 21269,
    NPC_COSMIC_INFUSER              = 21270,
    NPC_INFINITY_BLADES             = 21271,
    NPC_WARP_SLICER                 = 21272,
    NPC_PHASESHIFT_BULWARK          = 21273,
    NPC_STAFF_OF_DISINTEGRATION     = 21274,
    // NPC_NETHER_VAPOR             = 21002, // Unimplemented in AC; method needed if fixed
    NPC_PHOENIX                     = 21362,
    NPC_PHOENIX_EGG                 = 21364,
    NPC_FLAME_STRIKE_TRIGGER        = 21369,
};

enum class TkItems : uint32
{
    // Kael'thas Sunstrider
    ITEM_WARP_SLICER                = 30311,
    ITEM_INFINITY_BLADE             = 30312,
    ITEM_STAFF_OF_DISINTEGRATION    = 30313,
    ITEM_PHASESHIFT_BULWARK         = 30314,
    // ITEM_DRAENETHYST_MINE_CRYSTAL= 30315, // Noting that the ids are not all contiguous
    ITEM_DEVASTATION                = 30316,
    ITEM_COSMIC_INFUSER             = 30317,
    ITEM_NETHERSTRAND_LONGBOW       = 30318,
    ITEM_NETHER_SPIKES              = 30319,
};

// General

inline constexpr uint32 TK_MAP_ID = 550;

std::pair<Unit*, Unit*> GetTargetUnitPair(PlayerbotAI* botAI, uint32 entry);
Player* GetNearestNonTankPlayerInRadius(Player* bot, float radius);

// Trash

Unit* GetCenturionCastingArcaneFlurry(PlayerbotAI* botAI);

// Al'ar <Phoenix God>
// CombatReach is 15 yards

enum AlarLocationIndex
{
    PLATFORM_0_IDX, // West
    PLATFORM_1_IDX, // Northwest
    PLATFORM_2_IDX, // Northeast
    PLATFORM_3_IDX, // East
    POINT_QUILL_OR_DIVE_IDX,
    POINT_MIDDLE_IDX,
    LOCATION_NONE = -1
};

// 17.0f is intentionally a little lower than the actual balcony positions to capture melee that
// may be slightly down the ramps from Platform 0 or 3.
inline constexpr float ALAR_BALCONY_Z = 17.0f;
inline constexpr uint8 TOTAL_ALAR_LOCATIONS = 6;

inline Position const ALAR_LANDING_PLATFORM_0 = { 335.638f,  59.4879f, 17.9319f }; // West
inline Position const ALAR_LANDING_PLATFORM_1 = { 388.751f,  31.7312f, 20.2636f }; // Northwest
inline Position const ALAR_LANDING_PLATFORM_2 = { 388.791f, -33.1059f, 20.2636f }; // Northeast
inline Position const ALAR_LANDING_PLATFORM_3 = { 332.723f, -61.1590f, 17.9791f }; // East
inline std::array const ALAR_LANDING_PLATFORM_POSITIONS = {
    ALAR_LANDING_PLATFORM_0,
    ALAR_LANDING_PLATFORM_1,
    ALAR_LANDING_PLATFORM_2,
    ALAR_LANDING_PLATFORM_3,
};

inline Position const ALAR_TANK_PLATFORM_0 = { 329.433f,  59.914f, 17.548f };
inline Position const ALAR_TANK_PLATFORM_1 = { 395.772f,  26.779f, 20.181f };
inline Position const ALAR_TANK_PLATFORM_2 = { 387.498f, -41.909f, 20.182f };
inline Position const ALAR_TANK_PLATFORM_3 = { 330.342f, -61.902f, 17.719f };
inline std::array const ALAR_TANK_PLATFORM_POSITIONS = {
    ALAR_TANK_PLATFORM_0,
    ALAR_TANK_PLATFORM_1,
    ALAR_TANK_PLATFORM_2,
    ALAR_TANK_PLATFORM_3,
};

inline Position const ALAR_MELEE_DPS_PLATFORM_0 = { 344.037f,  64.339f, 18.350f };
inline Position const ALAR_MELEE_DPS_PLATFORM_1 = { 387.558f,  40.300f, 20.182f };
inline Position const ALAR_MELEE_DPS_PLATFORM_2 = { 395.275f, -28.573f, 20.182f };
inline Position const ALAR_MELEE_DPS_PLATFORM_3 = { 346.427f, -69.361f, 18.750f };
inline std::array const ALAR_MELEE_DPS_PLATFORM_POSITIONS = {
    ALAR_MELEE_DPS_PLATFORM_0,
    ALAR_MELEE_DPS_PLATFORM_1,
    ALAR_MELEE_DPS_PLATFORM_2,
    ALAR_MELEE_DPS_PLATFORM_3,
};

inline std::array const ALAR_GROUND_POSITIONS = {
    Position{ 336.439f,  48.181f, -2.389f }, // Underish West platform
    Position{ 379.122f,  25.146f, -2.385f }, // Underish Northwest platform
    Position{ 378.583f, -27.481f, -2.385f }, // Underish Northeast platform
    Position{ 331.631f, -49.716f, -2.389f }, // Underish East platform
};

inline Position const ALAR_ROOM_CENTER         = { 330.611f,  -2.540f, -2.389f };
inline Position const ALAR_POINT_QUILL_OR_DIVE = { 332.000f,   0.010f, 43.000f };
inline Position const ALAR_POINT_MIDDLE        = { 331.000f,   0.010f, -2.380f };
inline Position const ALAR_SE_RAMP_BASE        = { 281.064f, -36.590f, -2.389f };
inline Position const ALAR_SW_RAMP_BASE        = { 281.064f,  36.590f, -2.389f };
inline Position const ALAR_ROOM_S_CENTER       = { 281.064f,   0.000f, -2.389f };

inline constexpr float ALAR_FLAME_PATCH_SEARCH_DISTANCE = 40.0f;

extern std::unordered_map<uint32, bool> lastRebirthState;
extern std::unordered_map<uint32, bool> isAlarInPhase2;

bool IsAlarInPhase2(uint32 instanceId);
int8 GetAlarDestinationLocationIndex(Unit* alar);
int8 GetAlarCurrentLocationIndex(Unit* alar);
int8 GetAlarPlatformIndex(Unit* alar);
Position const& GetClosestGroundPosition(Position const& botPos);
bool IsPrimaryEmberTank(Player* bot);
bool IsFirstAlarTank(Player* bot);
bool IsSecondAlarTank(Player* bot);
Player* GetSecondaryEmberTank(Player* bot);
GuidVector FindFlamePatchGuids(Player* bot);
std::vector<Unit*> GetFlamePatches(PlayerbotAI* botAI);

// Void Reaver
// CombatReach is 15 yards

struct ArcaneOrbData
{
    Position destination;
    uint32 castTime;
};

inline constexpr uint32 ARCANE_ORB_DURATION_MS = 7000;
// Actual hazard range is 20 yards; the rest is buffer for the escape & multiplier.
inline constexpr float ARCANE_ORB_SAFE_DISTANCE = 22.0f;
inline constexpr float ARCANE_ORB_BUFFER_DISTANCE = 30.0f;

// Center of the room
inline Position const VOID_REAVER_TANK_POSITION = { 423.845f, 371.733f, 14.897f };

extern std::unordered_map<uint32, std::vector<ArcaneOrbData>> voidReaverArcaneOrbs;

std::vector<Position> GetActiveArcaneOrbs(uint32 instanceId);
bool IsNearArcaneOrb(Player* bot, std::vector<Position> const& orbs, float radius);
bool IsNearActiveArcaneOrb(Player* bot, float radius);

// High Astromancer Solarian

bool HasWrathOfTheAstromancer(Player* bot);

// Kael'thas Sunstrider <Lord of the Blood Elves>

// Matches the phase enum of the core's boss_kaelthas.cpp. Reading the phase means casting the
// boss AI to a mirror of that class (TKKaelthasBossAI.h).
enum KTPhases
{
    PHASE_NONE           = 0,
    PHASE_SINGLE_ADVISOR = 1,
    PHASE_WEAPONS        = 2,
    PHASE_TRANSITION     = 3,
    PHASE_ALL_ADVISORS   = 4,
    PHASE_FINAL          = 5
};

// About the exact distance from Kael to the entrances to his room.
inline constexpr float KAELTHAS_ROOM_SEARCH_DISTANCE = 125.0f;
inline constexpr uint32 KAELTHAS_DB_GUID = 158218;

// To the left of the advisors' starting position (when facing them), up against the wall.
inline Position const SANGUINAR_TANK_POSITION    = { 775.478f,  39.888f, 46.780f };
inline Position const SANGUINAR_WAITING_POSITION = { 761.850f,  27.459f, 46.779f };
// Next to Sanguinar
inline Position const TELONICUS_TANK_POSITION    = { 773.717f,  44.091f, 46.780f };
inline Position const TELONICUS_WAITING_POSITION = { 754.347f,  31.739f, 46.796f };
inline Position const CAPERNIAN_WAITING_POSITION = { 743.897f, -11.575f, 46.779f };
// A bit aways from the Sanguinar and Telonicus tanking positions.
inline Position const ADVISOR_HEAL_POSITION      = { 752.171f,  19.494f, 46.779f };
// Towards the front-center of the platform to ensure an open area for Gravity Lapse.
inline Position const KAELTHAS_TANK_POSITION     = { 774.008f,  -0.631f, 48.729f };

inline constexpr uint32 ADVISOR_DPS_WAIT_NOT_STARTED = 0;
extern std::unordered_map<uint32, uint32> advisorDpsWaitTimer;

uint32 GetKaelthasTkPhase(Unit* kaelthas);
bool IsAdvisorActive(Unit* advisor);
Player* GetCapernianTank(Player* bot);
bool IsCapernianTank(Player* bot);
bool IsSanguinarDebuffHunter(Player* bot);
GuidVector FindLegendaryWeaponGuids(Player* bot);
Unit* GetLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry);
Creature* GetDeadLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry);
bool HasDeadLegendaryWeapon(PlayerbotAI* botAI);
bool IsLegendaryWeaponItem(uint32 itemId);
bool HasEquippableItemForSlot(Player* bot, uint8 slot);
Item* GetEquippedItemInSlot(Player* bot, uint8 slot, uint32 itemId);
Creature* GetNearestFlameStrikeInRadius(Player* bot, float radius);
Creature* GetPhoenixEgg(Player* bot);

}

#endif
