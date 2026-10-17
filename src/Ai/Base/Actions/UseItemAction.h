/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_USEITEMACTION_H
#define PLAYERBOTS_USEITEMACTION_H

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
    bool UseItemOnGameObject(Item* item, ObjectGuid go);

protected:
    bool UseItemAuto(Item* item);
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

    bool Execute(Event event) override;
    bool isUseful() override;
};

class UseRandomRecipe : public UseItemAction
{
public:
    UseRandomRecipe(PlayerbotAI* botAI) : UseItemAction(botAI, "random recipe", true) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class UseRandomQuestItem : public UseItemAction
{
public:
    UseRandomQuestItem(PlayerbotAI* botAI) : UseItemAction(botAI, "random quest item", true) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

#endif
