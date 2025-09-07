/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PullStrategy.h"

#include "PassiveMultiplier.h"
#include "Playerbots.h"

class MagePullMultiplier : public PassiveMultiplier
{
public:
    MagePullMultiplier(PlayerbotAI* botAI, std::string const action) : PassiveMultiplier(botAI), actionName(action) {}

    float GetValue(Action* action) override;

private:
    std::string const actionName;
};

float MagePullMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string const name = action->getName();
    if (actionName == name || name == "reach spell" || name == "change strategy")
        return 1.0f;

    return PassiveMultiplier::GetValue(action);
}

NextAction** PullStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction(action, 105.0f), new NextAction("follow", 104.0f),
                             new NextAction("end pull", 103.0f), nullptr);
}

void PullStrategy::InitTriggers(std::vector<TriggerNode*>& triggers) { CombatStrategy::InitTriggers(triggers); }

void PullStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MagePullMultiplier(botAI, action));
    CombatStrategy::InitMultipliers(multipliers);
}

void PossibleAddsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("possible adds", NextAction::array(0, new NextAction("flee with pet", 60), nullptr)));
}
