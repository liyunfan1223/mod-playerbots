/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TaxiAction.h"
#include "Config.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
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
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "taxi_ready_next_flight", "I am ready for the next flight", {}));
        return true;
    }

    GuidVector units = *context->GetValue<GuidVector>("nearest npcs");
    for (ObjectGuid const guid : units)
    {
        Creature* npc = ObjectAccessor::GetCreature(*bot, guid);
        if (!npc || !npc->IsAlive())
            continue;

        if (!(npc->GetNpcFlags() & UNIT_NPC_FLAG_FLIGHTMASTER))
            continue;

        if (bot->GetDistance(npc) > sPlayerbotAIConfig.FarDistance)
            continue;

        bot->GetSession()->SendLearnNewTaxiNode(npc);
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

        // Only for follower bots
        if (botAI->HasGameClientMaster())
        {
            uint32 index = botAI->GetGroupSlotIndex(bot);
            uint32 delay = sPlayerbotAIConfig.BotTaxiDelayMin +
                          index * sPlayerbotAIConfig.BotTaxiGapMs +
                          urand(0, sPlayerbotAIConfig.BotTaxiGapJitterMs);

            delay = std::min(delay, sPlayerbotAIConfig.BotTaxiDelayMax);

            // Store the NPC's GUID so we can re-acquire the pointer later
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
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "taxi_cant_fly_with_you", "I can't fly with you", {}));
            return false;
        }

        return true;
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "taxi_no_flightmaster_nearby", "Cannot find any flightmaster to talk", {}));
    return false;
}
