
#ifndef _PLAYERRBOT_RAIDULDUARMULTIPLIERS_H_
#define _PLAYERRBOT_RAIDULDUARMULTIPLIERS_H_

#include "Multiplier.h"
#include "raids/ulduar/RaidUlduarBossHelper.h"

class FlameLeviathanMultiplier : public Multiplier
{
public:
    FlameLeviathanMultiplier(PlayerbotAI* ai) : Multiplier(ai, "flame leviathan") {}

public:
    virtual float GetValue(Action* action);
};

#endif