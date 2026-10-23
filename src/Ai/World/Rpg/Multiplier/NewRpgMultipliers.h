/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGMULTIPLIERS_H
#define PLAYERBOTS_NEWRPGMULTIPLIERS_H

#include "Multiplier.h"

// Drops "attack anything" while a giver the current RPG status would visit is nearby, so the status action runs.
class NewRpgQuestGiverMultiplier : public Multiplier
{
public:
    NewRpgQuestGiverMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "new rpg quest giver") {}

    float GetValue(Action* action) override;
};

#endif
