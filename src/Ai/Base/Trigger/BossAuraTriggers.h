/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BOSSAURATRIGGERS_H
#define PLAYERBOTS_BOSSAURATRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

enum BossAuraIDs
{
    SPELL_SHADOW_RESISTANCE_AURA_RANK_1 = 19876,
    SPELL_FROST_RESISTANCE_AURA_RANK_1 = 19888,
    SPELL_FIRE_RESISTANCE_AURA_RANK_1 = 19891,
    SPELL_SHADOW_RESISTANCE_AURA_RANK_2 = 19895,
    SPELL_SHADOW_RESISTANCE_AURA_RANK_3 = 19896,
    SPELL_FROST_RESISTANCE_AURA_RANK_2 = 19897,
    SPELL_FROST_RESISTANCE_AURA_RANK_3 = 19898,
    SPELL_FIRE_RESISTANCE_AURA_RANK_2 = 19899,
    SPELL_FIRE_RESISTANCE_AURA_RANK_3 = 19900,
    SPELL_ASPECT_OF_THE_WILD_RANK_1 = 20043,
    SPELL_ASPECT_OF_THE_WILD_RANK_2 = 20190,
    SPELL_ASPECT_OF_THE_WILD_RANK_3 = 27045,
    SPELL_SHADOW_RESISTANCE_AURA_RANK_4 = 27151,
    SPELL_FROST_RESISTANCE_AURA_RANK_4 = 27152,
    SPELL_FIRE_RESISTANCE_AURA_RANK_4 = 27153,
    SPELL_SHADOW_RESISTANCE_AURA_RANK_5 = 48943,
    SPELL_FROST_RESISTANCE_AURA_RANK_5 = 48945,
    SPELL_FIRE_RESISTANCE_AURA_RANK_5 = 48947,
    SPELL_ASPECT_OF_THE_WILD_RANK_4 = 49071
};

class BossFireResistanceTrigger : public Trigger
{
public:
    BossFireResistanceTrigger(PlayerbotAI* botAI, std::string const bossName)
        : Trigger(botAI, bossName + " fire resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossFrostResistanceTrigger : public Trigger
{
public:
    BossFrostResistanceTrigger(PlayerbotAI* botAI, std::string const bossName)
        : Trigger(botAI, bossName + " frost resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossNatureResistanceTrigger : public Trigger
{
public:
    BossNatureResistanceTrigger(PlayerbotAI* botAI, std::string const bossName)
        : Trigger(botAI, " nature resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossShadowResistanceTrigger : public Trigger
{
public:
    BossShadowResistanceTrigger(PlayerbotAI* botAI, std::string const bossName)
        : Trigger(botAI, " shadow resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

#endif
