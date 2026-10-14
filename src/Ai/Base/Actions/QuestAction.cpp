/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestAction.h"
#include "BroadcastHelper.h"
#include "Chat.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemTemplate.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "ServerFacade.h"
#include <algorithm>
#include <sstream>

bool QuestAction::Execute(Event event)
{
    ObjectGuid guid = event.getObject();
    Player* master = GetMaster();

    // Checks if the bot and botAI are valid
    if (!bot || !botAI)
        return false;

    // Sets guid based on bot or master target
    if (!guid)
    {
        if (!master)
        {
            guid = bot->GetTarget();
        }
        else
        {
            guid = master->GetTarget();
        }
    }

    if (guid)
    {
        return ProcessQuests(guid);
    }

    bool result = false;

    // Check the nearest NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
        {
            result |= ProcessQuests(unit);
        }
    }

    // Checks the nearest game objects
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto const& go : gos)
    {
        GameObject* gameobj = botAI->GetGameObject(go);
        if (gameobj && bot->GetDistance(gameobj) <= INTERACTION_DISTANCE)
        {
            result |= ProcessQuests(gameobj);
        }
    }

    return result;
}

bool QuestAction::CompleteQuest(Player* player, uint32 entry)
{
    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        return false;
    }

    // Add quest items for quests that require items
    for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->RequiredItemId[x];
        uint32 count = pQuest->RequiredItemCount[x];
        if (!id || !count)
        {
            continue;
        }

        uint32 curItemCount = player->GetItemCount(id, true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, id, true);
            player->SendNewItem(item, count - curItemCount, true, false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = pQuest->RequiredNpcOrGo[i];
        uint32 creaturecount = pQuest->RequiredNpcOrGoCount[i];

        // TODO check if we need a REQSPELL condition, this methods and sql entry dosent seem implemented ?
        /*if (uint32 spell_id = pQuest->GetReqSpell[i])
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->CastedCreatureOrGO(creature, ObjectGuid(), spell_id);
            }
        }*/
        /*else*/
        if (creature > 0)
        {
            if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creature))
                for (uint16 z = 0; z < creaturecount; ++z)
                {
                    player->KilledMonster(cInfo, ObjectGuid::Empty);
                }
        }
        else if (creature < 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->KillCreditGO(-creature);
            }
        }
    }

    // If the quest requires reputation to complete
    if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
            {
                player->GetReputationMgr().SetReputation(factionEntry, repValue);
            }
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0)
    {
        player->ModifyMoney(-ReqOrRewMoney);
    }

    const std::string text_quest = ChatHelper::FormatQuest(pQuest);
    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        LOG_INFO("playerbots", "{} => Quest [ {} ] completed", bot->GetName(), pQuest->GetTitle());
        bot->Say("Quest [ " + text_quest + " ] completed", LANG_UNIVERSAL);
    }
    botAI->TellMasterNoFacing("Quest completed " + text_quest);

    player->CompleteQuest(entry);

    return true;
}

bool QuestAction::ProcessQuests(ObjectGuid questGiver)
{
    if (GameObject* gameObject = botAI->GetGameObject(questGiver))
        if (gameObject->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            return ProcessQuests(gameObject);

    Creature* creature = botAI->GetCreature(questGiver);
    if (creature)
        return ProcessQuests(creature);

    return false;
}

bool QuestAction::ProcessQuests(WorldObject* questGiver)
{
    ObjectGuid guid = questGiver->GetGUID();

    if (bot->GetDistance(questGiver) > INTERACTION_DISTANCE && !sPlayerbotAIConfig.SyncQuestWithPlayer)
    {
        //if (botAI->HasStrategy("debug", BotState::BOT_STATE_COMBAT) || botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))

        botAI->TellError("Cannot talk to quest giver");
        return false;
    }

    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, questGiver, sPlayerbotAIConfig.SightDistance))
        bot->SetFacingToObject(questGiver);

    bot->SetTarget(guid);
    bot->PrepareQuestMenu(guid);

    QuestMenu& questMenu = bot->PlayerTalkClass->GetQuestMenu();
    for (uint32 i = 0; i < questMenu.GetMenuItemCount(); ++i)
    {
        QuestMenuItem const& menuItem = questMenu.GetItem(i);
        uint32 questID = menuItem.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        if (!quest)
            continue;

        ProcessQuest(quest, questGiver);
    }

    return true;
}

bool QuestAction::AcceptQuest(Quest const* quest, ObjectGuid questGiver)
{
    std::ostringstream out;

    uint32 questId = quest->GetQuestId();

    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        out << "Already completed";
    else if (!bot->CanTakeQuest(quest, false))
    {
        if (!bot->SatisfyQuestStatus(quest, false))
            out << "Already on";
        else
            out << "Can't take";
    }
    else if (!bot->SatisfyQuestLog(false))
        out << "Quest log is full";
    else if (!bot->CanAddQuest(quest, false))
        out << "Bags are full";
    else
    {
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        uint32 unk1 = 0;
        p << questGiver << questId << unk1;
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_NONE && sPlayerbotAIConfig.SyncQuestWithPlayer)
        {
            Object* pObject = ObjectAccessor::GetObjectByTypeMask(*bot, questGiver,
                                                                  TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
            bot->AddQuest(quest, pObject);
        }

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE && bot->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
        {
            BroadcastHelper::BroadcastQuestAccepted(botAI, bot, quest);
            out << "Accepted " << chat->FormatQuest(quest);
            botAI->TellMaster(out);
            return true;
        }
        out << "Cannot accept";
    }

    out << " " << chat->FormatQuest(quest);
    botAI->TellMaster(out);

    return false;
}

bool QuestUpdateCompleteAction::Execute(Event event)
{
    // the action can hardly be triggered
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId = 0;
    p >> questId;

    p.print_storage();
    // LOG_INFO("playerbots", "Packet: empty{} questId{}", p.empty(), questId);

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if (qInfo)
    {
        // std::map<std::string, std::string> placeholders;
        // placeholders["%quest_link"] = format;

        // if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        // {
            //     LOG_INFO("playerbots", "{} => Quest [ {} ] completed", bot->GetName(), qInfo->GetTitle());
            //     bot->Say("Quest [ " + format + " ] completed", LANG_UNIVERSAL);
            // }
        const auto format = ChatHelper::FormatQuest(qInfo);
        if (botAI->GetMaster())
            botAI->TellMasterNoFacing("Quest completed " + format);
        BroadcastHelper::BroadcastQuestUpdateComplete(botAI, bot, qInfo);
        botAI->rpgStatistic.questCompleted++;
        // LOG_DEBUG("playerbots", "[New rpg] {} complete quest {}", bot->GetName(), qInfo->GetQuestId());
        // botAI->rpgStatistic.questCompleted++;
    }

    return true;
}

/*
* For creature or gameobject
*/
bool QuestUpdateAddKillAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 entry, questId, available, required;
    p >> questId >> entry >> available >> required;
    // LOG_INFO("playerbots", "[New rpg] Quest {} -> Creature {} ({}/{})", questId, entry, available, required);
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if (qInfo && (entry & 0x80000000))
    {
        entry &= 0x7FFFFFFF;
        GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(entry);
        if (info)
        {
            std::string infoName = PlayerbotAI::GetLocalizedGameObjectName(entry);
            BroadcastHelper::BroadcastQuestUpdateAddKill(botAI, bot, qInfo, available, required, infoName);
            if (botAI->GetMaster())
            {
                std::ostringstream out;
                out << infoName << " " << available << "/" << required << " " << ChatHelper::FormatQuest(qInfo);
                botAI->TellMasterNoFacing(out.str());
            }
        }
    }
    else if (qInfo)
    {
        CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(entry);
        if (info)
        {
            std::string infoName = PlayerbotAI::GetLocalizedCreatureName(entry);
            BroadcastHelper::BroadcastQuestUpdateAddKill(botAI, bot, qInfo, available, required, infoName);
            if (botAI->GetMaster())
            {
                std::ostringstream out;
                out << infoName << " " << available << "/" << required << " " << ChatHelper::FormatQuest(qInfo);
                botAI->TellMasterNoFacing(out.str());
            }
        }
    }
    return false;
}

bool QuestUpdateAddItemAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 itemId, count;
    p >> itemId >> count;

    auto const* itemPrototype = sObjectMgr->GetItemTemplate(itemId);
    if (itemPrototype)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%item_link"] = botAI->GetChatHelper()->FormatItem(itemPrototype);
        uint32 availableItemsCount = botAI->GetInventoryItemsCountWithId(itemId);
        placeholders["%quest_obj_available"] = std::to_string(availableItemsCount);

        for (auto const& pair : botAI->GetCurrentQuestsRequiringItemId(itemId))
        {
            placeholders["%quest_link"] = chat->FormatQuest(pair.first);
            uint32 requiredItemsCount = pair.second;
            placeholders["%quest_obj_required"] = std::to_string(requiredItemsCount);
            if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_COMBAT) || botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT))
            {
                const auto text = PlayerbotTextMgr::instance().GetBotText("%quest_link - %item_link %quest_obj_available/%quest_obj_required", placeholders);
                botAI->Say(text);
                LOG_INFO("playerbots", "{} => {}", bot->GetName(), text);
            }

            BroadcastHelper::BroadcastQuestUpdateAddItem(botAI, bot, pair.first, availableItemsCount, requiredItemsCount, itemPrototype);
        }
    }
    return false;
}

bool QuestItemPushResultAction::Execute(Event event)
{
    WorldPacket packet = event.getPacket();
    ObjectGuid guid;
    uint32 received, created, sendChatMessage, itemSlot, itemEntry, itemSuffixFactory, count, itemCount;
    uint8 bagSlot;
    int32 itemRandomPropertyId;
    packet >> guid >> received >> created >> sendChatMessage;
    packet >> bagSlot >> itemSlot >> itemEntry >> itemSuffixFactory >> itemRandomPropertyId;
    packet >> count >> itemCount;

    if (guid != bot->GetGUID())
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
    if (!proto)
        return false;

    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = bot->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            return false;

        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
        {
            uint32 itemId = quest->RequiredItemId[i];
            if (!itemId)
                continue;

            int32 previousCount = itemCount - count;
            if (itemId == itemEntry && uint32(previousCount) < quest->RequiredItemCount[i])
            {
                if (botAI->GetMaster())
                {
                    std::string itemLink = ChatHelper::FormatItem(proto);
                    std::ostringstream out;
                    int32 required = quest->RequiredItemCount[i];
                    int32 available = std::min((int32)itemCount, required);
                    out << itemLink << " " << available << "/" << required << " " << ChatHelper::FormatQuest(quest);
                    botAI->TellMasterNoFacing(out.str());
                }
            }
        }
    }

    return false;
}

bool QuestUpdateFailedAction::Execute(Event /*event*/)
{
    //opcode SMSG_QUESTUPDATE_FAILED is never sent...(yet?)
    return false;
}

bool QuestUpdateFailedTimerAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId;
    p >> questId;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);

    if (qInfo)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = botAI->GetChatHelper()->FormatQuest(qInfo);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotText("Failed timer for %quest_link, abandoning", placeholders));
        BroadcastHelper::BroadcastQuestUpdateFailedTimer(botAI, bot, qInfo);
    }
    else
    {
        botAI->TellMaster("Failed timer for " + std::to_string(questId));
    }

    //drop quest
    bot->AbandonQuest(questId);

    return false;
}
