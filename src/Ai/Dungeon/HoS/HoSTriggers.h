/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSTRIGGERS_H
#define PLAYERBOTS_HOSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum HallsOfStoneIDs
{
    // Krystallus
    SPELL_GROUND_SLAM               = 50827,
    DEBUFF_GROUND_SLAM              = 50833,

    // Sjonnir The Ironshaper
    SPELL_LIGHTNING_RING_N          = 50840,
    SPELL_LIGHTNING_RING_H          = 59848,
};

#define SPELL_LIGHTNING_RING        DUNGEON_MODE(bot, SPELL_LIGHTNING_RING_N, SPELL_LIGHTNING_RING_H)

class KrystallusGroundSlamTrigger : public Trigger
{
public:
    KrystallusGroundSlamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "krystallus ground slam") {}
    bool IsActive() override;
};

class SjonnirLightningRingTrigger : public Trigger
{
public:
    SjonnirLightningRingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sjonnir lightning ring") {}
    bool IsActive() override;
};

#endif
