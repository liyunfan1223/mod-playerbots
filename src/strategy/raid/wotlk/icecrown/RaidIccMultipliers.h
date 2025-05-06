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
class IccBqlPactOfDarkfallenMultiplier : public Multiplier
{
public:
    IccBqlPactOfDarkfallenMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bql pact of darkfallen multiplier") {}    
    virtual float GetValue(Action* action) override;
};

class IccBqlVampiricBiteMultiplier : public Multiplier
{
public:
    IccBqlVampiricBiteMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc bql vampiric bite") {}
    virtual float GetValue(Action* action);
};

//VDW
class IccValithriaDreamCloudMultiplier : public Multiplier
{
public:
    IccValithriaDreamCloudMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc valithria dream cloud") {}
    virtual float GetValue(Action* action);
};

//SINDRAGOSA
class IccSindragosaTankPositionMultiplier : public Multiplier
{
public:
    IccSindragosaTankPositionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa tank position") {}
    virtual float GetValue(Action* action);
};

class IccSindragosaFrostBeaconMultiplier : public Multiplier
{
public:
    IccSindragosaFrostBeaconMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa frost beacon") {}
    virtual float GetValue(Action* action);
};

/*class IccSindragosaFlyingMultiplier : public Multiplier
{
public:
    IccSindragosaFlyingMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa flying") {}
    virtual float GetValue(Action* action);
};*/

class IccSindragosaMysticBuffetMultiplier : public Multiplier
{
public:
    IccSindragosaMysticBuffetMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa mystic buffet") {}
    virtual float GetValue(Action* action);
};

class IccSindragosaBlisteringColdPriorityMultiplier : public Multiplier
{
public:
    IccSindragosaBlisteringColdPriorityMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sindragosa blistering cold priority") {}

    virtual float GetValue(Action* action) override;
};

class IccSindragosaFrostBombMultiplier : public Multiplier
{
public:
    IccSindragosaFrostBombMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa frost bomb") {}
    virtual float GetValue(Action* action);
};

class IccLichKingNecroticPlagueMultiplier : public Multiplier
{
public:
    IccLichKingNecroticPlagueMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lich king necrotic plague") {}
    virtual float GetValue(Action* action);
};

class IccLichKingAddsMultiplier : public Multiplier
{
public:
    IccLichKingAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lich king adds") {}
    virtual float GetValue(Action* action);
};


#endif