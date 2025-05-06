#ifndef _PLAYERBOT_WOTLKDUNGEONDTKMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONDTKMULTIPLIERS_H

#include "Multiplier.h"

class NovosMultiplier : public Multiplier
{
    public:
        NovosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "novos the summoner") {}

    public:
        virtual float GetValue(Action* action);
};

class TharonjaMultiplier : public Multiplier
{
    public:
        TharonjaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "the prophet tharon'ja") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
