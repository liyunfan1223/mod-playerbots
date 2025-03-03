#include "UnlockItemAction.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"

#define PICK_LOCK_SPELL_ID 1804

bool UnlockItemAction::Execute(Event event)
{
    bool foundLockedItem = false;

    // Check main inventory slots
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (item && CanUnlockItem(item))
        {
            UnlockItem(item, INVENTORY_SLOT_BAG_0, slot);
            foundLockedItem = true;
        }
    }

    // Check items in the bags
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag* bagItem = bot->GetBagByPos(bag);
        if (!bagItem)
            continue;

        for (uint32 slot = 0; slot < bagItem->GetBagSize(); ++slot)
        {
            Item* item = bot->GetItemByPos(bag, slot);

            if (item && CanUnlockItem(item))
            {
                UnlockItem(item, bag, slot);
                foundLockedItem = true;
            }
            else
            {
                botAI->TellError("CanUnlockItem returned false");
            }
        }
    }

    // If no locked items found
    if (!foundLockedItem)
    {
        botAI->TellError("No locked items in inventory.");
    }

    return foundLockedItem;
}

bool UnlockItemAction::CanUnlockItem(Item* item)
{
    if (!item)
        return false;

    ItemTemplate const* itemTemplate = item->GetTemplate();
    if (!itemTemplate)
        return false;

    // Ensure the bot has Lockpicking skill
    if (!botAI->HasSkill(SKILL_LOCKPICKING))
        return false;

    // Ensure the item is locked
    if (itemTemplate->LockID == 0 || !item->IsLocked())
        return false;

    // Check if the bot's Lockpicking skill is high enough
    uint32 lockId = itemTemplate->LockID;
    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
    if (!lockInfo)
        return false;

    bool canUnlock = false;

    for (uint8 j = 0; j < 8; ++j)
    {
        if (lockInfo->Type[j] == LOCK_KEY_SKILL)
        {
            uint32 skillId = SkillByLockType(LockType(lockInfo->Index[j]));
            if (skillId == SKILL_LOCKPICKING)
            {
                uint32 requiredSkill = lockInfo->Skill[j];
                uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING);

                if (botSkill >= requiredSkill)
                {
                    canUnlock = true;
                }
                else
                {
                    std::ostringstream out;
                    out << "Lockpicking skill too low (" << botSkill << "/" << requiredSkill << ") to unlock: " 
                        << item->GetTemplate()->Name1;
                    botAI->TellMaster(out.str());
                }
            }
        }
    }

    return canUnlock;
}

void UnlockItemAction::UnlockItem(Item* item, uint8 bag, uint8 slot)
{
    if (!bot->HasSpell(PICK_LOCK_SPELL_ID))
    {
        botAI->TellError("Cannot unlock, Pick Lock spell is missing.");
        return;
    }

    // Use CastSpell to unlock the item
    if (botAI->CastSpell(PICK_LOCK_SPELL_ID, bot, item))
    {
        std::ostringstream out;
        out << "Used Pick Lock on: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
    {
        botAI->TellError("Failed to cast Pick Lock.");
    }
}

