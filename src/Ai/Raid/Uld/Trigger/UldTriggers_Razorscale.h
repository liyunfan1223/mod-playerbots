/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_RAZORSCALE_H
#define PLAYERBOTS_ULDTRIGGERS_RAZORSCALE_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

class RazorscaleFlyingAloneTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale flying alone";

    RazorscaleFlyingAloneTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleDevouringFlamesTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale avoid devouring flames";

    RazorscaleDevouringFlamesTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleAvoidSentinelTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale avoid sentinel";

    RazorscaleAvoidSentinelTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleAvoidWhirlwindTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale avoid whirlwind";

    RazorscaleAvoidWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleGroundedTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale grounded";

    RazorscaleGroundedTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleHarpoonAvailableTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale harpoon trigger";

    RazorscaleHarpoonAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleFuseArmorTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale fuse armor trigger";

    RazorscaleFuseArmorTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleKillTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale kill target trigger";

    RazorscaleKillTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscalePetControlTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale pet control trigger";

    RazorscalePetControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class RazorscaleFlameBreathTrigger : public Trigger
{
public:
    static constexpr char const* Name = "razorscale flame breath trigger";

    RazorscaleFlameBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
