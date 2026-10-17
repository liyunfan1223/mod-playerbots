/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKMULTIPLIERS_H
#define PLAYERBOTS_UKMULTIPLIERS_H

#include "Multiplier.h"

class PrinceKelesethMultiplier : public Multiplier
{
    public:
        PrinceKelesethMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "prince keleseth") {}

    public:
        float GetValue(Action* action) override;
};

class SkarvaldAndDalronnMultiplier : public Multiplier
{
    public:
        SkarvaldAndDalronnMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "skarvald and dalronn") {}

    public:
        float GetValue(Action* action) override;
};

class IngvarThePlundererMultiplier : public Multiplier
{
    public:
        IngvarThePlundererMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ingvar the plunderer") {}

    public:
        float GetValue(Action* action) override;
};

#endif
