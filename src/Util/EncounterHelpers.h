/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENCOUNTERHELPERS_H
#define PLAYERBOTS_ENCOUNTERHELPERS_H

#include "Common.h"
#include "Position.h"
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Action;
class Item;
class Player;
class PlayerbotAI;
class Unit;

namespace EncounterHelpers
{

// Cheap, rough proxies for how far along an encounter is. 95% HP means the boss and raid are
// positioned, the tank has threat, and the fight proper has started, so it's time to use cooldowns.
// 10% means the boss is almost dead, so ignore adds and finish off the boss.
inline constexpr float BOSS_ENGAGED_HEALTH_PCT = 95.0f;
inline constexpr float BOSS_BURN_HEALTH_PCT = 10.0f;

bool IsEncounterInProgress(Player* bot, uint32 mapId);
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ);
bool GetStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards);
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId);
bool MarkTargetWithSkull(Player* bot, Unit* target);
bool MarkTargetWithSquare(Player* bot, Unit* target);
bool MarkTargetWithStar(Player* bot, Unit* target);
bool MarkTargetWithCircle(Player* bot, Unit* target);
bool MarkTargetWithDiamond(Player* bot, Unit* target);
bool MarkTargetWithTriangle(Player* bot, Unit* target);
bool MarkTargetWithCross(Player* bot, Unit* target);
bool MarkTargetWithMoon(Player* bot, Unit* target);
bool ClearTargetIcon(Player* bot, uint8 iconId);
bool ClearStrategyTargetIcon(Player* bot, uint8 iconId, std::initializer_list<uint32> entries);
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName);
// Also points "rti target" at the unit, for a fight that keeps the focus on one creature
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName, Unit* target);
bool IsMechanicTrackerBot(Player* bot, uint32 mapId);
Player* GetGroupMainTank(Player* bot);
Player* GetGroupAssistTank(Player* bot, uint8 index);
// Feign death keeps a creature alive at 1 HP, so IsAlive() alone doesn't mean it's still up
bool IsDownOrFeigning(Unit const* unit);
Player* GetNearestPlayerInRadius(Player* bot, float radius);
bool IsBotInFrontalCone(Player* bot, Unit* source, float coneAngle, float range);
// Shortest sidestep out of the cone at the bot's current distance. coneAngle is the full arc width
Position GetPositionOutsideFrontalCone(Player* bot, Unit* source, float coneAngle, float margin = M_PI / 12.0f);
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId);
// One grid visit for several spells, e.g. both ids of a difficulty-mapped one. Results in visit order
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius,
                                                std::initializer_list<uint32> spellIds);

// A hazard and the distance a bot has to keep from it
using HazardCircle = std::pair<Position, float>;

// Collision answers for one sweep grid, so a fallback sweep over the same rings skips the raycasts.
// One per Execute: the answers only hold while the bot stands where it stood.
struct HazardSweepCache
{
    float originX = 0.0f;
    float originY = 0.0f;
    float originZ = 0.0f;
    float distanceStep = 0.0f;
    float angleStep = 0.0f;
    std::unordered_map<uint32, std::pair<bool, Position>> tested;
};

// Nearest spot at least clearRadius from every hazard, or Position() when nothing within maxRadius is.
// Unlike FleePosition it isn't capped at Playerbots.FleeDistance, so it can clear a large blast.
Position FindNearestPositionClearOfHazards(Player* bot, std::vector<Position> const& hazards, float clearRadius,
                                           float maxRadius, float distanceStep = 2.0f,
                                           float angleStep = static_cast<float>(M_PI) / 8.0f,
                                           HazardSweepCache* cache = nullptr);
Position FindNearestPositionClearOfHazards(Player* bot, std::vector<Position> const& hazards, float clearRadius,
                                           float maxRadius, HazardSweepCache* cache);
Position FindNearestPositionClearOfHazards(Player* bot, std::vector<HazardCircle> const& hazards, float maxRadius,
                                           HazardSweepCache* cache);
// preferNear picks the closest spot inside the first ring that has one. accept runs after the collision
// check, which can hand back the bot's own spot at a wall, so a caller that must move checks travel there.
Position FindNearestPositionClearOfHazards(Player* bot, std::vector<HazardCircle> const& hazards, float maxRadius,
                                           float distanceStep = 2.0f,
                                           float angleStep = static_cast<float>(M_PI) / 8.0f,
                                           Position const* preferNear = nullptr,
                                           std::function<bool(float, float)> const& accept = {},
                                           HazardSweepCache* cache = nullptr);
uint32 GetSelfImmunitySpell(Player* bot);
bool IsDpsCooldownAction(Player* bot, Action* action);
bool IsTauntAction(Player* bot, Action* action);
bool IsAoeThreatAction(Player* bot, Action* action);
void CommandPetAttack(PlayerbotAI* botAI, Unit* target);
void StopPet(PlayerbotAI* botAI);

// Snaps a point onto walkable ground and pulls it back to the first collision on the way there
Position ValidateFloorPoint(Player* bot, Position const& point);
// Same without the gameobject pass, for a point on or past a prop: walking bots never collide with one
Position ValidateStaticFloorPoint(Player* bot, Position const& point);
// false for a class without a taunt
bool CastClassTaunt(PlayerbotAI* botAI, Unit* target);
bool ClassTauntReady(PlayerbotAI* botAI);

// Nitro Boosts, the engineering boot tinker
inline constexpr uint32 NITRO_BOOSTS_ENCHANT = 3606;
inline constexpr uint32 NITRO_BOOSTS_USE_SPELL = 55004;
inline constexpr uint32 NITRO_BOOSTS_SPEED_SPELL = 54861;
inline constexpr float NITRO_BOOSTS_SPEED_FACTOR = 2.5f;
inline constexpr uint32 NITRO_BOOSTS_DURATION_MS = 5000;

Item* GetNitroBoostBoots(Player* bot);
// Boots on, off cooldown and usable now
bool NitroBoostsReady(Player* bot);
bool UseNitroBoosts(Player* bot);

// Sets the field if it is empty. Returns true if it did.
template <typename T, typename U>
bool EmplaceIfUnset(std::optional<T>& field, U&& value)
{
    if (field)
        return false;

    field.emplace(std::forward<U>(value));
    return true;
}

// Clears the field. Returns true if it had a value.
template <typename T>
bool ResetIfSet(std::optional<T>& field)
{
    bool const wasSet = field.has_value();
    field.reset();
    return wasSet;
}

}

#endif
