/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericMageNonCombatStrategy.h"
#include "AiFactory.h"
#include "Playerbots.h"

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["ice armor"] = &ice_armor;
    }

private:
    static ActionNode* ice_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ice armor",
                              /*P*/ {},
                              /*A*/ { NextAction("frost armor") },
                              /*C*/ {});
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("conjure water", { NextAction("conjure water", 23.0f) }));
    triggers.push_back(new TriggerNode("give water", { NextAction("give water", 22.0f) }));
    triggers.push_back(new TriggerNode("arcane intellect", { NextAction("arcane intellect", 21.0f) }));
    triggers.push_back(new TriggerNode("no focus magic", { NextAction("focus magic on party", 19.0f) }));
    triggers.push_back(new TriggerNode("often", { NextAction("apply oil", 1.0f) }));
    triggers.push_back(new TriggerNode("no mana gem", { NextAction("conjure mana gem", 20.0f) }));
}

void MageBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mage armor", { NextAction("mage armor", 19.0f) }));
}

void MageBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("molten armor", { NextAction("molten armor", 19.0f) }));
}

void MageBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("arcane intellect on party", { NextAction("arcane intellect on party", 20.0f) }));
}
