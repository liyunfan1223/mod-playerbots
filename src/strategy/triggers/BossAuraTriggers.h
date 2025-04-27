/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOSSAURATRIGGERS_H
#define _PLAYERBOT_BOSSAURATRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

enum BossAuraIDs
{
    SPELL_FROST_RESISTANCE_AURA = 48945,
    SPELL_FIRE_RESISTANCE_AURA = 48947,
    SPELL_ASPECT_OF_THE_WILD = 49071,
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
        : Trigger(ai, "kologarn nature resistance trigger"), bossName(bossName)
    {
    }
    bool IsActive() override;

private:
    std::string bossName;
};

#endif
