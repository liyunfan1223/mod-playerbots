/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUESTACTION_H
#define _PLAYERBOT_QUESTACTION_H

#include "Action.h"
#include "Object.h"

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
        virtual void ProcessQuest(Quest const* quest, Object* questGiver) = 0;
        bool AcceptQuest(Quest const* quest, ObjectGuid questGiver);
        bool ProcessQuests(ObjectGuid questGiver);
        bool ProcessQuests(WorldObject* questGiver);
};

class QuestObjectiveCompletedAction : public Action
{
    public:
        QuestObjectiveCompletedAction(PlayerbotAI* botAI) : Action(botAI, "quest objective completed") { }

        bool Execute(Event event) override;
};

#endif
