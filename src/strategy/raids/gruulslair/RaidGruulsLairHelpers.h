#ifndef RAID_GRUULSLAIRHELPERS_H
#define RAID_GRUULSLAIRHELPERS_H

#include "PlayerbotAI.h"
#include "RtiTargetValue.h"

enum class GruulsLairSpells
{
	// High King Maulgar
	WHIRLWIND  	   = 33238,

	// Krosh Firehand
	SPELL_SHIELD   = 33054,

	// Hunter
	MISDIRECTION   = 34477,

	// Warlock
	BANISH     	   = 18647, // Rank 2

	// Gruul the Dragonkiller
	GROUND_SLAM_1  = 33525,
	GROUND_SLAM_2  = 39187,
};

namespace GruulsLairHelpers
{

inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;

bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot);
bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot);
bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos);
bool FindSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos);

struct TankSpot 
{
	float x, y, z;
};

namespace GruulsLairTankSpots 
{
    extern const TankSpot Maulgar;
    extern const TankSpot Olm;
    extern const TankSpot Blindeye;
	extern const TankSpot Krosh;
	extern const TankSpot MaulgarRoomCenter;
    extern const TankSpot Gruul;
}

}

#endif
