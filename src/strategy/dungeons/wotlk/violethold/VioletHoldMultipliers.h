#ifndef _PLAYERBOT_WOTLKDUNGEONVHMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONVHMULTIPLIERS_H

#include "Multiplier.h"

class ErekemMultiplier : public Multiplier
{
    public:
        ErekemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "erekem") {}

    public:
        virtual float GetValue(Action* action);
};

class IchoronMultiplier : public Multiplier
{
    public:
        IchoronMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ichoron") {}

    public:
        virtual float GetValue(Action* action);
};

class ZuramatMultiplier : public Multiplier
{
    public:
        ZuramatMultiplier(PlayerbotAI* ai) : Multiplier(ai, "zuramat the obliterator") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
