/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericShamanStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "AiFactory.h"

class GenericShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericShamanStrategyActionNodeFactory()
    {
        creators["totem of wrath"] = &totem_of_wrath;
        creators["flametongue totem"] = &flametongue_totem;
        creators["magma totem"] = &magma_totem;
        creators["searing totem"] = &searing_totem;
        creators["strength of earth totem"] = &strength_of_earth_totem;
        creators["stoneskin totem"] = &stoneskin_totem;
        creators["cleansing totem"] = &cleansing_totem;
        creators["mana spring totem"] = &mana_spring_totem;
        creators["healing stream totem"] = &healing_stream_totem;
        creators["wrath of air totem"] = &wrath_of_air_totem;
        creators["windfury totem"] = &windfury_totem;
        creators["grounding totem"] = &grounding_totem;
        creators["wind shear"] = &wind_shear;
        creators["purge"] = &purge;
    }

private:
    // Passthrough totems are set up so lower level shamans will still cast totems.
    // Totem of Wrath -> Flametongue Totem -> Searing Totem
    // Magma Totem -> Searing Totem
    // Strength of Earth Totem -> Stoneskin Totem
    // Cleansing Totem -> Mana Spring Totem
    // Wrath of Air Totem -> Windfury Totem -> Grounding Totem

    static ActionNode* totem_of_wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("totem of wrath",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* flametongue_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flametongue totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("searing totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* magma_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("magma totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("searing totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* searing_totem(PlayerbotAI*) { return new ActionNode("searing totem", nullptr, nullptr, nullptr); }
    static ActionNode* strength_of_earth_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("strength of earth totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("stoneskin totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* stoneskin_totem(PlayerbotAI*) { return new ActionNode("stoneskin totem", nullptr, nullptr, nullptr); }
    static ActionNode* cleansing_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cleansing totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mana spring totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* mana_spring_totem(PlayerbotAI*) { return new ActionNode("mana spring totem", nullptr, nullptr, nullptr); }
    static ActionNode* healing_stream_totem(PlayerbotAI*) { return new ActionNode("healing stream totem", nullptr, nullptr, nullptr); }
    static ActionNode* wrath_of_air_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wrath of air totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("windfury totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* windfury_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("windfury totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("grounding totem"), nullptr),
                              /*C*/ nullptr);
    }
    static ActionNode* grounding_totem(PlayerbotAI*) { return new ActionNode("grounding totem", nullptr, nullptr, nullptr); }
    static ActionNode* wind_shear(PlayerbotAI*) { return new ActionNode("wind shear", nullptr, nullptr, nullptr); }
    static ActionNode* purge(PlayerbotAI*) { return new ActionNode("purge", nullptr, nullptr, nullptr); }
};

GenericShamanStrategy::GenericShamanStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericShamanStrategyActionNodeFactory());
}

void GenericShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("wind shear", NextAction::array(0, new NextAction("wind shear", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("wind shear on enemy healer", NextAction::array(0, new NextAction("wind shear on enemy healer", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("purge", NextAction::array(0, new NextAction("purge", ACTION_DISPEL), nullptr)));
    triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new NextAction("mana potion", ACTION_DISPEL), nullptr)));
    triggers.push_back(new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 65.0f), nullptr)));
}

void ShamanCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("cure poison", NextAction::array(0, new NextAction("cure poison", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure poison", NextAction::array(0, new NextAction("cure poison on party", 21.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit poison", NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cleanse spirit poison", NextAction::array(0, new NextAction("cleanse spirit poison on party", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("cure disease", NextAction::array(0, new NextAction("cure disease", 31.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cure disease", NextAction::array(0, new NextAction("cure disease on party", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit disease", NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cleanse spirit disease", NextAction::array(0, new NextAction("cleanse spirit disease on party", 23.0f), nullptr)));
    triggers.push_back(new TriggerNode("cleanse spirit curse", NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cleanse spirit curse", NextAction::array(0, new NextAction("cleanse spirit curse on party", 23.0f), nullptr)));
}

void ShamanBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("heroism", NextAction::array(0, new NextAction("heroism", 30.0f), nullptr)));
    triggers.push_back(new TriggerNode("bloodlust", NextAction::array(0, new NextAction("bloodlust", 30.0f), nullptr)));

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(new TriggerNode("fire elemental totem", NextAction::array(0, new NextAction("fire elemental totem", 23.0f), nullptr)));
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(new TriggerNode("fire elemental totem", NextAction::array(0, new NextAction("fire elemental totem melee", 24.0f), nullptr)));
    }
}

void ShamanAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{

    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Elemental
    {
        triggers.push_back(new TriggerNode("medium aoe",NextAction::array(0, new NextAction("fire nova", 23.0f), nullptr)));
        triggers.push_back(new TriggerNode("chain lightning no cd", NextAction::array(0, new NextAction("chain lightning", 5.6f), nullptr)));
    }
    else if (tab == 1)  // Enhancement
    {
        triggers.push_back(new TriggerNode("medium aoe",NextAction::array(0,
                                                    new NextAction("magma totem", 24.0f),
                                                    new NextAction("fire nova", 23.0f), nullptr)));

        triggers.push_back(new TriggerNode("maelstrom weapon 5 and medium aoe", NextAction::array(0, new NextAction("chain lightning", 22.0f), nullptr)));
        triggers.push_back(new TriggerNode("maelstrom weapon 4 and medium aoe", NextAction::array(0, new NextAction("chain lightning", 21.0f), nullptr)));
        triggers.push_back(new TriggerNode("enemy within melee", NextAction::array(0, new NextAction("fire nova", 5.1f), nullptr)));
    }
    else if (tab == 2)  // Restoration
    {
        // Handled by "Healer DPS" Strategy
    }
}
