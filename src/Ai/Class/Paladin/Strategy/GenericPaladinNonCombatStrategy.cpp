/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericPaladinNonCombatStrategy.h"
#include "AiFactory.h"
#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("party member dead", { NextAction("redemption", ACTION_CRITICAL_HEAL + 10) }));
    triggers.push_back(new TriggerNode("party member almost full health", { NextAction("flash of light on party", ACTION_MEDIUM_HEAL + 5.0f) }));
    triggers.push_back(new TriggerNode("party member medium health", { NextAction("flash of light on party", ACTION_MEDIUM_HEAL + 6.0f) }));
    triggers.push_back(new TriggerNode("party member low health", { NextAction("holy light on party", ACTION_MEDIUM_HEAL + 7.0f) }));
    triggers.push_back(new TriggerNode("party member critical health", { NextAction("holy light on party", ACTION_MEDIUM_HEAL + 8.0f) }));
    triggers.push_back(new TriggerNode("not sensing undead", { NextAction("sense undead", ACTION_IDLE + 1.0f) }));

    int specTab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (specTab == PALADIN_TAB_HOLY)
        triggers.push_back(new TriggerNode("often", { NextAction("apply oil", ACTION_IDLE + 1.0f) }));
    if (specTab == PALADIN_TAB_PROTECTION || specTab == PALADIN_TAB_RETRIBUTION)
        triggers.push_back(new TriggerNode("often", { NextAction("apply stone", ACTION_IDLE + 1.0f) }));

    triggers.push_back(new TriggerNode("blessing needed",
        { NextAction("cast blessing", ACTION_NORMAL) }));
}
