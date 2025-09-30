/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"
#include "Playerbots.h"

// These combat strategies are used to set the corresponding totems on the bar, and cast the totem when it's missing.
// There are special cases for Totem of Wrath, Windfury Totem, Wrath of Air totem, and Cleansing totem - these totems
// aren't learned at level 30, and have fallbacks in order to prevent the trigger from continuously firing.

// Earth Totems
StrengthOfEarthTotemStrategy::StrengthOfEarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StrengthOfEarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set strength of earth totem", NextAction::array(0, new NextAction("set strength of earth totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("strength of earth totem", 55.0f), nullptr)));
}

StoneclawTotemStrategy::StoneclawTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StoneclawTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set stoneskin totem", NextAction::array(0, new NextAction("set stoneskin totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("stoneskin totem", 55.0f), nullptr)));
}

EarthTotemStrategy::EarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set tremor totem", NextAction::array(0, new NextAction("set tremor totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("tremor totem", 55.0f), nullptr)));
}

EarthbindTotemStrategy::EarthbindTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthbindTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set earthbind totem", NextAction::array(0, new NextAction("set earthbind totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no earth totem", NextAction::array(0, new NextAction("earthbind totem", 55.0f), nullptr)));
}

// Fire Totems
SearingTotemStrategy::SearingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void SearingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set searing totem", NextAction::array(0, new NextAction("set searing totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("searing totem", 55.0f), nullptr)));
}

MagmaTotemStrategy::MagmaTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void MagmaTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set magma totem", NextAction::array(0, new NextAction("set magma totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("magma totem", 55.0f), nullptr)));
}

FlametongueTotemStrategy::FlametongueTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FlametongueTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set flametongue totem", NextAction::array(0, new NextAction("set flametongue totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("flametongue totem", 55.0f), nullptr)));
}

TotemOfWrathStrategy::TotemOfWrathStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void TotemOfWrathStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Totem of Wrath yet, set Flametongue Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(30706))
    {
        triggers.push_back(new TriggerNode("set totem of wrath", NextAction::array(0, new NextAction("set totem of wrath", 60.0f), nullptr)));
    }
    else if (bot->HasSpell(8227))
    {
        triggers.push_back(new TriggerNode("set flametongue totem", NextAction::array(0, new NextAction("set flametongue totem", 60.0f), nullptr)));
    }
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("totem of wrath", 55.0f), nullptr)));
}

FrostResistanceTotemStrategy::FrostResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FrostResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set frost resistance totem", NextAction::array(0, new NextAction("set frost resistance totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no fire totem", NextAction::array(0, new NextAction("frost resistance totem", 55.0f), nullptr)));
}

// Water Totems
HealingStreamTotemStrategy::HealingStreamTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void HealingStreamTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set healing stream totem", NextAction::array(0, new NextAction("set healing stream totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("healing stream totem", 55.0f), nullptr)));
}

ManaSpringTotemStrategy::ManaSpringTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void ManaSpringTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set mana spring totem", NextAction::array(0, new NextAction("set mana spring totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("mana spring totem", 55.0f), nullptr)));
}

CleansingTotemStrategy::CleansingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void CleansingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Cleansing Totem yet, set Mana Spring Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8170))
    {
        triggers.push_back(new TriggerNode("set cleansing totem", NextAction::array(0, new NextAction("set cleansing totem", 60.0f), nullptr)));
    }
    else if (bot->HasSpell(5675))
    {
        triggers.push_back(new TriggerNode("set mana spring totem", NextAction::array(0, new NextAction("set mana spring totem", 60.0f), nullptr)));
    }
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("cleansing totem", 55.0f), nullptr)));
}

FireResistanceTotemStrategy::FireResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FireResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set fire resistance totem", NextAction::array(0, new NextAction("set fire resistance totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no water totem", NextAction::array(0, new NextAction("fire resistance totem", 55.0f), nullptr)));
}

// Air Totems
WrathOfAirTotemStrategy::WrathOfAirTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WrathOfAirTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Wrath of Air Totem yet, set Grounding Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(3738))
    {
        triggers.push_back(new TriggerNode("set wrath of air totem", NextAction::array(0, new NextAction("set wrath of air totem", 60.0f), nullptr)));
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    }
    triggers.push_back(
        new TriggerNode("no air totem", NextAction::array(0, new NextAction("wrath of air totem", 55.0f), nullptr)));
}

WindfuryTotemStrategy::WindfuryTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WindfuryTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Windfury Totem yet, set Grounding Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8512))
    {
        triggers.push_back(new TriggerNode("set windfury totem", NextAction::array(0, new NextAction("set windfury totem", 60.0f), nullptr)));
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    }
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("windfury totem", 55.0f), nullptr)));
}

NatureResistanceTotemStrategy::NatureResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void NatureResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set nature resistance totem", NextAction::array(0, new NextAction("set nature resistance totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("nature resistance totem", 55.0f), nullptr)));
}

GroundingTotemStrategy::GroundingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void GroundingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set grounding totem", NextAction::array(0, new NextAction("set grounding totem", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("no air totem", NextAction::array(0, new NextAction("grounding totem", 55.0f), nullptr)));
}
