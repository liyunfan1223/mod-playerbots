
#include "AssassinationRogueStrategy.h"

#include "Playerbots.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
    }

private:
    static ActionNode* mutilate(PlayerbotAI* ai)
    {
        return new ActionNode("mutilate",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("sinister strike"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* envenom(PlayerbotAI* ai)
    {
        return new ActionNode("envenom",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("eviscerate"), NULL),
                              /*C*/ NULL);
    }
};

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

NextAction** AssassinationRogueStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("melee", ACTION_DEFAULT), NULL);
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("high energy available",
                                       NextAction::array(0, new NextAction("garrote", ACTION_HIGH + 7),
                                                         new NextAction("ambush", ACTION_HIGH + 6), nullptr)));

    triggers.push_back(new TriggerNode("high energy available",
                                       NextAction::array(0, new NextAction("mutilate", ACTION_NORMAL + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "hunger for blood", NextAction::array(0, new NextAction("hunger for blood", ACTION_HIGH + 6), NULL)));

    triggers.push_back(new TriggerNode("slice and dice",
                                       NextAction::array(0, new NextAction("slice and dice", ACTION_HIGH + 5), NULL)));

    triggers.push_back(new TriggerNode("combo points 3 available",
                                       NextAction::array(0, new NextAction("envenom", ACTION_HIGH + 4), NULL)));

    triggers.push_back(
        new TriggerNode("expose armor", NextAction::array(0, new NextAction("expose armor", ACTION_HIGH + 3), NULL)));

    triggers.push_back(
        new TriggerNode("medium threat", NextAction::array(0, new NextAction("vanish", ACTION_HIGH), NULL)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("evasion", ACTION_HIGH + 9),
                                                        new NextAction("feint", ACTION_HIGH + 8), nullptr)));

    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("cloak of shadows", ACTION_HIGH + 7), nullptr)));

    triggers.push_back(
        new TriggerNode("kick", NextAction::array(0, new NextAction("kick", ACTION_INTERRUPT + 2), NULL)));

    triggers.push_back(
        new TriggerNode("kick on enemy healer",
                        NextAction::array(0, new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("fan of knives", ACTION_NORMAL + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "low tank threat",
        NextAction::array(0, new NextAction("tricks of the trade on main tank", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array(0, new NextAction("stealth", ACTION_HIGH + 3), new NextAction("sprint", ACTION_HIGH + 2),
                          new NextAction("reach melee", ACTION_HIGH + 1), NULL)));
}
