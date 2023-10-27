/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ArcaneMageStrategy.h"
#include "Playerbots.h"

class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        ArcaneMageStrategyActionNodeFactory()
        {
            creators["arcane blast"] = &arcane_blast;
            creators["arcane barrage"] = &arcane_barrage;
            creators["arcane missiles"] = &arcane_missiles;
            // creators["firebolt"] = &firebolt;
        }

    private:
        static ActionNode* arcane_blast([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("arcane blast",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("arcane missiles"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* arcane_barrage([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("arcane barrage",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("arcane missiles"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* arcane_missiles([[maybe_unused]] PlayerbotAI* botAI)
        {
            return new ActionNode ("arcane missiles",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("fireball"), nullptr),
                /*C*/ nullptr);
        }

        // static ActionNode* firebolt([[maybe_unused]] PlayerbotAI* botAI)
        // {
        //     return new ActionNode ("firebolt",
        //         /*P*/ nullptr,
        //         /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
        //         /*C*/ nullptr);
        // }
};

ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new ArcaneMageStrategyActionNodeFactory());
}

NextAction** ArcaneMageStrategy::getDefaultActions()
{
    return NextAction::array(0, 
        new NextAction("arcane blast", ACTION_DEFAULT + 0.1f), 
        new NextAction("shoot", ACTION_DEFAULT), 
        NULL);
}

void ArcaneMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("arcane blast stack", NextAction::array(0, new NextAction("arcane missiles", 15.0f), NULL)));
}

void ArcaneMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
	// triggers.push_back(new TriggerNode(
	// 	"high aoe",
	// 	NextAction::array(0, new NextAction("arcane explosion", 39.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array(0, new NextAction("blizzard", 40.0f), NULL)));

}