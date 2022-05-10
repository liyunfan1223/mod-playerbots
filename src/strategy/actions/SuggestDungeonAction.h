/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SUGGESTINSTANCEACTION_H
#define _PLAYERBOT_SUGGESTINSTANCEACTION_H

#include "SuggestWhatToDoAction.h"
#include "PlayerbotDungeonSuggestionMgr.h"
#include "PlaceholderHelper.h"

typedef std::vector<DungeonSuggestion> DungeonSuggestions;

class SuggestDungeonAction : public SuggestWhatToDoAction
{
    public:
        SuggestDungeonAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
        bool isUseful() override { return true; }

    private:
        static DungeonSuggestions m_dungeonSuggestions;

        DungeonSuggestions const GetDungeonSuggestionsEligibleFor(Player* bot);
        PlaceholderMap MapPlaceholders(
            Player* bot,
            DungeonSuggestion const* dungeonSuggestion
        );
        std::string PlayerbotsTextKeyByMapKey(PlaceholderMap const& placeholders);
};

#endif
