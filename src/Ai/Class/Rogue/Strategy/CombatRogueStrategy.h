/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COMBATROGUESTRATEGY_H
#define PLAYERBOTS_COMBATROGUESTRATEGY_H

#include "GenericRogueStrategy.h"

class PlayerbotAI;

class CombatRogueStrategy : public GenericRogueStrategy
{
public:
    CombatRogueStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "combat"; }
    std::vector<NextAction> getDefaultActions() override;
};

#endif
