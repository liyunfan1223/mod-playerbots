/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MeleeShamanStrategy.h"
#include "Playerbots.h"

class MeleeShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        MeleeShamanStrategyActionNodeFactory()
        {
            creators["stormstrike"] = &stormstrike;
            creators["lava lash"] = &lava_lash;
            creators["magma totem"] = &magma_totem;
        }

    private:
        static ActionNode* stormstrike([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("stormstrike",
                /*P*/ nullptr,
                // /*A*/ NextAction::array(0, new NextAction("lava lash"), nullptr),
                nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* lava_lash([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("lava lash",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("magma totem",
                /*P*/ NULL,
                /*A*/ NextAction::array(0, new NextAction("searing totem"), NULL),
                /*C*/ NULL);
        }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new MeleeShamanStrategyActionNodeFactory());
}

NextAction** MeleeShamanStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("stormstrike", ACTION_DEFAULT + 0.5f), 
        new NextAction("earth shock", ACTION_DEFAULT + 0.4f), 
        new NextAction("feral spirit", ACTION_DEFAULT + 0.3f),
        new NextAction("fire nova", ACTION_DEFAULT + 0.2f),
        new NextAction("lava lash", ACTION_DEFAULT + 0.1f), 
        new NextAction("melee", ACTION_DEFAULT), 
        NULL);
}

void MeleeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new NextAction("flametongue weapon", 22.0f), nullptr)));
    // triggers.push_back(new TriggerNode("searing totem", NextAction::array(0, new NextAction("reach melee", 22.0f), new NextAction("searing totem", 22.0f), nullptr)));
    triggers.push_back(new TriggerNode("flame shock", NextAction::array(0, new NextAction("flame shock", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "maelstrom weapon",
        NextAction::array(0, new NextAction("lightning bolt", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode("not facing target", NextAction::array(0, new NextAction("set facing", ACTION_NORMAL + 7), nullptr)));
    // triggers.push_back(new TriggerNode("enemy too close for melee", NextAction::array(0, new NextAction("move out of enemy contact", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("strength of earth totem", ACTION_LIGHT_HEAL), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 8), nullptr)));

    triggers.push_back(new TriggerNode(
        "no fire totem",
        NextAction::array(0, 
        new NextAction("reach melee", 23.0f), 
        new NextAction("magma totem", 22.0f), nullptr)));

    triggers.push_back(new TriggerNode("fire elemental totem", 
        NextAction::array(0, 
            new NextAction("reach melee", 33.0f), 
            new NextAction("fire elemental totem", 32.0f), 
            nullptr)));

    triggers.push_back(new TriggerNode(
        "no air totem",
        NextAction::array(0, new NextAction("windfury totem", 20.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("shamanistic rage", 23.0f), nullptr)));
}

void MeleeAoeShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    
    // triggers.push_back(new TriggerNode("magma totem", NextAction::array(0, new NextAction("magma totem", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fire nova", 25.0f), nullptr)));
}
