/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCMULTIPLIERS_H
#define PLAYERBOTS_OCMULTIPLIERS_H

#include "Multiplier.h"
#include "Unit.h"

const float uromCoords[4][4] =
{   // Platform coordinates
    {1177.47f, 937.722f, 527.405f, 2.21657f},
    {968.66f, 1042.53f, 527.32f, 0.077f},
    {1164.02f, 1170.85f, 527.321f, 3.66f},
    {1118.31f, 1080.377f, 508.361f, 4.25f}      // Inner ring, actual boss fight
};

class MountingDrakeMultiplier : public Multiplier
{
    public:
        MountingDrakeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mounting drake") {}

    public:
        float GetValue(Action* action) override;
};

class OccFlyingMultiplier : public Multiplier
{
    public:
        OccFlyingMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "occ flying drake") {}

    public:
        float GetValue(Action* action) override;
};

class UromMultiplier : public Multiplier
{
    public:
        UromMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mage-lord urom") {}

    public:
        float GetValue(Action* action) override;
    protected:
        uint8 GetPhaseByCurrentPosition(Unit* boss);
};

class EregosMultiplier : public Multiplier
{
    public:
        EregosMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ley-guardian eregos") {}

    public:
        float GetValue(Action* action) override;
};

#endif
