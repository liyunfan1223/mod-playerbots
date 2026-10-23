/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LeashStrategy.h"

void LeashStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // 15.0f beats every independent-movement action in the codebase today
    // (new rpg: 3.0-11.0; grind is lower still per NewRpgStrategy's own
    // comment) without hardcoding a value tied to any one of them, so it
    // stays correct if those get retuned later. "leash too far" only
    // activates past Playerbots.LeashDistance, so "follow" is not even
    // proposed -- let alone competing with anything -- while the bot is
    // within range.
    triggers.push_back(new TriggerNode("leash too far", { NextAction("follow", 15.0f) }));
}
