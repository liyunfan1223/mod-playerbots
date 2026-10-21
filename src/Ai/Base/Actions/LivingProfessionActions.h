/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_PROFESSION_ACTIONS_H
#define PLAYERBOTS_LIVING_PROFESSION_ACTIONS_H

#include "MovementActions.h"
#include "NonCombatStrategy.h"
#include "Trainer.h"
#include "Value.h"

class SpellInfo;
namespace LivingProfessions
{
bool CanWork(PlayerbotAI* ai);
// False for profession ranks and recipes beyond the current era (also used by the bot factory).
bool WithinEra(Trainer::Spell const& lesson);
bool IsProfessionRecipe(Player* player, SpellInfo const* spell);
bool NeedsProduct(Player* player, SpellInfo const* spell);
}  // namespace LivingProfessions

struct LivingTrainingPlan
{
    ObjectGuid trainer;
    uint32 spell = 0;
};

struct LivingCraftPlan
{
    uint32 spell = 0;
    ObjectGuid item;
};

class LivingTrainingValue : public CalculatedValue<LivingTrainingPlan>
{
public:
    LivingTrainingValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "living training", 60) {}
    LivingTrainingPlan Calculate() override;
};

class LivingCraftValue : public CalculatedValue<LivingCraftPlan>
{
public:
    LivingCraftValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "living craft", 30) {}
    LivingCraftPlan Calculate() override;
};

class LivingTrainAction : public MovementAction
{
public:
    LivingTrainAction(PlayerbotAI* botAI) : MovementAction(botAI, "living train") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class LivingCraftAction : public Action
{
public:
    LivingCraftAction(PlayerbotAI* botAI) : Action(botAI, "living craft") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class LivingProfessionStrategy : public NonCombatStrategy
{
public:
    LivingProfessionStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "living professions"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
