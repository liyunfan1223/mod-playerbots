/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGSUBACTIONS_H
#define _PLAYERBOT_RPGSUBACTIONS_H

#include "Action.h"
#include "AiObject.h"
#include "Item.h"

class GuidPosition;
class ObjectGuid;
class PlayerbotAI;

class RpgHelper : public AiObject
{
public:
    RpgHelper(PlayerbotAI* botAI) : AiObject(botAI) {}
    virtual ~RpgHelper() = default;
    void OnExecute(std::string nextAction = "rpg");
    void BeforeExecute();
    void AfterExecute(bool doDelay = true, bool waitForGroup = false);

    virtual GuidPosition guidP();
    virtual ObjectGuid guid();
    virtual bool InRange();

private:
    void setFacingTo(GuidPosition guidPosition);
    void setFacing(GuidPosition guidPosition);
    void setDelay(bool waitForGroup);
};

class RpgEnabled
{
public:
    RpgEnabled(PlayerbotAI* botAI) { rpg = std::make_unique<RpgHelper>(botAI); }

protected:
    std::unique_ptr<RpgHelper> rpg;
};

class RpgSubAction : public Action, public RpgEnabled
{
public:
    RpgSubAction(PlayerbotAI* botAI, std::string const name = "rpg sub") : Action(botAI, name), RpgEnabled(botAI) {}

    // Long range is possible?
    bool isPossible() override;
    // Short range can we do the action now?
    bool isUseful() override;

    bool Execute(Event event) override;

protected:
    virtual std::string const ActionName();
    virtual Event ActionEvent(Event event);
};

class RpgStayAction : public RpgSubAction
{
public:
    RpgStayAction(PlayerbotAI* botAI, std::string const name = "rpg stay") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RpgWorkAction : public RpgSubAction
{
public:
    RpgWorkAction(PlayerbotAI* botAI, std::string const name = "rpg work") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RpgEmoteAction : public RpgSubAction
{
public:
    RpgEmoteAction(PlayerbotAI* botAI, std::string const name = "rpg emote") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RpgCancelAction : public RpgSubAction
{
public:
    RpgCancelAction(PlayerbotAI* botAI, std::string const name = "rpg cancel") : RpgSubAction(botAI, name) {}

    bool Execute(Event event) override;
};

class RpgTaxiAction : public RpgSubAction
{
public:
    RpgTaxiAction(PlayerbotAI* botAI, std::string const name = "rpg taxi") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RpgDiscoverAction : public RpgTaxiAction
{
public:
    RpgDiscoverAction(PlayerbotAI* botAI, std::string const name = "rpg discover") : RpgTaxiAction(botAI, name) {}

    bool Execute(Event event) override;
};

class RpgStartQuestAction : public RpgSubAction
{
public:
    RpgStartQuestAction(PlayerbotAI* botAI, std::string const name = "rpg start quest") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgEndQuestAction : public RpgSubAction
{
public:
    RpgEndQuestAction(PlayerbotAI* botAI, std::string const name = "rpg end quest") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgBuyAction : public RpgSubAction
{
public:
    RpgBuyAction(PlayerbotAI* botAI, std::string const name = "rpg buy") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgSellAction : public RpgSubAction
{
public:
    RpgSellAction(PlayerbotAI* botAI, std::string const name = "rpg sell") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgRepairAction : public RpgSubAction
{
public:
    RpgRepairAction(PlayerbotAI* botAI, std::string const name = "rpg repair") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
};

class RpgTrainAction : public RpgSubAction
{
public:
    RpgTrainAction(PlayerbotAI* botAI, std::string const name = "rpg train") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
};

class RpgHealAction : public RpgSubAction
{
public:
    RpgHealAction(PlayerbotAI* botAI, std::string const name = "rpg heal") : RpgSubAction(botAI, name) {}

    bool Execute(Event event) override;
};

class RpgHomeBindAction : public RpgSubAction
{
public:
    RpgHomeBindAction(PlayerbotAI* botAI, std::string const name = "rpg home bind") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
};

class RpgQueueBgAction : public RpgSubAction
{
public:
    RpgQueueBgAction(PlayerbotAI* botAI, std::string const name = "rpg queue bg") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
};

class RpgBuyPetitionAction : public RpgSubAction
{
public:
    RpgBuyPetitionAction(PlayerbotAI* botAI, std::string const name = "rpg buy petition") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
};

class RpgUseAction : public RpgSubAction
{
public:
    RpgUseAction(PlayerbotAI* botAI, std::string const name = "rpg use") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgSpellAction : public RpgSubAction
{
public:
    RpgSpellAction(PlayerbotAI* botAI, std::string const name = "rpg spell") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgCraftAction : public RpgSubAction
{
public:
    RpgCraftAction(PlayerbotAI* botAI, std::string const name = "rpg craft") : RpgSubAction(botAI, name) {}

private:
    std::string const ActionName() override;
    Event ActionEvent(Event event) override;
};

class RpgTradeUsefulAction : public RpgSubAction
{
public:
    RpgTradeUsefulAction(PlayerbotAI* botAI, std::string const name = "rpg trade useful") : RpgSubAction(botAI, name) {}

    std::vector<Item*> CanGiveItems(GuidPosition guidPosition);

    bool Execute(Event event) override;
};

class RpgDuelAction : public RpgSubAction
{
public:
    RpgDuelAction(PlayerbotAI* botAI, std::string const name = "rpg duel") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RpgMountAnimAction : public RpgSubAction
{
public:
    RpgMountAnimAction(PlayerbotAI* botAI, std::string const name = "rpg mount anim") : RpgSubAction(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
