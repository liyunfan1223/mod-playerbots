/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HolyPriestStrategy.h"

#include "Playerbots.h"

class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyPriestStrategyActionNodeFactory() { creators["smite"] = &smite; }

private:
    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("smite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                              /*C*/ nullptr);
    }
};

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* botAI) : HealPriestStrategy(botAI)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

NextAction** HolyPriestStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("smite", ACTION_DEFAULT + 0.2f),
                             new NextAction("mana burn", ACTION_DEFAULT + 0.1f),
                             new NextAction("starshards", ACTION_DEFAULT), nullptr);
}

void HolyPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("holy fire", NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("shadowfiend", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("mana burn", ACTION_HIGH), nullptr)));
}

HolyHealPriestStrategy::HolyHealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

NextAction** HolyHealPriestStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void HolyHealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("group heal setting",
                        NextAction::array(0, 
                            new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 9),
                            new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 8), nullptr)));

    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        NextAction::array(0, new NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("circle of healing on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, 
                          new NextAction("guardian spirit on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                          new NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 1),
                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 4),
                                          new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("circle of healing on party", ACTION_LIGHT_HEAL + 7),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 6),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 5),
                                          new NextAction("flash heal on party", ACTION_LIGHT_HEAL + 4),
                                          // new NextAction("renew on party", ACTION_LIGHT_HEAL + 8),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0,
                                          new NextAction("renew on party", ACTION_LIGHT_HEAL + 2),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 1),
                                          nullptr)));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10), nullptr)));
}
