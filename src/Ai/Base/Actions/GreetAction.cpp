/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GreetAction.h"
#include "Event.h"
#include "Playerbots.h"

GreetAction::GreetAction(PlayerbotAI* botAI) : Action(botAI, "greet") {}

bool GreetAction::Execute(Event /*event*/)
{
    ObjectGuid guid = AI_VALUE(ObjectGuid, "new player nearby");
    if (!guid || !guid.IsPlayer())
        return false;

    Player* player = dynamic_cast<Player*>(botAI->GetUnit(guid));
    if (!player)
        return false;

    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, player, sPlayerbotAIConfig.SightDistance))
        bot->SetFacingToObject(player);

    ObjectGuid oldSel = bot->GetTarget();
    bot->SetTarget(guid);
    // bot->HandleEmote(EMOTE_ONESHOT_WAVE);
    botAI->PlayEmote(TEXT_EMOTE_HELLO);
    bot->SetTarget(oldSel);

    GuidSet& alreadySeenPlayers = botAI->GetAiObjectContext()->GetValue<GuidSet&>("already seen players")->Get();
    alreadySeenPlayers.insert(guid);

    GuidVector nearestPlayers = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (ObjectGuid const guid : nearestPlayers)
    {
        alreadySeenPlayers.insert(guid);
    }

    return true;
}
