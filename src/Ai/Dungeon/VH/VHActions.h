/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHACTIONS_H
#define PLAYERBOTS_VHACTIONS_H

#include "Action.h"
#include "AttackAction.h"

class AttackErekemAction : public AttackAction
{
public:
    AttackErekemAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack erekem") {}
    bool Execute(Event event) override;
};

class AttackIchorGlobuleAction : public AttackAction
{
public:
    AttackIchorGlobuleAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack ichor globule") {}
    bool Execute(Event event) override;
};

class AttackVoidSentryAction : public AttackAction
{
public:
    AttackVoidSentryAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack void sentry") {}
    bool Execute(Event event) override;
};

#endif
