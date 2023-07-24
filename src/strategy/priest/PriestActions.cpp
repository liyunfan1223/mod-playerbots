/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestActions.h"
#include "Event.h"
#include "Playerbots.h"

bool CastRemoveShadowformAction::isUseful()
{
    return botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target"));
}

bool CastRemoveShadowformAction::isPossible()
{
    return true;
}

bool CastRemoveShadowformAction::Execute(Event event)
{
    botAI->RemoveAura("shadowform");
    return true;
}

Unit* CastPowerWordShieldOnAlmostFullHealthBelow::GetTarget()
{
    Group *group = bot->GetGroup();
    for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead()) {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig->almostFullHealth) {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance) {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", NULL)) {
            continue;
        }
        return player;
    }
    return NULL;
}

bool CastPowerWordShieldOnAlmostFullHealthBelow::isUseful()
{
    Group *group = bot->GetGroup();
    for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead()) {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig->almostFullHealth) {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance) {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", NULL)) {
            continue;
        }
        return true;
    }
    return false;
}