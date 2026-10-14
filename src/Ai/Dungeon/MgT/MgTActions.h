/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTACTIONS_H
#define PLAYERBOTS_MGTACTIONS_H

#include "AttackAction.h"
#include "MgTShared.h"
#include "MovementActions.h"

class MgTEscapeAction : public MovementAction
{
public:
    MgTEscapeAction(PlayerbotAI* botAI, std::string const name, std::string const value)
        : MovementAction(botAI, name), _value(value)
    {
    }

    bool Execute(Event event) override;

private:
    std::string const _value;
};

class MgTLeaveDampeningFieldAction : public MgTEscapeAction
{
public:
    MgTLeaveDampeningFieldAction(PlayerbotAI* botAI)
        : MgTEscapeAction(botAI, "mgt leave dampening field", "mgt dampening escape")
    {
    }
};

class MgTClearArcaneNovaAction : public MgTEscapeAction
{
public:
    MgTClearArcaneNovaAction(PlayerbotAI* botAI)
        : MgTEscapeAction(botAI, "mgt clear arcane nova", "mgt nova escape")
    {
    }
};

class MgTLeaveFlameStrikeAction : public MgTEscapeAction
{
public:
    MgTLeaveFlameStrikeAction(PlayerbotAI* botAI)
        : MgTEscapeAction(botAI, "mgt leave flame strike", "mgt flame strike escape")
    {
    }
};

class MgTLeavePhoenixBurnAction : public MgTEscapeAction
{
public:
    MgTLeavePhoenixBurnAction(PlayerbotAI* botAI)
        : MgTEscapeAction(botAI, "mgt leave phoenix burn", "mgt phoenix escape")
    {
    }
};

class MgTFocusAction : public AttackAction
{
public:
    MgTFocusAction(PlayerbotAI* botAI, std::string const name, std::string const value)
        : AttackAction(botAI, name), _value(value)
    {
    }

    bool Execute(Event event) override;

private:
    std::string const _value;
};

class MgTFocusTargetAction : public MgTFocusAction
{
public:
    MgTFocusTargetAction(PlayerbotAI* botAI) : MgTFocusAction(botAI, "mgt focus target", "mgt focus target") {}
};

class MgTDelrissaFocusTargetAction : public MgTFocusAction
{
public:
    MgTDelrissaFocusTargetAction(PlayerbotAI* botAI)
        : MgTFocusAction(botAI, "mgt delrissa focus target", "mgt delrissa focus target")
    {
    }
};

class MgTKaelFocusTargetAction : public MgTFocusAction
{
public:
    MgTKaelFocusTargetAction(PlayerbotAI* botAI)
        : MgTFocusAction(botAI, "mgt kael focus target", "mgt kael focus target")
    {
    }
};

class MgTKillCrystalAction : public AttackAction
{
public:
    MgTKillCrystalAction(PlayerbotAI* botAI) : AttackAction(botAI, "mgt kill crystal") {}
    bool Execute(Event event) override;
};

class MgTReturnToRoomAction : public MovementAction
{
public:
    MgTReturnToRoomAction(PlayerbotAI* botAI) : MovementAction(botAI, "mgt return to room") {}
    bool Execute(Event event) override;
};

class MgTCloseOnMageGuardAction : public MovementAction
{
public:
    MgTCloseOnMageGuardAction(PlayerbotAI* botAI) : MovementAction(botAI, "mgt close on mage guard") {}
    bool Execute(Event event) override;
};

class MgTPriorityInterruptAction : public Action
{
public:
    MgTPriorityInterruptAction(PlayerbotAI* botAI) : Action(botAI, "mgt priority interrupt") {}
    bool Execute(Event event) override;
};

class MgTTauntEnragedWretchedAction : public Action
{
public:
    MgTTauntEnragedWretchedAction(PlayerbotAI* botAI) : Action(botAI, "mgt taunt enraged wretched") {}
    bool Execute(Event event) override;
};

class MgTDelrissaInterruptAction : public Action
{
public:
    MgTDelrissaInterruptAction(PlayerbotAI* botAI) : Action(botAI, "mgt delrissa interrupt") {}
    bool Execute(Event event) override;
};

class MgTDelrissaTremorTotemAction : public Action
{
public:
    MgTDelrissaTremorTotemAction(PlayerbotAI* botAI) : Action(botAI, "mgt delrissa tremor totem") {}
    bool Execute(Event event) override;
};

class MgTKaelInterruptAction : public Action
{
public:
    MgTKaelInterruptAction(PlayerbotAI* botAI) : Action(botAI, "mgt kael interrupt") {}
    bool Execute(Event event) override;
};

class MgTTakeLapseSpotAction : public MovementAction
{
public:
    MgTTakeLapseSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, "mgt take lapse spot") {}
    bool Execute(Event event) override;

private:
    MagistersTerrace::KiteState _kite;
};

#endif
