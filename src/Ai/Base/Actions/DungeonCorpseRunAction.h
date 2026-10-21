/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DUNGEONCORPSERUNACTION_H
#define PLAYERBOTS_DUNGEONCORPSERUNACTION_H

#include "Action.h"

namespace DungeonCorpseRun
{
bool IsEligible(PlayerbotAI* botAI);
bool ShouldRelease(PlayerbotAI* botAI);
bool PrepareRelease(PlayerbotAI* botAI);
bool IsPending(PlayerbotAI* botAI);
}  // namespace DungeonCorpseRun

class DungeonCorpseRunAction : public Action
{
public:
    DungeonCorpseRunAction(PlayerbotAI* botAI) : Action(botAI, "dungeon corpse run") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class DungeonCorpseRunCatchUpAction : public Action
{
public:
    DungeonCorpseRunCatchUpAction(PlayerbotAI* botAI) : Action(botAI, "dungeon corpse run catch up") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
