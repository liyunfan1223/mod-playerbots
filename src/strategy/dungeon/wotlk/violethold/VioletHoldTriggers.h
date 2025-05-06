#ifndef _PLAYERBOT_WOTLKDUNGEONVHTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONVHTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

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
    ErekemTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "erekem target") {}
    bool IsActive() override;
};

class IchoronTargetTrigger : public Trigger
{
public:
    IchoronTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "ichoron target") {}
    bool IsActive() override;
};

class VoidShiftTrigger : public Trigger
{
public:
    VoidShiftTrigger(PlayerbotAI* ai) : Trigger(ai, "void shift") {}
    bool IsActive() override;
};

class ShroudOfDarknessTrigger : public Trigger
{
public:
    ShroudOfDarknessTrigger(PlayerbotAI* ai) : Trigger(ai, "shroud of darkness") {}
    bool IsActive() override;
};

class CyanigosaPositioningTrigger : public Trigger
{
public:
    CyanigosaPositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "cyanigosa positioning") {}
    bool IsActive() override;
};

#endif
