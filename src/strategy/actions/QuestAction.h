/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUESTACTION_H
#define _PLAYERBOT_QUESTACTION_H

#include "Action.h"
#include "Object.h"
#include "QuestDef.h"

class ObjectGuid;
class Quest;
class Player;
class PlayerbotAI;
class WorldObject;
class Object;

class QuestAction : public Action
{
public:
    QuestAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) { }
    bool Execute(Event event) override;

protected:
    bool CompleteQuest(Player* player, uint32 entry);
    virtual bool ProcessQuest(Quest const* quest, Object* questGiver) = 0;
    bool AcceptQuest(Quest const* quest, ObjectGuid questGiver);
    bool ProcessQuests(ObjectGuid questGiver);
    bool ProcessQuests(WorldObject* questGiver);
};

class QuestUpdateCompleteAction : public Action
{
public:
    QuestUpdateCompleteAction(PlayerbotAI* ai) : Action(ai, "quest update complete") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddKillAction : public Action
{
public:
    QuestUpdateAddKillAction(PlayerbotAI* ai) : Action(ai, "quest update add kill") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddItemAction : public Action
{
public:
    QuestUpdateAddItemAction(PlayerbotAI* ai) : Action(ai, "quest update add item") {}
    bool Execute(Event event) override;;
};

class QuestUpdateFailedAction : public Action
{
public:
    QuestUpdateFailedAction(PlayerbotAI* ai) : Action(ai, "quest update failed") {}
    bool Execute(Event event) override;
};

class QuestUpdateFailedTimerAction : public Action
{
public:
    QuestUpdateFailedTimerAction(PlayerbotAI* ai) : Action(ai, "quest update failed timer") {}
    bool Execute(Event event) override;
};

#endif
