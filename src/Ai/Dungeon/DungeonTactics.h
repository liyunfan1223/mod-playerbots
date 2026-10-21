// Engine glue shared by dungeon encounter tactics: action shapes, casts on a target and pets stood down for a mechanic.

#ifndef _PLAYERBOTS_DUNGEON_TACTICS_H
#define _PLAYERBOTS_DUNGEON_TACTICS_H

#include <mutex>
#include <unordered_map>

#include "DungeonTacticsPolicy.h"
#include "ObjectGuid.h"
#include "Unit.h"

class Action;
class Creature;
class Player;

namespace DungeonTactics
{
ActionShape ShapeOf(Action* action);

// Auto-repeat counts by default: a wand's Shoot keeps firing through a mechanic otherwise.
bool IsCastingAt(Unit* caster, Unit* target, bool withAutoRepeat = true);
void StopCastsAt(Player* bot, Unit* target);

// Pets a mechanic made passive, with the stance to give back once it ends; bots on different maps update in parallel.
class SuspendedPets
{
public:
    void Remember(Player* bot, Creature* pet);
    bool Any(Player* bot);
    std::unordered_map<ObjectGuid, ReactStates> Take(Player* bot);

private:
    std::mutex _lock;
    std::unordered_map<ObjectGuid, std::unordered_map<ObjectGuid, ReactStates>> _pets;
};

// A damaging totem is down, or a pet is still aggressive or attacking the victim (any victim when null).
bool PetsOrTotemsFighting(Player* bot, Unit* victim);
// Unsummons damaging totems and sends every pet passive to heel, remembering its stance.
void StandDownPets(Player* bot, SuspendedPets& store);
// Gives the pets the store suspended their stance back; false when there were none left.
bool RestorePets(Player* bot, SuspendedPets& store);
}  // namespace DungeonTactics

#endif
