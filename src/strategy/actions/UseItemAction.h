/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_USEITEMACTION_H
#define _PLAYERBOT_USEITEMACTION_H

#include "Action.h"

class Item;
class ObjectGuid;
class PlayerbotAI;
class Unit;

class UseItemAction : public Action
{
public:
    UseItemAction(PlayerbotAI* botAI, std::string const name = "use", bool selfOnly = false)
        : Action(botAI, name), selfOnly(selfOnly)
    {
    }

    bool Execute(Event event) override;
    bool isPossible() override;

protected:
    bool UseItemAuto(Item* item);
    bool UseItemOnGameObject(Item* item, ObjectGuid go);
    bool UseItemOnItem(Item* item, Item* itemTarget);
    bool UseItem(Item* item, ObjectGuid go, Item* itemTarget, Unit* unitTarget = nullptr);
    bool UseGameObject(ObjectGuid guid);
    void TellConsumableUse(Item* item, std::string const action, float percent);
    bool SocketItem(Item* item, Item* gem, bool replace = false);

private:
    bool selfOnly;
};

class UseSpellItemAction : public UseItemAction
{
public:
    UseSpellItemAction(PlayerbotAI* botAI, std::string const name, bool selfOnly = false)
        : UseItemAction(botAI, name, selfOnly)
    {
    }

    bool isUseful() override;
};

class UseHealingPotion : public UseItemAction
{
public:
    UseHealingPotion(PlayerbotAI* botAI) : UseItemAction(botAI, "healing potion") {}

    bool isUseful() override;
};

class UseManaPotion : public UseItemAction
{
public:
    UseManaPotion(PlayerbotAI* botAI) : UseItemAction(botAI, "mana potion") {}

    bool isUseful() override;
};

class UseHearthStone : public UseItemAction
{
public:
    UseHearthStone(PlayerbotAI* botAI) : UseItemAction(botAI, "hearthstone", true) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class UseRandomRecipe : public UseItemAction
{
public:
    UseRandomRecipe(PlayerbotAI* botAI) : UseItemAction(botAI, "random recipe", true) {}

    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class UseRandomQuestItem : public UseItemAction
{
public:
    UseRandomQuestItem(PlayerbotAI* botAI) : UseItemAction(botAI, "random quest item", true) {}

    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

#endif
