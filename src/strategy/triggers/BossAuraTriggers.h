/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOSSAURATRIGGERS_H
#define _PLAYERBOT_BOSSAURATRIGGERS_H

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
    BossFireResistanceTrigger(PlayerbotAI* ai, std::string const bossName)
        : Trigger(ai, bossName + " fire resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossFrostResistanceTrigger : public Trigger
{
public:
    BossFrostResistanceTrigger(PlayerbotAI* ai, std::string const bossName)
        : Trigger(ai, bossName + " frost resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossNatureResistanceTrigger : public Trigger
{
public:
    BossNatureResistanceTrigger(PlayerbotAI* ai, std::string const bossName)
        : Trigger(ai, " nature resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

class BossShadowResistanceTrigger : public Trigger
{
public:
    BossShadowResistanceTrigger(PlayerbotAI* ai, std::string const bossName)
        : Trigger(ai, " shadow resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

#endif
