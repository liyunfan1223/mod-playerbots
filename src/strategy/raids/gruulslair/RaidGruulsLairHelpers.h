#ifndef RAID_GRUULSLAIR_HELPERS_H
#define RAID_GRUULSLAIR_HELPERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

enum GruulsLairSpells
{
	// High King Maulgar
	SPELL_WHIRLWIND 		 = 33238,

	// Krosh Firehand
	SPELL_AURA_SPELL_SHIELD  = 33054,

	// Hunter
	SPELL_AURA_MISDIRECTION  = 34477,

	// Mage
	SPELL_SPELLSTEAL 		 = 30449,

	// Warlock
	SPELL_BANISH     		 = 18647, // Rank 2

	// Gruul the Dragonkiller
	SPELL_GROUND_SLAM_1 	 = 33525,
	SPELL_GROUND_SLAM_2 	 = 39187,
};

// Define constants for RTI indexes
inline constexpr int8 squareIcon = RtiTargetValue::squareIndex;
inline constexpr int8 starIcon = RtiTargetValue::starIndex;
inline constexpr int8 circleIcon = RtiTargetValue::circleIndex;
inline constexpr int8 diamondIcon = RtiTargetValue::diamondIndex;
inline constexpr int8 triangleIcon = RtiTargetValue::triangleIndex;

bool IsAnyOgreBossAlive(PlayerbotAI* botAI);
bool IsMaulgarTank(PlayerbotAI* botAI, Player* bot);
bool IsOlmTank(PlayerbotAI* botAI, Player* bot);
bool IsBlindeyeTank(PlayerbotAI* botAI, Player* bot);
bool IsMageTank(PlayerbotAI* botAI, Player* bot);
bool IsMoonkinTank(PlayerbotAI* botAI, Player* bot);
bool IsPositionSafe(PlayerbotAI* botAI, Unit* bot, Position pos);
Position FindSafePosition(PlayerbotAI* botAI, Unit* bot, Unit* target, float optimalDistance);

struct TankSpot 
{
	float x, y, z;
	float orientation;
};

namespace GruulsLairTankSpots 
{
    extern const TankSpot Maulgar;
    extern const TankSpot Olm;
    extern const TankSpot Blindeye;
    extern const TankSpot Gruul;
}

#endif
