/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTGIVERNEARBYVALUE_H
#define PLAYERBOTS_QUESTGIVERNEARBYVALUE_H

#include "Value.h"

class PlayerbotAI;

class QuestGiverNearbyValue : public BoolCalculatedValue
{
public:
    // The search walks the nearby NPC/GO caches and builds quest menus, so cache it.
    QuestGiverNearbyValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "quest giver nearby", 500) {}

    bool Calculate() override;
};

#endif
