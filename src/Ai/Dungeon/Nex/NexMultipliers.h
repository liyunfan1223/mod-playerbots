/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXMULTIPLIERS_H
#define PLAYERBOTS_NEXMULTIPLIERS_H

#include "Multiplier.h"

class FactionCommanderMultiplier : public Multiplier
{
    public:
        FactionCommanderMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "faction commander") {}

    public:
        float GetValue(Action* action) override;
};

class TelestraMultiplier : public Multiplier
{
    public:
        TelestraMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "grand magus telestra") {}

    public:
        float GetValue(Action* action) override;
};

class AnomalusMultiplier : public Multiplier
{
    public:
        AnomalusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anomalus") {}

    public:
        float GetValue(Action* action) override;
};

class OrmorokMultiplier : public Multiplier
{
    public:
        OrmorokMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ormorok the tree-shaper") {}

    public:
        float GetValue(Action* action) override;
};

#endif
