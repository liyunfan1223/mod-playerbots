/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BROADCASTHELPER_H
#define PLAYERBOTS_BROADCASTHELPER_H

#include "Define.h"
#include <cstdint>
#include <list>
#include <string>
#include <utility>
#include <vector>

class PlayerbotAI;
class Player;
class Quest;
class Creature;
class Group;

struct ItemTemplate;

class BroadcastHelper
{
public:
    BroadcastHelper();

public:
    enum ToChannel
    {
        TO_GUILD = 1,
        TO_WORLD = 2,
        TO_GENERAL = 3,
        TO_TRADE = 4,
        TO_LOOKING_FOR_GROUP = 5,
        TO_LOCAL_DEFENSE = 6,
        TO_WORLD_DEFENSE = 7,
        TO_GUILD_RECRUITMENT = 8
    };

    static uint8_t GetLocale();
    static bool BroadcastTest(
        PlayerbotAI* botAI,
        Player* bot
    );
    static bool BroadcastToChannelWithGlobalChance(
        PlayerbotAI* botAI,
        std::string message,
        std::list<std::pair<ToChannel, uint32_t>> toChannels
    );
    static bool BroadcastLootingItem(
        PlayerbotAI* botAI,
        Player* bot,
        ItemTemplate const* proto
    );
    static bool BroadcastQuestAccepted(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestUpdateAddKill(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest,
        uint32_t availableCount,
        uint32_t requiredCount,
        std::string obectiveName
    );
    static bool BroadcastQuestUpdateAddItem(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest,
        uint32_t availableCount,
        uint32_t requiredCount,
        ItemTemplate const* proto
    );
    static bool BroadcastQuestUpdateFailedTimer(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestUpdateComplete(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestTurnedIn(
        PlayerbotAI* botAI,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastKill(
        PlayerbotAI* botAI,
        Player* bot,
        Creature* creature
    );
    static bool BroadcastLevelup(
        PlayerbotAI* botAI,
        Player* bot
    );
    static bool BroadcastGuildMemberPromotion(
        PlayerbotAI* botAI,
        Player* bot,
        Player* player
    );
    static bool BroadcastGuildMemberDemotion(
        PlayerbotAI* botAI,
        Player* bot,
        Player* player
    );
    static bool BroadcastGuildGroupOrRaidInvite(
        PlayerbotAI* botAI,
        Player* bot,
        Player* player,
        Group* group
    );
    static bool BroadcastSuggestInstance(
        PlayerbotAI* botAI,
        std::vector<std::string>& allowedInstances,
        Player* bot
    );
    static bool BroadcastSuggestQuest(
        PlayerbotAI* botAI,
        std::vector<uint32>& quests,
        Player* bot
    );
    static bool BroadcastSuggestGrindMaterials(
        PlayerbotAI* botAI,
        std::string item,
        Player* bot
    );
    static bool BroadcastSuggestGrindReputation(
        PlayerbotAI* botAI,
        std::vector<std::string> levels,
        std::vector<std::string> allowedFactions,
        Player* bot
    );
    static bool BroadcastSuggestSell(
        PlayerbotAI* botAI,
        ItemTemplate const* proto,
        uint32_t count,
        uint32_t price,
        Player* bot
    );
    static bool BroadcastSuggestSomething(
        PlayerbotAI* botAI,
        Player* bot
    );
    static bool BroadcastSuggestSomethingToxic(
        PlayerbotAI* botAI,
        Player* bot
    );
    static bool BroadcastSuggestToxicLinks(
        PlayerbotAI* botAI,
        Player* bot
    );
    static bool BroadcastSuggestThunderfury(
        PlayerbotAI* botAI,
        Player* bot
    );
};

#endif
