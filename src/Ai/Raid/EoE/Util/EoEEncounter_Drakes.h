/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEENCOUNTER_DRAKES_H
#define PLAYERBOTS_EOEENCOUNTER_DRAKES_H

#include "EoEData.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Unit.h"

#include <vector>

void GetStaticFields(Player* bot, std::vector<Unit*>& fields);

bool IsClearOfStaticFields(float x, float y, std::vector<Unit*> const& fields, float safeRadius);

// Where the P3 flight parks. The heading is latched per instance and only ever advances.
bool GetDrakeStackPoint(Player* bot, std::vector<Unit*> const& fields, float& x, float& y, float& z);

// The next waypoint towards a stack point: a hop around the boss rather than a chord across him,
// forward around the ring once the drake is on it.
void GetDrakeApproachPoint(Unit* drake, Unit* boss, float destX, float destY, float& x, float& y);

// The drakes assigned to heal: guid-sorted, so every bot derives the same roster without talking.
void GetDrakeHealerGuids(PlayerbotAI* botAI, std::vector<ObjectGuid>& out);

bool IsDrakeHealer(PlayerbotAI* botAI, std::vector<ObjectGuid> const& healers);

// Every Skytalon the raid is flying, plus this bot's place in the healer queue: energy descending,
// guid ascending, so every bot derives the same order. Both come off one walk of the roster.
void GetDrakeFlightAndHealerRank(PlayerbotAI* botAI, std::vector<ObjectGuid> const& healers,
    std::vector<Unit*>& drakes, uint8& rank);

// Power check for a drake self-cast: CanCastVehicleSpell reports BAD_TARGETS there, and
// CastVehicleSpell reports success even when the cast it prepared was rejected.
bool DrakeCanAfford(Unit* drake, uint32 spellId);

bool DrakeCanAffordWithShield(Unit* drake, uint32 spellId);

// Reads the Life Burst buff as ground truth for who burst when, and how long ago.
uint32 DrakeAuraRemainingMs(Unit* drake, uint32 spellId);

bool IsDrakeSurgeTarget(PlayerbotAI* botAI);

// Milliseconds since the boss picked this drake, false when it isn't picked. The slots stay set for
// SURGE_CYCLE_MS but the beam only lands in [SURGE_BEAM_DELAY_MS, SURGE_BEAM_END_MS].
bool GetDrakeSurgeElapsedMs(PlayerbotAI* botAI, uint32& elapsedMs);

#endif
