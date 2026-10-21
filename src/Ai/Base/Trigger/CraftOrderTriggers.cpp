/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftOrderTriggers.h"

#include "CraftValue.h"
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
