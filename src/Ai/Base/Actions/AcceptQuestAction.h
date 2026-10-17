/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACCEPTQUESTACTION_H
#define PLAYERBOTS_ACCEPTQUESTACTION_H

#include "QuestAction.h"

class Quest;
class PlayerbotAI;
class WorldObject;

class AcceptAllQuestsAction : public QuestAction
{
public:
    AcceptAllQuestsAction(PlayerbotAI* botAI, std::string const name = "accept all quests") : QuestAction(botAI, name)
    {
    }

protected:
    bool ProcessQuest(Quest const* quest, Object* questGiver) override;
};

class AcceptQuestAction : public AcceptAllQuestsAction
{
public:
    AcceptQuestAction(PlayerbotAI* botAI) : AcceptAllQuestsAction(botAI, "accept quest") {}
    bool Execute(Event event) override;
};

class AcceptQuestShareAction : public Action
{
public:
    AcceptQuestShareAction(PlayerbotAI* botAI) : Action(botAI, "accept quest share") {}
    bool Execute(Event event) override;
};

class ConfirmQuestAction : public Action {
public:
    ConfirmQuestAction(PlayerbotAI* botAI) : Action(botAI, "confirm quest") {}
    bool Execute(Event event);
};

#endif
