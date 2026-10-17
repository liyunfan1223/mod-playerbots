/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLMULTIPLIERS_H
#define PLAYERBOTS_HOLMULTIPLIERS_H

#include "Multiplier.h"

class BjarngrimMultiplier : public Multiplier
{
    public:
        BjarngrimMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "general bjarngrim") {}

    public:
        float GetValue(Action* action) override;
};

class VolkhanMultiplier : public Multiplier
{
    public:
        VolkhanMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "volkhan") {}

    public:
        float GetValue(Action* action) override;
};

class IonarMultiplier : public Multiplier
{
    public:
        IonarMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ionar") {}

    public:
        float GetValue(Action* action) override;
};

class LokenMultiplier : public Multiplier
{
    public:
        LokenMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "loken") {}

    public:
        float GetValue(Action* action) override;
};

#endif
