/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHTRIGGERS_H
#define PLAYERBOTS_VHTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum VioletHoldIDs
{
    // Ichoron
    SPELL_DRAINED                      = 59820,
    NPC_ICHOR_GLOBULE                  = 29321,

    // Zuramat the Obliterator
    SPELL_VOID_SHIFTED                 = 54343,
    SPELL_SHROUD_OF_DARKNESS_N         = 54524,
    SPELL_SHROUD_OF_DARKNESS_H         = 59745,
    NPC_VOID_SENTRY                    = 29364,
};

#define SPELL_SHROUD_OF_DARKNESS    DUNGEON_MODE(bot, SPELL_SHROUD_OF_DARKNESS_N, SPELL_SHROUD_OF_DARKNESS_H)

class ErekemTargetTrigger : public Trigger
{
public:
    ErekemTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "erekem target") {}
    bool IsActive() override;
};

class IchoronTargetTrigger : public Trigger
{
public:
    IchoronTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ichoron target") {}
    bool IsActive() override;
};

class VoidShiftTrigger : public Trigger
{
public:
    VoidShiftTrigger(PlayerbotAI* botAI) : Trigger(botAI, "void shift") {}
    bool IsActive() override;
};

class ShroudOfDarknessTrigger : public Trigger
{
public:
    ShroudOfDarknessTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shroud of darkness") {}
    bool IsActive() override;
};

class CyanigosaPositioningTrigger : public Trigger
{
public:
    CyanigosaPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cyanigosa positioning") {}
    bool IsActive() override;
};

#endif
