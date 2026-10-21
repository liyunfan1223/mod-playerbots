/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUNGEONCORPSERUNTRIGGER_H
#define PLAYERBOTS_DUNGEONCORPSERUNTRIGGER_H

#include "Trigger.h"

class DungeonCorpseRunTrigger : public Trigger
{
public:
    DungeonCorpseRunTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dungeon corpse run") {}

    bool IsActive() override;
};

class DungeonCorpseRunCatchUpTrigger : public Trigger
{
public:
    DungeonCorpseRunCatchUpTrigger(PlayerbotAI* botAI) : Trigger(botAI, "dungeon corpse run catch up") {}

    bool IsActive() override;
};

#endif
