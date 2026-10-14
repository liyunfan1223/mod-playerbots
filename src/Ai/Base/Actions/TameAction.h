/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TAMEACTION_H
#define PLAYERBOTS_TAMEACTION_H

#include "Action.h"
#include "PlayerbotFactory.h"
#include <string>

class PlayerbotAI;

class TameAction : public Action
{
public:
    TameAction(PlayerbotAI* botAI) : Action(botAI, "tame") {}

    bool Execute(Event event) override;

private:
    bool SetPetByName(std::string const& name);
    bool SetPetById(uint32 id);
    bool SetPetByFamily(std::string const& family);
    bool RenamePet(std::string const& newName);
    bool CreateAndSetPet(uint32 creatureEntry);
    bool AbandonPet();

    std::string lastPetName;
    uint32 lastPetId = 0;
};

#endif
