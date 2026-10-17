/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FURYWARRIORSTRATEGY_H
#define PLAYERBOTS_FURYWARRIORSTRATEGY_H

#include "GenericWarriorStrategy.h"

class PlayerbotAI;

class FuryWarriorStrategy : public GenericWarriorStrategy
{
public:
    FuryWarriorStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fury"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
};

#endif
