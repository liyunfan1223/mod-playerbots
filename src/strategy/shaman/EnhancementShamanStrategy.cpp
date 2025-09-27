/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EnhancementShamanStrategy.h"

#include "Playerbots.h"

// ===== Action Node Factory =====
class EnhancementShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    EnhancementShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["feral spirit"] = &feral_spirit;
        creators["lightning bolt"] = &lightning_bolt;
        creators["earth shock"] = &earth_shock;
        creators["flame shock"] = &flame_shock;
        creators["shamanistic rage"] = &shamanistic_rage;
        creators["call of the elements"] = &call_of_the_elements;
        creators["lightning shield"] = &lightning_shield;
    }

private:
    static ActionNode* stormstrike(PlayerbotAI*) { return new ActionNode("stormstrike", nullptr, nullptr, nullptr); }
    static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lava lash",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* feral_spirit(PlayerbotAI*) { return new ActionNode("feral spirit", nullptr, nullptr, nullptr); }
    static ActionNode* lightning_bolt(PlayerbotAI*) { return new ActionNode("lightning bolt", nullptr, nullptr, nullptr); }
    static ActionNode* earth_shock(PlayerbotAI*) { return new ActionNode("earth shock", nullptr, nullptr, nullptr); }
    static ActionNode* flame_shock(PlayerbotAI*) { return new ActionNode("flame shock", nullptr, nullptr, nullptr); }
    static ActionNode* shamanistic_rage(PlayerbotAI*) { return new ActionNode("shamanistic rage", nullptr, nullptr, nullptr); }
    static ActionNode* call_of_the_elements(PlayerbotAI*) { return new ActionNode("call of the elements", nullptr, nullptr, nullptr); }
    static ActionNode* lightning_shield(PlayerbotAI*) { return new ActionNode("lightning shield", nullptr, nullptr, nullptr); }
};

// ===== Single Target Strategy =====
EnhancementShamanStrategy::EnhancementShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new EnhancementShamanStrategyActionNodeFactory());
}

// ===== Default Actions =====
NextAction** EnhancementShamanStrategy::getDefaultActions()
{
    return NextAction::array(0,
       new NextAction("stormstrike", 5.5f),
       new NextAction("feral spirit", 5.4f),
       new NextAction("earth shock", 5.3f),
       new NextAction("lava lash", 5.2f),
       new NextAction("melee", 5.0f), NULL);
}

// ===== Trigger Initialization ===
void EnhancementShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // Totem Trigger
    triggers.push_back(new TriggerNode("call of the elements and enemy within melee", NextAction::array(0, new NextAction("call of the elements", 60.0f), nullptr)));

    // Spirit Walk Trigger
    triggers.push_back(new TriggerNode("spirit walk ready", NextAction::array(0, new NextAction("spirit walk", 50.0f), nullptr)));

    // Damage Triggers
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", 40.0f), nullptr)));
    triggers.push_back(new TriggerNode("maelstrom weapon 5", NextAction::array(0, new NextAction("lightning bolt", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("maelstrom weapon 4", NextAction::array(0, new NextAction("lightning bolt", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("lightning shield", NextAction::array(0, new NextAction("lightning shield", 18.5f), nullptr)));

    // Health/Mana Triggers
    triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new NextAction("shamanistic rage", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("shamanistic rage", 23.0f), nullptr)));
}
