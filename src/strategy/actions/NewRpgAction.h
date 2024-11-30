#ifndef _PLAYERBOT_NEWRPGACTION_H
#define _PLAYERBOT_NEWRPGACTION_H

#include "Duration.h"
#include "MovementActions.h"
#include "NewRpgStrategy.h"
#include "TravelMgr.h"
#include "PlayerbotAI.h"

class TellRpgStatusAction : public Action
{
public:
    TellRpgStatusAction(PlayerbotAI* botAI) : Action(botAI, "rpg status") {}

    bool Execute(Event event) override;
};

class NewRpgStatusUpdateAction : public Action
{
public:
    NewRpgStatusUpdateAction(PlayerbotAI* botAI) : Action(botAI, "new rpg status update") {}
    bool Execute(Event event) override;
protected:
    const int32 setGrindInterval = 5 * 60 * 1000;
    const int32 setNpcInterval = 5 * 60 * 1000;
    WorldPosition SelectRandomGrindPos();
};

class NewRpgGoFarAwayPosAction : public MovementAction
{
public:
    NewRpgGoFarAwayPosAction(PlayerbotAI* botAI, std::string name) : MovementAction(botAI, name) {}
    // bool Execute(Event event) override;
    bool MoveFarTo(WorldPosition dest);

protected:
    // WorldPosition dest;
    float pathFinderDis = 70.0f; // path finder
};

class NewRpgGoGrindAction : public NewRpgGoFarAwayPosAction
{
public:
    NewRpgGoGrindAction(PlayerbotAI* botAI) : NewRpgGoFarAwayPosAction(botAI, "new rpg go grind") {}
    bool Execute(Event event) override;
};

class NewRpgMoveRandomAction : public MovementAction
{
public:
    NewRpgMoveRandomAction(PlayerbotAI* botAI) : MovementAction(botAI, "new rpg move random") {}
    bool Execute(Event event) override;
protected:
    const float moveStep = 50.0f;
};

#endif