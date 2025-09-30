/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHOOSETARGETACTIONS_H
#define _PLAYERBOT_CHOOSETARGETACTIONS_H

#include "AttackAction.h"

class PlayerbotAI;

class DpsAoeAction : public AttackAction
{
public:
    DpsAoeAction(PlayerbotAI* botAI) : AttackAction(botAI, "dps aoe") {}

    std::string const GetTargetName() override { return "dps aoe target"; }
};

class DpsAssistAction : public AttackAction
{
public:
    DpsAssistAction(PlayerbotAI* botAI) : AttackAction(botAI, "dps assist") {}

    std::string const GetTargetName() override { return "dps target"; }
    bool isUseful() override;
};

class TankAssistAction : public AttackAction
{
public:
    TankAssistAction(PlayerbotAI* botAI) : AttackAction(botAI, "tank assist") {}

    std::string const GetTargetName() override { return "tank target"; }
};

class AttackAnythingAction : public AttackAction
{
public:
    AttackAnythingAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack anything") {}

    std::string const GetTargetName() override { return "grind target"; }
    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class AttackLeastHpTargetAction : public AttackAction
{
public:
    AttackLeastHpTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack least hp target") {}

    std::string const GetTargetName() override { return "least hp target"; }
};

class AttackEnemyPlayerAction : public AttackAction
{
public:
    AttackEnemyPlayerAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack enemy player") {}

    std::string const GetTargetName() override { return "enemy player target"; }
    bool isUseful() override;
};

class AttackRtiTargetAction : public AttackAction
{
public:
    AttackRtiTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack rti target") {}

    std::string const GetTargetName() override { return "rti target"; }
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AttackEnemyFlagCarrierAction : public AttackAction
{
public:
    AttackEnemyFlagCarrierAction(PlayerbotAI* botAI) : AttackAction(botAI, "attack enemy flag carrier") {}

    std::string const GetTargetName() override { return "enemy flag carrier"; }
    bool isUseful() override;
};

class DropTargetAction : public Action
{
public:
    DropTargetAction(PlayerbotAI* botAI) : Action(botAI, "drop target") {}

    bool Execute(Event event) override;
};

#endif
