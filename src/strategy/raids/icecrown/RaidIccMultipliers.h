#ifndef _PLAYERRBOT_RAIDICCMULTIPLIERS_H_
#define _PLAYERRBOT_RAIDICCMULTIPLIERS_H_

#include "Multiplier.h"


class IccAddsDbsMultiplier : public Multiplier
{
public:
    IccAddsDbsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds dbs") {}
    virtual float GetValue(Action* action);
};

class FestergutMultiplier : public Multiplier
{
public:
    FestergutMultiplier(PlayerbotAI* ai) : Multiplier(ai, "festergut") {}

public:
    virtual float GetValue(Action* action);
};


class RotfaceMultiplier : public Multiplier
{
public:
    RotfaceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rotface") {}
    virtual float GetValue(Action* action);
};

/*class PutricideVolatileOozeMultiplier : public Multiplier
{
public:
    PutricideVolatileOozeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "putricide volatile ooze") {}
    virtual float GetValue(Action* action);
};*/    

class IccAddsPutricideMultiplier : public Multiplier
{
public:
    IccAddsPutricideMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds putricide") {}
    virtual float GetValue(Action* action);
};

#endif