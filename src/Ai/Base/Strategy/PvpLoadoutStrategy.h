/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTSTRATEGY_H
#define PLAYERBOTS_PVPLOADOUTSTRATEGY_H

#include "Multiplier.h"
#include "PvpLoadoutRules.h"
#include "Strategy.h"
#include "Value.h"
#include <optional>

class PlayerbotAI;

// Swaps random bots into a PvP loadout in arenas and back out afterwards (Playerbots.PvpLoadoutSwap).
class PvpLoadoutStrategy : public Strategy
{
public:
    PvpLoadoutStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "pvp loadout"; }
    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

// While a loadout is worn, stops gear upgrades from equipping PvE items over the match gear.
class PvpLoadoutMultiplier : public Multiplier
{
public:
    PvpLoadoutMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "pvp loadout") {}

    float GetValue(Action* action) override;

private:
    Value<std::optional<PvpLoadout::Snapshot>>* _loadout = nullptr;
};

#endif
