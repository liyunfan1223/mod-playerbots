/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_AURIAYA_H
#define PLAYERBOTS_ULDTRIGGERS_AURIAYA_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "RaidAntiFear.h"
#include "UldEncounter_Auriaya.h"
#include "Trigger.h"

class AuriayaFallFromFloorTrigger : public Trigger
{
public:
    static constexpr char const* Name = "auriaya fall from floor trigger";

    AuriayaFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AuriayaSeepingEssenceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "auriaya seeping essence trigger";

    AuriayaSeepingEssenceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AuriayaSentryTauntTrigger : public Trigger
{
public:
    static constexpr char const* Name = "auriaya sentry taunt trigger";

    AuriayaSentryTauntTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AuriayaRaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "auriaya raid position trigger";

    AuriayaRaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AuriayaSetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "auriaya set dps priority trigger";

    AuriayaSetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AuriayaAntiFearTrigger : public RaidAntiFearTrigger
{
public:
    static constexpr char const* Name = "auriaya anti fear trigger";

    AuriayaAntiFearTrigger(PlayerbotAI* botAI) : RaidAntiFearTrigger(botAI, Name) {}

protected:
    bool FearWindowActive() override { return AuriayaFearWindowActive(botAI); }
};

#endif
