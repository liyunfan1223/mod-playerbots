/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_OUTFITACTION_H
#define _PLAYERBOT_OUTFITACTION_H

#include "ChatHelper.h"
#include "EquipAction.h"

class PlayerbotAI;

class OutfitAction : public EquipAction
{
public:
    OutfitAction(PlayerbotAI* botAI) : EquipAction(botAI, "outfit") {}

    bool Execute(Event event) override;

private:
    void List();
    void Save(std::string const name, ItemIds outfit);
    void Update(std::string const name);
};

#endif
