/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattlegroundStrategy.h"

#include "BattleGroundTactics.h"
#include "Battleground.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Multiplier.h"
#include "Playerbots.h"
#include "PvpValues.h"
#include "ReachTargetActions.h"

namespace
{
class WarsongTacticsMultiplier : public Multiplier
{
public:
    WarsongTacticsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "warsong tactics") {}

    float GetValue(Action* action) override
    {
        Battleground* bg = bot->GetBattleground();
        if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS)
            return 1.0f;
        if (auto* attack = dynamic_cast<AttackAction*>(action))
        {
            if (BGTactics::ShouldYieldWsgTactics(botAI) || !IsWsgEscortTargetAllowed(botAI, attack->GetTarget()))
                return 0.0f;
            if (dynamic_cast<AttackEnemyPlayerAction*>(action))
            {
                ObjectGuid guid = context->GetValue<ObjectGuid>("wsg support target")->Get();
                Unit* support = guid.IsEmpty() ? nullptr : botAI->GetUnit(guid);
                if (IsWsgSupportTarget(botAI, support))
                    return 0.0f;
            }
        }
        // Class-specific reach actions can outrank regroup; don't chase a target that left the carrier behind.
        if (auto* reach = dynamic_cast<ReachTargetAction*>(action))
        {
            if (reach->GetTargetName() == "current target" && ShouldRegroupWsgEscort(botAI))
                return 0.0f;
        }
        if (action->GetTargetName() == "current target" && !IsWsgEscortTargetAllowed(botAI, action->GetTarget()) &&
            dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action) &&
            ShouldRegroupWsgEscort(botAI))
            return 0.0f;
        return 1.0f;
    }
};
}  // namespace

void WarsongStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new WarsongTacticsMultiplier(botAI));
}

void BGStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", { NextAction("bg join", relevance)}));
    triggers.push_back(new TriggerNode("bg invite active", { NextAction("bg status check", relevance)}));
    triggers.push_back(new TriggerNode("timer", { NextAction("bg strategy check", relevance)}));
}

BGStrategy::BGStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI) {}

void BattlegroundStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg waiting", { NextAction("bg move to start", ACTION_BG)}));
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg move to objective", ACTION_BG)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg check objective", ACTION_BG + 1)}));
    triggers.push_back(new TriggerNode("dead", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void WarsongStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("wsg flag state changed", {NextAction("bg refresh wsg objective", ACTION_BG + 3.0f)}));
    triggers.push_back(new TriggerNode("wsg support threat", {NextAction("attack wsg support", ACTION_HIGH + 1.0f)}));
    triggers.push_back(new TriggerNode("wsg escort separated", {NextAction("wsg regroup", ACTION_HIGH + 2.0f)}));
    triggers.push_back(new TriggerNode(
        "bg active", {NextAction("bg wsg route", ACTION_BG + 1.4f), NextAction("bg wsg approach", ACTION_BG + 1.3f),
                      NextAction("bg wsg corridor", ACTION_BG + 1.2f), NextAction("bg wsg direct", ACTION_BG + 1.1f)}));
    triggers.push_back(new TriggerNode("bg active", {NextAction("bg check flag", ACTION_EMERGENCY + 1.0f)}));
    triggers.push_back(
        new TriggerNode("enemy flagcarrier near", {NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f)}));
    triggers.push_back(new TriggerNode("bg active", {NextAction("bg protect fc", ACTION_BG + 2.0f)}));
    triggers.push_back(new TriggerNode("often", {NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", {NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", {NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("player has flag", {NextAction("bg wsg route", ACTION_EMERGENCY + 0.4f),
                                                           NextAction("bg wsg approach", ACTION_EMERGENCY + 0.3f),
                                                           NextAction("bg wsg corridor", ACTION_EMERGENCY + 0.2f),
                                                           NextAction("bg wsg direct", ACTION_EMERGENCY + 0.1f)}));
}

void AlteracStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("timer bg", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
}

void ArathiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
}

void EyeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { NextAction("attack enemy flag carrier", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("player has flag",{ NextAction("bg move to objective", ACTION_EMERGENCY)}));
}

//TODO: Do Priorities
void IsleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("timer", { NextAction("enter vehicle", ACTION_MOVE + 8.0f)}));
    triggers.push_back(new TriggerNode("random", { NextAction("leave vehicle", ACTION_MOVE + 7.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("hurl boulder", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("fire cannon", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("napalm", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { NextAction("steam blast", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("ram", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("enemy is close", { NextAction("ram", ACTION_MOVE + 9.1f)}));
    triggers.push_back(new TriggerNode("enemy out of melee", { NextAction("steam rush", ACTION_MOVE + 9.2f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("incendiary rocket", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("rocket blast", ACTION_MOVE + 9.0f)}));
    // this is bugged: it doesn't work, and stops glaive throw working (which is needed to take down gate)
    // triggers.push_back(new TriggerNode("in vehicle", { NextAction("blade salvo", ACTION_MOVE + 9.0f)}));
    triggers.push_back(new TriggerNode("in vehicle", { NextAction("glaive throw", ACTION_MOVE + 9.0f)}));
}

void ArenaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no possible targets", { NextAction("arena tactics", ACTION_BG)}));
}
