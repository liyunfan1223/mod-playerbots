/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RandomPlayerbotFactory.h"
#include "PlayerbotsDatabase.h"
#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CharacterCache.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotOperations.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "RaceMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "StringFormat.h"
#include "Timer.h"
#include "World.h"

std::string RandomPlayerbotFactory::GetLocalizedNameSelector(std::string const& column)
{
    LocaleConstant locale = sWorld->GetDefaultDbcLocale();
    if (locale == LOCALE_enUS || locale >= TOTAL_LOCALES)
        return column;

    // Fall back to the enUS column when the localized one is NULL or empty
    return Acore::StringFormat("COALESCE(NULLIF({}_{}, ''), {})", column, localeNames[locale], column);
}

constexpr RandomPlayerbotFactory::NameRaceAndGender RandomPlayerbotFactory::CombineRaceAndGender(uint8 race,
                                                                                                uint8 gender)
{
    NameRaceAndGender baseIndex;
    switch (race)
    {
        case RACE_ORC:        baseIndex = NameRaceAndGender::OrcMale; break;
        case RACE_DWARF:      baseIndex = NameRaceAndGender::DwarfMale; break;
        case RACE_NIGHTELF:   baseIndex = NameRaceAndGender::NightelfMale; break;
        case RACE_TAUREN:     baseIndex = NameRaceAndGender::TaurenMale; break;
        case RACE_GNOME:      baseIndex = NameRaceAndGender::GnomeMale; break;
        case RACE_TROLL:      baseIndex = NameRaceAndGender::TrollMale; break;
        case RACE_BLOODELF:   baseIndex = NameRaceAndGender::BloodelfMale; break;
        case RACE_DRAENEI:    baseIndex = NameRaceAndGender::DraeneiMale; break;
        case RACE_HUMAN:
        case RACE_UNDEAD_PLAYER:
        default:
            baseIndex = NameRaceAndGender::GenericMale;
            break;
    }

    return static_cast<NameRaceAndGender>(static_cast<uint8>(baseIndex) +
                                          ((gender >= GENDER_NONE) ? static_cast<uint8>(GENDER_MALE) : gender));
}

bool RandomPlayerbotFactory::IsValidRaceClassCombination(uint8 race, uint8 cls, uint32 expansion)
{
    // skip expansion races if not playing with expansion
    if (expansion < EXPANSION_THE_BURNING_CRUSADE && (race == RACE_BLOODELF || race == RACE_DRAENEI))
        return false;

    // skip expansion classes if not playing with expansion
    if (expansion < EXPANSION_WRATH_OF_THE_LICH_KING && cls == CLASS_DEATH_KNIGHT)
        return false;

    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, cls);
    return info != nullptr;
}

Player* RandomPlayerbotFactory::CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache)
{
    LOG_DEBUG("playerbots", "Creating a new random bot for class: {}", cls);

    const bool alliance = static_cast<bool>(urand(0, 1));

    std::vector<uint8> raceOptions;
    for (uint8 race = RACE_HUMAN; race < sRaceMgr->GetMaxRaces(); ++race)
    {
        // skip disabled with config races
        if ((1 << (race - 1)) & sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK))
            continue;

        // Try to get 50/50 faction distribution for random bot population balance.
        // Without this check, races from the faction with more class options would dominate.
        if (alliance == IsAlliance(race))
        {
            if (IsValidRaceClassCombination(race, cls, sWorld->getIntConfig(CONFIG_EXPANSION)))
                raceOptions.push_back(race);
        }
    }

    if (raceOptions.empty())
    {
        LOG_ERROR("playerbots", "No races are available for class: {}", cls);
        return nullptr;
    }

    const uint8 race = raceOptions[urand(0, raceOptions.size() - 1)];
    const uint8 gender = urand(0, 1) ? GENDER_MALE : GENDER_FEMALE;
    const auto raceAndGender = CombineRaceAndGender(race, gender);

    std::string name;
    if (!nameCache.empty())
    {
        if (nameCache[raceAndGender].empty())
        {
            LOG_ERROR("playerbots", "No names found for the specified race: {} and gender: {}",
                    race, gender);
            return nullptr;
        }

        uint32 i = urand(0, nameCache[raceAndGender].size() - 1);
        name = nameCache[raceAndGender][i];
        swap(nameCache[raceAndGender][i], nameCache[raceAndGender].back());
        nameCache[raceAndGender].pop_back();
    }
    else
    {
        name = CreateRandomBotName(raceAndGender);
    }

    if (name.empty())
    {
        LOG_ERROR("playerbots", "Failed to get a valid random bot name");
        return nullptr;
    }

    std::vector<uint8> skinColors, facialHairTypes;
    std::vector<std::pair<uint8, uint8>> faces, hairs;
    for (CharSectionsEntry const* charSection : sCharSectionsStore)
    {
        if (charSection->RaceID != race || charSection->SexID != gender)
            continue;

        switch (charSection->BaseSection)
        {
            case SECTION_TYPE_SKIN:
                skinColors.push_back(charSection->ColorIndex);
                break;
            case SECTION_TYPE_FACE:
                faces.push_back(std::pair<uint8, uint8>(charSection->VariationIndex, charSection->ColorIndex));
                break;
            case SECTION_TYPE_FACIAL_HAIR:
                facialHairTypes.push_back(charSection->VariationIndex);
                break;
            case SECTION_TYPE_HAIR:
                hairs.push_back(std::pair<uint8, uint8>(charSection->VariationIndex, charSection->ColorIndex));
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

        LOG_ERROR("playerbots", "Unable to create random bot - name: \"{}\", race: {}, class: {}",
                name.c_str(), race, cls);
        return nullptr;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    if (cls == CLASS_DEATH_KNIGHT)
    {
        player->learnSpell(50977, false);
    }

    LOG_DEBUG("playerbots", "Random bot created - name: \"{}\", race: {}, class: {}",
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
        for (size_t i = 0; i < botName.size(); i++)
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
        return botName;
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
        return botName;
    }
    LOG_ERROR("playerbots", "Random name generation failed.");
    botName.clear();
    return botName;
}

// Calculates the total number of required accounts, either using the specified randomBotAccountCount
// or determining it dynamically based on MaxRandomBots, EnablePeriodicOnlineOffline and its ratio,
// and AddClassAccountPoolSize. The system also factors in the types of existing account, as assigned by
// AssignAccountTypes()
uint32 RandomPlayerbotFactory::CalculateTotalAccountCount()
{
    // Reset account types if features are disabled
    // Reset is done here to precede needed accounts calculations
    if (sPlayerbotAIConfig.MaxRandomBots == 0 || sPlayerbotAIConfig.AddClassAccountPoolSize == 0)
    {
        if (sPlayerbotAIConfig.MaxRandomBots == 0)
        {
            PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE_UNASSIGN);
            stmt->SetData(0, uint8(1));
            PlayerbotsDatabase.DirectExecute(stmt);
            LOG_INFO("playerbots", "MaxRandomBots set to 0, any RNDbot accounts (type 1) will be unassigned (type 0)");
        }
        if (sPlayerbotAIConfig.AddClassAccountPoolSize == 0)
        {
            PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE_UNASSIGN);
            stmt->SetData(0, uint8(2));
            PlayerbotsDatabase.DirectExecute(stmt);
            LOG_INFO("playerbots", "AddClassAccountPoolSize set to 0, any AddClass accounts (type 2) will be unassigned (type 0)");
        }

        // Wait for DB to reflect the change, up to 1 second max. This is needed to make sure other logs don't show wrong info
        for (int waited = 0; waited < 1000; waited += 50)
        {
            PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ACCOUNT_TYPE_COUNT_BY_TYPES);
            stmt->SetData(0, int32(sPlayerbotAIConfig.MaxRandomBots == 0 ? 1 : -1));
            stmt->SetData(1, int32(sPlayerbotAIConfig.AddClassAccountPoolSize == 0 ? 2 : -1));
            PreparedQueryResult res = PlayerbotsDatabase.Query(stmt);

            if (!res || res->Fetch()[0].Get<uint64>() == 0)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));     // Extra 50ms fixed delay for safety.
        }
    }

    // Check existing account types
    uint32 existingRndBotAccounts = 0;
    uint32 existingAddClassAccounts = 0;
    uint32 existingUnassignedAccounts = 0;

    PlayerbotsDatabasePreparedStatement* typeStmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ACCOUNT_TYPE_COUNT_BY_TYPE);
    PreparedQueryResult typeCheck = PlayerbotsDatabase.Query(typeStmt);
    if (typeCheck)
    {
        do
        {
            Field* fields = typeCheck->Fetch();
            uint8 accountType = fields[0].Get<uint8>();
            uint32 count = static_cast<uint32>(fields[1].Get<uint64>());

            if (accountType == 0) existingUnassignedAccounts = count;
            else if (accountType == 1) existingRndBotAccounts = count;
            else if (accountType == 2) existingAddClassAccounts = count;
        } while (typeCheck->NextRow());
    }

    // Determine divisor based on Death Knight availability and requested A&H faction ratio
    int divisor = CalculateAvailableCharsPerAccount();

    // Calculate max bots
    int maxBots = sPlayerbotAIConfig.MaxRandomBots;
    // Take periodic online/offline into account
    if (sPlayerbotAIConfig.EnablePeriodicOnlineOffline)
        maxBots *= sPlayerbotAIConfig.PeriodicOnlineOfflineRatio;

    // Calculate number of accounts needed for RNDbots
    // Result is rounded up for maxBots not cleanly divisible by the divisor
    uint32 neededRndBotAccounts = (maxBots + divisor - 1) / divisor;
    uint32 neededAddClassAccounts = sPlayerbotAIConfig.AddClassAccountPoolSize;

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
    uint32 calculatedTotal = existingTotal + additionalAccountsNeeded;

    // Manually set randomBotAccountCount meets the requirements
    if (sPlayerbotAIConfig.RandomBotAccountCount >= calculatedTotal)
        return sPlayerbotAIConfig.RandomBotAccountCount;
    // Manually set randomBotAccountCount doesn't meet the requirements. Using calculated value
    if (sPlayerbotAIConfig.RandomBotAccountCount > 0)
        LOG_WARN("playerbots", "RandomBotAccountCount ({}) is lower than the required calculated value ({}). Using the calculated value instead.",
            sPlayerbotAIConfig.RandomBotAccountCount, calculatedTotal);

    return calculatedTotal;
}

uint32 RandomPlayerbotFactory::CalculateAvailableCharsPerAccount()
{
    // Death Knight availability according to their login eligibility, and if WotLK is enabled at all.
    bool noDK = sPlayerbotAIConfig.DisableDeathKnightLogin || sWorld->getIntConfig(CONFIG_EXPANSION) != EXPANSION_WRATH_OF_THE_LICH_KING;

    uint32 availableChars = noDK ? 9 : 10;

    uint32 hordeRatio = sPlayerbotAIConfig.RandomBotHordeRatio;
    uint32 allianceRatio = sPlayerbotAIConfig.RandomBotAllianceRatio;

    // horde : alliance = 50 : 50 -> 0%
    // horde : alliance = 0 : 50 -> 50%
    // horde : alliance = 10 : 50 -> 40%
    float unavailableRatio = static_cast<float>((std::max(hordeRatio, allianceRatio) - std::min(hordeRatio, allianceRatio))) /
        (std::max(hordeRatio, allianceRatio) * 2);

    // Conservative floor to ensure enough characters (may result in more accounts than needed).
    if (unavailableRatio != 0)
        availableChars = availableChars - availableChars * unavailableRatio;

    return availableChars;
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    /* multi-thread here is meaningless? since the async db operations */

    if (sPlayerbotAIConfig.DeleteRandomBotAccounts)
    {
        // Collect bot account ids from the login database so the cleanup below
        // never needs a cross-database subquery (the login database may live on
        // a different server than the character database)
        std::vector<uint32> botAccounts;
        QueryResult accountResult = LoginDatabase.Query("SELECT id FROM account WHERE username LIKE '{}%%' ORDER BY id",
            sPlayerbotAIConfig.RandomBotAccountPrefix.c_str());
        if (accountResult)
        {
            do
            {
                botAccounts.push_back(accountResult->Fetch()->Get<uint32>());
            } while (accountResult->NextRow());
        }

        std::string botAccountIds;
        for (uint32 accountId : botAccounts)
        {
            if (!botAccountIds.empty())
                botAccountIds += ", ";
            botAccountIds += std::to_string(accountId);
        }
        if (botAccountIds.empty())
            botAccountIds = "0";    // account ids start at 1, so this matches nothing

        LOG_INFO("playerbots", "Deleting all random bot characters and accounts ({} bot accounts found)...", botAccounts.size());

        // First execute all the cleanup SQL commands
        // Clear playerbots_random_bots and playerbots_account_type
        PlayerbotsDatabase.DirectExecute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
        PlayerbotsDatabase.DirectExecute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_ACCOUNT_TYPE));

        // Get the character database name dynamically (used in same-server subqueries below)
        std::string characterDBName = CharacterDatabase.GetConnectionInfo()->database;

        // Delete all characters from bot accounts (by explicit id list, no cross-database subquery)
        CharacterDatabase.Execute("DELETE FROM characters WHERE account IN (" + botAccountIds + ")");

        // Wait for the characters to be deleted before proceeding to dependent deletes
        while (CharacterDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Extra 100ms fixed delay for safety.

        // Clean up orphaned entries in playerbots_guild_tasks
        PlayerbotsDatabase.DirectExecute("DELETE FROM playerbots_guild_tasks WHERE owner NOT IN (SELECT guid FROM " + characterDBName + ".characters)");

        // Clean up orphaned entries in playerbots_db_store (explicit id list, no cross-database subquery)
        PlayerbotsDatabase.DirectExecute("DELETE FROM playerbots_db_store WHERE guid NOT IN (SELECT guid FROM " + characterDBName + ".characters WHERE account NOT IN (" + botAccountIds + "))");

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
                                             sPlayerbotAIConfig.RandomBotAccountPrefix.c_str());
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

        // Wait for all pending database operations to complete
        while (LoginDatabase.QueueSize() || CharacterDatabase.QueueSize() || PlayerbotsDatabase.QueueSize())
        {
            std::this_thread::sleep_for(1s);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    // Extra 100ms fixed delay for safety.

        // Flush tables to ensure all data in memory are written to disk
        LoginDatabase.Execute("FLUSH TABLES");
        CharacterDatabase.Execute("FLUSH TABLES");
        PlayerbotsDatabase.DirectExecute("FLUSH TABLES");

        LOG_INFO("playerbots", ">> Random bot accounts and data deleted in {} ms", GetMSTimeDiffToNow(timer));
        LOG_INFO("playerbots", "Please reset the Playerbots.DeleteRandomBotAccounts to 0 and restart the server...");
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
        out << sPlayerbotAIConfig.RandomBotAccountPrefix << accountNumber;
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
        if (sPlayerbotAIConfig.RandomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char)urand('!', 'z');
            }
        }
        else
            password = accountName;

        sAccountMgr->CreateAccount(accountName, password);

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
        out << sPlayerbotAIConfig.RandomBotAccountPrefix << accountNumber;
        std::string const accountName = out.str();

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->SetData(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (!result)
            continue;

        Field* fields = result->Fetch();
        uint32 accountId = fields[0].Get<uint32>();

        sPlayerbotAIConfig.RandomBotAccounts.push_back(accountId);

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
        RandomPlayerbotFactory factory;

        WorldSession* session = new WorldSession(accountId, "", 0x0, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING,
                                                time_t(0), LOCALE_enUS, 0, false, false, 0);
        sessionBots.push_back(session);

        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
        {
            // skip nonexistent classes
            if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
                continue;

            // skip disabled with config classes
            if ((1 << (cls - 1)) & sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK))
                continue;

            Player* playerBot = factory.CreateRandomBot(session, cls, nameCache);
            if (!playerBot)
            {
                LOG_ERROR("playerbots", "Fail to create character for account {}", accountId);
                continue;
            }

            playerBot->SaveToDB(true, false);
            sCharacterCache->AddCharacterCacheEntry(playerBot->GetGUID(), accountId, playerBot->GetName(),
                                                    playerBot->getGender(), playerBot->getRace(),
                                                    playerBot->getClass(), playerBot->GetLevel());
            playerBot->CleanupsBeforeDelete();
            delete playerBot;
            bot_creation++;
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

    for (uint32 accountId : sPlayerbotAIConfig.RandomBotAccounts)
    {
        totalRandomBotChars += AccountMgr::GetCharactersCount(accountId);
    }

    LOG_INFO("server.loading", ">> {} random bot accounts with {} characters available",
            sPlayerbotAIConfig.RandomBotAccounts.size(), totalRandomBotChars);
}

std::string const RandomPlayerbotFactory::CreateRandomGuildName()
{
    std::string guildName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbots_guild_names");
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return guildName;
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].Get<uint32>();

    uint32 id = urand(0, maxId);
    std::string nameExpr = GetLocalizedNameSelector("n.name");
    result = CharacterDatabase.Query(
        "SELECT {} FROM playerbots_guild_names n "
        "LEFT OUTER JOIN guild e ON e.name = {} "
        "WHERE e.guildid IS NULL AND n.name_id >= {} LIMIT 1",
        nameExpr, nameExpr, id);
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return guildName;
    }

    fields = result->Fetch();
    guildName = fields[0].Get<std::string>();

    return guildName;
}

bool RandomPlayerbotFactory::IsBotArenaTeam(ArenaTeam const* team)
{
    if (!team)
        return false;

    ObjectGuid captainGuid = team->GetCaptain();
    if (!captainGuid || !captainGuid.IsPlayer())
        return false;

    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(captainGuid);
    return accountId && sPlayerbotAIConfig.IsInRandomAccountList(accountId);
}

void RandomPlayerbotFactory::LoadArenaTeamData()
{
    _configTargets = {
        {ARENA_TYPE_2v2, sPlayerbotAIConfig.RandomBotArenaTeam2v2Count},
        {ARENA_TYPE_3v3, sPlayerbotAIConfig.RandomBotArenaTeam3v3Count},
        {ARENA_TYPE_5v5, sPlayerbotAIConfig.RandomBotArenaTeam5v5Count},
    };

    _botArenaTeamRegistry.clear();

    for (auto const& [id, team] : sArenaTeamMgr->GetArenaTeams())
    {
        if (!IsBotArenaTeam(team))
            continue;

        CharacterCacheEntry const* entry = sCharacterCache->GetCharacterCacheByGuid(team->GetCaptain());
        if (!entry)
            continue;

        ArenaType type = static_cast<ArenaType>(team->GetType());
        _botArenaTeamRegistry[type].push_back(id);
    }

    _availableArenaTeamNames.clear();

    // Join on the localized name so already-taken localized names are filtered out
    std::string nameExpr = GetLocalizedNameSelector("n.name");
    QueryResult result = CharacterDatabase.Query(
        "SELECT {} FROM playerbots_arena_team_names n "
        "LEFT OUTER JOIN arena_team e ON e.name = {} "
        "WHERE e.arenateamid IS NULL",
        nameExpr, nameExpr);

    if (!result)
    {
        LOG_WARN("playerbots", "No arena team names left in playerbots_arena_team_names");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        _availableArenaTeamNames.push_back(fields[0].Get<std::string>());
    } while (result->NextRow());

    for (size_t i = _availableArenaTeamNames.size() - 1; i > 0; --i)
    {
        size_t j = urand(0, i);
        std::swap(_availableArenaTeamNames[i], _availableArenaTeamNames[j]);
    }

    LOG_INFO("playerbots", "Loaded {} available arena team names", _availableArenaTeamNames.size());
}

bool RandomPlayerbotFactory::IsEligibleForBotArenaTeam(Player* bot)
{
    if (!bot || !bot->GetSession())
        return false;

    uint32 const accountId = bot->GetSession()->GetAccountId();
    if (!sPlayerbotAIConfig.IsInRandomAccountList(accountId))
        return false;

    if (sRandomPlayerbotMgr.IsAddClassAccount(accountId))
        return false;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
    {
        if (botAI->HasGameClientMaster())
            return false;
    }

    return true;
}

void RandomPlayerbotFactory::AssignBotToArenaTeam(Player* bot)
{
    if (!IsEligibleForBotArenaTeam(bot))
        return;

    if (sPlayerbotAIConfig.DeleteRandomBotArenaTeams)
        return;

    if (bot->GetLevel() < 70)
        return;

    for (uint8 arenaSlot = ARENA_SLOT_2v2; arenaSlot <= ARENA_SLOT_5v5; ++arenaSlot)
    {
        if (bot->GetArenaTeamId(arenaSlot))
            return;
    }

    PlayerbotWorldThreadProcessor::instance().QueueOperation(
        std::make_unique<ArenaTeamAssignOperation>(bot->GetGUID()));
}

void RandomPlayerbotFactory::AssignBotToArenaTeamInternal(Player* bot)
{
    // Check if bot has team, only one per bot to avoid queue conflicts
    for (uint8 arenaSlot = ARENA_SLOT_2v2; arenaSlot <= ARENA_SLOT_5v5; ++arenaSlot)
    {
        if (bot->GetArenaTeamId(arenaSlot) ||
            sCharacterCache->GetCharacterArenaTeamIdByGuid(bot->GetGUID(), arenaSlot))
            return;
    }

    TeamId const botTeam = bot->GetTeamId();

    // Randomize type order so no single type starves the others
    std::array<ArenaType, 3> order{ARENA_TYPE_2v2, ARENA_TYPE_3v3, ARENA_TYPE_5v5};
    for (size_t i = order.size() - 1; i > 0; --i)
        std::swap(order[i], order[urand(0, i)]);

    std::vector<ArenaTeam*> candidates;
    for (ArenaType type : order)
        CollectJoinableBotArenaTeams(type, botTeam, candidates);

    for (size_t i = candidates.size(); i > 0; --i)
    {
        size_t const index = urand(0, i - 1);
        ArenaTeam* team = candidates[index];
        candidates[index] = candidates[i - 1];

        if (!team->AddMember(bot->GetGUID()))
        {
            LOG_DEBUG("playerbots", "Failed to add bot {} to arena team '{}', trying next candidate",
                      bot->GetName(), team->GetName());
            continue;
        }

        if (team->GetMembersSize() >= static_cast<uint32>(team->GetType()))
        {
            uint32 teamRating = team->GetRating();
            team->SetRatingForAll(teamRating);

            // Keep MMR synchronized with team rating so matchmaking reflects artificial bot strength
            // (1000-2000 range) instead of the global CONFIG_ARENA_START_MATCHMAKER_RATING default.
            for (auto& member : team->GetMembers())
            {
                member.MatchMakerRating = member.PersonalRating;
                member.MaxMMR = std::max(member.MaxMMR, member.PersonalRating);
            }
            team->SaveToDB(true);
        }
        return;
    }

    // No joinable team available, create one if under target count
    for (ArenaType type : order)
    {
        if (GetBotArenaTeamCount(type) < _configTargets[type])
        {
            CreateBotArenaTeam(bot, type);
            return;
        }
    }
}

void RandomPlayerbotFactory::CreateBotArenaTeam(Player* bot, ArenaType type)
{
    std::string teamName = CreateRandomArenaTeamName();
    if (teamName.empty())
        return;

    ArenaTeam* arenateam = new ArenaTeam();
    if (!arenateam->Create(bot->GetGUID(), type, teamName, 0, 0, 0, 0, 0))
    {
        LOG_ERROR("playerbots", "Error creating arena team {}", teamName);
        delete arenateam;
        _availableArenaTeamNames.push_back(std::move(teamName));
        return;
    }

    arenateam->SetRatingForAll(
        urand(sPlayerbotAIConfig.RandomBotArenaTeamMinRating, sPlayerbotAIConfig.RandomBotArenaTeamMaxRating));

    uint32 const backgroundColor = urand(0xFF000000, 0xFFFFFFFF);
    uint8 const emblemStyle = urand(0, 101);
    uint32 const emblemColor = urand(0xFF000000, 0xFFFFFFFF);
    uint8 const borderStyle = urand(0, 5);
    uint32 const borderColor = urand(0xFF000000, 0xFFFFFFFF);
    arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);

    arenateam->SaveToDB();
    sArenaTeamMgr->AddArenaTeam(arenateam);
    _botArenaTeamRegistry[type].push_back(arenateam->GetId());

    LOG_DEBUG("playerbots", "Created {}v{} arena team '{}' with captain {}",
              type, type, teamName, bot->GetName());
}

uint32 RandomPlayerbotFactory::GetBotArenaTeamCount(ArenaType type)
{
    auto it = _botArenaTeamRegistry.find(type);
    return it != _botArenaTeamRegistry.end() ? static_cast<uint32>(it->second.size()) : 0;
}

void RandomPlayerbotFactory::CollectJoinableBotArenaTeams(ArenaType type, TeamId faction, std::vector<ArenaTeam*>& out)
{
    auto it = _botArenaTeamRegistry.find(type);
    if (it == _botArenaTeamRegistry.end())
        return;

    uint32 const capacity = static_cast<uint32>(type);
    for (uint32 teamId : it->second)
    {
        ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(teamId);
        if (!team || team->GetMembersSize() >= capacity)
            continue;

        CharacterCacheEntry const* entry = sCharacterCache->GetCharacterCacheByGuid(team->GetCaptain());
        if (entry && Player::TeamIdForRace(entry->Race) == faction)
            out.push_back(team);
    }
}

void RandomPlayerbotFactory::DeleteBotArenaTeams()
{
    LOG_INFO("playerbots", "Deleting random bot arena teams...");

    std::vector<uint32> teamsToDisband;
    for (auto const& [id, arenateam] : sArenaTeamMgr->GetArenaTeams())
    {
        if (IsBotArenaTeam(arenateam))
            teamsToDisband.push_back(id);
    }

    for (uint32 teamId : teamsToDisband)
    {
        ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(teamId);
        if (team)
            team->Disband(nullptr);
    }

    _botArenaTeamRegistry.clear();
    LOG_INFO("playerbots", "Deleted {} random bot arena teams", teamsToDisband.size());
}

std::string RandomPlayerbotFactory::CreateRandomArenaTeamName()
{
    if (_availableArenaTeamNames.empty())
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return "";
    }

    std::string name = std::move(_availableArenaTeamNames.back());
    _availableArenaTeamNames.pop_back();
    return name;
}
