/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERAURIAYA_H
#define PLAYERBOTS_ULDENCOUNTERAURIAYA_H

#include "Position.h"
#include "UldData.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;
class WorldObject;

enum UlduarAuriayaIds
{
    NPC_AURIAYA_SANCTUM_SENTRY = 34014,
    NPC_AURIAYA_FERAL_DEFENDER = 34035,
    NPC_AURIAYA_SEEPING_FERAL_ESSENCE = 34098,
};

inline constexpr float ULDUAR_AURIAYA_AXIS_Z_PATHING_ISSUE_DETECT = 410.0f;

// 64459's 5 yd radius (index 8, both sizes) plus 2 yd for the 1s tick and position lag.
inline constexpr float ULDUAR_AURIAYA_SEEPING_ESSENCE_RADIUS = 7.0f;

// Sonic Screech splits its damage across a 120 degree cone, so the raid soaks it instead of dodging.
// The stack sits this far out on her bearing to the tank, since she faces her victim.
inline constexpr float ULDUAR_AURIAYA_RAID_STANDOFF = 20.0f;
// Snapped so tank drift can't shuffle the stack. One step is 3.9 yd of arc at the standoff,
// within the ranged and healer tolerances.
inline constexpr float ULDUAR_AURIAYA_BEARING_QUANTUM = static_cast<float>(M_PI) / 16.0f;
inline constexpr float ULDUAR_AURIAYA_MAINTANK_SPOT_TOLERANCE = 3.0f;
inline constexpr float ULDUAR_AURIAYA_RANGED_SPOT_TOLERANCE = 5.0f;
inline constexpr float ULDUAR_AURIAYA_HEALER_SPOT_TOLERANCE = 8.0f;

// Pools never despawn before she dies, so the fight steps west through fixed stations as they
// pile up. A fourth station would put the stack past x 1908, off the floor.
inline constexpr int ULDUAR_AURIAYA_STATION_COUNT = 3;
// Raid footprint around a station point: 8 yd healer tolerance plus the 7 yd pool radius.
inline constexpr float ULDUAR_AURIAYA_STATION_FOUL_RADIUS = 15.0f;
inline constexpr float ULDUAR_AURIAYA_ROOM_SEARCH_RADIUS = 100.0f;

// How far a pool dodge may take a bot from its anchor, or from the boss for melee.
inline constexpr float ULDUAR_AURIAYA_ESSENCE_LEASH = 12.0f;
// Melee only take the Defender this close to her. It re-rolls aggro, so they'd chase it all fight.
inline constexpr float ULDUAR_AURIAYA_MELEE_DEFENDER_RANGE = 15.0f;

// One per station. Raid points only score a station, the stack's real anchor comes off the live boss.
extern const Position ULDUAR_AURIAYA_MAINTANK_SPOTS[];
extern const Position ULDUAR_AURIAYA_NOMINAL_RAID_POINTS[];

// Terrifying Screech fears every 35s from the pull, so the whole fight is the fear window.
bool AuriayaFearWindowActive(PlayerbotAI* botAI);

// By entry, not "find target": that only walks the bot's threat list, so a bot on an add loses her.
Unit* GetAuriaya(PlayerbotAI* botAI);
bool AuriayaEncounterActive(PlayerbotAI* botAI);

// Gate for anything that pulls. A Sentry's JustEngagedWith calls SetInCombatWithZone on her, so
// her combat flag covers however the raid opens.
bool IsAuriayaEngaged(PlayerbotAI* botAI);

// Sentries first: they stay dead and stop buffing her, the Defender only feigns and comes back.
Unit* GetAuriayaFocusTarget(PlayerbotAI* botAI);

// A Sentry not already on the off-tank. Two spawn, so "first alive sentry" leaves the second loose.
Unit* GetAuriayaLooseSentry(PlayerbotAI* botAI, Player* tank);

// Grid search, not "possible targets": the stalkers aren't selectable. Runs per bot per tick on the
// dodge path, so pass the smallest radius that answers the question.
std::vector<Unit*> CollectAuriayaEssencePools(WorldObject* from, float radius);

// Every pool in the room, scanned off the boss once per bot per tick. Empty while she's out of
// sight. Only good for the current tick.
std::vector<Unit*> GetAuriayaRoomPools(PlayerbotAI* botAI);

// Fewest pools near either spot wins, ties to the lowest index. Only the main tank reads this, the
// rest follow her. Never score off her live position: she follows the tank, so it'd oscillate.
int GetAuriayaStationIndex(std::vector<Unit*> const& roomPools);

// Main tank gets the station spot, ranged and healers a point off the live boss and her victim,
// melee and the off-tank get false. Trigger and action both call this so they can't disagree.
bool GetAuriayaAnchor(PlayerbotAI* botAI, Player* bot, Position& out, float& tolerance);

// Same, with the boss in hand. Only the main tank reads roomPools, null falls back to GetAuriayaRoomPools.
bool GetAuriayaAnchor(PlayerbotAI* botAI, Player* bot, Unit* boss, std::vector<Unit*> const* roomPools,
                      Position& out, float& tolerance);

#endif
