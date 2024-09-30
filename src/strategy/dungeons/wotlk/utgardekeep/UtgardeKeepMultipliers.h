#ifndef _PLAYERRBOT_WOTLKDUNGEONUKMULTIPLIERS_H_
#define _PLAYERRBOT_WOTLKDUNGEONUKMULTIPLIERS_H_

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
