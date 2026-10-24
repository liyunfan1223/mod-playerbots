/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEACTIONS_SPARKS_H
#define PLAYERBOTS_EOEACTIONS_SPARKS_H

#include "Action.h"
#include "AttackAction.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"

// P1: the DK half of the Power Spark answer, Death Grip and then Chains of Ice on what it pulled.
class PullPowerSparkAction : public Action
{
public:
    static constexpr char const* Name = "malygos pull power spark";

    PullPowerSparkAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    ObjectGuid _sparkGuid;
    bool _grip = false;
};

class KillPowerSparkAction : public AttackAction
{
public:
    static constexpr char const* Name = "malygos kill power spark";

    KillPowerSparkAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    ObjectGuid _sparkGuid;
};

#endif
