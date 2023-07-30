/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CombatStrategy.h"
#include "Playerbots.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell", ACTION_MOVE + 11), nullptr)));
    triggers.push_back(new TriggerNode("invalid target", NextAction::array(0, new NextAction("drop target", 100), nullptr)));
    triggers.push_back(new TriggerNode("mounted", NextAction::array(0, new NextAction("check mount state", 54), nullptr)));
    // triggers.push_back(new TriggerNode("out of react range", NextAction::array(0, new NextAction("flee to master", 55), nullptr)));
    triggers.push_back(new TriggerNode("combat stuck", NextAction::array(0, new NextAction("reset", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("not facing target", NextAction::array(0, new NextAction("set facing", ACTION_MOVE + 7), nullptr)));
    // triggers.push_back(new TriggerNode("combat long stuck", NextAction::array(0, new NextAction("hearthstone", 0.9f), new NextAction("repop", 0.8f), nullptr)));
}


AvoidAoeStrategy::AvoidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
}


class AvoidAoeStrategyMultiplier : public Multiplier
{
public:
    AvoidAoeStrategyMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "run away on area debuff") {}

public:
    virtual float GetValue(Action* action);

private:
};

float AvoidAoeStrategyMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string name = action->getName();
    if (name == "follow" || name == "co" || name == "nc" || name == "drop target")
        return 1.0f;

    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    const SpellInfo* const pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!pSpellInfo) return 1.0f;

    if (spellId && pSpellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        return 1.0f;
    else if (spellId && pSpellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        return 1.0f;

    uint32 castTime = pSpellInfo->CalcCastTime();

    if (AI_VALUE2(bool, "has area debuff", "self target") && spellId && castTime > 0)
    {
        return 0.0f;
    }

    return 1.0f;
}


void AvoidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
            "has area debuff",
            NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 5), NULL)));
}

void AvoidAoeStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AvoidAoeStrategyMultiplier(botAI));
}