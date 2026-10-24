/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BOSSRESISTANCEMULTIPLIERS_H
#define PLAYERBOTS_BOSSRESISTANCEMULTIPLIERS_H

#include "Multiplier.h"

#include <string>

class Action;
class PlayerbotAI;

// Holds the raid's Aspect of the Wild hunter in Wild while a nature damage boss is up, or its own aspect
// nodes recast Dragonhawk on the next GCD. Every other hunter stays free to go Viper for mana.
class BossNatureAspectHoldMultiplier : public Multiplier
{
public:
    BossNatureAspectHoldMultiplier(PlayerbotAI* botAI, std::string const bossName)
        : Multiplier(botAI, bossName + " nature aspect hold multiplier"), _bossName(bossName)
    {
    }

    float GetValue(Action* action) override;

private:
    std::string const _bossName;
};

#endif
