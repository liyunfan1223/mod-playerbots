/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattlegroundStrategy.h"
#include "ChooseTargetActions.h"
#include "Playerbots.h"
#include "SotaActions.h"

namespace
{
class SotaMovementMultiplier : public Multiplier
{
public:
    explicit SotaMovementMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sota movement") {}

    float GetValue(Action* action) override
    {
        SotaTactics tactics = GetSotaTactics(botAI);
        if (!tactics.enabled || !dynamic_cast<MovementAction*>(action))
            return 1.0f;
        if (dynamic_cast<AttackEnemyPlayerAction*>(action) || dynamic_cast<DpsAssistAction*>(action) ||
            dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
            dynamic_cast<AttackLeastHpTargetAction*>(action) || dynamic_cast<AggressiveTargetAction*>(action) ||
            dynamic_cast<AttackAnythingAction*>(action))
            if (Unit* target = GetSotaDefenseTarget(botAI, tactics))
                if (action->GetTarget() != target)
                    return 0.0f;
        if (tactics.active && bot->IsInCombat() && !bot->GetVehicle() && dynamic_cast<AttackAction*>(action))
            return 1.0f;
        if (dynamic_cast<SotaLandingAction*>(action) || dynamic_cast<SotaMoveAction*>(action) ||
            dynamic_cast<SotaEnterVehicleAction*>(action) || dynamic_cast<SotaDriveVehicleAction*>(action))
            return 1.0f;
        if (!tactics.active || tactics.landingRequired || bot->GetVehicle())
            return 0.0f;
        return 1.0f;
    }
};
}  // namespace

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
    triggers.push_back(new TriggerNode("bg active", { NextAction("bg check flag", ACTION_EMERGENCY )}));
    triggers.push_back(new TriggerNode("enemy flagcarrier near", { NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f)}));
    triggers.push_back(new TriggerNode("team flagcarrier near", { NextAction("bg protect fc", ACTION_RAID)}));
    triggers.push_back(new TriggerNode("often", { NextAction("bg use buff", ACTION_BG)}));
    triggers.push_back(new TriggerNode("low health", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("low mana", { NextAction("bg use buff", ACTION_MOVE)}));
    triggers.push_back(new TriggerNode("player has flag", { NextAction("bg move to objective", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode("timer bg", { NextAction("bg reset objective force", ACTION_EMERGENCY)}));
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

void SotaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "often", {NextAction("sota prepare", ACTION_EMERGENCY + 1.0f), NextAction("sota land", ACTION_EMERGENCY)}));
    triggers.push_back(new TriggerNode(
        "bg active",
        {NextAction("sota leave vehicle", ACTION_MOVE + 5.0f), NextAction("sota fire vehicle", ACTION_MOVE + 4.0f),
         NextAction("sota drive vehicle", ACTION_MOVE + 3.0f), NextAction("sota enter vehicle", ACTION_MOVE + 2.0f),
         NextAction("sota deploy", ACTION_MOVE + 2.0f), NextAction("sota fall back", ACTION_MOVE + 1.0f),
         NextAction("sota interact", ACTION_MOVE), NextAction("sota attack demolisher", ACTION_RAID),
         NextAction("sota move", ACTION_BG)}));
}

void SotaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SotaMovementMultiplier(botAI));
}
