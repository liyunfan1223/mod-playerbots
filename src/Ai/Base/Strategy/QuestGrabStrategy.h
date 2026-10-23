/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTGRABSTRATEGY_H
#define PLAYERBOTS_QUESTGRABSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// Two rules, meant to replace relying on "leash" + "new rpg" for a bot that
// should just stick close and pick up whatever quest-relevant thing is
// already in reach: if something within Playerbots.QuestGrabDistance
// satisfies an incomplete quest right now, interact with it (GrabQuestItemAction);
// otherwise do nothing here and let plain "follow" (relevance 1.0, already
// on by default) keep the bot near the leader.
//
// Deliberately does not invent a destination the way "new rpg" does (no POI
// guessing, no synthetic averaged positions, no Z-height guessing) -- it only
// ever reacts to an object the bot can already see, at its real position, so
// the whole class of bugs that came from guessing a destination in advance
// doesn't apply here by construction.
//
// Opt-in via "nc +grab" rather than added to every bot's permanent baseline
// (AiFactory.cpp:585, where "follow"/"loot"/"quest" already live) -- this is
// new, unvalidated code; fold it into that baseline later once proven.
class QuestGrabStrategy : public Strategy
{
public:
    QuestGrabStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "grab"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
