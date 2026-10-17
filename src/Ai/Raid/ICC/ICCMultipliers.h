/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCMULTIPLIERS_H
#define PLAYERBOTS_ICCMULTIPLIERS_H

#include "Multiplier.h"

//Lady Deathwhisper
class IccLadyDeathwhisperMultiplier : public Multiplier
{
public:
    IccLadyDeathwhisperMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc lady deathwhisper") {}
    float GetValue(Action* action) override;
};

//DBS
class IccAddsDbsMultiplier : public Multiplier
{
public:
    IccAddsDbsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc adds dbs") {}
    float GetValue(Action* action) override;
};

//DOGS

class IccDogsMultiplier : public Multiplier
{
public:
    IccDogsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc dogs") {}
    float GetValue(Action* action) override;
};

//FESTERGUT
class IccFestergutMultiplier : public Multiplier
{
public:
    IccFestergutMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc festergut") {}
    float GetValue(Action* action) override;
};

//ROTFACE
class IccRotfaceMultiplier : public Multiplier
{
public:
    IccRotfaceMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc rotface") {}
    float GetValue(Action* action) override;
};

/*class IccRotfaceGroupPositionMultiplier : public Multiplier
{
public:
    IccRotfaceGroupPositionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc rotface group position") {}
    float GetValue(Action* action) override;
};*/

//PP
class IccAddsPutricideMultiplier : public Multiplier
{
public:
    IccAddsPutricideMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc adds putricide") {}
    float GetValue(Action* action) override;
};

//BPC
class IccBpcAssistMultiplier : public Multiplier
{
public:
    IccBpcAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bpc assist") {}
    float GetValue(Action* action) override;
};

//BQL
class IccBqlMultiplier : public Multiplier
{
public:
    IccBqlMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bql multiplier") {}
    float GetValue(Action* action) override;
};

//VDW
class IccValithriaDreamCloudMultiplier : public Multiplier
{
public:
    IccValithriaDreamCloudMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc valithria dream cloud") {}
    float GetValue(Action* action) override;
};

//SINDRAGOSA
class IccSindragosaMultiplier : public Multiplier
{
public:
    IccSindragosaMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc sindragosa") {}
    float GetValue(Action* action) override;
};

//LK
class IccLichKingAddsMultiplier : public Multiplier
{
public:
    IccLichKingAddsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc lich king adds") {}
    float GetValue(Action* action) override;
};

class IccLichKingSpiritBombMultiplier : public Multiplier
{
public:
    IccLichKingSpiritBombMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc lich king spirit bomb") {}
    float GetValue(Action* action) override;
};

//GUNSHIP
class IccGunshipMultiplier : public Multiplier
{
public:
    IccGunshipMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc gunship") {}
    float GetValue(Action* action) override;
};

#endif
