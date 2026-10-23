/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPACTIONS_H
#define PLAYERBOTS_RAMPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Watchkeeper Gargolmar

class GargolmarMarkHellfireWatchersAction : public Action
{
public:
    GargolmarMarkHellfireWatchersAction(PlayerbotAI* botAI)
        : Action(botAI, "gargolmar mark hellfire watchers")
    {
    }
    bool Execute(Event event) override;
};

// Omor the Unscarred

class OmorTreacheryAuraFleeFromPlayersAction : public MovementAction
{
public:
    OmorTreacheryAuraFleeFromPlayersAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "omor treachery aura flee from players")
    {
    }
    bool Execute(Event event) override;
};

class OmorTreacheryAuraFleeFromTankAction : public MovementAction
{
public:
    OmorTreacheryAuraFleeFromTankAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "omor treachery aura flee from tank")
    {
    }
    bool Execute(Event event) override;
};

class OmorRangedSpreadAction : public MovementAction
{
public:
    OmorRangedSpreadAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "omor ranged spread")
    {
    }
    bool Execute(Event event) override;
};

class OmorMarkFiendishHoundAction : public Action
{
public:
    OmorMarkFiendishHoundAction(PlayerbotAI* botAI)
        : Action(botAI, "omor mark fiendish hound")
    {
    }
    bool Execute(Event event) override;
};

// Vazruden & Nazan

class VazrudenTankPositionBossAction : public AttackAction
{
public:
    VazrudenTankPositionBossAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "vazruden tank position boss")
    {
    }
    bool Execute(Event event) override;
};

class VazrudenMarkBossAction : public Action
{
public:
    VazrudenMarkBossAction(PlayerbotAI* botAI)
        : Action(botAI, "vazruden mark boss")
    {
    }
    bool Execute(Event event) override;
};

class NazanSetTremorTotemAction : public Action
{
public:
    NazanSetTremorTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "nazan set tremor totem")
    {
    }
    bool Execute(Event event) override;
};

class NazanSetFireResistanceTotemAction : public Action
{
public:
    NazanSetFireResistanceTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "nazan set fire resistance totem")
    {
    }
    bool Execute(Event event) override;
};

class NazanSetFireResistanceAuraAction : public Action
{
public:
    NazanSetFireResistanceAuraAction(PlayerbotAI* botAI) : Action(botAI, "nazan set fire resistance aura")
    {
    }
    bool Execute(Event event) override;
};

#endif
