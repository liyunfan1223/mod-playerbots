/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_KOLOGARN_H
#define PLAYERBOTS_ULDACTIONS_KOLOGARN_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class KologarnBodyTankAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn body tank action";

    KologarnBodyTankAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnOffTankAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn off tank action";

    KologarnOffTankAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnRubbleTankAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn rubble tank action";

    KologarnRubbleTankAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnDpsTargetAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn dps target action";

    KologarnDpsTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnSmashSwapAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn smash swap action";

    KologarnSmashSwapAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnBodyUncoveredAction : public AttackAction
{
public:
    static constexpr char const* Name = "kologarn body uncovered action";

    KologarnBodyUncoveredAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnFallFromFloorAction : public Action
{
public:
    static constexpr char const* Name = "kologarn fall from floor action";

    KologarnFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnRubbleSlowdownAction : public Action
{
public:
    static constexpr char const* Name = "kologarn rubble slowdown action";

    KologarnRubbleSlowdownAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class KologarnEyebeamAction : public MovementAction
{
public:
    static constexpr char const* Name = "kologarn eyebeam action";

    KologarnEyebeamAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
