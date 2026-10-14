/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SellAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemPackets.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

class SellItemsVisitor : public IterateItemsVisitor
{
public:
    SellItemsVisitor(SellAction* action) : IterateItemsVisitor(), action(action) {}

    bool Visit(Item* item) override
    {
        action->Sell(item);
        return true;
    }

private:
    SellAction* action;
};

class SellQualityItemsVisitor : public SellItemsVisitor
{
public:
    SellQualityItemsVisitor(SellAction* action, uint32 maxQuality, bool allClasses)
        : SellItemsVisitor(action), maxQuality(maxQuality), allClasses(allClasses)
    {
    }

    bool Visit(Item* item) override
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (proto->Quality > maxQuality)
            return true;

        if (IsProfessionTool(proto))
            return true;

        if (!allClasses && proto->Quality > ITEM_QUALITY_POOR && !IsEquipment(proto))
            return true;

        return SellItemsVisitor::Visit(item);
    }

private:
    static bool IsEquipment(ItemTemplate const* proto)
    {
        return proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON;
    }

    static bool IsProfessionTool(ItemTemplate const* proto)
    {
        if (proto->Class != ITEM_CLASS_WEAPON)
            return false;

        if (proto->SubClass == ITEM_SUBCLASS_WEAPON_MISC || proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return true;

        return proto->TotemCategory != 0;
    }

    uint32 maxQuality;
    bool allClasses;
};

class SellVendorItemsVisitor : public SellItemsVisitor
{
public:
    SellVendorItemsVisitor(SellAction* action, AiObjectContext* con) : SellItemsVisitor(action) { context = con; }

    AiObjectContext* context;

    bool Visit(Item* item) override
    {
        ItemUsage usage = context->GetValue<ItemUsage>("item usage", item->GetEntry())->Get();
        if (usage != ITEM_USAGE_VENDOR && usage != ITEM_USAGE_AH)
            return true;

        return SellItemsVisitor::Visit(item);
    }
};

bool SellAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "gray" || text == "*")
    {
        SellQualityItemsVisitor visitor(this, ITEM_QUALITY_POOR, false);
        IterateItems(&visitor);
        return true;
    }

    if (text == "vendor")
    {
        SellVendorItemsVisitor visitor(this, context);
        IterateItems(&visitor);
        return true;
    }

    std::string quality = text;
    bool allClasses = false;

    size_t const split = quality.rfind(' ');
    if (split != std::string::npos && quality.substr(split + 1) == "all")
    {
        quality.erase(split);
        allClasses = true;
    }

    uint32 const maxQuality = ChatHelper::parseItemQuality(quality);
    if (maxQuality != MAX_ITEM_QUALITY)
    {
        SellQualityItemsVisitor visitor(this, maxQuality, allClasses);
        IterateItems(&visitor);
        return true;
    }

    if (text != "")
    {
        std::vector<Item*> items = parseItems(text, ITERATE_ITEMS_IN_BAGS);
        for (Item* item : items)
        {
            Sell(item);
        }
        return true;
    }

    botAI->TellError("Usage: s gray/*/vendor/<quality> [all]/[item link]");
    return false;
}

void SellAction::Sell(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    for (Item* item : items)
    {
        Sell(item);
    }
}

void SellAction::Sell(Item* item)
{
    std::ostringstream out;

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    for (ObjectGuid const vendorguid : vendors)
    {
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        ObjectGuid itemguid = item->GetGUID();
        uint32 count = item->GetCount();

        uint32 botMoney = bot->GetMoney();

        WorldPacket p(CMSG_SELL_ITEM);
        p << vendorguid << itemguid << count;

        WorldPackets::Item::SellItem nicePacket(std::move(p));
        nicePacket.Read();
        bot->GetSession()->HandleSellItemOpcode(nicePacket);

        if (botAI->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(botMoney);
        }

        out << "Selling " << chat->FormatItem(item->GetTemplate());
        botAI->TellMaster(out);

        bot->PlayDistanceSound(120);
        break;
    }
}
