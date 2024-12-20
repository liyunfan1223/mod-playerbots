#ifndef _PLAYERBOT_RAIDICCMULTIPLIERS_H
#define _PLAYERBOT_RAIDICCMULTIPLIERS_H

#include "Multiplier.h"

class IccAddsDbsMultiplier : public Multiplier
{
public:
    IccAddsDbsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds dbs") {}
    virtual float GetValue(Action* action);
};

class IccFestergutMultiplier : public Multiplier
{
public:
    IccFestergutMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc festergut") {}

public:
    virtual float GetValue(Action* action);
};

//ROTFACE
class IccRotfaceMultiplier : public Multiplier
{
public:
    IccRotfaceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc rotface") {}
    virtual float GetValue(Action* action);
};

/*class IccRotfaceGroupPositionMultiplier : public Multiplier
{
public:
    IccRotfaceGroupPositionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc rotface group position") {}
    virtual float GetValue(Action* action);
};*/

//PP
class IccAddsPutricideMultiplier : public Multiplier
{
public:
    IccAddsPutricideMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds putricide") {}
    virtual float GetValue(Action* action);
};

//BPC
class IccBpcAssistMultiplier : public Multiplier
{
public:
    IccBpcAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bpc assist") {}
    float GetValue(Action* action) override;
};

//BQL
class IccBqlPactOfDarkfallenMultiplier : public Multiplier
{
public:
    IccBqlPactOfDarkfallenMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bql pact of darkfallen multiplier") {}

    float GetValue(Action* action) override;
};

class IccBqlVampiricBiteMultiplier : public Multiplier
{
public:
    IccBqlVampiricBiteMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc bql vampiric bite") {}
    virtual float GetValue(Action* action);
};

#endif
