#ifndef _PLAYERBOT_WOTLKDUNGEONGDMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONGDMULTIPLIERS_H

#include "Multiplier.h"

class SladranMultiplier : public Multiplier
{
    public:
        SladranMultiplier(PlayerbotAI* ai) : Multiplier(ai, "slad'ran") {}

    public:
        virtual float GetValue(Action* action);
};

class GaldarahMultiplier : public Multiplier
{
    public:
        GaldarahMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gal'darah") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
