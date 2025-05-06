#ifndef _PLAYERBOT_WOTLKDUNGEONFOSMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONFOSMULTIPLIERS_H

#include "Multiplier.h"

class BronjahmMultiplier : public Multiplier
{
    public:
    BronjahmMultiplier(PlayerbotAI* ai) : Multiplier(ai, "bronjahm") {}

    public:
        virtual float GetValue(Action* action);
};

class AttackFragmentMultiplier : public Multiplier
{
public:
    AttackFragmentMultiplier(PlayerbotAI* ai) : Multiplier(ai, "attack fragment") { }

    float GetValue(Action* action) override;
};


#endif
