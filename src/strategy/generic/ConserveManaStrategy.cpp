/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ConserveManaStrategy.h"

#include "GenericSpellActions.h"
#include "LastSpellCastValue.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

// float ConserveManaMultiplier::GetValue(Action* action)
// {
//     if (!action)
//         return 1.0f;

//     uint8 health = AI_VALUE2(uint8, "health", "self target");
//     uint8 targetHealth = AI_VALUE2(uint8, "health", "current target");
//     uint8 mana = AI_VALUE2(uint8, "mana", "self target");
//     bool hasMana = AI_VALUE2(bool, "has mana", "self target");
//     bool mediumMana = hasMana && mana < sPlayerbotAIConfig->mediumMana;

//     if (health < sPlayerbotAIConfig->lowHealth)
//         return 1.0f;

//     Unit* target = AI_VALUE(Unit*, "current target");
//     if (action->GetTarget() != target)
//         return 1.0f;

//     CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
//     if (!spellAction)
//         return 1.0f;

//     std::string const spell = spellAction->getName();
//     uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
//     SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
//     if (!spellInfo || spellInfo->PowerType != POWER_MANA)
//         return 1.0f;

//     if (mediumMana && dynamic_cast<CastBuffSpellAction*>(action))
//         return 0.0f;

//     if (target && ((int)target->GetLevel() - (int)bot->GetLevel()) >= 0)
//         return 1.0f;

//     return 1.0f;
// }

// float SaveManaMultiplier::GetValue(Action* action)
// {
//     if (!action)
//         return 1.0f;

//     if (action->GetTarget() != AI_VALUE(Unit*, "current target"))
//         return 1.0f;

//     double saveLevel = AI_VALUE(double, "mana save level");
//     if (saveLevel <= 1.0)
//         return 1.0f;

//     CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
//     if (!spellAction)
//         return 1.0f;

//     std::string const spell = spellAction->getName();
//     uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
//     SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
//     if (!spellInfo || spellInfo->PowerType != POWER_MANA)
//         return 1.0f;

//     int32 cost = spellInfo->ManaCost;
//     if (!cost)
//         return 1.0f;

//     time_t lastCastTime = AI_VALUE2(time_t, "last spell cast time", spell);
//     if (!lastCastTime)
//         return 1.0f;

//     time_t elapsed = time(nullptr) - lastCastTime;
//     if ((double)elapsed < 10 * saveLevel)
//         return 0.0f;

//     return 1.0f;
// }

// void ConserveManaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
// {
//     multipliers.push_back(new ConserveManaMultiplier(botAI));
// }

float HealerAutoSaveManaMultiplier::GetValue(Action* action)
{
    uint8 mana = bot->GetPowerPct(Powers::POWER_MANA);
    if (mana > sPlayerbotAIConfig->saveManaThreshold)
        return 1.0f;
    CastHealingSpellAction* healingAction = dynamic_cast<CastHealingSpellAction*>(action);

    if (!healingAction)
        return 1.0f;

    Unit* target = healingAction->GetTarget();
    if (!target)
        return 1.0f;
    bool isTank = target->ToPlayer() ? botAI->IsTank(target->ToPlayer()) : false;
    uint8 health = target->GetHealthPct();
    HealingManaEfficiency manaEfficiency = healingAction->manaEfficiency;
    uint8 estAmount = healingAction->estAmount;
    uint8 lossAmount = 100 - health;
    if (isTank)
    {
        estAmount /= 1.5;  // tanks have more health
        if (health >= sPlayerbotAIConfig->mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        if (health >= sPlayerbotAIConfig->lowHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW))
            return 0.0f;
    }
    else
    {
        if (health >= sPlayerbotAIConfig->mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        if (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW)
            return 0.0f;
    }
    return 1.0f;
}

void HealerAutoSaveManaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HealerAutoSaveManaMultiplier(botAI));
}