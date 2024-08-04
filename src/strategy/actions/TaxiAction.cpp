/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TaxiAction.h"

#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"

bool TaxiAction::Execute(Event event)
{
    botAI->RemoveShapeshift();

    LastMovement& movement = context->GetValue<LastMovement&>("last taxi")->Get();

    WorldPacket& p = event.getPacket();
    std::string const param = event.getParam();
    if ((!p.empty() && (p.GetOpcode() == CMSG_TAXICLEARALLNODES || p.GetOpcode() == CMSG_TAXICLEARNODE)) ||
        param == "clear")
    {
        movement.taxiNodes.clear();
        movement.Set(nullptr);
        botAI->TellMaster("I am ready for the next flight");
        return true;
    }

    GuidVector units = *context->GetValue<GuidVector>("nearest npcs");
    for (ObjectGuid const guid : units)
    {
        Creature* npc = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
        if (!npc)
            continue;

        uint32 curloc = sObjectMgr->GetNearestTaxiNode(npc->GetPositionX(), npc->GetPositionY(), npc->GetPositionZ(),
                                                       npc->GetMapId(), bot->GetTeamId());

        std::vector<uint32> nodes;
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
                if (entry->from == curloc)
                {
                    uint8 field = uint8((i - 1) / 32);
                    if (field < TaxiMaskSize)
                        nodes.push_back(i);
                }
        }

        if (param == "?")
        {
            botAI->TellMasterNoFacing("=== Taxi ===");

            uint32 index = 1;
            for (uint32 node : nodes)
            {
                TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(node);
                if (!entry)
                    continue;

                TaxiNodesEntry const* dest = sTaxiNodesStore.LookupEntry(entry->to);
                if (!dest)
                    continue;

                std::ostringstream out;
                out << index++ << ": " << dest->name[0];
                botAI->TellMasterNoFacing(out.str());
            }

            return true;
        }

        uint32 selected = atoi(param.c_str());
        if (selected)
        {
            uint32 path = nodes[selected - 1];
            TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
            if (!entry)
                return false;

            return bot->ActivateTaxiPathTo({entry->from, entry->to}, npc, 0);
        }

        if (!movement.taxiNodes.empty() && !bot->ActivateTaxiPathTo(movement.taxiNodes, npc))
        {
            movement.taxiNodes.clear();
            movement.Set(nullptr);
            botAI->TellError("I can't fly with you");
            return false;
        }

        return true;
    }

    botAI->TellError("Cannot find any flightmaster to talk");
    return false;
}
