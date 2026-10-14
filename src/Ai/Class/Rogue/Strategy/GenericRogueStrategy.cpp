/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericRogueStrategy.h"

class GenericRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericRogueStrategyActionNodeFactory()
    {
        creators["use deadly poison on off hand"] = &use_deadly_poison_on_off_hand;
    }

private:
    static ActionNode* use_deadly_poison_on_off_hand([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("use deadly poison on off hand",
                            /*P*/ {},
                            /*A*/ { NextAction("use instant poison on off hand") },
                            /*C*/ {});
    }
};

GenericRogueStrategy::GenericRogueStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericRogueStrategyActionNodeFactory());
}

void GenericRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // The right priority for poisons is probably above any attack but below any survival ability.
    // Everything about Rogues needs to be redone, but right now 26 is just below Cloak of Shadows
    // and Evasion and just above Slice and Dice.
    triggers.push_back(
        new TriggerNode(
            "main hand weapon no enchant",
            {
                NextAction("use instant poison on main hand", 26.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "off hand weapon no enchant",
            {
                NextAction("use deadly poison on off hand", 25.5f)
            }
        )
    );
}
