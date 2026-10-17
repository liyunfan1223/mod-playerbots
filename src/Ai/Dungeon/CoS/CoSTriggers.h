/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSTRIGGERS_H
#define PLAYERBOTS_COSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum CullingOfStratholmeIDs
{
    // Salramm the Fleshcrafter
    NPC_GHOUL_MINION                   = 27733,
};

class ExplodeGhoulTrigger : public Trigger
{
public:
    ExplodeGhoulTrigger(PlayerbotAI* botAI) : Trigger(botAI, "explode ghoul") {}
    bool IsActive() override;
};

class EpochRangedTrigger : public Trigger
{
public:
    EpochRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chrono-lord epoch ranged") {}
    bool IsActive() override;
};

#endif
