#ifndef _PLAYERBOT_WOTLKDUNGEONPOSMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSMULTIPLIERS_H

#include "Multiplier.h"

class IckMultiplier : public Multiplier
{
    public:
    IckMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ick") {}

    public:
        virtual float GetValue(Action* action);
};


#endif
