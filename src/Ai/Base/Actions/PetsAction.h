/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PETSACTION_H
#define PLAYERBOTS_PETSACTION_H

#include "Action.h"
#include <string>

class PlayerbotAI;

class PetsAction : public Action
{
public:
    PetsAction(PlayerbotAI* botAI, std::string const& defaultCmd = "") : Action(botAI, "pet"), defaultCmd(defaultCmd) {}

    bool Execute(Event event) override;

private:
    std::string defaultCmd;
};

class TogglePetSpellAutoCastAction : public Action
{
public:
    TogglePetSpellAutoCastAction(PlayerbotAI* botAI) : Action(botAI, "toggle pet spell") {}
    virtual bool Execute(Event event) override;
};

class PetAttackAction : public Action
{
public:
    PetAttackAction(PlayerbotAI* botAI) : Action(botAI, "pet attack") {}
    virtual bool Execute(Event event) override;
};

class SetPetStanceAction : public Action
{
public:
    SetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "set pet stance") {}

    bool Execute(Event event) override;
};

#endif
