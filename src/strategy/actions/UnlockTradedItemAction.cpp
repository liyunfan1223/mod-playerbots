#include "UnlockTradedItemAction.h"
#include "Playerbots.h"
#include "TradeData.h"
#include "SpellInfo.h"

#define PICK_LOCK_SPELL_ID 1804

bool UnlockTradedItemAction::Execute(Event event)
{
    Player* trader = bot->GetTrader();
    if (!trader)
        return false; // No active trade session

    TradeData* tradeData = trader->GetTradeData();
    if (!tradeData)
        return false; // No trade data available

    Item* lockbox = tradeData->GetItem(TRADE_SLOT_NONTRADED);
    if (!lockbox)
    {
        botAI->TellError("No item in the Do Not Trade slot.");
        return false;
    }

    if (!CanUnlockItem(lockbox))
    {
        botAI->TellError("Cannot unlock this item.");
        return false;
    }

    UnlockItem(lockbox);
    return true;
}

bool UnlockTradedItemAction::CanUnlockItem(Item* item)
{
    if (!item)
        return false;

    ItemTemplate const* itemTemplate = item->GetTemplate();
    if (!itemTemplate)
        return false;

    // Ensure the bot is a rogue and has Lockpicking skill
    if (bot->getClass() != CLASS_ROGUE || !botAI->HasSkill(SKILL_LOCKPICKING))
        return false;

    // Ensure the item is actually locked
    if (itemTemplate->LockID == 0 || !item->IsLocked())
        return false;

    // Check if the bot's Lockpicking skill is high enough
    uint32 lockId = itemTemplate->LockID;
    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
    if (!lockInfo)
        return false;

    uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING);
    for (uint8 j = 0; j < 8; ++j)
    {
        if (lockInfo->Type[j] == LOCK_KEY_SKILL && SkillByLockType(LockType(lockInfo->Index[j])) == SKILL_LOCKPICKING)
        {
            uint32 requiredSkill = lockInfo->Skill[j];
            if (botSkill >= requiredSkill)
                return true;
            else
            {
                std::ostringstream out;
                out << "Lockpicking skill too low (" << botSkill << "/" << requiredSkill << ") to unlock: "
                    << item->GetTemplate()->Name1;
                botAI->TellMaster(out.str());
            }
        }
    }

    return false;
}

void UnlockTradedItemAction::UnlockItem(Item* item)
{
    if (!bot->HasSpell(PICK_LOCK_SPELL_ID))
    {
        botAI->TellError("Cannot unlock, Pick Lock spell is missing.");
        return;
    }

    // Use CastSpell to unlock the item
    if (botAI->CastSpell(PICK_LOCK_SPELL_ID, bot->GetTrader(), item)) // Unit target is trader
    {
        std::ostringstream out;
        out << "Picking Lock on traded item: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
    {
        botAI->TellError("Failed to cast Pick Lock.");
    }
}
