/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
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
        void spam(std::string msg, uint8 flags = 0, bool worldChat = false, bool guild = false);

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
        bool isUseful() override { return true; }
};

class SuggestDungeonAction : public SuggestWhatToDoAction
{
    public:
        SuggestDungeonAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
        bool isUseful() override { return true; }
    private:
        static std::map<std::string, uint8> instances;
};

#endif
