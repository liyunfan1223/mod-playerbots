#ifndef _PLAYERBOT_WOTLKDUNGEONOKMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONOKMULTIPLIERS_H

#include "Multiplier.h"

class ElderNadoxMultiplier : public Multiplier
{
    public:
        ElderNadoxMultiplier(PlayerbotAI* ai) : Multiplier(ai, "elder nadox") {}

    public:
        virtual float GetValue(Action* action);
};

class JedogaShadowseekerMultiplier : public Multiplier
{
    public:
        JedogaShadowseekerMultiplier(PlayerbotAI* ai) : Multiplier(ai, "jedoga shadowseeker") {}

    public:
        virtual float GetValue(Action* action);
};

class ForgottenOneMultiplier : public Multiplier
{
    public:
        ForgottenOneMultiplier(PlayerbotAI* ai) : Multiplier(ai, "forgotten one") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
