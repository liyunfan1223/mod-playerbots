/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ConserveManaStrategy.h"
#include "GenericSpellActions.h"
#include "LastSpellCastValue.h"
#include "Playerbots.h"

float ConserveManaMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    uint8 health = AI_VALUE2(uint8, "health", "self target");
    uint8 targetHealth = AI_VALUE2(uint8, "health", "current target");
    uint8 mana = AI_VALUE2(uint8, "mana", "self target");
    bool hasMana = AI_VALUE2(bool, "has mana", "self target");
    bool mediumMana = hasMana && mana < sPlayerbotAIConfig->mediumMana;

    if (health < sPlayerbotAIConfig->lowHealth)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (action->GetTarget() != target)
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    std::string const spell = spellAction->getName();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || spellInfo->PowerType != POWER_MANA)
        return 1.0f;

    if (mediumMana && dynamic_cast<CastBuffSpellAction*>(action))
        return 0.0f;

    if (target && ((int)target->getLevel() - (int)bot->getLevel()) >= 0)
        return 1.0f;

    return 1.0f;
}

float SaveManaMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    if (action->GetTarget() != AI_VALUE(Unit*, "current target"))
        return 1.0f;

    double saveLevel = AI_VALUE(double, "mana save level");
    if (saveLevel <= 1.0)
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    std::string const spell = spellAction->getName();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || spellInfo->PowerType != POWER_MANA)
        return 1.0f;

    int32 cost = spellInfo->ManaCost;
    if (!cost)
        return 1.0f;

    time_t lastCastTime = AI_VALUE2(time_t, "last spell cast time", spell);
    if (!lastCastTime)
        return 1.0f;

    time_t elapsed = time(nullptr) - lastCastTime;
    if ((double)elapsed < 10 * saveLevel)
        return 0.0f;

    return 1.0f;
}

void ConserveManaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ConserveManaMultiplier(botAI));
}
