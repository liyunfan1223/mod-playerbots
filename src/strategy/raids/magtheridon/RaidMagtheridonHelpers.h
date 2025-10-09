#ifndef _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONHELPERS_H

#include <ctime>
#include <unordered_map>
#include <vector>

#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

enum class MagtheridonSpells
{
    // Magtheridon
    SHADOW_CAGE        = 30205,
    BLAST_NOVA         = 30616,
    SHADOW_GRASP      = 30410,

    // Warlock
    BANISH             = 18647,
    CURSE_OF_TONGUES   = 11719,
    FEAR               =  6215,

    // Hunter
    MISDIRECTION  = 34477,
};

enum class MagtheridonNPCs
{
    BURNING_ABYSSAL      = 17454,
    HELLFIRE_CHANNELER   = 17256,
    TARGET_TRIGGER       = 17474,
};

namespace MagtheridonHelpers
{

constexpr uint32 SOUTH_CHANNELER     = 90978;
constexpr uint32 WEST_CHANNELER      = 90979;
constexpr uint32 NORTHWEST_CHANNELER = 90980;
constexpr uint32 EAST_CHANNELER      = 90982;
constexpr uint32 NORTHEAST_CHANNELER = 90981;

inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;
inline constexpr int8 crossIcon = RtiTargetValue::crossIndex;

Creature* GetChanneler(Player* bot, uint32 dbGuid);
void UpdateTransitionTimer(Unit* unit, bool transitionCondition, std::unordered_map<uint32, bool>& lastStateMap, 
                           std::unordered_map<uint32, time_t>& timerMap);
bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z);

struct TankSpot
{
    float x, y, z, orientation;
};

namespace MagtheridonTankSpots 
{
    extern const TankSpot WaitingForMagtheridon;
    extern const TankSpot Magtheridon;
    extern const TankSpot NWChanneler;
    extern const TankSpot NEChanneler;
}

struct CubeInfo
{
    ObjectGuid guid;
    float x, y, z;
};

extern const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS;
extern std::unordered_map<ObjectGuid, CubeInfo> botToCubeAssignment;
std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, const std::vector<uint32>& cubeDbGuids);
void AssignBotsToCubesByGuidAndCoords(Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI);
extern std::unordered_map<uint32, bool> lastShadowCageState;
extern std::unordered_map<uint32, bool> lastBlastNovaState;

}

#endif
