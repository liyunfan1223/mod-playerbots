/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LOOTACTION_H
#define _PLAYERBOT_LOOTACTION_H

#include "InventoryAction.h"
#include "MovementActions.h"

class GameObject;
class LootObject;
class PlayerbotAI;
class SpellInfo;

class LootAction : public MovementAction
{
public:
    LootAction(PlayerbotAI* botAI) : MovementAction(botAI, "loot") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class OpenLootAction : public MovementAction
{
public:
    OpenLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "open loot") {}

    bool Execute(Event event) override;

private:
    bool DoLoot(LootObject& lootObject);
    uint32 GetOpeningSpell(LootObject& lootObject);
    uint32 GetOpeningSpell(LootObject& lootObject, GameObject* go);
    bool CanOpenLock(LootObject& lootObject, SpellInfo const* spellInfo, GameObject* go);
    bool CanOpenLock(uint32 skillId, uint32 reqSkillValue);
};

class StoreLootAction : public InventoryAction
{
public:
    StoreLootAction(PlayerbotAI* botAI) : InventoryAction(botAI, "store loot") {}

    bool Execute(Event event) override;
    static bool IsLootAllowed(uint32 itemid, PlayerbotAI* botAI);
};

class ReleaseLootAction : public InventoryAction
{
public:
    ReleaseLootAction(PlayerbotAI* botAI) : InventoryAction(botAI, "release loot") {}

    bool Execute(Event event) override;
};

#endif
