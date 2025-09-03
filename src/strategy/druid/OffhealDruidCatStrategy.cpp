/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

 #include "OffhealDruidCatStrategy.h"

 #include "Playerbots.h"
 #include "Strategy.h"

 class OffhealDruidCatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    OffhealDruidCatStrategyActionNodeFactory()
    {
        creators["cat form"] = &cat_form;
        creators["mangle (cat)"] = &mangle_cat;
        creators["shred"] = &shred;
        creators["rake"] = &rake;
        creators["rip"] = &rip;
        creators["ferocious bite"] = &ferocious_bite;
        creators["savage roar"] = &savage_roar;
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["healing touch on party"] = &healing_touch_on_party;
        creators["regrowth on party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
    }

private:
    static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cat form",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mangle (cat)",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* shred([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("shred",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("claw"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rake",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rip",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ferocious bite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("rip"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* savage_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("savage roar",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire (feral)",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }
};

OffhealDruidCatStrategy::OffhealDruidCatStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new OffhealDruidCatStrategyActionNodeFactory());
}

NextAction** OffhealDruidCatStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("mangle (cat)", ACTION_DEFAULT + 0.5f),
                             new NextAction("shred", ACTION_DEFAULT + 0.4f),
                             new NextAction("rake", ACTION_DEFAULT + 0.3f), new NextAction("melee", ACTION_DEFAULT),
                             new NextAction("cat form", ACTION_DEFAULT - 0.1f), nullptr);
}

void OffhealDruidCatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("cat form", NextAction::array(0, new NextAction("cat form", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(
        new TriggerNode("savage roar", NextAction::array(0, new NextAction("savage roar", ACTION_HIGH + 7), nullptr)));
    triggers.push_back(new TriggerNode("combo points available",
                                       NextAction::array(0, new NextAction("rip", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(new TriggerNode(
        "ferocious bite time", NextAction::array(0, new NextAction("ferocious bite", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("target with combo points almost dead",
                        NextAction::array(0, new NextAction("ferocious bite", ACTION_HIGH + 4), nullptr)));
    triggers.push_back(new TriggerNode("mangle (cat)",
                                       NextAction::array(0, new NextAction("mangle (cat)", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode("rake", NextAction::array(0, new NextAction("rake", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode("almost full energy available",
                                       NextAction::array(0, new NextAction("shred", ACTION_DEFAULT + 0.4f), nullptr)));
    triggers.push_back(new TriggerNode("combo points not full",
                                       NextAction::array(0, new NextAction("shred", ACTION_DEFAULT + 0.4f), nullptr)));
    triggers.push_back(new TriggerNode(
        "faerie fire (feral)", NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("enemy out of melee",
                                       NextAction::array(0, new NextAction("feral charge - cat", ACTION_HIGH + 9),
                                                         new NextAction("dash", ACTION_HIGH + 8), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "low energy", NextAction::array(0, new NextAction("tiger's fury", ACTION_NORMAL + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 5), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 8), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("innervate", ACTION_HIGH + 4), nullptr)));
}
