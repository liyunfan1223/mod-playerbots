/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OutfitAction.h"

#include "Event.h"
#include "ItemVisitors.h"
#include "Playerbots.h"
#include "ItemPackets.h"

bool OutfitAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param == "?")
    {
        List();
        botAI->TellMaster("outfit <name> +[item] to add items");
        botAI->TellMaster("outfit <name> -[item] to remove items");
        botAI->TellMaster("outfit <name> equip/replace to equip items");
    }
    else
    {
        std::string name = parseOutfitName(param);
        ItemIds items = parseOutfitItems(param);
        if (!name.empty())
        {
            Save(name, items);

            std::ostringstream out;
            out << "Setting outfit " << name << " as " << param;
            botAI->TellMaster(out);
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
            std::ostringstream out;
            out << "Equipping outfit " << name;
            botAI->TellMaster(out);

            EquipItems(outfit);
            return true;
        }
        else if (command == "replace")
        {
            std::ostringstream out;
            out << "Replacing current equip with outfit " << name;
            botAI->TellMaster(out);

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
            std::ostringstream out;
            out << "Resetting outfit " << name;
            botAI->TellMaster(out);

            Save(name, ItemIds());
            return true;
        }
        else if (command == "update")
        {
            std::ostringstream out;
            out << "Updating with current items outfit " << name;
            botAI->TellMaster(out);

            Update(name);
            return true;
        }

        bool remove = param.size() > 1 && param.substr(space + 1, 1) == "-";
        for (uint32 itemid : items)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);

            std::ostringstream out;
            out << chat->FormatItem(proto);
            if (remove)
            {
                std::set<uint32>::iterator j = outfit.find(itemid);
                if (j != outfit.end())
                    outfit.erase(j);

                out << " removed from ";
            }
            else
            {
                outfit.insert(itemid);
                out << " added to ";
            }

            out << name;
            botAI->TellMaster(out.str());
        }

        Save(name, outfit);
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
