/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinBuffStrategies.h"

#include "Playerbots.h"

void PaladinBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of wisdom on party",
                        NextAction::array(0, new NextAction("blessing of wisdom on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of wisdom",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffHealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of kings on party",
                        NextAction::array(0, new NextAction("blessing of kings on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of kings",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of might on party",
                        NextAction::array(0, new NextAction("blessing of might on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of might",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shadow resistance aura",
                        NextAction::array(0, new NextAction("shadow resistance aura", ACTION_NORMAL), nullptr)));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("frost resistance aura",
                        NextAction::array(0, new NextAction("frost resistance aura", ACTION_NORMAL), nullptr)));
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "fire resistance aura", NextAction::array(0, new NextAction("fire resistance aura", ACTION_NORMAL), nullptr)));
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("devotion aura",
                                       NextAction::array(0, new NextAction("devotion aura", ACTION_NORMAL), nullptr)));
}

void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "retribution aura", NextAction::array(0, new NextAction("retribution aura", ACTION_NORMAL), nullptr)));
}

void PaladinBuffCastStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "concentration aura", NextAction::array(0, new NextAction("concentration aura", ACTION_NORMAL), nullptr)));
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "righteous fury", NextAction::array(0, new NextAction("righteous fury", ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffStatsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of kings on party",
                        NextAction::array(0, new NextAction("blessing of kings on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of kings",
    // ACTION_HIGH + 8), nullptr)));
}
