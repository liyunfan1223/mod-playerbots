/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BudgetValues.h"

#include "Playerbots.h"

uint32 MaxGearRepairCostValue::Calculate()
{
    uint32 TotalCost = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        if (!maxDurability)
            continue;

        uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        if (i >= EQUIPMENT_SLOT_END && curDurability >= maxDurability)  // Only count items equiped or already damanged.
            continue;

        ItemTemplate const* ditemProto = item->GetTemplate();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
            continue;

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
            continue;

        uint32 dmultiplier =
            dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];

        uint32 costs = uint32(maxDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        TotalCost += costs;
    }

    return TotalCost;
}

uint32 RepairCostValue::Calculate()
{
    uint32 TotalCost = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        if (!maxDurability)
            continue;

        uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        uint32 LostDurability = maxDurability - curDurability;

        if (LostDurability == 0)
            continue;

        ItemTemplate const* ditemProto = item->GetTemplate();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
            continue;

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
            continue;

        uint32 dmultiplier =
            dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
        uint32 costs = uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        TotalCost += costs;
    }

    return TotalCost;
}

uint32 TrainCostValue::Calculate()
{
    uint32 TotalCost = 0;

    std::set<uint32> spells;

    if (CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates())
    {
        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            if (itr->second.trainer_type != TRAINER_TYPE_CLASS && itr->second.trainer_type != TRAINER_TYPE_TRADESKILLS)
                continue;

            if (itr->second.trainer_type == TRAINER_TYPE_CLASS && itr->second.trainer_class != bot->getClass())
                continue;

            TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(itr->first);
            if (!trainer_spells)
                continue;

            for (TrainerSpellMap::const_iterator iter = trainer_spells->spellList.begin();
                 iter != trainer_spells->spellList.end(); ++iter)
            {
                TrainerSpell const* tSpell = &iter->second;
                if (!tSpell)
                    continue;

                TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
                if (state != TRAINER_SPELL_GREEN)
                    continue;

                if (itr->second.trainer_type == TRAINER_TYPE_TRADESKILLS)
                    continue;

                if (spells.find(tSpell->spell) != spells.end())
                    continue;

                TotalCost += tSpell->spellCost;
                spells.insert(tSpell->spell);
            }
        }
    }

    return TotalCost;
}

uint32 MoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    uint32 moneyWanted = 0;

    uint32 level = bot->GetLevel();

    switch (needMoneyFor)
    {
        case NeedMoneyFor::none:
            moneyWanted = 0;
            break;
        case NeedMoneyFor::repair:
            moneyWanted = AI_VALUE(uint32, "max repair cost");
            break;
        case NeedMoneyFor::ammo:
            moneyWanted = (bot->getClass() == CLASS_HUNTER)
                              ? (level * level * level) / 10
                              : 0;  // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo replace
                                    // (should be best ammo buyable x 8 stacks cost)
            break;
        case NeedMoneyFor::spells:
            moneyWanted = AI_VALUE(uint32, "train cost");
            break;
        case NeedMoneyFor::travel:
            moneyWanted =
                bot->isTaxiCheater()
                    ? 0
                    : 1500;  // 15s for traveling half a continent. Todo: Add better calculation (Should be ???)
            break;
        case NeedMoneyFor::gear:
            moneyWanted = level * level * level;  // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80):
                                                  // Todo replace (Should be ~total cost of all >green gear equiped)
            break;
        case NeedMoneyFor::consumables:
            moneyWanted =
                (level * level * level) / 10;  // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo
                                               // replace (Should be best food/drink x 2 stacks cost)
            break;
        case NeedMoneyFor::guild:
            if (botAI->HasStrategy("guild", BOT_STATE_NON_COMBAT))
            {
                if (bot->GetGuildId())
                    moneyWanted = AI_VALUE2(uint32, "item count", chat->FormatQItem(5976)) ? 0 : 10000;  // 1g (tabard)
                else
                    moneyWanted =
                        AI_VALUE2(uint32, "item count", chat->FormatQItem(5863)) ? 0 : 10000;  // 10s (guild charter)
            }
            break;
        case NeedMoneyFor::tradeskill:
            moneyWanted = (level * level *
                           level);  // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80): Todo replace
                                    // (Should be buyable reagents that combined allow crafting of usefull items)
            break;
        default:
            break;
    }

    return moneyWanted;
};

uint32 TotalMoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    uint32 moneyWanted = AI_VALUE2(uint32, "money needed for", (uint32)needMoneyFor);

    auto needPtr = std::find(saveMoneyFor.begin(), saveMoneyFor.end(), needMoneyFor);

    while (needPtr != saveMoneyFor.begin())
    {
        needPtr--;

        NeedMoneyFor alsoNeed = *needPtr;

        moneyWanted = moneyWanted + AI_VALUE2(uint32, "money needed for", (uint32)alsoNeed);
    }

    return moneyWanted;
}

uint32 FreeMoneyForValue::Calculate()
{
    uint32 money = bot->GetMoney();

    if (botAI->HasCheat(BotCheatMask::gold))
        return 10000000;

    if (botAI->HasActivePlayerMaster())
        return money;

    uint32 savedMoney = AI_VALUE2(uint32, "total money needed for", getQualifier()) -
                        AI_VALUE2(uint32, "money needed for", getQualifier());

    if (savedMoney > money)
        return 0;

    return money - savedMoney;
};

bool ShouldGetMoneyValue::Calculate() { return !AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::anything); };
