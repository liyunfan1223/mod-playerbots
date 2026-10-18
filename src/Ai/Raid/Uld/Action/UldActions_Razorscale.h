/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_RAZORSCALE_H
#define PLAYERBOTS_ULDACTIONS_RAZORSCALE_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class RazorscaleAvoidDevouringFlameAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale avoid devouring flames";

    RazorscaleAvoidDevouringFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // isUseful() and Execute() run back to back on the same tick and ask the same question, so the
    // boss lookup, the radius and the patch search are done once and reused.
    struct FlameScan
    {
        uint32 atMs = 0;
        ObjectGuid flame;
        float clearRadius = 0.0f;
    };

    FlameScan const& Scan();
    bool StepClearOfFlames(Unit* flame, float clearRadius);

    FlameScan _scan;
};

class RazorscaleAvoidSentinelAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale avoid sentinel";

    RazorscaleAvoidSentinelAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleIgnoreBossAction : public AttackAction
{
public:
    static constexpr char const* Name = "razorscale ignore flying alone";

    RazorscaleIgnoreBossAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleAvoidWhirlwindAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale avoid whirlwind";

    RazorscaleAvoidWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleGroundedAction : public AttackAction
{
public:
    static constexpr char const* Name = "razorscale grounded";

    RazorscaleGroundedAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleHarpoonAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale harpoon action";

    RazorscaleHarpoonAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleFuseArmorAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale fuse armor action";

    RazorscaleFuseArmorAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Sole owner of the skull icon for this encounter: adds while she is airborne, the boss herself the
// moment she is on the floor. DpsTargetValue prefers the RTI target, so this is what the raid hits.
class RazorscaleKillTargetAction : public Action
{
public:
    static constexpr char const* Name = "razorscale kill target action";

    RazorscaleKillTargetAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// The generic pet-attack node is commented out engine-wide, so a pet keeps whatever it last hit
// unless a script hands it a new order.
class RazorscalePetControlAction : public Action
{
public:
    static constexpr char const* Name = "razorscale pet control action";

    RazorscalePetControlAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleFlameBreathAction : public MovementAction
{
public:
    static constexpr char const* Name = "razorscale flame breath action";

    RazorscaleFlameBreathAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
