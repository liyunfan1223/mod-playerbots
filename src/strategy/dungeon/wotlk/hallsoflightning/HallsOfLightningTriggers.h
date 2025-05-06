#ifndef _PLAYERBOT_WOTLKDUNGEONHOLTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONHOLTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

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
    StormforgedLieutenantTrigger(PlayerbotAI* ai) : Trigger(ai, "stormforged lieutenant") {}
    bool IsActive() override;
};

class BjarngrimWhirlwindTrigger : public Trigger
{
public:
    BjarngrimWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "bjarngrim whirlwind") {}
    bool IsActive() override;
};

class VolkhanTrigger : public Trigger
{
public:
    VolkhanTrigger(PlayerbotAI* ai) : Trigger(ai, "volkhan") {}
    bool IsActive() override;
};

class IonarStaticOverloadTrigger : public Trigger
{
public:
    IonarStaticOverloadTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar static overload") {}
    bool IsActive() override;
};

class IonarBallLightningTrigger : public Trigger
{
public:
    IonarBallLightningTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar ball lightning spread") {}
    bool IsActive() override;
};

class IonarTankAggroTrigger : public Trigger
{
public:
    IonarTankAggroTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar tank aggro") {}
    bool IsActive() override;
};

class IonarDisperseTrigger : public Trigger
{
public:
    IonarDisperseTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar disperse") {}
    bool IsActive() override;
};

class LokenRangedTrigger : public Trigger
{
public:
    LokenRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "loken ranged") {}
    bool IsActive() override;
};

class LokenLightningNovaTrigger : public Trigger
{
public:
    LokenLightningNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "lightning nova") {}
    bool IsActive() override;
};

#endif
