/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RogueActions.h"
#include "Event.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Playerbots.h"

bool CastStealthAction::isPossible()
{
    // do not use with WSG flag or EYE flag
    return !botAI->HasAura(23333, bot) && !botAI->HasAura(23335, bot) && !botAI->HasAura(34976, bot);
}

bool UnstealthAction::Execute(Event event)
{
    botAI->RemoveAura("stealth");
    // botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);

    return true;
}

bool CheckStealthAction::Execute(Event event)
{
    if (botAI->HasAura("stealth", bot))
    {
        botAI->ChangeStrategy("-dps,+stealthed", BOT_STATE_COMBAT);
    }
    else
    {
        botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);
    }

    return true;
}

bool CastVanishAction::isUseful()
{
    // do not use with WSG flag or EYE flag
    return !botAI->HasAura(23333, bot) && !botAI->HasAura(23335, bot) && !botAI->HasAura(34976, bot);
}

bool CastTricksOfTheTradeOnMainTankAction::isUseful() {
    return CastSpellAction::isUseful() && AI_VALUE2(float, "distance", GetTargetName()) < 20.0f;
}

bool UseDeadlyPoisonAction::Execute(Event event) {
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix: poison_suffixs) {
        poison_name = getName() + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty()) {
            break;
        }
    }
    if (items.empty()) {
        return false;
    }
    Item* const itemForSpell = bot->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND );
    return UseItem(*items.begin(), ObjectGuid::Empty, itemForSpell);
    // return UseItemAuto(*items.begin());
}

bool UseDeadlyPoisonAction::isPossible() {
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix: poison_suffixs) {
        poison_name = getName() + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty()) {
            break;
        }
    }
    return !items.empty();
}

bool UseInstantPoisonAction::Execute(Event event) {
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix: poison_suffixs) {
        poison_name = getName() + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty()) {
            break;
        }
    }
    if (items.empty()) {
        return false;
    }
    Item* const itemForSpell = bot->GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND );
    return UseItem(*items.begin(), ObjectGuid::Empty, itemForSpell);
}

bool UseInstantPoisonAction::isPossible() {
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix: poison_suffixs) {
        poison_name = getName() + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty()) {
            break;
        }
    }
    return !items.empty();
}