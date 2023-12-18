/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AcceptQuestAction.h"
#include "Event.h"
#include "Playerbots.h"

void AcceptAllQuestsAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    AcceptQuest(quest, questGiver->GetGUID());
    bot->PlayDistanceSound(620);
}

bool AcceptQuestAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    Player* bot = botAI->GetBot();
    ObjectGuid guid;
    uint32 quest = 0;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(master);
    quest = ch.extractQuestId(text);

    if (event.getPacket().empty())
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (unit && quest && unit->hasQuest(quest))
            {
                guid = unit->GetGUID();
                break;
            }

            if (unit && text == "*" && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
                QuestAction::ProcessQuests(unit);
        }

        GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
        for (GuidVector::iterator i = gos.begin(); i != gos.end(); i++)
        {
            GameObject* go = botAI->GetGameObject(*i);
            if (go && quest && go->hasQuest(quest))
            {
                guid = go->GetGUID();
                break;
            }

            if (go && text == "*" && bot->GetDistance(go) <= INTERACTION_DISTANCE)
                QuestAction::ProcessQuests(go);
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

    return AcceptQuest(qInfo, guid);
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

    if (bot->CanAddQuest( qInfo, false))
    {
        bot->AddQuest(qInfo, master);

        if (bot->CanCompleteQuest(quest))
            bot->CompleteQuest(quest);

        // Runsttren: did not add typeid switch from WorldSession::HandleQuestgiverAcceptQuestOpcode!
        // I think it's not needed, cause typeid should be TYPEID_PLAYER - and this one is not handled
        // there and there is no default case also.

        if (qInfo->GetSrcSpell() > 0)
        {
            bot->CastSpell( bot, qInfo->GetSrcSpell(), true);
        }

        botAI->TellMaster("Quest accepted");
        return true;
    }

    return false;
}
