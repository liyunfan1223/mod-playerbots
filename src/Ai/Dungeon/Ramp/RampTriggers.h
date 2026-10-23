/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPTRIGGERS_H
#define PLAYERBOTS_RAMPTRIGGERS_H

#include "Trigger.h"
#include "RampBossHelper.h"

// Watchkeeper Gargolmar

class GargolmarHellfireWatchersAreActiveTrigger : public Trigger
{
public:
    GargolmarHellfireWatchersAreActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gargolmar hellfire watchers are active")
    {
    }

    bool IsActive() override;
};

// Omor the Unscarred

class OmorTreacheryAuraTrigger : public Trigger
{
public:
    OmorTreacheryAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor treachery aura") {}

    bool IsActive() override;

private:
    OmorBossHelper helper;
};

class OmorTankHasTreacheryAuraTrigger : public Trigger
{
public:
    OmorTankHasTreacheryAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor tank has treachery aura") {}

    bool IsActive() override;

private:
    OmorBossHelper helper;
};

class OmorRangedSpreadTrigger : public Trigger
{
public:
    OmorRangedSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor ranged spread") {}

    bool IsActive() override;
};

class OmorFiendishHoundIsActiveTrigger : public Trigger
{
public:
    OmorFiendishHoundIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "omor fiendish hound is active") {}

    bool IsActive() override;
};

// Vazruden & Nazan

class VazrudenTankPositionBossTrigger : public Trigger
{
public:
    VazrudenTankPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vazruden tank position boss") {}

    bool IsActive() override;
};

class VazrudenBossIsActiveTrigger : public Trigger
{
public:
    VazrudenBossIsActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vazruden boss is active") {}

    bool IsActive() override;
};

class NazanBossTremorTotemTrigger : public Trigger
{
public:
    NazanBossTremorTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nazan boss tremor totem")
    {
    }

    bool IsActive() override;
};

class NazanBossFireResistanceTotemTrigger : public Trigger
{
public:
    NazanBossFireResistanceTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nazan boss fire resistance totem") {}

    bool IsActive() override;
};

class NazanBossFireResistanceAuraTrigger : public Trigger
{
public:
    NazanBossFireResistanceAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, "nazan boss fire resistance aura") {}

    bool IsActive() override;
};

#endif
