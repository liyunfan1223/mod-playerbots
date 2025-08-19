/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "StartDuelAction.h"
#include "Playerbots.h"
#include "Player.h"
#include "WorldSession.h"
#include "DuelTargetValue.h"

#define SPELL_DUEL 7266

bool StartDuelAction::Execute(Event event)
{
    Player* target = AI_VALUE(Player*, "rpg target"); // ou "duel target", se ajustar o trigger
    if (!target)
        return false;

    // Already in a duel?
    if (bot->duel || target->duel)
        return false;

    // Distance and same map
    if (bot->GetMapId() != target->GetMapId() || !bot->IsWithinDistInMap(target, INTERACTION_DISTANCE))
        return false;

    // The area must allow for dueling
    AreaTableEntry const* targetAreaEntry = sAreaTableStore.LookupEntry(target->GetAreaId());
    if (!targetAreaEntry || !(targetAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
        return false;

    // Not dueling with very different players
    if (abs(int32(bot->GetLevel()) - int32(target->GetLevel())) > 10)
        return false;

    // Interrupts any current action
    bot->AttackStop();
    bot->CastStop();

    // Safest method: Cast the spell "Duel" (ID 7266)
    bot->CastSpell(target, SPELL_DUEL, true);

    botAI->TellMaster("I challenge " + target->GetName() + " to a duel!");
    return true;
}

bool StartDuelAction::isUseful()
{
    if (!bot->IsAlive() || bot->IsInCombat() || bot->duel)
        return false;

    if (bot->InBattleground() || bot->InArena())
        return false;

    return true;
}
