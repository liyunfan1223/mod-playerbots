/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TaxiAction.h"

#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "Config.h"

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

        // stagger bot takeoff
        uint32 delayMin = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMinMs", 350u, false);
        uint32 delayMax = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMaxMs", 5000u, false);
        uint32 gapMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapMs", 200u, false);
        uint32 gapJitterMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapJitterMs", 100u, false);

        // Only for follower bots
        if (botAI->HasRealPlayerMaster())
        {
            uint32 index = botAI->GetGroupSlotIndex(bot);
            uint32 delay = delayMin + index * gapMs + urand(0, gapJitterMs);

            delay = std::min(delay, delayMax);

            // Store the npc’s GUID so we can re-acquire the pointer later
            ObjectGuid npcGuid = npc->GetGUID();

            // schedule the take-off
            botAI->AddTimedEvent(
                [bot = bot, &movement, npcGuid]() -> void
                {
                    if (Creature* npcPtr = ObjectAccessor::GetCreature(*bot, npcGuid))
                        if (!movement.taxiNodes.empty())
                            bot->ActivateTaxiPathTo(movement.taxiNodes, npcPtr, 0);
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
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

        if (!movement.taxiNodes.empty() && !bot->ActivateTaxiPathTo(movement.taxiNodes, npc, 0))
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
