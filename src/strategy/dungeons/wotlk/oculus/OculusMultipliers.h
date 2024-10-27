#ifndef _PLAYERBOT_WOTLKDUNGEONOCCMULTIPLIERS_H
#define _PLAYERBOT_WOTLKDUNGEONOCCMULTIPLIERS_H

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
        MountingDrakeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mounting drake") {}

    public:
        virtual float GetValue(Action* action);
};

class FlyingMultiplier : public Multiplier
{
    public:
        FlyingMultiplier(PlayerbotAI* ai) : Multiplier(ai, "flying drake") {}

    public:
        virtual float GetValue(Action* action);
};

class UromMultiplier : public Multiplier
{
    public:
        UromMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mage-lord urom") {}

    public:
        virtual float GetValue(Action* action);
    protected:
        uint8 GetPhaseByCurrentPosition(Unit* boss);
};

class EregosMultiplier : public Multiplier
{
    public:
        EregosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ley-guardian eregos") {}

    public:
        virtual float GetValue(Action* action);
};

#endif
