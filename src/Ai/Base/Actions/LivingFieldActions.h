/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_FIELD_ACTIONS_H
#define PLAYERBOTS_LIVING_FIELD_ACTIONS_H

#include "MovementActions.h"
#include "Strategy.h"
#include "TravelMgr.h"
#include "Value.h"

struct LivingFieldPlan
{
    WorldPosition position;
    ObjectGuid::LowType spawn = 0;
    uint32 item = 0;
    uint32 target = 0;
    uint32 skill = 0;
    uint32 requiredSkill = 0;
    bool fishing = false;
    ObjectGuid pool;
};

struct LivingFieldState
{
    LivingFieldPlan plan;
    uint32 started = 0;
    ObjectGuid mainHand;
    ObjectGuid offHand;
    ObjectGuid pole;
};

namespace LivingProfessions
{
bool FieldReservedItem(PlayerbotAI* ai, ObjectGuid item);
}

class LivingFieldPlanValue : public CalculatedValue<LivingFieldPlan>
{
public:
    LivingFieldPlanValue(PlayerbotAI* ai) : CalculatedValue(ai, "living field plan", 60000) {}
    LivingFieldPlan Calculate() override;
};

class LivingFieldStateValue : public ManualSetValue<LivingFieldState>
{
public:
    LivingFieldStateValue(PlayerbotAI* ai) : ManualSetValue(ai, {}, "living field state") {}
};

class LivingFieldAction : public MovementAction
{
public:
    LivingFieldAction(PlayerbotAI* ai) : MovementAction(ai, "living fieldwork") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class LivingRestoreWeaponsAction : public Action
{
public:
    LivingRestoreWeaponsAction(PlayerbotAI* ai) : Action(ai, "living restore weapons") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class LivingFieldStrategy : public Strategy
{
public:
    LivingFieldStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "living fieldwork"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
