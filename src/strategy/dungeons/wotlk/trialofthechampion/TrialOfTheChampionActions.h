#ifndef _PLAYERBOT_WOTLKDUNGEONTOCACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONTOCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "TrialOfTheChampionTriggers.h"
#include "MovementActions.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "EquipAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "StatsWeightCalculator.h"

class ToCLanceAction : public AttackAction
{
public:
    ToCLanceAction(PlayerbotAI* ai) : AttackAction(ai, "toc lance") {}
    bool Execute(Event event) override;
};

class ToCUELanceAction : public AttackAction
{
public:
    ToCUELanceAction(PlayerbotAI* ai) : AttackAction(ai, "toc ue lance") {}
    bool Execute(Event event) override;
};

class ToCMountedAction : public Action
{
public:
    ToCMountedAction(PlayerbotAI* botAI, std::string const name = "toc mounted")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class ToCMountAction : public MovementAction
{
public:
    ToCMountAction(PlayerbotAI* botAI, std::string const name = "toc mount")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class ToCEadricAction : public MovementAction
{
public:
    ToCEadricAction(PlayerbotAI* botAI, std::string const name = "toc eadric")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
#endif
