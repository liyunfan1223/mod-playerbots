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
        TalkToQuestGiverAction(PlayerbotAI* botAI) : QuestAction(botAI, "talk to quest giver") { }

    protected:
        void ProcessQuest(Quest const* quest, WorldObject* questGiver) override;

    private:
        void TurnInQuest(Quest const* quest, WorldObject* questGiver, std::ostringstream& out);
        void RewardNoItem(Quest const* quest, WorldObject* questGiver, std::ostringstream& out);
        void RewardSingleItem(Quest const* quest, WorldObject* questGiver, std::ostringstream& out);
        std::set<uint32> BestRewards(Quest const* quest);
        void RewardMultipleItem(Quest const* quest, WorldObject* questGiver, std::ostringstream& out);
        void AskToSelectReward(Quest const* quest, std::ostringstream& out, bool forEquip);
};

#endif
