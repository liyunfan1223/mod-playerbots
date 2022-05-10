/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SuggestDungeonAction.h"
#include "AiFactory.h"
#include "Player.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "PlayerbotDungeonSuggestionMgr.h"

DungeonSuggestions SuggestDungeonAction::m_dungeonSuggestions;

SuggestDungeonAction::SuggestDungeonAction(PlayerbotAI* botAI) :
    SuggestWhatToDoAction(botAI, "suggest instance")
{
    if (m_dungeonSuggestions.empty())
    {
        m_dungeonSuggestions = sPlayerbotDungeonSuggestionMgr->GetDungeonSuggestions();
    }
}

bool SuggestDungeonAction::Execute(Event event)
{
    bool const isRealPlayer = !sRandomPlayerbotMgr->IsRandomBot(bot);
    bool const isInGroup = bot->GetGroup();
    bool const isInInstance = bot->GetInstanceId();
    if (isRealPlayer || isInGroup || isInInstance)
    {
        return false;
    }

    DungeonSuggestions const dungeonSuggestions = GetDungeonSuggestionsEligibleFor(bot);
    if (dungeonSuggestions.empty())
    {
        return false;
    }

    uint32 const randomDungeonIndex = urand(0, dungeonSuggestions.size() - 1);
    DungeonSuggestion const* dungeonSuggestion = &dungeonSuggestions[randomDungeonIndex];
    PlaceholderMap const placeholders = MapPlaceholders(bot, dungeonSuggestion);
    std::string playerbotsTextKey = PlayerbotsTextKeyByMapKey(placeholders);
    std::string message = sPlayerbotTextMgr->Format(playerbotsTextKey, placeholders);
    bool isRandomlyLowerCase = sPlayerbotAIConfig->suggestDungeonsInLowerCaseRandomly
        ? urand(0, 1)
        : false;
    spam(message, 1, isRandomlyLowerCase);

    return true;
}

DungeonSuggestions const SuggestDungeonAction::GetDungeonSuggestionsEligibleFor(Player* bot)
{
    DungeonSuggestions dungeonSuggestionsEligibleFor;
    for (DungeonSuggestions::const_iterator i = m_dungeonSuggestions.begin();
        i != m_dungeonSuggestions.end(); ++i)
    {
        uint8 const level = bot->getLevel();
        bool const isEligible = level >= i->min_level && level <= i->max_level;
        if (isEligible)
        {
            dungeonSuggestionsEligibleFor.push_back(*i);
        }
    }

    return dungeonSuggestionsEligibleFor;
}

PlaceholderMap SuggestDungeonAction::MapPlaceholders(
    Player* bot,
    DungeonSuggestion const* dungeonSuggestion
)
{
    PlaceholderMap placeholders;
    bool const isRandomlyMappingRole = urand(0, 1);
    if (isRandomlyMappingRole)
    {
        PlaceholderHelper::MapRole(placeholders, bot);
    }
    PlaceholderHelper::MapDungeon(placeholders, dungeonSuggestion, bot);

    return placeholders;
}

std::string SuggestDungeonAction::PlayerbotsTextKeyByMapKey(PlaceholderMap const& placeholders)
{
    bool const isRoleMapped = placeholders.find("%role") != placeholders.end();
    std::string playerbotsTextKey = "suggest_dungeon";
    if (isRoleMapped)
    {
        playerbotsTextKey = "suggest_dungeon_role";
    }

    return playerbotsTextKey;
}
