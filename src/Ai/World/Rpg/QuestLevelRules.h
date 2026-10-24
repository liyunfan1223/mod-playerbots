/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTLEVELRULES_H
#define PLAYERBOTS_QUESTLEVELRULES_H

// How many levels above the bot a quest may be before the bot refuses it. Shared by the NewRPG
// capable check and the quest-giver teleport index so both agree.
constexpr int32 QUEST_MAX_LEVELS_ABOVE_BOT = 3;

#endif
