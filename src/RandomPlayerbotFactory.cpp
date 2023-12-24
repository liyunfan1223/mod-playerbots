/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomPlayerbotFactory.h"
#include "ArenaTeamMgr.h"
#include "AccountMgr.h"
#include "DatabaseEnv.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "PlayerbotFactory.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"

std::map<uint8, std::vector<uint8>> RandomPlayerbotFactory::availableRaces;

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
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);
    }

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
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
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
        availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD_PLAYER);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);
    }

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
    {
        availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);
    }

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
    if(expansion >= EXPANSION_THE_BURNING_CRUSADE)
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

Player* RandomPlayerbotFactory::CreateRandomBot(WorldSession* session, uint8 cls, std::unordered_map<uint8, std::vector<std::string>> names)
{
    LOG_DEBUG("playerbots",  "Creating new random bot for class {}", cls);

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;
    uint8 alliance = rand() % 2;
    uint8 race;
    for (int attempt = 0; attempt < 15; attempt++) {
        race = availableRaces[cls][urand(0, availableRaces[cls].size() - 1)];
        if ((alliance && IsAlliance(race)) || (!alliance && !IsAlliance(race))) {
            break;
        }
    }
    std::string name;
    // int tries = 10;
    // while (tries--) {
    //     std::string cur_name;
    //     if (names.empty())
    name = CreateRandomBotName(gender);
    //     else
    //     {
    //         if (names[gender].empty())
    //             return nullptr;

    //         uint32 i = urand(0, names[gender].size() - 1);
    //         cur_name = names[gender][i];
    //         std::swap(names[gender][i], names[gender].back());
    //         names[gender].pop_back();
    //     }
    //     if (ObjectMgr::CheckPlayerName(name) != CHAR_NAME_SUCCESS)
    //     {
    //         name.clear();
    //         continue;
    //     }
    // }

    if (name.empty()) {
        LOG_ERROR("playerbots", "Unable to get random bot name!");
        return nullptr;
    }
    CharacterDatabase.DirectExecute("UPDATE playerbots_names SET in_use=1 WHERE name='{}'", name);

    std::vector<uint8> skinColors, facialHairTypes;
    std::vector<std::pair<uint8,uint8>> faces, hairs;
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

    uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)];
    std::pair<uint8, uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8, uint8> hair = hairs[urand(0, hairs.size() - 1)];

	bool excludeCheck = (race == RACE_TAUREN) || (race == RACE_DRAENEI) || (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
	uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];

    std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>(name, race, cls, gender, face.second, face.first, hair.first, hair.second, facialHair);

    Player* player = new Player(session);
    player->GetMotionMaster()->Initialize();
	if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), characterInfo.get()))
    {
        player->CleanupsBeforeDelete();
        delete player;

        LOG_ERROR("playerbots", "Unable to create random bot for account {} - name: \"{}\"; race: {}; class: {}", accountId, name.c_str(), race, cls);
        return nullptr;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    if (player->getClass() == CLASS_DEATH_KNIGHT)
	{
		player->learnSpell(50977, false);
	}
    // player->SaveToDB(true, false);
    // player->RewardQuest(const Quest *quest, uint32 reward, Object *questGiver)
    LOG_DEBUG("playerbots", "Random bot created for account {} - name: \"{}\"; race: {}; class: {}", accountId, name.c_str(), race, cls);

    return player;
}

std::string const RandomPlayerbotFactory::CreateRandomBotName(uint8 gender)
{
    std::string botName = "";
    int tries = 10;
    while(--tries) {
        QueryResult result = CharacterDatabase.Query("SELECT name FROM playerbots_names "
                "WHERE in_use = 0 AND gender = {} ORDER BY RAND() LIMIT 1", gender);
        if (!result)
        {
            break;
        }
        Field* fields = result->Fetch();
        std::string ret = fields[0].Get<std::string>();
        if (ObjectMgr::CheckPlayerName(ret) == CHAR_NAME_SUCCESS)
        {
            return ret;
        }
    }
    LOG_ERROR("playerbots", "No more names left for random bots. Simply random.");
    tries = 10;
    while(--tries) {
        for (uint8 i = 0; i < 10; i++) {
            botName += (i == 0 ? 'A' : 'a') + rand() % 26;
        }
        if (ObjectMgr::CheckPlayerName(botName) != CHAR_NAME_SUCCESS ||
            (sObjectMgr->IsReservedName(botName) || sObjectMgr->IsProfanityName(botName)))
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

void RandomPlayerbotFactory::CreateRandomBots()
{
    /* multi-thread here is meaningless? since the async db operations */
    if (sPlayerbotAIConfig->deleteRandomBotAccounts)
    {
        std::vector<uint32> botAccounts;
        std::vector<uint32> botFriends;

        for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig->randomBotAccountCount; ++accountNumber)
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
            std::string const accountName = out.str();

            if (uint32 accountId = AccountMgr::GetId(accountName))
                botAccounts.push_back(accountId);
        }

        LOG_INFO("playerbots", "Deleting all random bot characters, {} accounts collected...", botAccounts.size());
        QueryResult results = LoginDatabase.Query("SELECT id FROM account WHERE username LIKE '{}%%'", sPlayerbotAIConfig->randomBotAccountPrefix.c_str());
        int32 deletion_count = 0;
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                uint32 accId = fields[0].Get<uint32>();
                LOG_INFO("playerbots", "Deleting account accID: {}({})...", accId, ++deletion_count);
                AccountMgr::DeleteAccount(accId);
            } while (results->NextRow());
        }

        PlayerbotsDatabase.Execute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
        CharacterDatabase.DirectExecute("UPDATE playerbots_names SET in_use = 0 WHERE in_use = 1");
        /* TODO(yunfan): we need to sleep here to wait for async account deleted, or the newly account won't be created correctly
           the better way is turning the async db operation to sync db operation */
        std::this_thread::sleep_for(10ms * sPlayerbotAIConfig->randomBotAccountCount);
        LOG_INFO("playerbots", "Random bot characters deleted.");
        LOG_INFO("playerbots", "Please reset the AiPlayerbot.DeleteRandomBotAccounts to 0 and restart the server...");
        World::StopNow(SHUTDOWN_EXIT_CODE);
        return;
    }
    
    uint32 totalAccCount = sPlayerbotAIConfig->randomBotAccountCount;

    LOG_INFO("playerbots", "Creating random bot accounts...");

    std::vector<std::future<void>> account_creations;
    bool account_creation = false;
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig->randomBotAccountCount; ++accountNumber)
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
        account_creation = true;
        std::string password = "";
        if (sPlayerbotAIConfig->randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char) urand('!', 'z');
            }
        }
        else
            password = accountName;

        AccountMgr::CreateAccount(accountName, password);

        LOG_DEBUG("playerbots", "Account {} created for random bots", accountName.c_str());
    }

    if (account_creation) {
        /* wait for async accounts create to make character create correctly, same as account delete */
        std::this_thread::sleep_for(10ms * sPlayerbotAIConfig->randomBotAccountCount);
    }

    LOG_INFO("playerbots", "Creating random bot characters...");
    uint32 totalRandomBotChars = 0;
    uint32 totalCharCount = sPlayerbotAIConfig->randomBotAccountCount * 10;

    std::unordered_map<uint8,std::vector<std::string>> names;
    std::vector<std::pair<Player*, uint32>> playerBots;
    std::vector<WorldSession*> sessionBots;
    bool bot_creation = false;
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig->randomBotAccountCount; ++accountNumber)
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
        bot_creation = true;
        LOG_INFO("playerbots", "Creating random bot characters for account: [{}/{}]", accountNumber + 1, sPlayerbotAIConfig->randomBotAccountCount);
        RandomPlayerbotFactory factory(accountId);

        WorldSession* session = new WorldSession(accountId, "", nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0), LOCALE_enUS, 0, false, false, 0, true);
        sessionBots.push_back(session);

        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
        {
            // skip nonexistent classes
            if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
                continue;

            if (bool const isClassDeathKnight = cls == CLASS_DEATH_KNIGHT;
                isClassDeathKnight &&
                sWorld->getIntConfig(CONFIG_EXPANSION) !=
                EXPANSION_WRATH_OF_THE_LICH_KING)
            {
                continue;
            }

            if (cls != 10)
                if (Player* playerBot = factory.CreateRandomBot(session, cls, names)) {
                    playerBot->SaveToDB(true, false);
                    sCharacterCache->AddCharacterCacheEntry(playerBot->GetGUID(), accountId, playerBot->GetName(), 
                        playerBot->getGender(), playerBot->getRace(), playerBot->getClass(), playerBot->getLevel());
                    playerBot->CleanupsBeforeDelete();
                    delete playerBot;
                }
        }
    }

    if (bot_creation) {
        LOG_INFO("playerbots", "Waiting for {} characters loading into database...", totalCharCount);
        /* wait for characters load into database, or characters will fail to loggin */
        std::this_thread::sleep_for(10s);
    }

    for (WorldSession* session : sessionBots)
        delete session;
    
    for (uint32 accountId : sPlayerbotAIConfig->randomBotAccounts) {
        totalRandomBotChars += AccountMgr::GetCharactersCount(accountId);
    }

    LOG_INFO("server.loading", "{} random bot accounts with {} characters available", sPlayerbotAIConfig->randomBotAccounts.size(), totalRandomBotChars);
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
        }
        while (result->NextRow());
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
            LOG_ERROR("playerbots", "ObjectAccessor Cannot find player to set leader for guild {} . Skipped...", guildName.c_str());
            continue;
        }

        if (player->GetGuildId())
            continue;

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            LOG_ERROR("playerbots", "Error creating guild [ {} ] with leader [ {} ]", guildName.c_str(), player->GetName().c_str());
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
    result = CharacterDatabase.Query("SELECT n.name FROM playerbots_guild_names n "
            "LEFT OUTER JOIN guild e ON e.name = n.name WHERE e.guildid IS NULL AND n.name_id >= {} LIMIT 1", id);
    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random guilds");
        return std::move(guildName);
    }

    fields = result->Fetch();
    guildName = fields[0].Get<std::string>();

    return std::move(guildName);
}

void RandomPlayerbotFactory::CreateRandomArenaTeams()
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
        }
        while (result->NextRow());
    }

    if (sPlayerbotAIConfig->deleteRandomBotArenaTeams)
    {
        LOG_INFO("playerbots", "Deleting random bot arena teams...");

        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid captain = ObjectGuid::Create<HighGuid::Player>(*i);
            ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(captain);
            if (arenateam)
                //sObjectMgr->RemoveArenaTeam(arenateam->GetId());
                arenateam->Disband(nullptr);
        }

        LOG_INFO("playerbots", "Random bot arena teams deleted");
    }

    uint32 arenaTeamNumber = 0;
    GuidVector availableCaptains;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid captain = ObjectGuid::Create<HighGuid::Player>(*i);
        ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamByCaptain(captain);
        if (arenateam)
        {
            ++arenaTeamNumber;
            sPlayerbotAIConfig->randomBotArenaTeams.push_back(arenateam->GetId());
        }
        else
        {
            Player* player = ObjectAccessor::FindConnectedPlayer(captain);

            if (!arenateam && player && player->getLevel() >= 70)
                availableCaptains.push_back(captain);
        }
    }

    for (; arenaTeamNumber < sPlayerbotAIConfig->randomBotArenaTeamCount; ++arenaTeamNumber)
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

        if (player->getLevel() < 70)
        {
            LOG_ERROR("playerbots", "Bot {} must be level 70 to create an arena team", captain.ToString().c_str());
            continue;
        }

        QueryResult results = CharacterDatabase.Query("SELECT `type` FROM playerbots_arena_team_names WHERE name = '{}'", arenaTeamName.c_str());
        if (!results)
        {
            LOG_ERROR("playerbots", "No valid types for arena teams");
            return;
        }

        Field* fields = results->Fetch();
        uint8 slot = fields[0].Get<uint8>();

        ArenaType type;
        switch (slot)
        {
            case 2:
                type = ARENA_TYPE_2v2;
                break;
            case 3:
                type = ARENA_TYPE_3v3;
                break;
            case 5:
                type = ARENA_TYPE_5v5;
                break;
        }

        ArenaTeam* arenateam = new ArenaTeam();
        if (!arenateam->Create(player->GetGUID(), type, arenaTeamName, 0, 0, 0, 0, 0))
        {
            LOG_ERROR("playerbots", "Error creating arena team {}", arenaTeamName.c_str());
            continue;
        }

        arenateam->SetCaptain(player->GetGUID());

        // set random rating
        arenateam->SetRatingForAll(urand(1500, 2700));

        // set random emblem
        uint32 backgroundColor = urand(0xFF000000, 0xFFFFFFFF);
        uint32 emblemStyle = urand(0, 101);
        uint32 emblemColor = urand(0xFF000000, 0xFFFFFFFF);
        uint32 borderStyle = urand(0, 5);
        uint32 borderColor = urand(0xFF000000, 0xFFFFFFFF);
        arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);

        // set random kills (wip)
        //arenateam->SetStats(STAT_TYPE_GAMES_WEEK, urand(0, 30));
        //arenateam->SetStats(STAT_TYPE_WINS_WEEK, urand(0, arenateam->GetStats().games_week));
        //arenateam->SetStats(STAT_TYPE_GAMES_SEASON, urand(arenateam->GetStats().games_week, arenateam->GetStats().games_week * 5));
        //arenateam->SetStats(STAT_TYPE_WINS_SEASON, urand(arenateam->GetStats().wins_week, arenateam->GetStats().games
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
    result = CharacterDatabase.Query("SELECT n.name FROM playerbots_arena_team_names n LEFT OUTER JOIN arena_team e ON e.name = n.name "
        "WHERE e.arenateamid IS NULL AND n.name_id >= {} LIMIT 1", id);

    if (!result)
    {
        LOG_ERROR("playerbots", "No more names left for random arena teams");
        return std::move(arenaTeamName);
    }

    fields = result->Fetch();
    arenaTeamName = fields[0].Get<std::string>();

    return std::move(arenaTeamName);
}
