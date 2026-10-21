/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftOrderTriggers.h"

#include "CraftOrder.h"
#include "CraftValue.h"
#include "MailAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool CraftOrderReadyTrigger::IsActive()
{
    return botAI->GetCraftOrder().HasReagentsForNextCraft() && !bot->IsNonMeleeSpellCast(false);
}

bool CraftOrderStaleTrigger::IsActive()
{
    CraftData const& data = botAI->GetCraftOrder();
    if (data.IsEmpty())
        return false;

    Player* master = botAI->GetMaster();
    if (master && master->GetGUID() != data.owner)
        return true;

    return getMSTimeDiff(data.lastProgress, getMSTime()) >= sPlayerbotAIConfig.CraftOrderTimeout * IN_MILLISECONDS;
}

// Bots that mailed a refund in the last month collect what comes back as they pass a mailbox, never out in the world.
bool MailToCollectTrigger::IsActive()
{
    return CraftOrder::HasCollectableMail(botAI) && !MailProcessor::FindMailbox(botAI).IsEmpty();
}
