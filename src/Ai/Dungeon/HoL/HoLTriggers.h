/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLTRIGGERS_H
#define PLAYERBOTS_HOLTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum HallsOfLightningIDs
{
    // General Bjarngrim
    NPC_STORMFORGED_LIEUTENANT      = 29240,
    SPELL_WHIRLWIND_BJARNGRIM       = 52027,

    // Ionar
    SPELL_STATIC_OVERLOAD_N         = 52658,
    SPELL_STATIC_OVERLOAD_H         = 59795,
    SPELL_BALL_LIGHTNING_N          = 52780,
    SPELL_BALL_LIGHTNING_H          = 59800,
    SPELL_DISPERSE                  = 52770,
    NPC_SPARK_OF_IONAR              = 28926,

    // Loken
    SPELL_LIGHTNING_NOVA_N          = 52960,
    SPELL_LIGHTNING_NOVA_H          = 59835,
};

#define SPELL_STATIC_OVERLOAD       DUNGEON_MODE(bot, SPELL_STATIC_OVERLOAD_N, SPELL_STATIC_OVERLOAD_H)
#define SPELL_BALL_LIGHTNING        DUNGEON_MODE(bot, SPELL_BALL_LIGHTNING_N, SPELL_BALL_LIGHTNING_H)
#define SPELL_LIGHTNING_NOVA        DUNGEON_MODE(bot, SPELL_LIGHTNING_NOVA_N, SPELL_LIGHTNING_NOVA_H)

class StormforgedLieutenantTrigger : public Trigger
{
public:
    StormforgedLieutenantTrigger(PlayerbotAI* botAI) : Trigger(botAI, "stormforged lieutenant") {}
    bool IsActive() override;
};

class BjarngrimWhirlwindTrigger : public Trigger
{
public:
    BjarngrimWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bjarngrim whirlwind") {}
    bool IsActive() override;
};

class VolkhanTrigger : public Trigger
{
public:
    VolkhanTrigger(PlayerbotAI* botAI) : Trigger(botAI, "volkhan") {}
    bool IsActive() override;
};

class IonarStaticOverloadTrigger : public Trigger
{
public:
    IonarStaticOverloadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ionar static overload") {}
    bool IsActive() override;
};

class IonarBallLightningTrigger : public Trigger
{
public:
    IonarBallLightningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ionar ball lightning spread") {}
    bool IsActive() override;
};

class IonarTankAggroTrigger : public Trigger
{
public:
    IonarTankAggroTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ionar tank aggro") {}
    bool IsActive() override;
};

class IonarDisperseTrigger : public Trigger
{
public:
    IonarDisperseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ionar disperse") {}
    bool IsActive() override;
};

class LokenRangedTrigger : public Trigger
{
public:
    LokenRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "loken ranged") {}
    bool IsActive() override;
};

class LokenLightningNovaTrigger : public Trigger
{
public:
    LokenLightningNovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lightning nova") {}
    bool IsActive() override;
};

#endif
