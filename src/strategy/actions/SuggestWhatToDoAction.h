/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SUGGESTWHATTODOACTION_H
#define _PLAYERBOT_SUGGESTWHATTODOACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class SuggestWhatToDoAction : public InventoryAction
{
public:
    SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name = "suggest what to do");

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    using Suggestion = std::function<void()>;
    std::vector<Suggestion> suggestions;
    void specificQuest();
    void grindReputation();
    void grindMaterials();
    void something();
    void toxicLinks();
    void somethingToxic();
    void thunderfury();

    std::vector<uint32> GetIncompletedQuests();

private:
    static std::map<std::string, uint8> factions;
    const int32_t _dbc_locale;
};

class SuggestTradeAction : public SuggestWhatToDoAction
{
public:
    SuggestTradeAction(PlayerbotAI* botAI);
    bool Execute(Event event) override;
};

class SuggestDungeonAction : public SuggestWhatToDoAction
{
public:
    SuggestDungeonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
private:
    static std::map<std::string, uint8> instances;
};

#endif
