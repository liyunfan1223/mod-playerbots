/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotGuildMgr.h"
#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotFactory.h"
#include "ScriptMgr.h"

void PlayerbotGuildMgr::Init()
{
    _guildCache.clear();
    if (sPlayerbotAIConfig.DeleteRandomBotGuilds)
        DeleteRandomBotGuilds();

    LoadGuildNames();
    ValidateGuildCache();
}

bool PlayerbotGuildMgr::CreateGuild(Player* player, std::string guildName)
{
    Guild* guild = new Guild();
    if (!guild->Create(player, guildName))
    {
        LOG_ERROR("playerbots", "Error creating guild [ {} ] with leader [ {} ]", guildName,
            player->GetName());
        delete guild;
        return false;
    }
    sGuildMgr->AddGuild(guild);

    LOG_DEBUG("playerbots", "Guild created: id={} name='{}'", guild->GetId(), guildName);
    SetGuildEmblem(guild->GetId());

    GuildCache entry;
    entry.name = guildName;
    entry.memberCount = 1;
    entry.status = 1;
    entry.maxMembers = sPlayerbotAIConfig.RandomBotGuildSizeMax;
    entry.faction = player->GetTeamId();

    _guildCache[guild->GetId()] = entry;
    return true;
}

bool PlayerbotGuildMgr::SetGuildEmblem(uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    // create random emblem
    uint32 st, cl, br, bc, bg;
    bg = urand(0, 51);
    bc = urand(0, 17);
    cl = urand(0, 17);
    br = urand(0, 7);
    st = urand(0, 180);

    guild->HandleSetEmblem(EmblemInfo(st, cl, br, bc, bg));
    return true;
}

std::string PlayerbotGuildMgr::AssignToGuild(Player* player)
{
    if (!player)
        return "";

    uint8_t playerFaction = player->GetTeamId();
    std::vector<GuildCache*> partiallyfilledguilds;
    partiallyfilledguilds.reserve(_guildCache.size());

    for (auto& keyValue : _guildCache)
    {
        GuildCache& cached = keyValue.second;
        if (!cached.hasRealPlayer && cached.status == 1 && cached.faction == playerFaction)
            partiallyfilledguilds.push_back(&cached);
    }

    if (!partiallyfilledguilds.empty())
    {
        size_t idx = static_cast<size_t>(urand(0, static_cast<int>(partiallyfilledguilds.size()) - 1));
        return (partiallyfilledguilds[idx]->name);
    }

    size_t count = std::count_if(
        _guildCache.begin(), _guildCache.end(),
        [](std::pair<const uint32, GuildCache> const& pair)
        {
            return !pair.second.hasRealPlayer;
        }
        );

    if (count < sPlayerbotAIConfig.RandomBotGuildCount)
    {
        for (auto& key : _shuffled_guild_keys)
        {
            if (_guildNames[key])
            {
                LOG_INFO("playerbots","Assigning player [{}] to guild [{}]", player->GetName(), key);
                return key;
            }
        }
        LOG_ERROR("playerbots","No available guild names left.");
    }
    return "";
}

void PlayerbotGuildMgr::OnGuildUpdate(Guild* guild)
{
    auto it = _guildCache.find(guild->GetId());
    if (it == _guildCache.end())
        return;

    GuildCache& entry = it->second;
    entry.memberCount = guild->GetMemberCount();
    if (entry.memberCount < entry.maxMembers)
        entry.status = 1;
    else if (entry.memberCount >= entry.maxMembers)
        entry.status = 2; // Full
    std::string guildName = guild->GetName();
    for (auto& it : _guildNames)
    {
        if (it.first == guildName)
        {
            it.second = false;
            break;
        }
    }
}

void PlayerbotGuildMgr::ResetGuildCache()
{
    _guildCache.clear();

    for (auto& nameEntry : _guildNames)
        nameEntry.second = true;
}

void PlayerbotGuildMgr::LoadGuildNames()
{
    LOG_INFO("playerbots", "Loading guild names from playerbots_guild_names...");

    _guildNames.clear();
    _shuffled_guild_keys.clear();

    QueryResult result = CharacterDatabase.Query(
        "SELECT {} FROM playerbots_guild_names",
        RandomPlayerbotFactory::GetLocalizedNameSelector("name"));

    if (!result)
    {
        LOG_ERROR("playerbots", "No entries found in playerbots_guild_names. List is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        std::string name = fields[0].Get<std::string>();
        if (!name.empty())
            _guildNames[name] = true;
    } while (result->NextRow());

    for (auto& pair : _guildNames)
        _shuffled_guild_keys.push_back(pair.first);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(_shuffled_guild_keys.begin(), _shuffled_guild_keys.end(), g);
    LOG_INFO("playerbots", "Loaded {} guild entries from playerbots_guild_names table.", _guildNames.size());
}

void PlayerbotGuildMgr::ValidateGuildCache()
{
    QueryResult result = CharacterDatabase.Query("SELECT guildid, name FROM guild");
    if (!result)
    {
        LOG_ERROR("playerbots", "No guilds found in database, resetting guild cache");
        ResetGuildCache();
        return;
    }

    std::unordered_map<uint32, std::string> dbGuilds;
    do
    {
        Field* fields = result->Fetch();
        uint32 guildId = fields[0].Get<uint32>();
        std::string guildName = fields[1].Get<std::string>();
        dbGuilds[guildId] = guildName;
    } while (result->NextRow());

    for (auto it = dbGuilds.begin(); it != dbGuilds.end(); it++)
    {
        uint32 guildId = it->first;
        GuildCache cache;
        cache.name = it->second;
        cache.maxMembers = sPlayerbotAIConfig.RandomBotGuildSizeMax;

        Guild* guild = sGuildMgr ->GetGuildById(guildId);
        if (!guild)
            continue;

        cache.memberCount = guild->GetMemberCount();
        ObjectGuid leaderGuid = guild->GetLeaderGUID();
        CharacterCacheEntry const* leaderEntry = sCharacterCache->GetCharacterCacheByGuid(leaderGuid);
        uint32 leaderAccount = leaderEntry->AccountId;
        cache.hasRealPlayer = !(sPlayerbotAIConfig.IsInRandomAccountList(leaderAccount));
        cache.faction = Player::TeamIdForRace(leaderEntry->Race);
        if (cache.memberCount == 0)
            cache.status = 0; // empty
        else if (cache.memberCount < cache.maxMembers)
            cache.status = 1; // partially filled
        else
            cache.status = 2; // full

        _guildCache.insert_or_assign(guildId, cache);
        for (auto& it : _guildNames)
        {
            if (it.first == cache.name)
            {
                it.second = false;
                break;
            }
        }
    }
}

void PlayerbotGuildMgr::DeleteRandomBotGuilds()
{
    LOG_INFO("playerbots", "Deleting randombot guilds...");
    std::vector<uint32> guildsToDisband;

    if (QueryResult result = CharacterDatabase.Query("SELECT guildid, leaderguid FROM guild"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guildId = fields[0].Get<uint32>();
            ObjectGuid leader = ObjectGuid::Create<HighGuid::Player>(fields[1].Get<uint32>());

            // The leader's account is checked instead of the 'add' event or anything else that depends on the
            // bot being online.
            if (sPlayerbotAIConfig.IsInRandomAccountList(sCharacterCache->GetCharacterAccountIdByGuid(leader)))
                guildsToDisband.push_back(guildId);
        } while (result->NextRow());
    }

    for (uint32 guildId : guildsToDisband)
    {
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
            guild->Disband();
    }
    LOG_INFO("playerbots", "Randombot guilds deleted");
}

bool PlayerbotGuildMgr::IsRealGuild(Player* bot)
{
    if (!bot)
        return false;
    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    return IsRealGuild(guildId);
}

bool PlayerbotGuildMgr::IsRealGuild(uint32 guildId)
{
    if (!guildId)
        return false;

    auto it = _guildCache.find(guildId);
    if (it == _guildCache.end())
        return false;

    // A "real guild" is one whose leader's account is not in the bot accounts list.
    // Guild membership by real players does not affect this, only the leader's account type does.
    return it->second.hasRealPlayer;
}

class BotGuildCacheWorldScript : public WorldScript
{
    public:

        BotGuildCacheWorldScript() : WorldScript("BotGuildCacheWorldScript"), _validateTimer(0){}

        void OnUpdate(uint32 diff) override
        {
            _validateTimer += diff;

            if (_validateTimer >= _validateInterval) // Validate every hour
            {
                _validateTimer = 0;
                PlayerbotGuildMgr::instance().ValidateGuildCache();
                LOG_INFO("playerbots", "Scheduled guild cache validation");
            }
        }

    private:
        uint32 _validateInterval = HOUR*IN_MILLISECONDS;
        uint32 _validateTimer;
};

void PlayerBotsGuildValidationScript()
{
    new BotGuildCacheWorldScript();
}
