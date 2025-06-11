#ifndef _PLAYERBOT_RAIDICCMULTIPLIERS_H
#define _PLAYERBOT_RAIDICCMULTIPLIERS_H

#include "Multiplier.h"

//Lady Deathwhisper
class IccLadyDeathwhisperMultiplier : public Multiplier
{
public:
    IccLadyDeathwhisperMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lady deathwhisper") {}
    virtual float GetValue(Action* action);
};

//DBS
class IccAddsDbsMultiplier : public Multiplier
{
public:
    IccAddsDbsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds dbs") {}
    virtual float GetValue(Action* action);
};

//DOGS

class IccDogsMultiplier : public Multiplier
{
public:
    IccDogsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc dogs") {}
    virtual float GetValue(Action* action);
};

//FESTERGUT
class IccFestergutMultiplier : public Multiplier
{
public:
    IccFestergutMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc festergut") {}
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
    virtual float GetValue(Action* action);
};

//BQL
class IccBqlMultiplier : public Multiplier
{
public:
    IccBqlMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bql multiplier") {}    
    virtual float GetValue(Action* action) override;
};

//VDW
class IccValithriaDreamCloudMultiplier : public Multiplier
{
public:
    IccValithriaDreamCloudMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc valithria dream cloud") {}
    virtual float GetValue(Action* action);
};

//SINDRAGOSA
class IccSindragosaMultiplier : public Multiplier
{
public:
    IccSindragosaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa") {}
    virtual float GetValue(Action* action);
};

//LK
class IccLichKingAddsMultiplier : public Multiplier
{
public:
    IccLichKingAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lich king adds") {}
    virtual float GetValue(Action* action);
};


#endif
