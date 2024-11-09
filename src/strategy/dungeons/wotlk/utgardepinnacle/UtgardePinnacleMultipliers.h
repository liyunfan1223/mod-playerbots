#ifndef _PLAYERBOT_WOTLKDUNGEONUPMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONUPMULTIPLIERS_H

#include "Multiplier.h"

class SkadiMultiplier : public Multiplier
{
    public:
        SkadiMultiplier(PlayerbotAI* ai) : Multiplier(ai, "skadi the ruthless") {}

    public:
        virtual float GetValue(Action* action);
};

class YmironMultiplier : public Multiplier
{
    public:
        YmironMultiplier(PlayerbotAI* ai) : Multiplier(ai, "king ymiron") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
