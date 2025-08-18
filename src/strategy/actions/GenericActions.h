/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICACTIONS_H
#define _PLAYERBOT_GENERICACTIONS_H

#include "AttackAction.h"
#include "Action.h"
#include "PlayerbotAI.h"

class PlayerbotAI;

class MeleeAction : public AttackAction
{
public:
    MeleeAction(PlayerbotAI* botAI) : AttackAction(botAI, "melee") {}

    std::string const GetTargetName() override { return "current target"; }
    bool isUseful() override;
};

class TogglePetSpellAutoCastAction : public Action
{
public:
    TogglePetSpellAutoCastAction(PlayerbotAI* ai) : Action(ai, "toggle pet spell") {}
    virtual bool Execute(Event event) override;
};

class PetAttackAction : public Action
{
public:
    PetAttackAction(PlayerbotAI* ai) : Action(ai, "pet attack") {}
    virtual bool Execute(Event event) override;
};

class SetPetStanceAction : public Action
{
public:
    SetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "set pet stance") {}

    bool Execute(Event event) override;
};

#endif
