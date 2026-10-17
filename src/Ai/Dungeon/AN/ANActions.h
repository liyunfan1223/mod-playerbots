/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANACTIONS_H
#define PLAYERBOTS_ANACTIONS_H

#include "ANTriggers.h"
#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class AttackWebWrapAction : public AttackAction
{
public:
    AttackWebWrapAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack web wrap") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class WatchersTargetAction : public AttackAction
{
public:
    WatchersTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "krik'thir priority") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AnubarakDodgePoundAction : public AttackAction
{
public:
    AnubarakDodgePoundAction(PlayerbotAI* botAI) : AttackAction(botAI, "anub'arak dodge pound") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
