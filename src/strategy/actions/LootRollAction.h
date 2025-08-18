/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTROLLACTION_H
#define _PLAYERBOT_LOOTROLLACTION_H

#include "QueryItemUsageAction.h"

class PlayerbotAI;

struct ItemTemplate;

enum RollVote : uint8;

class LootRollAction : public QueryItemUsageAction
{
public:
    LootRollAction(PlayerbotAI* botAI, std::string const name = "loot roll") : QueryItemUsageAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    RollVote CalculateRollVote(ItemTemplate const* proto);
};

bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

class MasterLootRollAction : public LootRollAction
{
public:
    MasterLootRollAction(PlayerbotAI* botAI) : LootRollAction(botAI, "master loot roll") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RollAction : public Action
{
public:
    RollAction(PlayerbotAI* botAI) : Action(botAI, "roll") {}

    bool Execute(Event event) override;
};

#endif
