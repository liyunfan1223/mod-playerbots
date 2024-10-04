/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericShamanStrategy.h"
#include "HealShamanStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

class GenericShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericShamanStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["windfury weapon"] = &windfury_weapon;
        creators["lesser healing wave"] = &lesser_healing_wave;
        creators["lesser healing wave on party"] = &lesser_healing_wave_on_party;
        creators["chain heal on party"] = &chain_heal;
        creators["riptide"] = &riptide;
        creators["riptide on party"] = &riptide_on_party;
        creators["earth shock"] = &earth_shock;
    }

private:
    static ActionNode* earth_shock([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("earth shock",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flame shock"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* flametongue_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("rockbiter weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* frostbrand_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frostbrand weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* windfury_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* lesser_healing_wave([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser healing wave",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* lesser_healing_wave_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser healing wave on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave on party"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* chain_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("chain heal on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("lesser healing wave on party"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* riptide([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("riptide",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing wave"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* riptide_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("riptide on party",
                              /*P*/ nullptr,
                              // /*A*/ NextAction::array(0, new NextAction("healing wave on party"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

GenericShamanStrategy::GenericShamanStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericShamanStrategyActionNodeFactory());
}

void GenericShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("wind shear", NextAction::array(0, new NextAction("wind shear", 23.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("wind shear on enemy healer",
                        NextAction::array(0, new NextAction("wind shear on enemy healer", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("purge", NextAction::array(0, new NextAction("purge", ACTION_DISPEL), nullptr)));
    // triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("lesser
    // healing wave on party", 25.0f), nullptr))); triggers.push_back(new TriggerNode("party member low health",
    // NextAction::array(0, new NextAction("riptide on party", 25.0f), nullptr))); triggers.push_back(new
    // TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("chain heal on party", 27.0f), nullptr)));
    // triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("lesser healing
    // wave", 26.0f), nullptr))); triggers.push_back(new TriggerNode("low health", NextAction::array(0, new
    // NextAction("riptide", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("heroism", NextAction::array(0, new NextAction("heroism", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode("bloodlust", NextAction::array(0, new NextAction("bloodlust", 30.0f), nullptr)));
}

void ShamanBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("lightning shield", NextAction::array(0, new NextAction("lightning shield", 22.0f), nullptr)));
}

void ShamanBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 22.0f), nullptr)));
}

void ShamanCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("cure poison", NextAction::array(0, new NextAction("cure poison", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure poison",
                                       NextAction::array(0, new NextAction("cure poison on party", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit poison",
                                       NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member cleanse spirit poison",
                        NextAction::array(0, new NextAction("cleanse spirit poison on party", 23.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("cure disease", NextAction::array(0, new NextAction("cure disease", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure disease",
                                       NextAction::array(0, new NextAction("cure disease on party", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit disease",
                                       NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member cleanse spirit disease",
                        NextAction::array(0, new NextAction("cleanse spirit disease on party", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit curse",
                                       NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member cleanse spirit curse",
                        NextAction::array(0, new NextAction("cleanse spirit curse on party", 23.0f), nullptr)));
}

void ShamanHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        NextAction::array(0, 
                            new NextAction("flame shock", ACTION_DEFAULT + 0.2f),
                            new NextAction("lava burst", ACTION_DEFAULT + 0.1f),
                            new NextAction("lightning bolt", ACTION_DEFAULT), nullptr)));
    
    triggers.push_back(
        new TriggerNode("medium aoe and healer should attack",
                        NextAction::array(0, 
                            new NextAction("chain lightning", ACTION_DEFAULT + 0.3f), nullptr)));
}