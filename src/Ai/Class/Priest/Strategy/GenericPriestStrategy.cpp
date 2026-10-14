/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericPriestStrategy.h"
#include "GenericPriestStrategyActionNodeFactory.h"
#include "Playerbots.h"

GenericPriestStrategy::GenericPriestStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

void GenericPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("medium threat", { NextAction("fade", 55.0f) }));
    triggers.push_back(new TriggerNode("critical health", { NextAction("desperate prayer",
        ACTION_HIGH + 5) }));
    triggers.push_back(new TriggerNode(
        "critical health", { NextAction("power word: shield", ACTION_NORMAL) }));
    triggers.push_back(new TriggerNode("fear ward on main tank",
        { NextAction("fear ward on main tank", ACTION_HIGH + 3) }));
    triggers.push_back(
        new TriggerNode("low health", { NextAction("power word: shield", ACTION_HIGH) }));
    triggers.push_back(
        new TriggerNode("medium mana",
            {
                NextAction("shadowfiend", ACTION_HIGH + 2),
                NextAction("inner focus", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("low mana", { NextAction("hymn of hope", ACTION_HIGH) }));
    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       { NextAction("flee", ACTION_MOVE + 9) }));
    triggers.push_back(new TriggerNode("often", { NextAction("apply oil", 1.0f) }));
    triggers.push_back(new TriggerNode("being attacked",
        { NextAction("power word: shield", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));
}

PriestCureStrategy::PriestCureStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    actionNodeFactories.Add(new CurePriestStrategyActionNodeFactory());
}

void PriestCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("dispel magic", { NextAction("dispel magic", 41.0f) }));
    triggers.push_back(new TriggerNode("dispel magic on party",
                                       { NextAction("dispel magic on party", 40.0f) }));
    triggers.push_back(
        new TriggerNode("cure disease", { NextAction("abolish disease", 31.0f) }));
    triggers.push_back(new TriggerNode(
        "party member cure disease", { NextAction("abolish disease on party", 30.0f) }));
}

void PriestBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("power infusion", { NextAction("power infusion", 41.0f) }));
    triggers.push_back(new TriggerNode("boost", { NextAction("shadowfiend", 20.0f) }));
}

void PriestCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shackle undead", { NextAction("shackle undead", 31.0f) }));
}

void PriestHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        {
                            NextAction("shadow word: pain", ACTION_DEFAULT + 0.5f),
                            NextAction("holy fire", ACTION_DEFAULT + 0.4f),
                            NextAction("smite", ACTION_DEFAULT + 0.3f),
                            NextAction("mind blast", ACTION_DEFAULT + 0.2f),
                            NextAction("shoot", ACTION_DEFAULT) }));

    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        {
                            NextAction("mind sear", ACTION_DEFAULT + 0.5f) }));
}
