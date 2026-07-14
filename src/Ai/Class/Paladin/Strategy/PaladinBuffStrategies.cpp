/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PaladinBuffStrategies.h"
#include "Playerbots.h"

void PaladinBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
    // Intentionally empty: enabling "bwisdom" signals a forced Wisdom assignment
}

void PaladinBuffHealthStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
    // Intentionally empty: enabling "bsanc" signals a forced Sanctuary assignment
}

void PaladinBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
    // Intentionally empty: enabling "bmight" signals a forced Might assignment
}

void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shadow resistance aura",
                        { NextAction("shadow resistance aura", ACTION_NORMAL) }));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("frost resistance aura",
                        { NextAction("frost resistance aura", ACTION_NORMAL) }));
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "fire resistance aura", { NextAction("fire resistance aura", ACTION_NORMAL) }));
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("devotion aura",
                                       { NextAction("devotion aura", ACTION_NORMAL) }));
}

void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "retribution aura", { NextAction("retribution aura", ACTION_NORMAL) }));
}

void PaladinBuffCastStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "concentration aura", { NextAction("concentration aura", ACTION_NORMAL) }));
}

void PaladinBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "crusader aura", { NextAction("crusader aura", ACTION_NORMAL) }));
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "righteous fury", { NextAction("righteous fury", ACTION_HIGH + 8) }));
}

void PaladinBuffStatsStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
    // Intentionally empty: enabling "bkings" signals a forced Kings assignment
}
