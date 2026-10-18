/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_KOLOGARN_H
#define PLAYERBOTS_ULDTRIGGERS_KOLOGARN_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

class KologarnBodyTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn body tank trigger";

    KologarnBodyTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnOffTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn off tank trigger";

    KologarnOffTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnRubbleTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn rubble tank trigger";

    KologarnRubbleTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnDpsTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn dps target trigger";

    KologarnDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnSmashSwapTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn smash swap trigger";

    KologarnSmashSwapTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnBodyUncoveredTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn body uncovered trigger";

    KologarnBodyUncoveredTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnFallFromFloorTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn fall from floor trigger";

    KologarnFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnRubbleSlowdownTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn rubble slowdown trigger";

    KologarnRubbleSlowdownTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class KologarnEyebeamTrigger : public Trigger
{
public:
    static constexpr char const* Name = "kologarn eyebeam trigger";

    KologarnEyebeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
