/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKTRIGGERS_H
#define PLAYERBOTS_UKTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum UtgardeKeepIDs
{
    // Prince Keleseth
    SPELL_FROST_TOMB               = 48400,
    NPC_FROST_TOMB                 = 23965,

    // Ingvar the Plunderer
    SPELL_STAGGERING_ROAR_N         = 42708,
    SPELL_STAGGERING_ROAR_H         = 59708,
    SPELL_SMASH_N                   = 42669,
    SPELL_SMASH_H                   = 59706,
    SPELL_DREADFUL_ROAR_N           = 42729,
    SPELL_DREADFUL_ROAR_H           = 59734,
    SPELL_WOE_STRIKE_N              = 42730,
    SPELL_WOE_STRIKE_H              = 59735,
    SPELL_DARK_SMASH_N              = 42723,
    SPELL_DARK_SMASH_H              = 59709,
};

#define SPELL_STAGGERING_ROAR       DUNGEON_MODE(bot, SPELL_STAGGERING_ROAR_N, SPELL_STAGGERING_ROAR_H)
#define SPELL_DREADFUL_ROAR         DUNGEON_MODE(bot, SPELL_DREADFUL_ROAR_N, SPELL_DREADFUL_ROAR_H)
#define SPELL_SMASH                 DUNGEON_MODE(bot, SPELL_SMASH_N, SPELL_SMASH_H)
#define SPELL_DARK_SMASH            DUNGEON_MODE(bot, SPELL_DARK_SMASH_N, SPELL_DARK_SMASH_H)

class KelesethFrostTombTrigger : public Trigger
{
public:
    KelesethFrostTombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "keleseth frost tomb") {}
    bool IsActive() override;
};

class DalronnDpsTrigger : public Trigger
{
public:
    DalronnDpsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dalronn dps") {}
    bool IsActive() override;
};

class IngvarDreadfulRoarTrigger : public Trigger
{
public:
    IngvarDreadfulRoarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ingvar dreadful roar") {}
    bool IsActive() override;
};

class IngvarSmashTankTrigger : public Trigger
{
public:
    IngvarSmashTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ingvar smash tank") {}
    bool IsActive() override;
};

class IngvarSmashTankReturnTrigger : public Trigger
{
public:
    IngvarSmashTankReturnTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ingvar smash tank return") {}
    bool IsActive() override;
};

class NotBehindIngvarTrigger : public Trigger
{
public:
    NotBehindIngvarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "not behind ingvar") {}
    bool IsActive() override;
};

#endif
