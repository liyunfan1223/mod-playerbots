#ifndef _PLAYERBOT_WOTLKDUNGEONUKTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONUKTRIGGERS_H

#include "EventMap.h"
#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

// Taken from:
// src/server/scripts/Northrend/UtgardeKeep/UtgardeKeep/boss_ingvar_the_plunderer.cpp
enum eSpells
{
    SPELL_SUMMON_VALKYR             = 42912,
    SPELL_RESURRECTION_BEAM         = 42857,
    SPELL_RESURRECTION_BALL         = 42862,
    SPELL_RESURRECTION_HEAL         = 42704,
    SPELL_INGVAR_TRANSFORM          = 42796,

    SPELL_STAGGERING_ROAR_N         = 42708,
    SPELL_STAGGERING_ROAR_H         = 59708,
    SPELL_CLEAVE                    = 42724,
    SPELL_SMASH_N                   = 42669,
    SPELL_SMASH_H                   = 59706,
    SPELL_ENRAGE_N                  = 42705,
    SPELL_ENRAGE_H                  = 59707,

    SPELL_DREADFUL_ROAR_N           = 42729,
    SPELL_DREADFUL_ROAR_H           = 59734,
    SPELL_WOE_STRIKE_N              = 42730,
    SPELL_WOE_STRIKE_H              = 59735,
    SPELL_DARK_SMASH                = 42723,
    SPELL_SHADOW_AXE                = 42749,

    // Added
    DEBUFF_FROST_TOMB               = 48400,
};

#define SPELL_STAGGERING_ROAR       DUNGEON_MODE(bot, SPELL_STAGGERING_ROAR_N, SPELL_STAGGERING_ROAR_H)
#define SPELL_DREADFUL_ROAR         DUNGEON_MODE(bot, SPELL_DREADFUL_ROAR_N, SPELL_DREADFUL_ROAR_H)
#define SPELL_WOE_STRIKE            DUNGEON_MODE(bot, SPELL_WOE_STRIKE_N, SPELL_WOE_STRIKE_H)
#define SPELL_SMASH                 DUNGEON_MODE(bot, SPELL_SMASH_N, SPELL_SMASH_H)
#define SPELL_ENRAGE                DUNGEON_MODE(bot, SPELL_ENRAGE_N, SPELL_ENRAGE_H)

class KelesethFrostTombTrigger : public Trigger
{
public:
    KelesethFrostTombTrigger(PlayerbotAI* ai) : Trigger(ai, "keleseth frost tomb") {}
    bool IsActive() override;
};

class DalronnNontankTrigger : public Trigger
{
public:
    DalronnNontankTrigger(PlayerbotAI* ai) : Trigger(ai, "dalronn non-tank") {}
    bool IsActive() override;
};

class IngvarStaggeringRoarTrigger : public Trigger
{
public:
    IngvarStaggeringRoarTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar staggering roar") {}
    bool IsActive() override;
};

class IngvarDreadfulRoarTrigger : public Trigger
{
public:
    IngvarDreadfulRoarTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar dreadful roar") {}
    bool IsActive() override;
};

class IngvarSmashTankTrigger : public Trigger
{
public:
    IngvarSmashTankTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar smash tank") {}
    bool IsActive() override;
};

class IngvarSmashTankReturnTrigger : public Trigger
{
public:
    IngvarSmashTankReturnTrigger(PlayerbotAI* ai) : Trigger(ai, "ingvar smash tank return") {}
    bool IsActive() override;
};

class NotBehindIngvarTrigger : public Trigger
{
public:
    NotBehindIngvarTrigger(PlayerbotAI* ai) : Trigger(ai, "not behind ingvar") {}
    bool IsActive() override;
};

#endif
