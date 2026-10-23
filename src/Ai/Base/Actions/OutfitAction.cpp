/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OutfitAction.h"
#include "Event.h"
#include "ItemPackets.h"
#include "ItemVisitors.h"
#include "PlayerbotRepository.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool OutfitAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param == "?")
    {
        List();
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "outfit_usage_add", "outfit <name> +[item] to add items", {}));
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "outfit_usage_remove", "outfit <name> -[item] to remove items", {}));
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "outfit_usage_equip", "outfit <name> equip/replace to equip items", {}));
    }
    else
    {
        std::string name = parseOutfitName(param);
        ItemIds items = parseOutfitItems(param);
        if (!name.empty())
        {
            Save(name, items);
            PlayerbotRepository::instance().Save(botAI);

            std::ostringstream out;
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "outfit_set_as",
                "Setting outfit %name as %param",
                {{"%name", name}, {"%param", param}}));
            return true;
        }

        items = chat->parseItems(param);

        int32 space = param.find(" ");
        if (space == -1)
            return false;

        name = param.substr(0, space);

        ItemIds outfit = FindOutfitItems(name);
        std::string const command = param.substr(space + 1);
        if (command == "equip")
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "outfit_equipping",
                "Equipping outfit %name",
                {{"%name", name}}));

            EquipItems(outfit, true);
            return true;
        }
        else if (command == "replace")
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "outfit_replace_current",
                "Replacing current equip with outfit %name",
                {{"%name", name}}));

            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
            {
                Item const* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (!pItem)
                    continue;

                uint8 bagIndex = pItem->GetBagSlot();
                uint8 dstBag = NULL_BAG;

                WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
                packet << bagIndex << slot << dstBag;
                WorldPackets::Item::AutoStoreBagItem nicePacket(std::move(packet));
                nicePacket.Read();
                bot->GetSession()->HandleAutoStoreBagItemOpcode(nicePacket);
            }

            EquipItems(outfit);
            return true;
        }
        else if (command == "reset")
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "outfit_resetting",
                "Resetting outfit %name",
                {{"%name", name}}));

            Save(name, ItemIds());
            PlayerbotRepository::instance().Save(botAI);
            return true;
        }
        else if (command == "update")
        {
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "outfit_updating_current",
                "Updating with current items outfit %name",
                {{"%name", name}}));

            Update(name);
            PlayerbotRepository::instance().Save(botAI);
            return true;
        }

        bool remove = param.size() > 1 && param.substr(space + 1, 1) == "-";
        for (uint32 itemid : items)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);

            if (remove)
            {
                std::set<uint32>::iterator j = outfit.find(itemid);
                if (j != outfit.end())
                    outfit.erase(j);

                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "outfit_item_removed_from",
                    "%item removed from %name",
                    {{"%item", chat->FormatItem(proto)}, {"%name", name}}));
            }
            else
            {
                outfit.insert(itemid);
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "outfit_item_added_to",
                    "%item added to %name",
                    {{"%item", chat->FormatItem(proto)}, {"%name", name}}));
            }
        }

        Save(name, outfit);
        PlayerbotRepository::instance().Save(botAI);
    }

    return true;
}

void OutfitAction::Save(std::string const name, ItemIds items)
{
    std::vector<std::string>& outfits = AI_VALUE(std::vector<std::string>&, "outfit list");
    for (std::vector<std::string>::iterator i = outfits.begin(); i != outfits.end(); ++i)
    {
        std::string const outfit = *i;
        if (name == parseOutfitName(outfit))
        {
            outfits.erase(i);
            break;
        }
    }

    if (items.empty())
        return;

    std::ostringstream out;
    out << name << "=";

    bool first = true;
    for (ItemIds::iterator i = items.begin(); i != items.end(); i++)
    {
        if (first)
            first = false;
        else
            out << ",";

        out << *i;
    }

    outfits.push_back(out.str());
}

void OutfitAction::List()
{
    std::vector<std::string>& outfits = AI_VALUE(std::vector<std::string>&, "outfit list");
    for (std::vector<std::string>::iterator i = outfits.begin(); i != outfits.end(); ++i)
    {
        std::string const outfit = *i;
        std::string const name = parseOutfitName(outfit);
        ItemIds items = parseOutfitItems(outfit);

        std::ostringstream out;
        out << name << ": ";

        for (uint32 itemId : items)
            if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
                out << chat->FormatItem(proto) << " ";

        botAI->TellMaster(out);
    }
}

void OutfitAction::Update(std::string const name)
{
    ListItemsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_EQUIP);

    ItemIds items;
    for (std::map<uint32, uint32>::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
        items.insert(i->first);

    Save(name, items);
}
