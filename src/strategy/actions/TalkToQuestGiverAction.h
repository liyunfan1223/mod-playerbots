/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TALKTOQUESTGIVERACTION_H
#define _PLAYERBOT_TALKTOQUESTGIVERACTION_H

#include "QuestAction.h"

class Quest;
class PlayerbotAI;
class WorldObject;

class TalkToQuestGiverAction : public QuestAction
{
    public:
        TalkToQuestGiverAction(PlayerbotAI* botAI, std::string name = "talk to quest giver") : QuestAction(botAI, name) { }

    protected:
        void ProcessQuest(Quest const* quest, Object* questGiver) override;
        void TurnInQuest(Quest const* quest, Object* questGiver, std::ostringstream& out);

    private:
        void RewardNoItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
        void RewardSingleItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
        std::set<uint32> BestRewards(Quest const* quest);
        void RewardMultipleItem(Quest const* quest, Object* questGiver, std::ostringstream& out);
        void AskToSelectReward(Quest const* quest, std::ostringstream& out, bool forEquip);
};

class TurnInQueryQuestAction : public TalkToQuestGiverAction
{
    public:
        TurnInQueryQuestAction(PlayerbotAI* botAI) : TalkToQuestGiverAction(botAI, "turn in query quest") {}
        bool Execute(Event event) override;
};
#endif
