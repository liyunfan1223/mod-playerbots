#ifndef _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRMULTIPLIERS_H

#include "Multiplier.h"

class HighKingMaulgarMultiplier : public Multiplier
{
public:
    HighKingMaulgarMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "high king maulgar multiplier") {}
    virtual float GetValue(Action* action);
};

class GruulTheDragonkillerMultiplier : public Multiplier
{
public:
    GruulTheDragonkillerMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gruul the dragonkiller multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
