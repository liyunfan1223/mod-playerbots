/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PETACTION_H
#define _PLAYERBOT_PETACTION_H

#include <string>

#include "Action.h"
#include "PlayerbotFactory.h"

class PlayerbotAI;

class PetAction : public Action
{
public:
    PetAction(PlayerbotAI* botAI) : Action(botAI, "pet") {}

    bool Execute(Event event) override;

private:
    bool SetPetByName(const std::string& name);
    bool SetPetById(uint32 id);
    bool SetPetByFamily(const std::string& family);
    bool RenamePet(const std::string& newName);

    bool CreateAndSetPet(uint32 creatureEntry);

    std::string lastPetName;
    uint32 lastPetId = 0;
};
#endif
