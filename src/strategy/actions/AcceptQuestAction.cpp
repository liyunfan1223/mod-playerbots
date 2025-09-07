/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AcceptQuestAction.h"

#include "Event.h"
#include "Playerbots.h"

bool AcceptAllQuestsAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    if (!AcceptQuest(quest, questGiver->GetGUID())) return false;

    auto text_quest = ChatHelper::FormatQuest(quest);
    bot->PlayDistanceSound(620);

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        LOG_INFO("playerbots", "{} => Quest [{}] accepted", bot->GetName(), quest->GetTitle());
        bot->Say("Quest [" + text_quest + "] accepted", LANG_UNIVERSAL);
    }

    return true;
}

bool AcceptQuestAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    Player* bot = botAI->GetBot();
    uint64_t guid;
    uint32 quest = 0;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(requester);
    quest = ch.extractQuestId(text);

    bool hasAccept = false;

    if (event.getPacket().empty())
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (unit && quest && unit->hasQuest(quest))
            {
                guid = unit->GetGUID().GetRawValue();
                break;
            }
            if (unit && text == "*" && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
                hasAccept |= QuestAction::ProcessQuests(unit);
        }
        GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
        for (auto i = gos.begin(); i != gos.end(); i++)
        {
            GameObject* go = botAI->GetGameObject(*i);
            if (go && quest && go->hasQuest(quest))
            {
                guid = go->GetGUID().GetRawValue();
                break;
            }
            if (go && text == "*" && bot->GetDistance(go) <= INTERACTION_DISTANCE)
                hasAccept |= QuestAction::ProcessQuests(go);
        }
    }
    else
    {
        WorldPacket& p = event.getPacket();
        p.rpos(0);
        p >> guid >> quest;
    }

    if (!quest || !guid)
        return false;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);
    if (!qInfo)
        return false;

    hasAccept |= AcceptQuest(qInfo, ObjectGuid(guid));

    if (hasAccept)
    {
        std::stringstream ss;
        ss << "AcceptQuestAction [" << qInfo->GetTitle() << "] - [" << std::to_string(qInfo->GetQuestId()) << "]";
        LOG_DEBUG("playerbots", "{}", ss.str().c_str());
        // botAI->TellMaster(ss.str());
    }

    return hasAccept;
}

bool AcceptQuestShareAction::Execute(Event event)
{
    Player* master = GetMaster();
    Player* bot = botAI->GetBot();

    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 quest;
    p >> quest;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);
    if (!qInfo || !bot->GetDivider())
        return false;

    quest = qInfo->GetQuestId();

    if (bot->HasQuest(quest))
    {
        bot->SetDivider(ObjectGuid::Empty);
        botAI->TellError("I have this quest");
        return false;
    }

    if (!bot->CanTakeQuest(qInfo, false))
    {
        // can't take quest
        bot->SetDivider(ObjectGuid::Empty);
        botAI->TellError("I can't take this quest");

        return false;
    }

    if (!bot->GetDivider().IsEmpty())
    {
        // send msg to quest giving player
        master->SendPushToPartyResponse(bot, QUEST_PARTY_MSG_ACCEPT_QUEST);
        bot->SetDivider(ObjectGuid::Empty);
    }

    if (bot->CanAddQuest(qInfo, false))
    {
        bot->AddQuest(qInfo, master);

        if (bot->CanCompleteQuest(quest))
            bot->CompleteQuest(quest);

        // Runsttren: did not add typeid switch from WorldSession::HandleQuestgiverAcceptQuestOpcode!
        // I think it's not needed, cause typeid should be TYPEID_PLAYER - and this one is not handled
        // there and there is no default case also.

        if (qInfo->GetSrcSpell() > 0)
        {
            bot->CastSpell(bot, qInfo->GetSrcSpell(), true);
        }

        botAI->TellMaster("Quest accepted");
        return true;
    }

    return false;
}

bool ConfirmQuestAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 quest;
    p >> quest;
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);

    if (!qInfo)
        return false;

    quest = qInfo->GetQuestId();
    if (!bot->CanTakeQuest(qInfo, false))
    {
        // can't take quest
        // botAI->TellError("quest_cant_take");
        return false;
    }

    if (bot->CanAddQuest(qInfo, false))
    {
        bot->AddQuest(qInfo, requester);

        if (bot->CanCompleteQuest(quest))
            bot->CompleteQuest(quest);

        if (qInfo->GetSrcSpell() > 0)
        {
            bot->CastSpell(bot, qInfo->GetSrcSpell(), true);
        }

        // botAI->TellMaster("quest_accept");
        return true;
    }

    return false;
}
