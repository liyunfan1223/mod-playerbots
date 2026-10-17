/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSTRIGGERS_H
#define PLAYERBOTS_FOSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum ForgeOfSoulsBronjahmIDs
{
    // Bronjahm
    NPC_CORRUPTED_SOUL_FRAGMENT = 36535,

    SPELL_CORRUPT_SOUL          = 68839,
    SPELL_SOULSTORM_VISUAL      = 68870,
    SPELL_SOULSTORM_VISUAL2     = 68904,
    SPELL_SOULSTORM             = 68872,

    // Devourer of Souls
    SPELL_WAILING_SOULS         = 68899,
};

class MoveFromBronjahmTrigger : public Trigger
{
public:
    MoveFromBronjahmTrigger(PlayerbotAI* botAI) : Trigger(botAI, "move from bronjahm") {}

    bool IsActive() override;
};

class SwitchToSoulFragment : public Trigger
{
public:
    SwitchToSoulFragment(PlayerbotAI* botAI) : Trigger(botAI, "switch to soul fragment") {}

    bool IsActive() override;
};

class BronjahmPositionTrigger : public Trigger
{
public:
    BronjahmPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bronjahm position") {}
    bool IsActive() override;
};

class DevourerOfSoulsTrigger : public Trigger
{
public:
    DevourerOfSoulsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer of souls") {}
    bool IsActive() override;
};

#endif
