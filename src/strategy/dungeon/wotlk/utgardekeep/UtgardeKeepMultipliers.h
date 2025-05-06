#ifndef _PLAYERBOT_WOTLKDUNGEONUKMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONUKMULTIPLIERS_H

#include "Multiplier.h"

class PrinceKelesethMultiplier : public Multiplier
{
    public:
        PrinceKelesethMultiplier(PlayerbotAI* ai) : Multiplier(ai, "prince keleseth") {}

    public:
        virtual float GetValue(Action* action);
};

class SkarvaldAndDalronnMultiplier : public Multiplier
{
    public:
        SkarvaldAndDalronnMultiplier(PlayerbotAI* ai) : Multiplier(ai, "skarvald and dalronn") {}

    public:
        virtual float GetValue(Action* action);
};

class IngvarThePlundererMultiplier : public Multiplier
{
    public:
        IngvarThePlundererMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ingvar the plunderer") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
