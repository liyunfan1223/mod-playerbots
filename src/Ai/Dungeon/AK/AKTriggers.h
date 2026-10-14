/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AKTRIGGERS_H
#define PLAYERBOTS_AKTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum OldKingdomIDs
{
    // Elder Nadox
    BUFF_GUARDIAN_AURA                 = 56153,

    // Jedoga Shadowseeker
    NPC_TWILIGHT_VOLUNTEER             = 30385,

    // Forgotten One(s)
    SPELL_SHADOW_CRASH_N               = 60833,
    SPELL_SHADOW_CRASH_H               = 60848,
};

#define SPELL_SHADOW_CRASH             DUNGEON_MODE(bot, SPELL_SHADOW_CRASH_N, SPELL_SHADOW_CRASH_H)

class NadoxGuardianTrigger : public Trigger
{
public:
    NadoxGuardianTrigger(PlayerbotAI* botAI) : Trigger(botAI, "elder nadox guardian") {}
    bool IsActive() override;
};

class JedogaVolunteerTrigger : public Trigger
{
public:
    JedogaVolunteerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "jedoga volunteer") {}
    bool IsActive() override;
};

class ShadowCrashTrigger : public Trigger
{
public:
    ShadowCrashTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shadow crash") {}
    bool IsActive() override;
};

#endif
