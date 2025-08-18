/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlaceholderHelper.h"

#include "AiFactory.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Util.h"

void PlaceholderHelper::MapDungeon(PlaceholderMap& placeholders, DungeonSuggestion const* dungeonSuggestion,
                                   Player* bot)
{
    std::ostringstream out;
    Insertion insertion = {out, dungeonSuggestion};
    InsertDungeonName(insertion);
    InsertDungeonStrategy(insertion);
    InsertDifficulty(insertion, bot);

    placeholders["%dungeon"] = out.str();
}

void PlaceholderHelper::MapRole(PlaceholderMap& placeholders, Player* bot)
{
    BotRoles const role = AiFactory::GetPlayerRoles(bot);
    std::string roleText;
    switch (role)
    {
        case BOT_ROLE_TANK:
            roleText = "Tank";
            break;
        case BOT_ROLE_HEALER:
            roleText = "Healer";
            break;
        case BOT_ROLE_DPS:
            roleText = "DPS";
            break;
        case BOT_ROLE_NONE:
        default:
            return;
    }

    bool const hasRole = !roleText.empty();
    if (hasRole)
    {
        placeholders["%role"] = roleText;
    }
}

void PlaceholderHelper::InsertDungeonName(Insertion& insertion)
{
    std::string name = insertion.dungeonSuggestion->name;
    bool const hasAbbrevation = !insertion.dungeonSuggestion->abbrevation.empty();
    if (hasAbbrevation)
    {
        name = insertion.dungeonSuggestion->abbrevation;
    }

    insertion.out << "|c00b000b0" << name << "|r";
}

void PlaceholderHelper::InsertDungeonStrategy(Insertion& insertion)
{
    bool const hasStrategy = !insertion.dungeonSuggestion->strategy.empty();
    bool const isRandomlyUsingStrategy = urand(0, 1);
    if (hasStrategy && isRandomlyUsingStrategy)
    {
        std::string strategy = insertion.dungeonSuggestion->strategy;
        insertion.out << " " + strategy;
    }
}

void PlaceholderHelper::InsertDifficulty(Insertion& insertion, [[maybe_unused]] Player* bot)
{
    bool const hasHeroic = insertion.dungeonSuggestion->difficulty == DUNGEON_DIFFICULTY_HEROIC;
    std::string difficultyText;
    if (hasHeroic)
    {
        bool const isRandomlyNormal = urand(0, 1);
        bool const isRandomlyHeroic = urand(0, 1);
        std::vector<std::string> normalAbbrevations = {"Normal", "N"};
        std::vector<std::string> heroicAbbrevations = {"Heroic", "HC", "H"};
        uint32 const randomAbbrevationIndex = urand(0, 1);
        if (isRandomlyNormal)
        {
            difficultyText = normalAbbrevations[randomAbbrevationIndex];
        }
        else if (isRandomlyHeroic)
        {
            difficultyText = heroicAbbrevations[randomAbbrevationIndex];
        }

        insertion.out << " " << difficultyText;
    }
}
