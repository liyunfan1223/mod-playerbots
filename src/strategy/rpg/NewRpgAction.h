#ifndef _PLAYERBOT_NEWRPGACTION_H
#define _PLAYERBOT_NEWRPGACTION_H

#include "Duration.h"
#include "MovementActions.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Object.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "QuestDef.h"
#include "TravelMgr.h"
#include "PlayerbotAI.h"
#include "NewRpgBaseAction.h"

class TellRpgStatusAction : public Action
{
public:
    TellRpgStatusAction(PlayerbotAI* botAI) : Action(botAI, "rpg status") {}

    bool Execute(Event event) override;
};

class StartRpgDoQuestAction : public Action
{
public:
    StartRpgDoQuestAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do quest") {}

    bool Execute(Event event) override;
};

class NewRpgStatusUpdateAction : public NewRpgBaseAction
{
public:
    NewRpgStatusUpdateAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg status update")
    {
        // int statusCount = RPG_STATUS_END - 1;

        // transitionMat.resize(statusCount, std::vector<int>(statusCount, 0));

        // transitionMat[RPG_IDLE][RPG_GO_GRIND] = 20;
        // transitionMat[RPG_IDLE][RPG_GO_INNKEEPER] = 15;
        // transitionMat[RPG_IDLE][RPG_NEAR_NPC] = 30;
        // transitionMat[RPG_IDLE][RPG_DO_QUEST] = 35;
    }
    bool Execute(Event event) override;
protected:
    // static NewRpgStatusTransitionProb transitionMat;
    const int32 statusNearNpcDuration = 5 * 60 * 1000;
    const int32 statusNearRandomDuration = 5 * 60 * 1000;
    const int32 statusRestDuration = 30 * 1000;
    const int32 statusDoQuestDuration = 30 * 60 * 1000;
};

class NewRpgGoGrindAction : public NewRpgBaseAction
{
public:
    NewRpgGoGrindAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go grind") {}
    bool Execute(Event event) override;
};

class NewRpgGoInnKeeperAction : public NewRpgBaseAction
{
public:
    NewRpgGoInnKeeperAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go innkeeper") {}
    bool Execute(Event event) override;
};


class NewRpgMoveRandomAction : public NewRpgBaseAction
{
public:
    NewRpgMoveRandomAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg move random") {}
    bool Execute(Event event) override;
};

class NewRpgMoveNpcAction : public NewRpgBaseAction
{
public:
    NewRpgMoveNpcAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg move npcs") {}
    bool Execute(Event event) override;

    const uint32 npcStayTime = 8 * 1000;
};

class NewRpgDoQuestAction : public NewRpgBaseAction
{
public:
    NewRpgDoQuestAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do quest") {}
    bool Execute(Event event) override;
protected:
    bool DoIncompleteQuest();
    bool DoCompletedQuest();
    
    const uint32 poiStayTime = 5 * 60 * 1000;
};

#endif