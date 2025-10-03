/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GossipHelloAction.h"

#include "Event.h"
#include "GossipDef.h"
#include "Playerbots.h"

bool GossipHelloAction::Execute(Event event)
{
    ObjectGuid guid;

    WorldPacket& p = event.getPacket();
    if (p.empty())
    {
        Player* master = GetMaster();
        if (master)
            guid = master->GetTarget();
    }
    else
    {
        p.rpos(0);
        p >> guid;
    }

    std::string const text = event.getParam();
    int32 menuToSelect = -1;
    if (!text.empty())
        menuToSelect = atoi(text.c_str());

    return Execute(guid, menuToSelect, false);
}

void GossipHelloAction::TellGossipText(uint32 textId)
{
    if (!textId)
        return;

    if (GossipText const* text = sObjectMgr->GetGossipText(textId))
    {
        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; i++)
        {
            std::string const text0 = text->Options[i].Text_0;
            if (!text0.empty())
                botAI->TellMasterNoFacing(text0);

            std::string const text1 = text->Options[i].Text_1;
            if (!text1.empty())
                botAI->TellMasterNoFacing(text1);
        }
    }
}

void GossipHelloAction::TellGossipMenus()
{
    if (!bot->PlayerTalkClass)
        return;

    Creature* pCreature = bot->GetNPCIfCanInteractWith(GetMaster()->GetTarget(), UNIT_NPC_FLAG_NONE);
    GossipMenu& menu = bot->PlayerTalkClass->GetGossipMenu();
    if (pCreature)
    {
        uint32 textId = bot->GetGossipTextId(menu.GetMenuId(), pCreature);
        TellGossipText(textId);
    }

    GossipMenuItemContainer const& items = menu.GetMenuItems();
    for (auto iter = items.begin(); iter != items.end(); iter++)
    {
        GossipMenuItem const* item = &(iter->second);
        std::ostringstream out;
        out << "[" << iter->first << "] " << item->Message;
        botAI->TellMasterNoFacing(out.str());
    }
}

bool GossipHelloAction::ProcessGossip(int32 menuToSelect, bool silent)
{
    GossipMenu& menu = bot->PlayerTalkClass->GetGossipMenu();
    if (menuToSelect != -1 && !menu.GetItem(menuToSelect))
    {
        if (!silent)
            botAI->TellError("Unknown gossip option");
        return false;
    }

    WorldPacket p;
    std::string code;
    p << GetMaster()->GetTarget();
    p << menu.GetMenuId() << menuToSelect;
    p << code;
    bot->GetSession()->HandleGossipSelectOptionOpcode(p);

    if (!silent)
        TellGossipMenus();

    return true;
}

bool GossipHelloAction::Execute(ObjectGuid guid, int32 menuToSelect, bool silent)
{
    if (!guid)
        return false;

    Creature* pCreature = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!pCreature)
    {
        LOG_DEBUG("playerbots",
                  "[PlayerbotMgr]: HandleMasterIncomingPacket - Received  CMSG_GOSSIP_HELLO {} not found or you can't "
                  "interact with him.",
                  guid.ToString().c_str());
        return false;
    }

    GossipMenuItemsMapBounds pMenuItemBounds =
        sObjectMgr->GetGossipMenuItemsMapBounds(pCreature->GetCreatureTemplate()->GossipMenuId);
    if (pMenuItemBounds.first == pMenuItemBounds.second)
        return false;

    if (menuToSelect == -1)
    {
        WorldPacket p1;
        p1 << guid;
        bot->GetSession()->HandleGossipHelloOpcode(p1);
        bot->SetFacingToObject(pCreature);

        if (!silent)
        {
            std::ostringstream out;
            out << "--- " << pCreature->GetName() << " ---";
            botAI->TellMasterNoFacing(out.str());
            TellGossipMenus();
        }
    }
    else if (!bot->PlayerTalkClass)
    {
        if (!silent)
            botAI->TellError("I need to talk first");
        return false;
    }
    else
    {
        if (!ProcessGossip(menuToSelect, silent))
            return false;
    }

    bot->TalkedToCreature(pCreature->GetEntry(), pCreature->GetGUID());
    return true;
}
