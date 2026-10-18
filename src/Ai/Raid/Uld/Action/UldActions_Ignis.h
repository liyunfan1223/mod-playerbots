/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_IGNIS_H
#define PLAYERBOTS_ULDACTIONS_IGNIS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldTriggers.h"
#include "Vehicle.h"

// Ignis the Furnace Master
class IgnisScorchedGroundAction : public MovementAction
{
public:
    static constexpr char const* Name = "ignis scorched ground action";

    IgnisScorchedGroundAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Holds the boss on a three-slot arc around the room centre, which picks where every Scorch patch lands.
class IgnisMainTankPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "ignis main tank position action";

    IgnisMainTankPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool _spotReached = false;
};

// Walks an activated construct through the only chain that kills it: Scorched Ground until it turns
// Molten, then this tank's own water pool until it turns Brittle.
class IgnisConstructTankAction : public AttackAction
{
public:
    static constexpr char const* Name = "ignis construct tank action";

    IgnisConstructTankAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// One hit of 5000 (10-man) / 3000 (25-man) shatters a Brittle construct, the only way it dies.
class IgnisAttackBrittleConstructAction : public AttackAction
{
public:
    static constexpr char const* Name = "ignis attack brittle construct action";

    IgnisAttackBrittleConstructAction(PlayerbotAI* botAI)
        : AttackAction(botAI, Name)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IgnisAttackBossAction : public AttackAction
{
public:
    static constexpr char const* Name = "ignis attack boss action";

    IgnisAttackBossAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IgnisMoltenConstructAvoidAction : public MovementAction
{
public:
    static constexpr char const* Name = "ignis molten construct avoid action";

    IgnisMoltenConstructAvoidAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Flame Jets knocks the whole raid back and locks casting for six seconds afterwards, so a cast that
// cannot land before it hits is thrown away either way.
class IgnisFlameJetsHoldCastAction : public Action
{
public:
    static constexpr char const* Name = "ignis flame jets hold cast action";

    IgnisFlameJetsHoldCastAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IgnisSlagPotHealAction : public Action
{
public:
    static constexpr char const* Name = "ignis slag pot heal action";

    IgnisSlagPotHealAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
