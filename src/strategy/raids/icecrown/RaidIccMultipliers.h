#ifndef _PLAYERBOT_RAIDICCMULTIPLIERS_H
#define _PLAYERBOT_RAIDICCMULTIPLIERS_H

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

//BPC
class BpcAssistMultiplier : public Multiplier
{
public:
    BpcAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "bpc assist") {}
    float GetValue(Action* action) override;
};

class IccBqlPactOfDarkfallenMultiplier : public Multiplier
{
public:
    IccBqlPactOfDarkfallenMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "pact of darkfallen multiplier") {}

    float GetValue(Action* action) override;
};

class IccBqlVampiricBiteMultiplier : public Multiplier
{
public:
    IccBqlVampiricBiteMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc bql vampiric bite") {}
    virtual float GetValue(Action* action);
};

#endif