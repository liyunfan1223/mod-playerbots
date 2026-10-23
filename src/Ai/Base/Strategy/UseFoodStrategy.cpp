/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UseFoodStrategy.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

void UseFoodStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);
    // Above "attack anything" (4.0), which is what GrindingStrategy already does with its
    // own 4.1/4.2. Outside grind nobody did, so eating lost the arbitration every time and
    // a bot that had just fled a fight at 30% health walked straight into the next one --
    // back then AttackAnythingAction::isUseful did not look at the bot's health (it now
    // gates on lowHealth as well).
    if (botAI->HasCheat(BotCheatMask::food))
    {
        triggers.push_back(new TriggerNode("medium health", { NextAction("food", 4.1f) }));
        triggers.push_back(new TriggerNode("high mana", { NextAction("drink", 4.2f) }));
    }
    else
    {
        triggers.push_back(new TriggerNode("low health", { NextAction("food", 4.1f) }));
        triggers.push_back(new TriggerNode("low mana", { NextAction("drink", 4.2f) }));
    }
}
