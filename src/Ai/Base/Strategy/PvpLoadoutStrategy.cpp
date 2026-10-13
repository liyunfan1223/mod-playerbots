/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutStrategy.h"
#include "Playerbots.h"

void PvpLoadoutStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("pvp loadout mismatch", {NextAction("swap pvp loadout", ACTION_HIGH)}));
}

void PvpLoadoutStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PvpLoadoutMultiplier(botAI));
}

float PvpLoadoutMultiplier::GetValue(Action* action)
{
    // Runs for every queued action, so the cheap loadout check comes before the name copy.
    if (!_loadout)
        _loadout = context->GetValue<std::optional<PvpLoadout::Snapshot>>("pvp loadout");

    if (!_loadout->RefGet())
        return 1.0f;

    std::string const actionName = action->getName();
    return actionName == "equip upgrades packet action" || actionName == "equip upgrade" ? 0.0f : 1.0f;
}
