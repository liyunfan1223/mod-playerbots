/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
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

/**
 * @brief Create a random bot character.
 *
 * This function creates a random bot character for the given class. It generates a random gender and race,
 * creates a random bot name, initializes the player object, and saves it to the database.
 *
 * Example:
 * If cls = CLASS_WARRIOR, the function will create a random bot character of class warrior with a random gender,
 * race, and name.
 *
 * @param session The WorldSession for the bot.
 * @param cls The class of the bot to be created.
 * @param nameCache A cache of names for each race and gender combination.
 * @return A pointer to the created Player object, or nullptr if the creation failed.
 */
Player* RandomPlayerbotFactory::CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache)
{
    LOG_DEBUG("playerbots", "Creating new random bot for class {}", cls);

    // Generate a random gender (0 for male, 1 for female)
    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;

    // Generate a random alliance flag
    bool alliance = rand() % 2 ? true : false;

    // Select a random race based on the alliance flag
    uint8 race = SelectRandomRace(cls, alliance);
    if (race == 0)
    {
        return nullptr;
    }

    // Combine the race and gender to create a unique identifier
    const auto raceAndGender = CombineRaceAndGender(gender, race);

    // Retrieve or generate a random bot name
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

    // Check if the name creation was successful
    if (name.empty())
    {
        LOG_ERROR("playerbots", "Unable to get random bot name!");
        return nullptr;
    }

    // Collect customization options (skin color, face, hair, facial hair) based on the race and gender
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

    // Select random customization options
    uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)];
    std::pair<uint8, uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8, uint8> hair = hairs[urand(0, hairs.size() - 1)];

    // Determine if facial hair should be excluded
    bool excludeCheck = (race == RACE_TAUREN) || (race == RACE_DRAENEI) ||
                        (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
    uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];

    // Create a CharacterCreateInfo object with the collected information
    std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>(
        name, race, cls, gender, face.second, face.first, hair.first, hair.second, facialHair);

    // Create a new Player object and initialize it
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

    // Set various player flags and initialize stats
    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    // Learn specific spells for certain classes
    if (player->getClass() == CLASS_DEATH_KNIGHT)
    {
        player->learnSpell(50977, false);
    }

    LOG_DEBUG("playerbots", "Random bot created for account {} - name: \"{}\"; race: {}; class: {}", accountId,
              name.c_str(), race, cls);

    // Return the created player object
    return player;
}

/**
 * @brief Filter available races based on the alliance flag and select a random race.
 *
 * @param cls The class of the bot to be created.
 * @param alliance The alliance flag (true for alliance, false for horde).
 * @return A random race for the given class and alliance flag.
 */
uint8 RandomPlayerbotFactory::SelectRandomRace(uint8 cls, bool alliance)
{
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
        return 0;
    }

    return raceOptions[urand(0, raceOptions.size() - 1)];
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
        return std::move(botName);
    }
    LOG_ERROR("playerbots", "Random name generation failed.");
    botName.clear();
    return std::move(botName);
}

/**
 * @brief Create random bot accounts and characters.
 *
 * This function creates random bot accounts and characters based on the configuration.
 * It first checks if random bot accounts should be deleted and calls DeleteRandomBotAccounts if true.
 * It then creates a cache of names, gets the next bot account counter, creates new bot accounts,
 * and creates random bot characters.
 *
 * Example:
 * If sPlayerbotAIConfig->randomBotAccountPrefix = "RNDBOT" and sPlayerbotAIConfig->randomBotAccountCount = 100,
 * the function will create 100 random bot accounts and characters.
 */
void RandomPlayerbotFactory::CreateRandomBots()
{
    if (sPlayerbotAIConfig->deleteRandomBotAccounts)
    {
        DeleteRandomBotAccounts();
        return;
    }

    LOG_INFO("playerbots", "Creating random bot accounts...");
    std::unordered_map<NameRaceAndGender, std::vector<std::string>> nameCache;
    CreateNameCache(nameCache);

    if (nameCache.empty())
    {
        LOG_ERROR("playerbots", "Failed to create name cache. Aborting random bot creation.");
        return;
    }

    uint32 botAccountCounter = GetNextBotAccountCounter();

    CreateBotAccounts(botAccountCounter);

    LOG_INFO("playerbots", "Creating random bot characters...");
    CreateBotCharacters(nameCache);
}

/**
 * @brief Delete all random bot accounts.
 *
 * This function deletes all accounts with usernames that match the configured bot account prefix.
 * It logs the deletion process, deletes associated bot characters, executes a prepared statement to delete random bots from the PlayerbotsDatabase,
 * and stops the server.
 */
void RandomPlayerbotFactory::DeleteRandomBotAccounts()
{
    std::vector<uint32> botAccounts = GetBotAccountIds();
    LOG_INFO("playerbots", "Deleting all random bot characters, {} accounts collected...", botAccounts.size());

    int32 deletion_count = 0;
    for (uint32 accId : botAccounts)
    {
        LOG_INFO("playerbots", "Deleting characters for account accID: {}({})...", accId, ++deletion_count);
        DeleteBotCharacters(accId);
        LOG_INFO("playerbots", "Deleting account accID: {}({})...", accId, deletion_count);
        AccountMgr::DeleteAccount(accId);
    }

    PlayerbotsDatabase.Execute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
    /* TODO(yunfan): we need to sleep here to wait for async account deleted, or the newly account won't be created
           correctly the better way is turning the async db operation to sync db operation */
    std::this_thread::sleep_for(10ms * sPlayerbotAIConfig->randomBotAccountCount);
    LOG_INFO("playerbots", "Random bot characters deleted.");
    LOG_INFO("playerbots", "Please reset the AiPlayerbot.DeleteRandomBotAccounts to 0 and restart the server...");
    World::StopNow(SHUTDOWN_EXIT_CODE);
}

/**
 * @brief Delete all characters associated with a bot account.
 *
 * This function deletes all characters associated with the given bot account ID.
 * It first checks if the account is a bot account by verifying the prefix in the account name.
 *
 * Example:
 * If accountId = 1 and the account name starts with the configured bot account prefix,
 * the function will delete all characters associated with the account ID 1.
 *
 * @param accountId The ID of the bot account whose characters are to be deleted.
 */
void RandomPlayerbotFactory::DeleteBotCharacters(uint32 accountId)
{
    // Retrieve the account name from the database
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_USERNAME_BY_ID);
    stmt->SetData(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);
    if (!result)
        return;

    Field* fields = result->Fetch();
    std::string accountName = fields[0].Get<std::string>();

    // Check if the account name starts with the configured bot account prefix
    if (accountName.find(sPlayerbotAIConfig->randomBotAccountPrefix) != 0)
    {
        LOG_ERROR("playerbots", "Account ID {} is not a bot account. Skipping character deletion.", accountId);
        return;
    }

    QueryResult charactersResult = CharacterDatabase.Query("SELECT guid FROM characters WHERE account = {}", accountId);
    if (charactersResult)
    {
        do
        {
            Field* fields = charactersResult->Fetch();
            uint64 charGuid = fields[0].Get<uint64>();
            LOG_INFO("playerbots", "Deleting character guid: {}", charGuid);
            Player::DeleteFromDB(charGuid, accountId, false, true);
        } while (charactersResult->NextRow());
    }
}

/**
 * @brief Create a cache of names for each race and gender combination.
 *
 * This function queries the database for names and genders from the playerbots_names table.
 * It checks if each name is valid and adds it to the nameCache for the corresponding race and gender combination.
 *
 * @param nameCache A cache of names for each race and gender combination.
 */
void RandomPlayerbotFactory::CreateNameCache(std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache)
{
    LOG_INFO("playerbots", "Creating cache for names per gender and race.");
    QueryResult result = CharacterDatabase.Query("SELECT name, gender FROM playerbots_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more unused names left");
        return;
    }

     // Estimate the number of names and reserve space in the vectors to reserve space and avoid multiple reallocations.
    size_t estimatedSize = result->GetRowCount();
    for (auto& entry : nameCache)
    {
        entry.second.reserve(estimatedSize);
    }

    do
    {
        Field* fields = result->Fetch();
        std::string name = fields[0].Get<std::string>();
        NameRaceAndGender raceAndGender = static_cast<NameRaceAndGender>(fields[1].Get<uint8>());
        if (sObjectMgr->CheckPlayerName(name) == CHAR_NAME_SUCCESS)
            nameCache[raceAndGender].push_back(name);
    } while (result->NextRow());
}

/**
 * @brief Get the next available bot account counter.
 *
 * This function queries the database to find the highest existing bot account number
 * based on the configured bot account prefix. It extracts the numeric suffix from the
 * highest bot account username and calculates the next available bot account number.
 *
 * This is important to ensure that new bot accounts are created with unique and sequential
 * account numbers, avoiding conflicts with existing accounts.
 *
 * @return The next available bot account counter.
 */
uint32 RandomPlayerbotFactory::GetNextBotAccountCounter()
{
    QueryResult botAccountsResult = LoginDatabase.Query("SELECT username FROM account WHERE username LIKE '{}%%' ORDER BY username DESC LIMIT 1",
                                                        sPlayerbotAIConfig->randomBotAccountPrefix.c_str());
    uint32 botAccountCounter = 1; // Initialize the bot account counter
    if (botAccountsResult)
    {
        Field* fields = botAccountsResult->Fetch();
        std::string highestBotAccount = fields[0].Get<std::string>();
        std::string suffix = highestBotAccount.substr(sPlayerbotAIConfig->randomBotAccountPrefix.length());
        botAccountCounter = std::stoi(suffix) + 1;
    }
    return botAccountCounter;
}

/**
 * @brief Create new bot accounts.
 *
 * This function creates new bot accounts up to the configured number of random bot accounts.
 * It uses the botAccountCounter to determine the starting point for creating new accounts.
 * For each new account, it constructs the account name using the configured prefix and the counter value,
 * checks if the account already exists, and creates the account if it does not exist.
 *
 * Example:
 * If sPlayerbotAIConfig->randomBotAccountPrefix = "RNDBOT" and sPlayerbotAIConfig->randomBotAccountCount = 100,
 * and botAccountCounter = 51, the function will create accounts "RNDBOT51" to "RNDBOT100".
 *
 * @param botAccountCounter The starting counter value for creating new bot accounts.
 */
void RandomPlayerbotFactory::CreateBotAccounts(uint32& botAccountCounter)
{
    // Create new bot accounts if the current bot account counter is less than the configured bot account count
    if (botAccountCounter < sPlayerbotAIConfig->randomBotAccountCount)
    {
        // Create new bot accounts until the configured bot account count is reached
        for (uint32 i = botAccountCounter; i <= sPlayerbotAIConfig->randomBotAccountCount; ++i)
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->randomBotAccountPrefix << botAccountCounter++;
            std::string const accountName = out.str();

            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
            stmt->SetData(0, accountName);
            PreparedQueryResult result = LoginDatabase.Query(stmt);
            if (result)
            {
                continue;
            }
            CreateAccount(accountName);
        }
    }
}

/**
 * @brief Create a new bot account.
 *
 * This function creates a new bot account with the given account name. It generates a password for the account
 * based on the configuration. If the configuration specifies a random password, it generates a random 10-character
 * password. Otherwise, it uses the account name as the password. The function then calls AccountMgr::CreateAccount
 * to create the account in the database.
 *
 * Example:
 * If accountName = "RNDBOT51" and sPlayerbotAIConfig->randomBotRandomPassword = true,
 * the function will create an account "RNDBOT51" with a random 10-character password.
 *
 * @param accountName The name of the account to be created.
 */
void RandomPlayerbotFactory::CreateAccount(const std::string& accountName)
{
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

/**
 * @brief Create random bot characters for each bot account.
 *
 * This function iterates over the list of bot account IDs, checks the number of characters for each account,
 * and creates new characters if the account has fewer characters than the configured maximum.
 * It retrieves the account name from the database and creates random bot characters for each class
 * that the account does not already have.
 *
 * Example:
 * If sPlayerbotAIConfig->randomBotAccountPrefix = "RNDBOT", sPlayerbotAIConfig->randomBotAccountCount = 100,
 * sPlayerbotAIConfig->maxCharactersPerAccount = 10, and botAccountIds = {1, 2, 3}, the function will create
 * random bot characters for accounts "RNDBOT1", "RNDBOT2", and "RNDBOT3" if they have fewer than 10 characters.
 *
 * @param nameCache A cache of names for each race and gender combination.
 */
void RandomPlayerbotFactory::CreateBotCharacters(std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache)
{
    uint32 totalRandomBotChars = 0;
    uint32 totalCharCount = sPlayerbotAIConfig->randomBotAccountCount * 10;
    std::vector<std::pair<Player*, uint32>> playerBots;
    std::vector<WorldSession*> sessionBots;
    int bot_creation = 0;

    std::vector<uint32> botAccountIds = GetBotAccountIds();

    for (uint32 accountId : botAccountIds)
    {
        sPlayerbotAIConfig->randomBotAccounts.push_back(accountId);

        uint32 count = AccountMgr::GetCharactersCount(accountId);
        if (count >= sPlayerbotAIConfig->MaxAddedBots) // Check against the config value
        {
            continue;
        }
        LOG_INFO("playerbots", "Creating random bot characters for account: [{}/{}]", accountId,
            sPlayerbotAIConfig->randomBotAccountCount);
        RandomPlayerbotFactory factory(accountId);

        WorldSession* session = new WorldSession(accountId, "", nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING,
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
        LOG_INFO("playerbots", "Waiting for {} characters loading into database...", bot_creation);
        /* wait for characters load into database, or characters will fail to loggin */
        // TODO: find a better way to wait for characters to be loaded into the database
        std::this_thread::sleep_for(5s + bot_creation * 5ms);
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

/**
 * @brief Get the list of bot account IDs.
 *
 * This function queries the database for accounts with usernames that match the configured bot account prefix.
 * It collects the IDs of these accounts and returns them in a vector.
 *
 * Example:
 * If sPlayerbotAIConfig->randomBotAccountPrefix = "RNDBOT" and the database contains accounts with usernames
 * "RNDBOT1", "RNDBOT2", and "RNDBOT3", the function will return a vector containing the IDs of these accounts.
 *
 * @return A vector containing the IDs of the bot accounts.
 */
std::vector<uint32> RandomPlayerbotFactory::GetBotAccountIds()
{
    std::vector<uint32> botAccountIds;
    QueryResult botAccountsResult = LoginDatabase.Query("SELECT id FROM account WHERE username LIKE '{}%%'",
                                                        sPlayerbotAIConfig->randomBotAccountPrefix.c_str());
    if (botAccountsResult)
    {
        do
        {
            Field* fields = botAccountsResult->Fetch();
            uint32 accountId = fields[0].Get<uint32>();
            botAccountIds.push_back(accountId);
        } while (botAccountsResult->NextRow());
    }
    return botAccountIds;
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

    for (; guildNumber < sPlayerbotAIConfig->randomBotGuildCount; ++guildNumber)
    {
        std::string const guildName = CreateRandomGuildName();
        if (guildName.empty())
            continue;

        if (sGuildMgr->GetGuildByName(guildName))
            continue;

        if (availableLeaders.empty())
        {
            LOG_ERROR("playerbots", "No leaders for random guilds available");
            continue;
        }

        uint32 index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        Player* player = ObjectAccessor::FindPlayer(leader);
        if (!player)
        {
            LOG_ERROR("playerbots", "ObjectAccessor Cannot find player to set leader for guild {} . Skipped...",
                      guildName.c_str());
            continue;
        }

        if (player->GetGuildId())
            continue;

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            LOG_ERROR("playerbots", "Error creating guild [ {} ] with leader [ {} ]", guildName.c_str(),
                      player->GetName().c_str());
            delete guild;
            continue;
        }

        sGuildMgr->AddGuild(guild);

        // create random emblem
        uint32 st, cl, br, bc, bg;
        bg = urand(0, 51);
        bc = urand(0, 17);
        cl = urand(0, 17);
        br = urand(0, 7);
        st = urand(0, 180);
        EmblemInfo emblemInfo(st, cl, br, bc, bg);
        guild->HandleSetEmblem(emblemInfo);

        sPlayerbotAIConfig->randomBotGuilds.push_back(guild->GetId());
    }

    LOG_INFO("playerbots", "{} random bot guilds available", guildNumber);
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

    LOG_INFO("playerbots", "{} random bot arena teams available", arenaTeamNumber);
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
