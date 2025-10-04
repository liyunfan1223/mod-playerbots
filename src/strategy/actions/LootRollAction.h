/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTROLLACTION_H
#define _PLAYERBOT_LOOTROLLACTION_H

#include "QueryItemUsageAction.h"
#include "ItemTemplate.h"

class PlayerbotAI;

struct ItemTemplate;

enum RollVote : uint8;

class LootRollAction : public QueryItemUsageAction
{
public:
    LootRollAction(PlayerbotAI* botAI, std::string const name = "loot roll") : QueryItemUsageAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    /**
     * Default roll rule (outside Master Loot & outside tokens):
     * - NEED if direct upgrade (ItemUsage = EQUIP/REPLACE)
     * - GREED if useful but not an upgrade (BAD_EQUIP, USE, SKILL, DISENCHANT, AH, VENDOR, KEEP, AMMO)
     * - PASS otherwise
     *
     * Safeguards:
     *   - SmartNeedBySpec: downgrade NEED->GREED if the item does not match the bot's main spec
     *   - BoP: if at least one relevant slot is empty, allow NEED (if spec is valid)
     *   - BoE/BoU: NEED blocked unless explicitly allowed by config (AllowBoENeedIfUpgrade / AllowBoUNeedIfUpgrade)
     *   - Cross-armor: BAD_EQUIP can become NEED only if newScore >= bestScore * CrossArmorExtraMargin
     *
     * Specific cases:
     *   - Tokens: NEED only if the targeted slot is a likely upgrade (ilvl heuristic),
     *             otherwise GREED (tokens with unknown slot remain GREED by default)
     *   - Disenchant (NBG): if ItemUsage = DISENCHANT and config enabled, vote DISENCHANT
     *                       (the core enforces if the DE button is actually available)
     */
     
    // randomProperty: 0 (none) ; >0 = itemRandomPropId ; <0 = -itemRandomSuffix
    RollVote CalculateRollVote(ItemTemplate const* proto, int32 randomProperty = 0);
};

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
