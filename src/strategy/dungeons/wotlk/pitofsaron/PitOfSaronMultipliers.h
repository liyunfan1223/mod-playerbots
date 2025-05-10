#ifndef _PLAYERBOT_WOTLKDUNGEONPOSMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSMULTIPLIERS_H

#include "Multiplier.h"

class IckAndKrickMultiplier : public Multiplier
{
    public:
    IckAndKrickMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ick and krick") {}

    public:
        virtual float GetValue(Action* action);
};

class GarfrostMultiplier : public Multiplier
{
public:
    GarfrostMultiplier(PlayerbotAI* ai) : Multiplier(ai, "garfrost") { }

    float GetValue(Action* action) override;
};


#endif
