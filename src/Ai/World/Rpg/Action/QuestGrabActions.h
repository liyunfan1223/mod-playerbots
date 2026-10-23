/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTGRABACTIONS_H
#define PLAYERBOTS_QUESTGRABACTIONS_H

#include "MovementActions.h"
#include "NewRpgBaseAction.h"

// "quest grab" actions: they only react to things within Playerbots.QuestGrabDistance of the bot.

// Accept or turn in quests at a nearby quest giver.
class QuestGrabGiverAction : public NewRpgBaseAction
{
public:
    explicit QuestGrabGiverAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "quest grab giver") {}
    bool Execute(Event event) override;
};

// Use a carried quest item on a nearby creature or gameobject an incomplete objective still needs.
class QuestGrabUseItemAction : public MovementAction
{
public:
    explicit QuestGrabUseItemAction(PlayerbotAI* botAI) : MovementAction(botAI, "quest grab use item") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Use a carried quest item with no target whose spell creates an item a quest needs
// (e.g. Jade Phial -> Filled Jade Phial near a spell focus).
class QuestGrabCreateItemAction : public MovementAction
{
public:
    explicit QuestGrabCreateItemAction(PlayerbotAI* botAI) : MovementAction(botAI, "quest grab create item") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Walk to and use the nearest quest-relevant gameobject. Chests are handed to the loot pipeline.
class QuestGrabObjectAction : public MovementAction
{
public:
    explicit QuestGrabObjectAction(PlayerbotAI* botAI) : MovementAction(botAI, "quest grab object") {}
    bool Execute(Event event) override;
};

#endif
