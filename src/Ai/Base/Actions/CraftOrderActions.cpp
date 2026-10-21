/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftOrderActions.h"

#include "CraftOrder.h"
#include "CraftValue.h"
#include "Event.h"
#include "MailAction.h"
#include "Playerbots.h"

bool CraftOrderAction::isPossible()
{
    CraftData& data = botAI->GetCraftOrder();
    CraftOrder::SyncWithBags(bot, data);
    if (!data.HasReagentsForNextCraft())
        return false;

    // The reagents are used up before the item is created, so there must be room for everything one cast can make.
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(data.spellId);
    if (!proto || !spellInfo)
        return false;

    ItemPosCountVec dest;
    uint32 const created = CraftOrder::GetMaxCreated(spellInfo, proto);
    if (bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, data.itemId, created) != EQUIP_ERR_OK)
        return false;

    return botAI->CanCastSpell(data.spellId, bot);
}

bool CraftOrderAction::Execute(Event /*event*/)
{
    CraftData& data = botAI->GetCraftOrder();
    data.castPending = botAI->CastSpell(data.spellId, bot);
    return data.castPending;
}

bool RefundCraftOrderAction::Execute(Event /*event*/)
{
    CraftOrder::Refund(botAI);
    CraftOrder::TellStatus(botAI);
    return true;
}

bool CollectMailPaymentsAction::isUseful() { return CraftOrder::HasCollectableMail(botAI); }

bool CollectMailPaymentsAction::Execute(Event /*event*/)
{
    GameObject* mailbox = botAI->GetGameObject(MailProcessor::FindMailbox(botAI));
    if (!mailbox)
        return false;

    if (!bot->IsWithinDistInMap(mailbox, INTERACTION_DISTANCE))
        return MoveNear(mailbox);

    CraftOrder::CollectMail(botAI, mailbox->GetGUID());
    return true;
}
