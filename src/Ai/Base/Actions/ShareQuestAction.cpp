/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ShareQuestAction.h"
#include "Event.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "QuestPackets.h"

bool ShareQuestAction::Execute(Event event)
{
    std::string const link = event.getParam();
    if (!GetMaster())
        return false;

    PlayerbotChatHandler handler(GetMaster());
    uint32 entry = handler.extractQuestId(link);
    if (!entry)
        return false;

    Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
    if (!quest)
        return false;

    // remove all quest entries for 'entry' from quest log
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);
        if (logQuest == entry)
        {
            WorldPacket p(CMSG_PUSHQUESTTOPARTY);
            p << entry;
            WorldPackets::Quest::PushQuestToParty pushQuest(std::move(p));
            pushQuest.Read();
            bot->GetSession()->HandlePushQuestToParty(pushQuest);
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "quest_shared", "Quest shared", {}));
            return true;
        }
    }

    return false;
}

bool AutoShareQuestAction::Execute(Event /*event*/)
{
    bool shared = false;

    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);

        if (!quest)
            continue;

        bool partyNeedsQuest = false;

        for (GroupReference* itr = bot->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* player = itr->GetSource();

            if (!player || player == bot || !player->IsInWorld() || !botAI->IsSafe(player)) // skip self
                continue;

            if (bot->GetDistance(player) > 10)
                continue;

            if (!player->SatisfyQuestStatus(quest, false))
                continue;

            if (player->GetQuestStatus(logQuest) == QUEST_STATUS_COMPLETE)
                continue;

            if (!player->CanTakeQuest(quest, false))
                continue;

            if (!player->SatisfyQuestLog(false))
                continue;

            if (player->GetDivider())
                continue;

            if (auto botAI = GET_PLAYERBOT_AI(player))
            {
                if (PAI_VALUE(uint8, "free quest log slots") < 15 || !urand(0,5))
                {
                    WorldPacket packet(CMSG_PUSHQUESTTOPARTY, 20);
                    packet << logQuest;
                    botAI->HandleMasterIncomingPacket(packet);
                }
            }
            else
                partyNeedsQuest = true;
        }

        if (!partyNeedsQuest)
            continue;

        WorldPacket p(CMSG_PUSHQUESTTOPARTY);
        p << logQuest;
        WorldPackets::Quest::PushQuestToParty pushQuest(std::move(p));
        pushQuest.Read();
        bot->GetSession()->HandlePushQuestToParty(pushQuest);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "quest_shared", "Quest shared", {}));
        shared = true;
    }

    return shared;
}

bool AutoShareQuestAction::isUseful()
{
    return bot->GetGroup() && !IsRealPlayer(botAI->GetMaster());
}
