/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_PROCESSING_ACTIONS_H
#define PLAYERBOTS_LIVING_PROCESSING_ACTIONS_H

#include "Action.h"
#include "Value.h"

namespace LivingProfessions
{
bool IsProcessingLoot(Player* player, ObjectGuid guid);
}

struct LivingProcessingPlan
{
    uint32 spell = 0;
    ObjectGuid item;
};

class LivingProcessingValue : public CalculatedValue<LivingProcessingPlan>
{
public:
    LivingProcessingValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "living processing", 30000) {}
    LivingProcessingPlan Calculate() override;
};

class LivingProcessingAction : public Action
{
public:
    LivingProcessingAction(PlayerbotAI* botAI) : Action(botAI, "living processing") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

#endif
