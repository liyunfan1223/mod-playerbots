/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AUTOTELEPORTFORLEVELACTION_H
#define _PLAYERBOT_AUTOTELEPORTFORLEVELACTION_H

#include "Action.h"

class PlayerbotAI;

class AutoMaintenanceOnLevelupAction : public Action
{
public:
    AutoMaintenanceOnLevelupAction(PlayerbotAI* botAI, std::string const name = "auto maintenance on levelup")
        : Action(botAI, name)
    {
    }

    bool Execute(Event event);

protected:
    void AutoTeleportForLevel();
    void AutoPickTalents();
    void AutoLearnSpell();
    void AutoUpgradeEquip();
    void LearnSpells(std::ostringstream* out);
    void LearnTrainerSpells(std::ostringstream* out);
    void LearnQuestSpells(std::ostringstream* out);
    void LearnSpell(uint32 spellId, std::ostringstream* out);
    std::string const FormatSpell(SpellInfo const* sInfo);
};

#endif
