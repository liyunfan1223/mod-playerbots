/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BURSTCOOLDOWNS_H
#define PLAYERBOTS_BURSTCOOLDOWNS_H

#include <string>

#include "Define.h"
#include "ObjectGuid.h"

class Player;
class Unit;

// Offensive throughput cooldowns and raid-wide haste only, tank and healer cooldowns keep their own logic.
// Takes the name because Action::getName() returns by value and this runs per action per tick.
bool IsBurstCooldownAction(std::string const& actionName);

// Burst cooldowns the class also refills its mana with. A phase-long hold on one starves the caster, so
// those holds let it through: shadow priests carry no mana potions.
bool IsManaReturnCooldown(Player* bot, std::string const& actionName);

// Caller-owned dwell state, keyed on the boss so a new boss re-arms the gate
struct BurstHoldState
{
    ObjectGuid boss;
    uint32 sinceMs = 0;

    void Reset()
    {
        boss.Clear();
        sinceMs = 0;
    }
};

// True once the boss's victim has been a tank of the bot's group, any tank, for dwellMs straight. Resets
// while it isn't. Callers Reset() on paths that skip this, or a stale timer lets the next pull through.
bool TankHasHeldBoss(Player* bot, Unit* boss, BurstHoldState& state, uint32 dwellMs);

#endif
