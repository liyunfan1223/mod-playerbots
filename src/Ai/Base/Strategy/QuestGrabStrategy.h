/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTGRABSTRATEGY_H
#define PLAYERBOTS_QUESTGRABSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// Lets a following bot handle quest givers, quest items and quest objects already within
// Playerbots.QuestGrabDistance, without picking destinations of its own. Opt-in: "nc +quest grab".
class QuestGrabStrategy : public Strategy
{
public:
    explicit QuestGrabStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "quest grab"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
