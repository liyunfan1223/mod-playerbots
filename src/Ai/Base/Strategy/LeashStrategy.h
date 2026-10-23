/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LEASHSTRATEGY_H
#define PLAYERBOTS_LEASHSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// Keeps a bot from wandering past Playerbots.LeashDistance from the group
// leader while an independent-movement strategy (new rpg, grind, ...) is also
// active. Those strategies propose their own movement actions at relevance
// 3.0-11.0 (see NewRpgStrategy::getDefaultActions, "the relevance should be
// greater than grind"), well above FollowMasterStrategy's default "follow"
// relevance of 1.0 -- so simply also enabling "follow" never wins and never
// actually leashes anything.
//
// This only proposes "follow" once the bot has already exceeded the leash
// distance (see "leash too far" in TriggerContext.h), at a relevance above
// every independent-movement action, so it has zero effect the rest of the
// time. It does not by itself stop new rpg from choosing a target beyond the
// leash distance again immediately after being recalled -- that half is
// handled separately, everywhere new rpg picks a destination: idle NPCs/GOs
// via PossibleNewRpgTargetsValue::AcceptUnit and
// PossibleNewRpgGameObjectsValue::Calculate (PossibleRpgTargetsValue.cpp),
// and grind/camp/quest destinations via WithinLeashRange
// (NewRpgBaseAction.cpp) -- so new rpg's own candidate search never proposes
// something this strategy would just have to fight.
class LeashStrategy : public Strategy
{
public:
    LeashStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "leash"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
