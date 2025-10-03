/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
* and/or modify it under version 3 of the License, or (at your option), any later version.
*/

#include "RandomPlayerbotFactory.h"

#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "DatabaseEnv.h"
#include "GuildMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "Timer.h"
#include "Guild.h"            // EmblemInfo::SaveToDB
#include "Log.h"
#include "GuildMgr.h"

std::map<uint8, std::vector<uint8>> RandomPlayerbotFactory::availableRaces;

constexpr RandomPlayerbotFactory::NameRaceAndGender RandomPlayerbotFactory::CombineRaceAndGender(uint8 gender,
                                                                                                uint8 race)
{
    switch (race)
    {
        case RACE_HUMAN:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
        case RACE_ORC:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::OrcMale) + gender);
        case RACE_DWARF:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DwarfMale) + gender);
        case RACE_NIGHTELF:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::NightelfMale) + gender);
        case RACE_UNDEAD_PLAYER:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
        case RACE_TAUREN:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TaurenMale) + gender);
        case RACE_GNOME:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GnomeMale) + gender);
        case RACE_TROLL:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TrollMale) + gender);
        case RACE_DRAENEI:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DraeneiMale) + gender);
        case RACE_BLOODELF:
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::BloodelfMale) + gender);
        default:
            LOG_ERROR("playerbots", "The race with ID %d does not have a naming category", race);
            return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
    }
}

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
    uint32 const expansion = sWorld->getIntConfig(CONFIG_EXPANSION);

    availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
    availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);
    }

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_PALADIN].push_back(RACE_DRAENEI);
        availableRaces[CLASS_PALADIN].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
    availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
    availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
    availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
    availableRaces[CLASS_ROGUE].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD_PLAYER);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
        availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
        availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);
    }

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
    if (expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_HUNTER].push_back(RACE_DRAENEI);
        availableRaces[CLASS_HUNTER].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);

    if (expansion == EXPANSION_WRATH_OF_THE_LICH_KING)
    {
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_NIGHTELF);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TAUREN);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_HUMAN);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_ORC);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_UNDEAD_PLAYER);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TROLL);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_BLOODELF);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DRAENEI);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_GNOME);
        availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DWARF);
    }
}

Player* RandomPlayerbotFactory::CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache)
{
    LOG_DEBUG("playerbots", "Creating new random bot for class {}", cls);

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;
    bool alliance = rand() % 2 ? true : false;
    std::vector<uint8> raceOptions;
    for (const auto& race : availableRaces[cls])
    {
        if (alliance == IsAlliance(race))
        {
            raceOptions.push_back(race);
        }
    }

    if (raceOptions.empty())
    {
        LOG_ERROR("playerbots", "No races available for class: {}", cls);
        return nullptr;
    }

    uint8 race = raceOptions[urand(0, raceOptions.size() - 1)];

    const auto raceAndGender = CombineRaceAndGender(gender, race);

    std::string name;
    if (nameCache.empty())
    {
        name = CreateRandomBotName(raceAndGender);
    }
    else
    {
        if (nameCache[raceAndGender].empty())
        {
            LOG_ERROR("playerbots", "No name found for race and gender: {}", raceAndGender);
            return nullptr;
        }
        uint32 i = urand(0, nameCache[raceAndGender].size() - 1);
        name = nameCache[raceAndGender][i];
        swap(nameCache[raceAndGender][i], nameCache[raceAndGender].back());
        nameCache[raceAndGender].pop_back();
    }
    if (name.empty())
    {
        LOG_ERROR("playerbots", "Unable to get random bot name!");
        return nullptr;
    }

    std::vector<uint8> skinColors, facialHairTypes;
    std::vector<std::pair<uint8, uint8>> faces, hairs;
    for (CharSectionsEntry const* charSection : sCharSectionsStore)
    {
        if (charSection->Race != race || charSection->Gender != gender)
            continue;

        switch (charSection->GenType)
        {
            case SECTION_TYPE_SKIN:
                skinColors.push_back(charSection->Color);
                break;
            case SECTION_TYPE_FACE:
                faces.push_back(std::pair<uint8, uint8>(charSection->Type, charSection->Color));
                break;
            case SECTION_TYPE_FACIAL_HAIR:
                facialHairTypes.push_back(charSection->Type);
                break;
            case SECTION_TYPE_HAIR:
                hairs.push_back(std::pair<uint8, uint8>(charSection->Type, charSection->Color));
                break;
        }
    }

    //uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)]; //not used, line marked for removal.
    std::pair<uint8, uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8, uint8> hair = hairs[urand(0, hairs.size() - 1)];

    bool excludeCheck = (race == RACE_TAUREN) || (race == RACE_DRAENEI) ||
                        (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
    uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];

    std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>(
        name, race, cls, gender, face.second, face.first, hair.first, hair.second, facialHair);

    Player* player = new Player(session);
    player->GetMotionMaster()->Initialize();
    if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), characterInfo.get()))
    {
        player->CleanupsBeforeDelete();
        delete player;

        LOG_ERROR("playerbots", "Unable to create random bot for account {} - name: \"{}\"; race: {}; class: {}",
                accountId, name.c_str(), race, cls);
        return nullptr;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    if (player->getClass() == CLASS_DEATH_KNIGHT)
    {
        player->learnSpell(50977, false);
    }
    LOG_DEBUG("playerbots", "Random bot created for account {} - name: \"{}\"; race: {}; class: {}", accountId,
            name.c_str(), race, cls);

    return player;
}

std::string const RandomPlayerbotFactory::CreateRandomBotName(NameRaceAndGender raceAndGender)
{
    std::string botName = "";
    int tries = 3;
    while (--tries)
    {
        QueryResult result = CharacterDatabase.Query(
            "SELECT n.name "
            "FROM playerbots_names n "
            "LEFT OUTER JOIN characters c ON c.name = n.name "
            "WHERE c.guid IS NULL and n.gender = '{}' "
            "ORDER BY RAND() LIMIT 1",
            static_cast<uint8>(raceAndGender));
        if (!result)
        {
            break;
        }

        Field* fields = result->Fetch();
        botName = fields[0].Get<std::string>();
        if (ObjectMgr::CheckPlayerName(botName) == CHAR_NAME_SUCCESS)  // Checks for reservation & profanity, too
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            stmt->SetData(0, botName);

            if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
                continue;

            return botName;
        }
    }

    // CONLANG NAME GENERATION
    LOG_ERROR("playerbots", "No more names left for random bots. Attempting conlang name generation.");
    const std::string groupCategory = "SCVKRU";
    const std::string groupFormStart[2][4] = {{"SV", "SV", "VK", "RV"}, {"V", "SU", "VS", "RV"}};
    const std::string groupFormMid[2][6] = {{"CV", "CVC", "CVC", "CVK", "VC", "VK"},
                                            {"CV", "CVC", "CVK", "KVC", "VC", "KV"}};
    const std::string groupFormEnd[2][4] = {{"CV", "VC", "VK", "CV"}, {"RU", "UR", "VR", "V"}};
    const std::string groupLetter[2][6] = {
        // S           C                            V               K           R         U
        {"dtspkThfS", "bcCdfghjkmnNqqrrlsStTvwxyz", "aaeeiouA", "ppttkkbdg", "lmmnrr", "AEO"},
        {"dtskThfS", "bcCdfghjkmmnNqrrlssStTvwyz", "aaaeeiiuAAEIO", "ppttkbbdg", "lmmnrrr", "AEOy"}};
    const std::string replaceRule[2][17] = {
        {"ST", "ka", "ko", "ku", "kr", "S", "T", "C", "N", "jj", "AA", "AI", "A", "E", "O", "I", "aa"},
        {"sth", "ca", "co", "cu", "cr", "sh", "th", "ch", "ng", "dg", "A", "ayu", "ai", "ei", "ou", "iu", "ae"}};

    const auto gender = static_cast<uint8>(raceAndGender) % 2;

    tries = 10;
    while (--tries)
    {
        botName.clear();
        // Build name from groupForms
        // Pick random start group
        botName = groupFormStart[gender][rand() % 4];
        // Pick up to 2 and then up to 1 additional middle group
        for (int i = 0; i < rand() % 3 + rand() % 2; i++)
        {
            botName += groupFormMid[gender][rand() % 6];
        }
        // Pick up to 1 end group
        botName += rand() % 2 ? groupFormEnd[gender][rand() % 4] : "";
        // If name is single letter add random end group
        botName += (botName.size() < 2) ? groupFormEnd[gender][rand() % 4] : "";

        // Replace Catagory value with random Letter from that Catagory's Letter string for a given bot gender
        for (int i = 0; i < botName.size(); i++)
        {
            botName[i] = groupLetter[gender][groupCategory.find(botName[i])]
                                    [rand() % groupLetter[gender][groupCategory.find(botName[i])].size()];
        }

        // Itterate over replace rules
        for (int i = 0; i < 17; i++)
        {
            int j = botName.find(replaceRule[0][i]);
            while (j > -1)
            {
                botName.replace(j, replaceRule[0][i].size(), replaceRule[1][i]);
                j = botName.find(replaceRule[0][i]);
            }
        }

        // Capitalize first letter
        botName[0] -= 32;

        if (ObjectMgr::CheckPlayerName(botName) != CHAR_NAME_SUCCESS) // Checks for reservation & profanity, too
        {
            botName.clear();
            continue;
        }
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->SetData(0, botName);

        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            botName.clear();
            continue;
        }
        return std::move(botName);
    }

    // TRUE RANDOM NAME GENERATION
    LOG_ERROR("playerbots", "Con​lang name generation failed. True random name fallback.");
    tries = 10;
    while (--tries)
    {
        for (uint8 i = 0; i < 10; i++)
        {
            botName += (i == 0 ? 'A' : 'a') + rand() % 26;
        }
        if (ObjectMgr::CheckPlayerName(botName) != CHAR_NAME_SUCCESS)  // Checks for reservation & profanity, too
        {
            botName.clear();
            continue;
        }
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->SetData(0, botName);

        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            botName.clear();
            continue;
        }
        return std::move(botName);
    }
    LOG_ERROR("playerbots", "Random name generation failed.");
    botName.clear();
    return std::move(botName);
}

// Calculates the total number of required accounts, either using the specified randomBotAccountCount
// or determining it dynamically based on MaxRandomBots, EnablePeriodicOnlineOffline and its ratio,
// and AddClassAccountPoolSize. The system also factors in the types of existing account, as assigned by
// AssignAccountTypes()
uint32 RandomPlayerbotFactory::CalculateTotalAccountCount()
{
    // Reset account types if features are disabled
    // Reset is done here to precede needed accounts calculations
    if (sPlayerbotAIConfig->maxRandomBots == 0 || sPlayerbotAIConfig->addClassAccountPoolSize == 0)
    {
        if (sPlayerbotAIConfig->maxRandomBots == 0)
        {
            PlayerbotsDatabase.Execute("UPDATE playerbots_account_type SET account_type = 0 WHERE account_type = 1");
            LOG_INFO("playerbots", "MaxRandomBots set to 0, any RNDbot accounts (type 1) will be unassigned (type 0)");
        }
        if (sPlayerbotAIConfig->addClassAccountPoolSize == 0)
        {
            PlayerbotsDatabase.Execute("UPDATE playerbots_account_type SET account_type = 0 WHERE account_type = 2");
            LOG_INFO("playerbots", "AddClassAccountPoolSize set to 0, any AddClass accounts (type 2) will be unassigned (type 0)");
        }

        // Wait for DB to reflect the change, up to 1 second max. This is needed to make sure other logs don't show wrong info
        for (int waited = 0; waited < 1000; waited += 50)
        {
            QueryResult res = PlayerbotsDatabase.Query("SELECT COUNT(*) FROM playerbots_account_type WHERE account_type IN ({}, {})",
                sPlayerbotAIConfig->maxRandomBots == 0 ? 1 : -1,
                sPlayerbotAIConfig->addClassAccountPoolSize == 0 ? 2 : -1);

            if (!res || res->Fetch()[0].Get<uint64>() == 0)
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));     // Extra 50ms fixed delay for safety.
        }
    }

    // Checks if randomBotAccountCount is set, otherwise calculate it dynamically.
    if (sPlayerbotAIConfig->randomBotAccountCount > 0)
        return sPlayerbotAIConfig->randomBotAccountCount;

    // Check existing account types
    uint32 existingRndBotAccounts = 0;
    uint32 existingAddClassAccounts = 0;
    uint32 existingUnassignedAccounts = 0;

    QueryResult typeCheck = PlayerbotsDatabase.Query("SELECT account_type, COUNT(*) FROM playerbots_account_type GROUP BY account_type");
    if (typeCheck)
    {
        do
        {
            Field* fields = typeCheck->Fetch();
            uint8 accountType = fields[0].Get<uint8>();
            uint32 count = fields[1].Get<uint32>();

            if (accountType == 0) existingUnassignedAccounts = count;
            else if (accountType == 1) existingRndBotAccounts = count;
            else if (accountType == 2) existingAddClassAccounts = count;
        } while (typeCheck->NextRow());
    }

    // Determine divisor based on Death Knight login eligibility and requested A&H faction ratio
    int divisor = CalculateAvailableCharsPerAccount();

    // Calculate max bots
    int maxBots = sPlayerbotAIConfig->maxRandomBots;
    // Take periodic online - offline into account
    if (sPlayerbotAIConfig->enablePeriodicOnlineOffline)
    {
        maxBots *= sPlayerbotAIConfig->periodicOnlineOfflineRatio;
    }

    // Calculate number of accounts needed for RNDbots
    // Result is rounded up for maxBots not cleanly divisible by the divisor
    uint32 neededRndBotAccounts = (maxBots + divisor - 1) / divisor;
    uint32 neededAddClassAccounts = sPlayerbotAIConfig->addClassAccountPoolSize;

    // Start with existing total
    uint32 existingTotal = existingRndBotAccounts + existingAddClassAccounts + existingUnassignedAccounts;

    // Calculate shortfalls after using unassigned accounts
    uint32 availableUnassigned = existingUnassignedAccounts;
    uint32 additionalAccountsNeeded = 0;

    // Check RNDbot needs
    if (neededRndBotAccounts > existingRndBotAccounts)
    {
        uint32 rndBotShortfall = neededRndBotAccounts - existingRndBotAccounts;
        if (rndBotShortfall <= availableUnassigned)
            availableUnassigned -= rndBotShortfall;
        else
        {
            additionalAccountsNeeded += (rndBotShortfall - availableUnassigned);
            availableUnassigned = 0;
        }
    }

    // Check AddClass needs
    if (neededAddClassAccounts > existingAddClassAccounts)
    {
        uint32 addClassShortfall = neededAddClassAccounts - existingAddClassAccounts;
        if (addClassShortfall <= availableUnassigned)
            availableUnassigned -= addClassShortfall;
        else
        {
            additionalAccountsNeeded += (addClassShortfall - availableUnassigned);
            availableUnassigned = 0;
        }
    }

    // Return existing total plus any additional accounts needed
    return existingTotal + additionalAccountsNeeded;
}

uint32 RandomPlayerbotFactory::CalculateAvailableCharsPerAccount()
{
    bool noDK = sPlayerbotAIConfig->disableDeathKnightLogin || sWorld->getIntConfig(CONFIG_EXPANSION) != EXPANSION_WRATH_OF_THE_LICH_KING;

    uint32 availableChars = noDK ? 9 : 10;

    uint32 hordeRatio = sPlayerbotAIConfig->randomBotHordeRatio;
    uint32 allianceRatio = sPlayerbotAIConfig->randomBotAllianceRatio;

    // horde : alliance = 50 : 50 -> 0%
    // horde : alliance = 0 : 50 -> 50%
    // horde : alliance = 10 : 50 -> 40%
    float unavailableRatio = static_cast<float>((std::max(hordeRatio, allianceRatio) - std::min(hordeRatio, allianceRatio))) /
        (std::max(hordeRatio, allianceRatio) * 2);

    if (unavailableRatio != 0)
    {
        // conservative floor to ensure enough chars (may result in more accounts than needed)
        availableChars = availableChars - availableChars * unavailableRatio;
    }

    return availableChars;
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    /* multi-thread here is meaningless? since the async db operations */

    if (sPlayerbotAIConfig->deleteRandomBotAccounts)
    {
        std::vector<uint32> botAccounts;
        std::vector<uint32> botFriends;

        // Calculates the total number of required accounts.
        uint32 totalAccountCount = CalculateTotalAccountCount();

        for (uint32 accountNumber = 0; accountNumber < totalAccountCount; ++accountNumber)
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
            std::string const accountName = out.str();

            if (uint32 accountId = AccountMgr::GetId(accountName))
                botAccounts.push_back(accountId);
        }

        LOG_INFO("playerbots", "Deleting all random bot characters and accounts...");

        // First execute all the cleanup SQL commands
        // Clear playerbots_random_bots and playerbots_account_type
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_random_bots");
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_account_type");

        // Get the database names dynamically
        std::string loginDBName = LoginDatabase.GetConnectionInfo()->database;
        std::string characterDBName = CharacterDatabase.GetConnectionInfo()->database;

        // Delete all characters from bot accounts
        CharacterDatabase.Execute("DELETE FROM characters WHERE account IN (SELECT id FROM " + loginDBName + ".account WHERE username LIKE '{}%%')",
            sPlayerbotAIConfig->randomBotAccountPrefix.c_str());

        // Wait for the characters to be deleted before proceeding to dependent deletes
        while (CharacterDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Extra 100ms fixed delay for safety.

        // Clean up orphaned entries in playerbots_guild_tasks
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_guild_tasks WHERE owner NOT IN (SELECT guid FROM " + characterDBName + ".characters)");

        // Clean up orphaned entries in playerbots_db_store
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_db_store WHERE guid NOT IN (SELECT guid FROM " + characterDBName + ".characters WHERE account IN (SELECT id FROM " + loginDBName + ".account WHERE username NOT LIKE '{}%%'))",
            sPlayerbotAIConfig->randomBotAccountPrefix.c_str());

        // Clean up orphaned records in character-related tables
        CharacterDatabase.Execute("DELETE FROM arena_team_member WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM arena_team WHERE arenaTeamId NOT IN (SELECT arenaTeamId FROM arena_team_member)");
        CharacterDatabase.Execute("DELETE FROM character_account_data WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_action WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_arena_stats WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_aura WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_entry_point WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_glyphs WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_homebind WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_inventory WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM item_instance WHERE owner_guid NOT IN (SELECT guid FROM characters) AND owner_guid > 0");

        // Clean up pet data
        CharacterDatabase.Execute("DELETE FROM character_pet WHERE owner NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM pet_aura WHERE guid NOT IN (SELECT id FROM character_pet)");
        CharacterDatabase.Execute("DELETE FROM pet_spell WHERE guid NOT IN (SELECT id FROM character_pet)");
        CharacterDatabase.Execute("DELETE FROM pet_spell_cooldown WHERE guid NOT IN (SELECT id FROM character_pet)");

        // Clean up character data
        CharacterDatabase.Execute("DELETE FROM character_queststatus WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_queststatus_rewarded WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_reputation WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_skills WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_social WHERE friend NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_spell WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_spell_cooldown WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM character_talent WHERE guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM corpse WHERE guid NOT IN (SELECT guid FROM characters)");

        // Clean up group data
        CharacterDatabase.Execute("DELETE FROM `groups` WHERE leaderGuid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM group_member WHERE memberGuid NOT IN (SELECT guid FROM characters)");

        // Clean up mail
        CharacterDatabase.Execute("DELETE FROM mail WHERE receiver NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM mail_items WHERE receiver NOT IN (SELECT guid FROM characters)");

        // Clean up guild data
        CharacterDatabase.Execute("DELETE FROM guild WHERE leaderguid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM guild_bank_eventlog WHERE guildid NOT IN (SELECT guildid FROM guild)");
        CharacterDatabase.Execute("DELETE FROM guild_member WHERE guildid NOT IN (SELECT guildid FROM guild) OR guid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM guild_rank WHERE guildid NOT IN (SELECT guildid FROM guild)");

        // Clean up petition data
        CharacterDatabase.Execute("DELETE FROM petition WHERE ownerguid NOT IN (SELECT guid FROM characters)");
        CharacterDatabase.Execute("DELETE FROM petition_sign WHERE ownerguid NOT IN (SELECT guid FROM characters) OR playerguid NOT IN (SELECT guid FROM characters)");

        // Finally, delete the bot accounts themselves
        LOG_INFO("playerbots", "Deleting random bot accounts...");
        QueryResult results = LoginDatabase.Query("SELECT id FROM account WHERE username LIKE '{}%%'",
                                             sPlayerbotAIConfig->randomBotAccountPrefix.c_str());
        int32 deletion_count = 0;
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint32 accId = fields[0].Get<uint32>();
                LOG_DEBUG("playerbots", "Deleting account accID: {}({})...", accId, ++deletion_count);
                AccountMgr::DeleteAccount(accId);
            } while (results->NextRow());
        }

        uint32 timer = getMSTime();

        // After ALL deletions, make sure data is commited to DB
        LoginDatabase.Execute("COMMIT");
        CharacterDatabase.Execute("COMMIT");
        PlayerbotsDatabase.Execute("COMMIT");

        // Wait for all pending database operations to complete
        while (LoginDatabase.QueueSize() || CharacterDatabase.QueueSize() || PlayerbotsDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Extra 100ms fixed delay for safety.

        // Flush tables to ensure all data in memory are written to disk
        LoginDatabase.Execute("FLUSH TABLES");
        CharacterDatabase.Execute("FLUSH TABLES");
        PlayerbotsDatabase.Execute("FLUSH TABLES");

        LOG_INFO("playerbots", ">> Random bot accounts and data deleted in {} ms", GetMSTimeDiffToNow(timer));
        LOG_INFO("playerbots", "Please reset the AiPlayerbot.DeleteRandomBotAccounts to 0 and restart the server...");
        World::StopNow(SHUTDOWN_EXIT_CODE);
        return;
    }

    LOG_INFO("playerbots", "Creating random bot accounts...");
    std::unordered_map<NameRaceAndGender, std::vector<std::string>> nameCache;
    std::vector<std::future<void>> account_creations;
    int account_creation = 0;

    // Calculates the total number of required accounts.
    uint32 totalAccountCount = CalculateTotalAccountCount();
    uint32 timer = getMSTime();

    for (uint32 accountNumber = 0; accountNumber < totalAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string const accountName = out.str();

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->SetData(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (result)
        {
            continue;
        }
        account_creation++;
        std::string password = "";
        if (sPlayerbotAIConfig->randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char)urand('!', 'z');
            }
        }
        else
            password = accountName;

        AccountMgr::CreateAccount(accountName, password);

        LOG_DEBUG("playerbots", "Account {} created for random bots", accountName.c_str());
    }
    if (account_creation)
    {
        LOG_INFO("playerbots", "Waiting for {} accounts loading into database ({} queries)...", account_creation, LoginDatabase.QueueSize());
        /* wait for async accounts create to make character create correctly */

        while (LoginDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        LOG_INFO("playerbots", ">> {} Accounts loaded into database in {} ms", account_creation, GetMSTimeDiffToNow(timer));
    }

    LOG_INFO("playerbots", "Creating random bot characters...");
    uint32 totalRandomBotChars = 0;
    std::vector<std::pair<Player*, uint32>> playerBots;
    std::vector<WorldSession*> sessionBots;
    int bot_creation = 0;
    timer = getMSTime();
    bool nameCached = false;
    for (uint32 accountNumber = 0; accountNumber < totalAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string const accountName = out.str();

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->SetData(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (!result)
            continue;

        Field* fields = result->Fetch();
        uint32 accountId = fields[0].Get<uint32>();

        sPlayerbotAIConfig->randomBotAccounts.push_back(accountId);

        uint32 count = AccountMgr::GetCharactersCount(accountId);
        if (count >= 10)
        {
            continue;
        }

        if (!nameCached)
        {
            nameCached = true;
            LOG_INFO("playerbots", "Creating cache for names per gender and race...");
            QueryResult result = CharacterDatabase.Query("SELECT name, gender FROM playerbots_names");
            if (!result)
            {
                LOG_ERROR("playerbots", "No more unused names left");
                return;
            }
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].Get<std::string>();
                NameRaceAndGender raceAndGender = static_cast<NameRaceAndGender>(fields[1].Get<uint8>());
                if (sObjectMgr->CheckPlayerName(name) == CHAR_NAME_SUCCESS)
                {
                    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
                    stmt->SetData(0, name);

                    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
                        continue;

                    nameCache[raceAndGender].push_back(name);
                }

            } while (result->NextRow());
        }

        LOG_DEBUG("playerbots", "Creating random bot characters for account: [{}/{}]", accountNumber + 1, totalAccountCount);
        RandomPlayerbotFactory factory(accountId);

        WorldSession* session = new WorldSession(accountId, "", 0x0, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING,
                                                time_t(0), LOCALE_enUS, 0, false, false, 0, true);
        sessionBots.push_back(session);

        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
        {
            // skip nonexistent classes
            if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
                continue;

            if (bool const isClassDeathKnight = cls == CLASS_DEATH_KNIGHT;
                isClassDeathKnight && sWorld->getIntConfig(CONFIG_EXPANSION) != EXPANSION_WRATH_OF_THE_LICH_KING)
            {
                continue;
            }

            if (cls != 10)
            {
                if (Player* playerBot = factory.CreateRandomBot(session, cls, nameCache))
                {
                    playerBot->SaveToDB(true, false);
                    sCharacterCache->AddCharacterCacheEntry(playerBot->GetGUID(), accountId, playerBot->GetName(),
                                                            playerBot->getGender(), playerBot->getRace(),
                                                            playerBot->getClass(), playerBot->GetLevel());
                    playerBot->CleanupsBeforeDelete();
                    delete playerBot;
                    bot_creation++;
                }
                else
                {
                    LOG_ERROR("playerbots", "Fail to create character for account {}", accountId);
                }
            }
        }
    }

    if (bot_creation)
    {
        LOG_INFO("playerbots", "Waiting for {} characters loading into database ({} queries)...", bot_creation, CharacterDatabase.QueueSize());
        /* wait for characters load into database, or characters will fail to loggin */
        while (CharacterDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        LOG_INFO("playerbots", ">> {} Characters loaded into database in {} ms", bot_creation, GetMSTimeDiffToNow(timer));
    }

    for (WorldSession* session : sessionBots)
        delete session;

    for (uint32 accountId : sPlayerbotAIConfig->randomBotAccounts)
    {
        totalRandomBotChars += AccountMgr::GetCharactersCount(accountId);
    }

    LOG_INFO("server.loading", ">> {} random bot accounts with {} characters available",
            sPlayerbotAIConfig->randomBotAccounts.size(), totalRandomBotChars);
}

void RandomPlayerbotFactory::CreateRandomGuilds()
{
    std::vector<uint32> randomBots;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BOT);
    stmt->SetData(0, "add");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            randomBots.push_back(bot);
        } while (result->NextRow());
    }

    if (sPlayerbotAIConfig->deleteRandomBotGuilds)
    {
        LOG_INFO("playerbots", "Deleting random bot guilds...");
        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            if (Guild* guild = sGuildMgr->GetGuildByLeader(ObjectGuid::Create<HighGuid::Player>(*i)))
                guild->Disband();
        }

        LOG_INFO("playerbots", "Random bot guilds deleted");
    }

    uint32 guildNumber = 0;
    GuidVector availableLeaders;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid leader = ObjectGuid::Create<HighGuid::Player>(*i);
        if (Guild* guild = sGuildMgr->GetGuildByLeader(leader))
        {
            ++guildNumber;
            sPlayerbotAIConfig->randomBotGuilds.push_back(guild->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindPlayer(leader);
            if (player && !player->GetGuildId())
                availableLeaders.push_back(leader);
        }
    }

    // Create up to randomBotGuildCount by counting only EFFECTIVE creations
    uint32 createdThisRun = 0;
    for (; guildNumber < sPlayerbotAIConfig->randomBotGuildCount; /* ++guildNumber -> done only if creation */)
    {
        std::string const guildName = CreateRandomGuildName();
        if (guildName.empty())
            continue;

        if (sGuildMgr->GetGuildByName(guildName))
            continue;

        if (availableLeaders.empty())
        {
            LOG_ERROR("playerbots", "No leaders for random guilds available");
            break; // no more leaders: we can no longer progress without distorting the counter
        }

        uint32 index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        availableLeaders.erase(availableLeaders.begin() + index); // Removes the chosen leader to avoid re-selecting it repeatedly

        Player* player = ObjectAccessor::FindPlayer(leader);
        if (!player)
        {
            LOG_ERROR("playerbots", "ObjectAccessor Cannot find player to set leader for guild {} . Skipped...",
                    guildName.c_str());
            // we will try with other leaders in the next round (guildNumber is not incremented)
            continue;
        }

        if (player->GetGuildId())
        {
            // leader already in guild -> we don't advance the counter, we move on to the next one
            continue;
        }

        LOG_DEBUG("playerbots", "Creating guild name='{}' leader='{}'...", guildName.c_str(), player->GetName().c_str());

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            LOG_ERROR("playerbots", "Error creating guild [ {} ] with leader [ {} ]", guildName.c_str(),
                    player->GetName().c_str());
            delete guild;
            continue;
        }

        sGuildMgr->AddGuild(guild);

        LOG_DEBUG("playerbots", "Guild created: id={} name='{}'", guild->GetId(), guildName.c_str());

        // create random emblem
        uint32 st, cl, br, bc, bg;
        bg = urand(0, 51);
        bc = urand(0, 17);
        cl = urand(0, 17);
        br = urand(0, 7);
        st = urand(0, 180);

        LOG_DEBUG("playerbots",
                 "[TABARD] new guild id={} random -> style={}, color={}, borderStyle={}, borderColor={}, bgColor={}",
                 guild->GetId(), st, cl, br, bc, bg);

        // populate guild table with a random tabard design
        CharacterDatabase.Execute(
            "UPDATE guild SET EmblemStyle={}, EmblemColor={}, BorderStyle={}, BorderColor={}, BackgroundColor={} "
            "WHERE guildid={}",
            st, cl, br, bc, bg, guild->GetId());
        LOG_DEBUG("playerbots", "[TABARD] UPDATE done for guild id={}", guild->GetId());

        // Immediate reading for log
        if (QueryResult qr = CharacterDatabase.Query(
                "SELECT EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor FROM guild WHERE guildid={}",
                guild->GetId()))
        {
            Field* f = qr->Fetch();
            LOG_DEBUG("playerbots",
                     "[TABARD] DB check guild id={} => style={}, color={}, borderStyle={}, borderColor={}, bgColor={}",
                     guild->GetId(), f[0].Get<uint8>(), f[1].Get<uint8>(), f[2].Get<uint8>(), f[3].Get<uint8>(), f[4].Get<uint8>());
        }

        sPlayerbotAIConfig->randomBotGuilds.push_back(guild->GetId());
        // The guild is only counted if it is actually created
        ++guildNumber;
        ++createdThisRun;
    }

    // Shows the true total and how many were created during this run
    LOG_INFO("playerbots", "{} random bot guilds available (created this run: {})",
             uint32(sPlayerbotAIConfig->randomBotGuilds.size()), createdThisRun);
}

std::string const RandomPlayerbotFactory::CreateRandomGuildName()
{
    std::string guildName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbots_guild_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return std::move(guildName);
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].Get<uint32>();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.Query(
        "SELECT n.name FROM playerbots_guild_names n "
        "LEFT OUTER JOIN guild e ON e.name = n.name WHERE e.guildid IS NULL AND n.name_id >= {} LIMIT 1",
        id);
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return std::move(guildName);
    }

    fields = result->Fetch();
    guildName = fields[0].Get<std::string>();

    return std::move(guildName);
}

void RandomPlayerbotFactory::CreateRandomArenaTeams(ArenaType type, uint32 count)
{
    std::vector<uint32> randomBots;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BOT);
    stmt->SetData(0, "add");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            randomBots.push_back(bot);
        } while (result->NextRow());
    }

    uint32 arenaTeamNumber = 0;
    GuidVector availableCaptains;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid captain = ObjectGuid::Create<HighGuid::Player>(*i);
        ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(captain, type);
        if (arenateam)
        {
            ++arenaTeamNumber;
            sPlayerbotAIConfig->randomBotArenaTeams.push_back(arenateam->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(captain);

            if (!arenateam && player && player->GetLevel() >= 70)
                availableCaptains.push_back(captain);
        }
    }

    for (; arenaTeamNumber < count; ++arenaTeamNumber)
    {
        std::string const arenaTeamName = CreateRandomArenaTeamName();
        if (arenaTeamName.empty())
            continue;

        if (availableCaptains.empty())
        {
            LOG_ERROR("playerbots", "No captains for random arena teams available");
            continue;
        }

        uint32 index = urand(0, availableCaptains.size() - 1);
        ObjectGuid captain = availableCaptains[index];
        Player* player = ObjectAccessor::FindConnectedPlayer(captain);
        if (!player)
        {
            LOG_ERROR("playerbots", "Cannot find player for captain {}", captain.ToString().c_str());
            continue;
        }

        if (player->GetLevel() < 70)
        {
            LOG_ERROR("playerbots", "Bot {} must be level 70 to create an arena team", captain.ToString().c_str());
            continue;
        }

        // Below query no longer required as now user has control over the number of each type of arena team they want
        // to create. Keeping commented for potential future reference. QueryResult results =
        // CharacterDatabase.Query("SELECT `type` FROM playerbots_arena_team_names WHERE name = '{}'",
        // arenaTeamName.c_str()); if (!results)
        // {
        //     LOG_ERROR("playerbots", "No valid types for arena teams");
        //     return;
        // }

        // Field* fields = results->Fetch();
        // uint8 slot = fields[0].Get<uint8>();

        ArenaTeam* arenateam = new ArenaTeam();
        if (!arenateam->Create(player->GetGUID(), type, arenaTeamName, 0, 0, 0, 0, 0))
        {
            LOG_ERROR("playerbots", "Error creating arena team {}", arenaTeamName.c_str());
            continue;
        }

        arenateam->SetCaptain(player->GetGUID());

        // set random rating
        arenateam->SetRatingForAll(
            urand(sPlayerbotAIConfig->randomBotArenaTeamMinRating, sPlayerbotAIConfig->randomBotArenaTeamMaxRating));

        // set random emblem
        uint32 backgroundColor = urand(0xFF000000, 0xFFFFFFFF);
        uint32 emblemStyle = urand(0, 101);
        uint32 emblemColor = urand(0xFF000000, 0xFFFFFFFF);
        uint32 borderStyle = urand(0, 5);
        uint32 borderColor = urand(0xFF000000, 0xFFFFFFFF);
        arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);

        // set random kills (wip)
        // arenateam->SetStats(STAT_TYPE_GAMES_WEEK, urand(0, 30));
        // arenateam->SetStats(STAT_TYPE_WINS_WEEK, urand(0, arenateam->GetStats().games_week));
        // arenateam->SetStats(STAT_TYPE_GAMES_SEASON, urand(arenateam->GetStats().games_week,
        // arenateam->GetStats().games_week * 5)); arenateam->SetStats(STAT_TYPE_WINS_SEASON,
        // urand(arenateam->GetStats().wins_week, arenateam->GetStats().games
        arenateam->SaveToDB();

        sArenaTeamMgr->AddArenaTeam(arenateam);
        sPlayerbotAIConfig->randomBotArenaTeams.push_back(arenateam->GetId());
    }

    LOG_DEBUG("playerbots", "{} random bot {}vs{} arena teams available", arenaTeamNumber, type, type);
}

std::string const RandomPlayerbotFactory::CreateRandomArenaTeamName()
{
    std::string arenaTeamName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbots_arena_team_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return std::move(arenaTeamName);
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].Get<uint32>();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.Query(
        "SELECT n.name FROM playerbots_arena_team_names n LEFT OUTER JOIN arena_team e ON e.name = n.name "
        "WHERE e.arenateamid IS NULL AND n.name_id >= {} LIMIT 1",
        id);

    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return std::move(arenaTeamName);
    }

    fields = result->Fetch();
    arenaTeamName = fields[0].Get<std::string>();

    return std::move(arenaTeamName);
}
