/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AUTOLEARNSPELLACTION_H
#define _PLAYERBOT_AUTOLEARNSPELLACTION_H

#include "Action.h"

class PlayerbotAI;

class AutoLearnSpellAction : public Action
{
public:
    AutoLearnSpellAction(PlayerbotAI* botAI, std::string const name = "auto learn spell") : Action(botAI, name) {}

    bool Execute(Event event);

private:
    void LearnSpells(std::ostringstream* out);
    void LearnTrainerSpells(std::ostringstream* out);
    void LearnQuestSpells(std::ostringstream* out);
    void LearnSpell(uint32 spellId, std::ostringstream* out);
};

class AutoUpgradeEquipAction : public Action
{
public:
    AutoUpgradeEquipAction(PlayerbotAI* botAI, std::string const name = "auto upgrade equip") : Action(botAI, name) {}

    bool Execute(Event event);
};

#endif
