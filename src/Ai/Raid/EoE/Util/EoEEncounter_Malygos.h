/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEENCOUNTER_MALYGOS_H
#define PLAYERBOTS_EOEENCOUNTER_MALYGOS_H

#include "EoEData.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Unit.h"

#include <limits>
#include <utility>
#include <vector>

// One creature cache per instance, shared by every bot. Stores guids, so a despawn can't dangle.
void GetEoECreatures(Player* bot, uint32 entry, std::vector<Unit*>& out);
Unit* GetNearestEoECreature(Player* bot, uint32 entry, float maxDist = std::numeric_limits<float>::max());
bool AnyEoECreature(Player* bot, uint32 entry);

// Finds Malygos even while he is flagged non-attackable (P2 flight / phase transitions).
Unit* GetMalygos(Player* bot);

enum class MalygosPhase : uint8
{
    None,        // off the map, or not pulled
    P1,
    P2,          // Nexus Lords and Scions up
    P3,          // this bot is on a Skytalon
    Transition,  // between phases, and the pull intro
};

MalygosPhase GetMalygosPhase(Player* bot);

struct MalygosAggro
{
    bool mainTank = false;
    bool victim = false;        // he is hitting this bot
    bool victimIsTank = false;  // he is hitting a tank, this bot or another
};

// IsMainTank walks the group and P1 asks for this per queued action, so it's cached for the tick.
MalygosAggro const& GetMalygosAggro(PlayerbotAI* botAI);

struct MalygosP1Layout
{
    std::pair<float, float> tank;
    std::pair<float, float> stack;
    std::pair<float, float> hunter;
    std::pair<float, float> grip;
};

// The P1 offsets rotated onto the landing bearing, latched per instance for the whole pull.
MalygosP1Layout const& GetMalygosP1Layout(Player* bot);

// A killed spark stays IsAlive() for 60 s as an unselectable corpse while its ground buff ticks.
// Anything that looks for sparks has to go through these or it ends up on the corpse.
void GetLivePowerSparks(Player* bot, std::vector<Unit*>& out);
bool AnyLivePowerSpark(Player* bot);

// Anchored on a point rather than on the bot: the grip spot is what the answer is wanted for, and
// the spark nearest the DK is regularly not the spark nearest where he parks.
Unit* GetNearestPowerSparkTo(PlayerbotAI* botAI, float x, float y);

// The spark this bot should hit: of the ones it can reach standing still, the one nearest Malygos.
// currentTarget keeps it from swapping off a spark that has drifted just past reach.
Unit* GetPowerSparkToKill(PlayerbotAI* botAI, Unit* currentTarget);

// The spark a DK should chain: pulled in close and not snared yet.
Unit* GetPowerSparkToSnare(PlayerbotAI* botAI);

// Read by both the position action and the grip, so they cannot disagree about where the DK is.
bool IsOnPowerSparkGripDuty(PlayerbotAI* botAI);

float GetBubbleShrinkFactor(Unit* bubble);

bool IsSafelySheltered(Player* bot);

// Landed, selectable and free doubles as "its Nexus Lord is dead".
Unit* FindFreeHoverDisk(Player* bot);

// Disk duty is melee dps only. Everything that reasons about disks has to agree on this.
bool IsEligibleDiskRider(Player* bot);

bool AnyScionAlive(Player* bot);

#endif
