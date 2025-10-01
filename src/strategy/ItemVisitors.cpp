/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ItemVisitors.h"

#include "Playerbots.h"

bool FindUsableItemVisitor::Visit(Item* item)
{
    if (bot->CanUseItem(item->GetTemplate()) == EQUIP_ERR_OK)
        return FindItemVisitor::Visit(item);

    return true;
}

bool FindPotionVisitor::Accept(ItemTemplate const* proto)
{
    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK))
    {
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (!spellInfo)
                return false;

            for (uint8 i = 0; i < 3; i++)
            {
                if (spellInfo->Effects[i].Effect == effectId)
                    return true;
            }
        }
    }

    return false;
}

bool FindMountVisitor::Accept(ItemTemplate const* proto)
{
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
        if (!spellInfo)
            return false;

        for (uint8 i = 0; i < 3; i++)
        {
            if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOUNTED)
                return true;
        }
    }

    return false;
}

bool FindPetVisitor::Accept(ItemTemplate const* proto)
{
    if (proto->Class == ITEM_CLASS_MISC)
    {
        for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (!spellInfo)
                return false;

            for (uint8 i = 0; i < 3; i++)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SUMMON_PET)
                    return true;
            }
        }
    }

    return false;
}

FindItemUsageVisitor::FindItemUsageVisitor(Player* bot, ItemUsage usage) : FindUsableItemVisitor(bot), usage(usage)
{
    context = GET_PLAYERBOT_AI(bot)->GetAiObjectContext();
};

bool FindItemUsageVisitor::Accept(ItemTemplate const* proto)
{
    if (AI_VALUE2(ItemUsage, "item usage", proto->ItemId) == usage)
        return true;

    return false;
}

bool FindUsableNamedItemVisitor::Accept(ItemTemplate const* proto)
{
    return proto && !proto->Name1.empty() && strstri(proto->Name1.c_str(), name.c_str());
}