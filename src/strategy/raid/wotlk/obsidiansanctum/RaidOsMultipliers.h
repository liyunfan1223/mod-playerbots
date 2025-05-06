
#ifndef _PLAYERRBOT_RAIDOSMULTIPLIERS_H
#define _PLAYERRBOT_RAIDOSMULTIPLIERS_H

#include "Multiplier.h"

class SartharionMultiplier : public Multiplier
{
public:
    SartharionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sartharion") {}

public:
    virtual float GetValue(Action* action);
};

#endif
