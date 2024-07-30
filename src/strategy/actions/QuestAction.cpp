/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "QuestAction.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "ServerFacade.h"

bool QuestAction::Execute(Event event)
{
    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();

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
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (const auto npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(unit);
    }
    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects");
    for (const auto go : gos)
    {
        GameObject* gameobj = botAI->GetGameObject(go);
        if (gameobj && bot->GetDistance(gameobj) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(gameobj);
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
        /*else*/ if (creature > 0)
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

    player->CompleteQuest(entry);
    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        LOG_INFO("playerbots", "Quest [ {} ] completed", pQuest->GetTitle());
        bot->Say("Quest [ " + ChatHelper::FormatQuest(pQuest) + " ] completed", LANG_UNIVERSAL);
    }

    botAI->TellMasterNoFacing("Quest completed " + ChatHelper::FormatQuest(pQuest));
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

    if (bot->GetDistance(questGiver) > INTERACTION_DISTANCE && !sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        //if (botAI->HasStrategy("debug", BotState::BOT_STATE_COMBAT) || botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))

        botAI->TellError("Cannot talk to quest giver");
        return false;
    }

    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, questGiver, sPlayerbotAIConfig->sightDistance))
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

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_NONE && sPlayerbotAIConfig->syncQuestWithPlayer)
        {
            Object* pObject = ObjectAccessor::GetObjectByTypeMask(*bot, questGiver, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
            bot->AddQuest(quest, pObject);
        }

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE && bot->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
        {
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
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 entry, questId, available, required;
    ObjectGuid guid;
    p >> questId >> entry >> available >> required >> guid;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if (qInfo)
    {
        if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        {
            LOG_INFO("playerbots", "Quest [ {} ] completed", qInfo->GetTitle());
            bot->Say("Quest [ " + ChatHelper::FormatQuest(qInfo) + " ] completed", LANG_UNIVERSAL);
        }
        botAI->TellMasterNoFacing("Quest completed " + ChatHelper::FormatQuest(qInfo));
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
    ObjectGuid guid;
    p >> questId >> entry >> available >> required >> guid;

    std::stringstream ss;
    ss << "Update progression kill questid {" << std::to_string(questId) << "} {" << std::to_string(available) << "} / {" << std::to_string(required) << "}";
    botAI->TellMasterNoFacing(ss.str());
    return false;
}

bool QuestUpdateAddItemAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 itemId, count;
    p >> itemId >> count;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    auto const* itemPrototype = sObjectMgr->GetItemTemplate(itemId);

    std::stringstream ss;
    ss << "Update progression itemid {" << std::to_string(itemId) << "} count: {" << std::to_string(count) << "}";
    botAI->TellMasterNoFacing(ss.str());

    return false;
}

bool QuestUpdateFailedAction::Execute(Event event)
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

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);

    if (qInfo)
    {
        botAI->TellMaster("Failed timer for " + botAI->GetChatHelper()->FormatQuest(qInfo) +", abandoning");
    }
    else
    {
        botAI->TellMaster("Failed timer for " + std::to_string(questId));
    }

    //drop quest
    bot->AbandonQuest(questId);

    return false;
}
