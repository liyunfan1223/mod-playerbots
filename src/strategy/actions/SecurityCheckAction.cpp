/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SecurityCheckAction.h"

#include "Event.h"
#include "Playerbots.h"

bool SecurityCheckAction::isUseful()
{
    return sRandomPlayerbotMgr->IsRandomBot(bot) && botAI->GetMaster() &&
           botAI->GetMaster()->GetSession()->GetSecurity() < SEC_GAMEMASTER && !GET_PLAYERBOT_AI(botAI->GetMaster());
}

bool SecurityCheckAction::Execute(Event event)
{
    if (Group* group = bot->GetGroup())
    {
        LootMethod method = group->GetLootMethod();
        ItemQualities threshold = group->GetLootThreshold();
        if (method == MASTER_LOOT || method == FREE_FOR_ALL || threshold > ITEM_QUALITY_UNCOMMON)
        {
            if ((botAI->GetGroupMaster()->GetSession()->GetSecurity() == SEC_PLAYER) &&
                (!bot->GetGuildId() || bot->GetGuildId() != botAI->GetGroupMaster()->GetGuildId()))
            {
                botAI->TellError("I will play with this loot type only if I'm in your guild :/");
                botAI->ChangeStrategy("+passive,+stay", BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy("+passive,+stay", BOT_STATE_COMBAT);
            }
            return true;
        }
    }

    return false;
}
