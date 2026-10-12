/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTGIVERTOINTERACTVALUE_H
#define PLAYERBOTS_QUESTGIVERTOINTERACTVALUE_H

#include "Value.h"

class PlayerbotAI;

class QuestGiverToInteractValue : public ObjectGuidCalculatedValue
{
public:
    // The search walks the nearby NPC/GO caches and builds quest menus, so cache it.
    QuestGiverToInteractValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "quest giver to interact", 500) {}

    ObjectGuid Calculate() override;
};

#endif
