/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDSPLASH_H
#define PLAYERBOTS_RAIDSPLASH_H

#include <set>
#include <string>

class Action;
class PlayerbotAI;

// Whether the action's spell lands on an area, heals never count. Read off the spell because the threat
// type misses whirlwind, divine storm, consecration, death and decay, swipe and pestilence.
bool SpellSplashes(PlayerbotAI* botAI, Action* action);

// Cleaves, chains and melee area abilities by action name: their own spells carry no area
extern std::set<std::string> const MULTI_TARGET_ACTIONS;

#endif
