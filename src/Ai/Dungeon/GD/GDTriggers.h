/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDTRIGGERS_H
#define PLAYERBOTS_GDTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum GundrakIDs
{
    // Slad'ran
    SPELL_POISON_NOVA_N             = 55081,
    SPELL_POISON_NOVA_H             = 59842,
    SPELL_SNAKE_WRAP                = 55126,
    NPC_SNAKE_WRAP                  = 29742,
    NPC_SLADRAN_VIPER               = 29680,
    NPC_SLADRAN_CONSTRICTOR         = 29713,

    // Gal'darah
    SPELL_WHIRLING_SLASH_N          = 55250,
    SPELL_WHIRLING_SLASH_H          = 59824,
};

#define SPELL_POISON_NOVA           DUNGEON_MODE(bot, SPELL_POISON_NOVA_N, SPELL_POISON_NOVA_H)
#define SPELL_WHIRLING_SLASH        DUNGEON_MODE(bot, SPELL_WHIRLING_SLASH_N, SPELL_WHIRLING_SLASH_H)

namespace GundrakSladran
{
constexpr float STACK_LEASH_YD = 10.0f;
constexpr float STACK_CLOSE_TO_YD = 5.0f;
constexpr float TANK_PICKUP_YD = 12.0f;
constexpr uint32 SNAKE_WRAP_SCAN_INTERVAL = 200;

bool IsAdd(Unit* unit);
ObjectGuid CalculateAssignedSnakeWrap(PlayerbotAI* botAI);
Unit* GetAssignedSnakeWrap(PlayerbotAI* botAI);
Player* GetStackTank(PlayerbotAI* botAI);
Unit* GetTankHoldTarget(PlayerbotAI* botAI);
}

class SladranPoisonNovaTrigger : public Trigger
{
public:
    SladranPoisonNovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "slad'ran poison nova") {}
    bool IsActive() override;
};

class SladranSnakeWrapTrigger : public Trigger
{
public:
    SladranSnakeWrapTrigger(PlayerbotAI* botAI) : Trigger(botAI, "slad'ran snake wrap") {}
    bool IsActive() override;
};

class SladranStackOnTankTrigger : public Trigger
{
public:
    SladranStackOnTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "slad'ran stack on tank") {}
    bool IsActive() override;
};

class SladranTankHoldTrigger : public Trigger
{
public:
    SladranTankHoldTrigger(PlayerbotAI* botAI) : Trigger(botAI, "slad'ran tank hold") {}
    bool IsActive() override;
};

class GaldarahWhirlingSlashTrigger : public Trigger
{
public:
    GaldarahWhirlingSlashTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gal'darah whirling slash") {}
    bool IsActive() override;
};

#endif
