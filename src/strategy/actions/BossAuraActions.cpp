/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include <HunterBuffStrategies.h>
#include <PaladinBuffStrategies.h>
#include <PlayerbotAI.h>

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"

const std::string ADD_STRATEGY_CHAR = "+";

bool BossFireResistanceAction::isUseful()
{
    BossFireResistanceTrigger bossFireResistanceTrigger(botAI, bossName);
    return bossFireResistanceTrigger.IsActive();
}

bool BossFireResistanceAction::Execute(Event event)
{
    PaladinFireResistanceStrategy paladinFireResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + paladinFireResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    return true;
}

bool BossFrostResistanceAction::isUseful()
{
    BossFrostResistanceTrigger bossFrostResistanceTrigger(botAI, bossName);
    return bossFrostResistanceTrigger.IsActive();
}

bool BossFrostResistanceAction::Execute(Event event)
{
    PaladinFrostResistanceStrategy paladinFrostResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + paladinFrostResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    return true;
}

bool BossNatureResistanceAction::isUseful()
{
    BossNatureResistanceTrigger bossNatureResistanceTrigger(botAI, bossName);
    return bossNatureResistanceTrigger.IsActive();
}

bool BossNatureResistanceAction::Execute(Event event)
{
    HunterNatureResistanceStrategy hunterNatureResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + hunterNatureResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    return true;
}
