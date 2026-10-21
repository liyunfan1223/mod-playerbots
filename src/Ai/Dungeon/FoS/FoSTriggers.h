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
    NPC_WELL_OF_SOULS           = 36536,

    SPELL_PHANTOM_BLAST         = 68982,
    SPELL_PHANTOM_BLAST_H       = 70322,
    SPELL_MIRRORED_SOUL         = 69051,
    SPELL_WAILING_SOULS         = 68899,
    SPELL_WAILING_SOULS_SWEEP   = 68875,
    SPELL_WAILING_SOULS_SWEEP_2 = 68876,
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

class DevourerWailingSoulsTrigger : public Trigger
{
public:
    DevourerWailingSoulsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer wailing souls") {}
    bool IsActive() override;
};

class DevourerWellOfSoulsTrigger : public Trigger
{
public:
    DevourerWellOfSoulsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer well of souls") {}
    bool IsActive() override;
};

class DevourerMirroredSoulTrigger : public Trigger
{
public:
    DevourerMirroredSoulTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer mirrored soul") {}
    bool IsActive() override;
};

class DevourerMirroredSoulFadedTrigger : public Trigger
{
public:
    DevourerMirroredSoulFadedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer mirrored soul faded") {}
    bool IsActive() override;
};

class DevourerPhantomBlastTrigger : public Trigger
{
public:
    DevourerPhantomBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer phantom blast") {}
    bool IsActive() override;
};

class DevourerStopCastTrigger : public Trigger
{
public:
    DevourerStopCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "devourer stop cast") {}
    bool IsActive() override;
};

#endif
