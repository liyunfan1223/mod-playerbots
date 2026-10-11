/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALHELPERS_H
#define PLAYERBOTS_HYJALHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace HyjalHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class HyjalSpells : uint32
{
    // Rage Winterchill
    SPELL_DEATH_AND_DECAY     = 31258,

    // Anetheron
    SPELL_INFERNO             = 31299,

    // Kaz'rogal
    SPELL_MARK_OF_KAZROGAL    = 31447,

    // Azgalor
    SPELL_RAIN_OF_FIRE        = 31340,
    SPELL_DOOM                = 31347,

    // Archimonde
    SPELL_DOOMFIRE_TRAIL      = 31943, // 6y persistent area aura dropped along the trail, lasts 18s
    SPELL_DOOMFIRE            = 31944, // Damaging part of trail
    SPELL_DOOMFIRE_DOT        = 31969, // DoT after exiting trail
    SPELL_AIR_BURST           = 32014,
    SPELL_PROTECTION_OF_ELUNE = 38528,

    // Hunter
    SPELL_ASPECT_OF_THE_VIPER = 34074,
    SPELL_MISDIRECTION        = 35079,

    // Shaman
    SPELL_TREMOR_TOTEM        = 8143,
};

enum class HyjalNpcs : uint32
{
    // Anetheron
    NPC_TOWERING_INFERNAL = 17818,

    // Archimonde
    NPC_DOOMFIRE          = 18095,
};

// A span of headings around a ring that is unavailable.
struct BlockedArc
{
    float center;
    float halfWidth;
};

// General

inline constexpr uint32 HYJAL_MAP_ID = 534;
// The interval matches the default Playerbots.ReactDelay.
inline constexpr uint32 HAZARD_CACHE_INTERVAL = 100;
// The additional distance beyond a hazard's edge in which movement is still suppressed by a
// multiplier relating to such hazard. Used for Winterchill D&D and Azgalor RoF.
inline constexpr float HAZARD_CONTROL_MARGIN = 5.0f;
// The additional distance beyond the hazard's edge and control margin through which the search
// for the hazard continues. Used for Winterchill D&D, Azgalor RoF, and Archimonde DF.
inline constexpr float HAZARD_SEARCH_MARGIN = 2.0f;
// This distance is subtracted from the edge of melee range so rounding and drift cannot leave a
// bot just out of reach.
inline constexpr float MELEE_RANGE_INSET = 1.0f;

bool GetMeleeHazardManeuverStep(
    Player* bot, Unit* boss, std::vector<Position> const& hazards, float hazardRadius,
    std::vector<BlockedArc> const& extraBlocked, float& stepX, float& stepY, float& stepZ,
    std::function<bool(float, float)> const& isAcceptable = {});
std::vector<Player*> GetRangedMembers(Player* bot);
bool GetRangedRingStep(
    Player* bot, Position const& center, float healerRadius, float dpsRadius, float& stepX,
    float& stepY, float& stepZ, bool& reached);

// Rage Winterchill

// 20y radius + 1.95y max player CombatReach (1.5y increased by 30% during Bloodlust). Range should
// not be increased by CombatReach, but the range of persistent ground-based AoEs is bugged in AC.
inline constexpr float DEATH_AND_DECAY_RADIUS = 22.0f;
// Out to this distance, movement is controlled only by the avoidance action.
inline constexpr float DEATH_AND_DECAY_CONTROL_RADIUS =
    DEATH_AND_DECAY_RADIUS + HAZARD_CONTROL_MARGIN;
inline constexpr float DEATH_AND_DECAY_SEARCH_RADIUS =
    DEATH_AND_DECAY_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;

// Back towards the center of the base
inline Position const WINTERCHILL_TANK_POSITION = { 5031.061f, -1784.521f, 1321.626f };

bool GetDeathAndDecayPosition(PlayerbotAI* botAI, Position& deathAndDecay);
bool IsNearDeathAndDecay(PlayerbotAI* botAI, float radius);
bool IsInDeathAndDecay(PlayerbotAI* botAI);

// Anetheron
// Towering Infernal's CombatReach is 4y

inline constexpr float INFERNAL_SEARCH_RADIUS = 100.0f;
// Longer than the hazard interval because this is a creature grid search.
inline constexpr uint32 INFERNAL_CACHE_INTERVAL = 200;
// A landing Infernal stuns everybody within 10y for 2s (31302) and then burns everything within
// 10y of itself for as long as it lives with Immolation (31304 triggering 31303).
inline constexpr float INFERNAL_DANGER_RADIUS = 10.0f;
inline constexpr float INFERNAL_ESCAPE_DISTANCE = INFERNAL_DANGER_RADIUS + 2.0f;
// Past this, ranged stay on the boss rather than switching to the Infernal. Arbitrary, but near
// enough that ranged do not bunch up and risk too many getting hit by a Carrion Swarm.
inline constexpr float INFERNAL_RANGED_ENGAGE_DISTANCE = 50.0f;

// Back towards the center of the base, near the crossroads
inline Position const ANETHERON_TANK_POSITION       = { 5033.177f, -1765.996f, 1324.195f };
inline Position const ANETHERON_E_INFERNAL_POSITION = { 5016.578f, -1800.233f, 1323.070f };
inline Position const ANETHERON_W_INFERNAL_POSITION = { 5048.911f, -1722.164f, 1321.408f };
// Whichever of the two spots the Infernal tank stands nearer.
Position const& GetInfernalTankPosition(Player* bot);

Player* GetInfernoTarget(Unit* anetheron);
// Every living Towering Infernal, oldest first, read through the "hyjal infernals" value.
GuidVector FindInfernalGuids(Player* bot);
GuidVector const& GetInfernalGuids(PlayerbotAI* botAI);
// The first Infernal that the Infernal tank does not have aggro on.
Unit* GetLooseInfernal(PlayerbotAI* botAI);
Unit* GetNearestInfernal(PlayerbotAI* botAI);
// The Infernal a ranged bot should attack instead of the boss, if any. It is the oldest Infernal
// alive within engage range, but in practice, a raid should have only one up at a time.
Unit* GetInfernalToAttack(PlayerbotAI* botAI, Unit* anetheron);
Unit* GetInfernalTargetingBot(PlayerbotAI* botAI);
// Both resolve the first assist tank among the living, so keep them in step.
bool IsInfernalTank(Player* bot);
Player* GetInfernalTank(Player* bot);

// Kaz'rogal
// CombatReach is 7.875 yards

// War Stomp is 12y + up to 1.95y CombatReach.
inline constexpr float KAZROGAL_RANGED_ARC_RADIUS = 15.0f;
// For keeping extra distance during the pull to avoid War Stomp.
inline constexpr float KAZROGAL_RANGED_ARC_APPROACH_RADIUS = 25.0f;
// The heading from Kaz'rogal for the center of the ranged arc, measured ingame.
inline constexpr float KAZROGAL_RANGED_ARC_CENTER = 4.225f;
// This is about the maximum width that allows reasonable escape paths due to obstacles.
inline constexpr float KAZROGAL_RANGED_ARC_HALF_WIDTH = 10.0f;
// Mark of Kaz'rogal (31447) drains 600 mana a tick, five 1s ticks for 3000 in all. It detonates on
// the first tick where the victim has less than 600 mana.
inline constexpr uint32 MARK_TICK_DRAIN = 600;
inline constexpr uint32 MARK_FULL_DRAIN = 3000;
// Mana levels for running away from the group and coming back to it, respectively. Also the mana
// levels for forcing Hunters to use Aspect of the Viper and allowing other Aspects, respectively.
inline constexpr uint32 MARK_DANGER_MANA = 3200;
inline constexpr uint32 MARK_REJOIN_MANA = 4000;
// The standard Warlock strategy casts Life Tap at 85% mana but with very low priority. It is
// increased to emergency+ priority at Playerbots.LowMana (default 20%). Against Kaz'rogal,
// Life Tap is instead forced to emergency+ priority at the below level, and the requirement for
// a minimum health threshold (which applies to the class strategies) is removed.
inline constexpr uint32 MARK_LIFE_TAP_MANA = 5000;
// The radius of the Mark of Kaz'rogal explosion (31463) is 15y, with a 1y buffer added.
inline constexpr float MARK_ESCAPE_DISTANCE = 16.0f;

// Near the gate, so the raid can get started immediately to beat the soft enrage due to Marks.
inline Position const KAZROGAL_TANK_POSITION = { 5505.440f, -2665.059f, 1480.598f };

float GetKazrogalRangedArcRadius(Unit* kazrogal);
float GetKazrogalRangedArcSpan(float radius);
bool IsKazrogalManaUser(PlayerbotAI* botAI);
bool HasMarkOfKazrogal(Player* bot);

// Azgalor
// CombatReach is 8.8 yards
// Doomguard CombatReach is 3.75 yards

// 15y radius + 1.95y max player CombatReach. Like for D&D, the addition of CombatReach to the
// range is due to an AC bug.
inline constexpr float RAIN_OF_FIRE_RADIUS = 17.0f;
// Out to this distance, movement is controlled only by the avoidance action.
inline constexpr float RAIN_OF_FIRE_CONTROL_RADIUS = RAIN_OF_FIRE_RADIUS + HAZARD_CONTROL_MARGIN;
inline constexpr float RAIN_OF_FIRE_SEARCH_RADIUS =
    RAIN_OF_FIRE_CONTROL_RADIUS + HAZARD_SEARCH_MARGIN;
// Cleave chains from Azgalor's victim to up to 4 more players within 12y inside his frontal arc.
inline constexpr float CLEAVE_CHAIN_RADIUS = 12.0f;
inline constexpr float CLEAVE_DANGER_ARC = 200.0f * static_cast<float>(M_PI) / 180.0f;

// Back toward the base, just past the path leading to Thrall's keep.
inline Position const AZGALOR_TANK_POSITION      = { 5494.594f, -2747.069f, 1487.800f };
// On top of Thrall's starting position, in order to get Thrall to aggro as soon as he is hit.
inline Position const AZGALOR_DOOMGUARD_POSITION = { 5452.166f, -2723.282f, 1485.480f };

bool IsSafeFromAzgalorCleave(Unit* azgalor, float x, float y);
// Each Rain of Fire is its own dynamic object that expires after 10s; there can be 2 up at a time.
std::vector<Position> GetRainOfFirePositions(PlayerbotAI* botAI);
bool GetNearestRainOfFirePosition(PlayerbotAI* botAI, Position& pool);
bool IsNearRainOfFire(PlayerbotAI* botAI, float radius);
bool IsInRainOfFire(PlayerbotAI* botAI);
bool IsDoomed(Player* bot);
// The first assist tank picks up Doomguards, unless it has Doom, in which case the second assist
// tank takes over.
bool IsDoomguardTank(Player* bot);
bool AnyGroupMemberHasDoom(Player* bot);

// Archimonde

struct AirBurstData
{
    ObjectGuid targetGuid = ObjectGuid::Empty;
    uint32 castTime = 0;
};

inline constexpr float AIR_BURST_SAFE_DISTANCE = 15.0f;
// The actual damaging radius of Doomfire (31495), which is a 1s periodic that drops a
// Doomfire (31943) pool, which has a persistent area aura of 6y (plus 1.95y max CombatReach).
inline constexpr float DOOMFIRE_BURN_RADIUS = 8.0f;
// How far from Doomfires will avoidance place the bot.
inline constexpr float DOOMFIRE_DANGER_RADIUS = DOOMFIRE_BURN_RADIUS + 2.0f;
// Out to this distance, movement is controlled only by the avoidance action.
inline constexpr float DOOMFIRE_CONTROL_RADIUS = DOOMFIRE_DANGER_RADIUS + 2.0f;
// The maximum distance away from the bot at which a Doomfire affects the escape direction.
inline constexpr float DOOMFIRE_FIELD_RADIUS = 18.0f;
inline constexpr float DOOMFIRE_SEARCH_RADIUS =
    DOOMFIRE_FIELD_RADIUS + DOOMFIRE_DANGER_RADIUS + HAZARD_SEARCH_MARGIN; // 30y
// General spread to prevent clumping due to the risk of Air Burst.
inline constexpr float ARCHIMONDE_RANGED_SPREAD_DISTANCE = 10.0f;

// Up the hill a bit, for space from the World Tree. The tank walks him here at the pull only.
// Archimonde slaps so the tank will pause moving whenever below 60% HP.
inline Position const ARCHIMONDE_INITIAL_POSITION = { 5640.502f, -3421.238f, 1587.453f };

extern std::unordered_map<uint32, AirBurstData> archimondeAirBurstTargets;

// Invincibility applied by Tyrande when Archimonde is at 10% HP. Used to cut off boss strategies
// since the fight is effectively over at this point.
bool HasProtectionOfElune(Player* bot);
bool IsNearDoomfire(PlayerbotAI* botAI, float radius);
bool IsPositionNearDoomfire(PlayerbotAI* botAI, float x, float y, float radius);
// Every live Doomfire trail patch the cache holds, for callers that rely on a scoring system to
// determine where bots should move to.
std::vector<Position> GetDoomfirePositions(PlayerbotAI* botAI);
bool GetPendingAirBurstCast(uint32 instanceId, AirBurstData& airBurst);

}

#endif
