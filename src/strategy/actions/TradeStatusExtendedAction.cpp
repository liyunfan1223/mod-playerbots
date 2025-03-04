#include "TradeStatusExtendedAction.h"
#include "Event.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "WorldPacket.h"
#include "TradeData.h"

bool TradeStatusExtendedAction::Execute(Event event)
{
    Player* trader = bot->GetTrader();
    if (!trader)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint8 isTraderData;
    uint32 unknown1, slotCount1, slotCount2, tradeGold, spellCast;
    p >> isTraderData;
    p >> unknown1;
    p >> slotCount1;
    p >> slotCount2;
    p >> tradeGold;
    p >> spellCast;

    for (uint8 i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        uint8 tradeSlot;
        p >> tradeSlot;

        if (tradeSlot >= TRADE_SLOT_COUNT)
            break; // End of packet

        uint32 itemId, displayId, count, wrapped, lockId;
        uint64 giftCreator, creator;
        uint32 permEnchant, gem1, gem2, gem3;
        uint32 spellCharges, suffixFactor, randomProp, maxDurability, durability;

        p >> itemId;
        p >> displayId;
        p >> count;
        p >> wrapped;
        p >> giftCreator;
        p >> permEnchant;
        p >> gem1;
        p >> gem2;
        p >> gem3;
        p >> creator;
        p >> spellCharges;
        p >> suffixFactor;
        p >> randomProp;
        p >> lockId;
        p >> maxDurability;
        p >> durability;

        // Check for locked items in "Do Not Trade" slot
        if (tradeSlot == TRADE_SLOT_NONTRADED && lockId > 0)
        {
            botAI->TellMaster("Detecting locked item in trade...");

            // Get the actual item reference from TradeData
            Item* lockbox = tradeData->GetItem(TRADE_SLOT_NONTRADED);
            if (!lockbox)
            {
                botAI->TellMaster("Error: Cannot find the lockbox item.");
                return false;
            }

            if (bot->getClass() == CLASS_ROGUE && bot->HasSpell(1804) && lockbox->IsLocked()) // Pick Lock spell
            {
                botAI->TellMaster("Let me unlock that for you...");
                botAI->CastSpell(1804, bot, lockbox); // Attempt to cast Pick Lock on the lockbox
                botAI->SetNextCheckDelay(4000); // Delay before accepting trade
            }
            else
            {
                botAI->TellMaster("I can't unlock this item.");
            }
        }
    }

    return true;
}
