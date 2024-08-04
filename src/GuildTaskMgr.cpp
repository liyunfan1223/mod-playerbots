/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildTaskMgr.h"

#include "ChatHelper.h"
#include "Group.h"
#include "GuildMgr.h"
#include "Mail.h"
#include "MapMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "ServerFacade.h"

char* strstri(char const* str1, char const* str2);

enum GuildTaskType
{
    GUILD_TASK_TYPE_NONE = 0,
    GUILD_TASK_TYPE_ITEM = 1,
    GUILD_TASK_TYPE_KILL = 2
};

void GuildTaskMgr::Update(Player* player, Player* guildMaster)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
        return;

    if (!GetTaskValue(0, 0, "advert_cleanup"))
    {
        CleanupAdverts();
        RemoveDuplicatedAdverts();
        SetTaskValue(0, 0, "advert_cleanup", 1, sPlayerbotAIConfig->guildTaskAdvertCleanupTime);
    }

    PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(guildMaster);
    uint32 guildId = guildMaster->GetGuildId();
    if (!guildId || !masterBotAI || !guildMaster->GetGuildId())
        return;

    if (!player->IsFriendlyTo(guildMaster))
        return;

    Guild* guild = sGuildMgr->GetGuildById(guildMaster->GetGuildId());

    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel secLevel = masterBotAI->GetSecurity()->LevelFor(player, &reason);
    if (secLevel == PLAYERBOT_SECURITY_DENY_ALL ||
        (secLevel == PLAYERBOT_SECURITY_TALK && reason != PLAYERBOT_DENY_FAR))
    {
        LOG_DEBUG("playerbots", "{} / {}: skipping guild task update - not enough security level, reason = {}",
                  guild->GetName().c_str(), player->GetName().c_str(), reason);
        return;
    }

    LOG_DEBUG("playerbots", "{}: guild task update for player {}", guild->GetName().c_str(), player->GetName().c_str());

    ObjectGuid::LowType owner = player->GetGUID().GetCounter();

    uint32 activeTask = GetTaskValue(owner, guildId, "activeTask");
    if (!activeTask)
    {
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "itemTask", 0, 0);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "killCount", 0, 0);
        SetTaskValue(owner, guildId, "payment", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);

        uint32 task = CreateTask(player, guildId);

        if (task == GUILD_TASK_TYPE_NONE)
        {
            LOG_ERROR("playerbots", "{} / {}: error creating guild task", guild->GetName().c_str(),
                      player->GetName().c_str());
        }

        uint32 time = urand(sPlayerbotAIConfig->minGuildTaskChangeTime, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "activeTask", task, time);
        SetTaskValue(owner, guildId, "advertisement", 1,
                     urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime,
                           sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));

        LOG_DEBUG("playerbots", "{} / {}: guild task {} is set for {} secs", guild->GetName().c_str(),
                  player->GetName().c_str(), task, time);
        return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 advertisement = GetTaskValue(owner, guildId, "advertisement");
    if (!advertisement)
    {
        LOG_DEBUG("playerbots", "{} / {}: sending advertisement", guild->GetName().c_str(), player->GetName().c_str());

        if (SendAdvertisement(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "advertisement", 1,
                         urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime,
                               sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));
        }
        else
        {
            LOG_DEBUG("playerbots", "{} / {}: error sending advertisement", guild->GetName().c_str(),
                      player->GetName().c_str());
        }
    }

    uint32 thanks = GetTaskValue(owner, guildId, "thanks");
    if (!thanks)
    {
        LOG_DEBUG("playerbots", "{} / {}: sending thanks", guild->GetName().c_str(), player->GetName().c_str());

        if (SendThanks(trans, owner, guildId, GetTaskValue(owner, guildId, "payment")))
        {
            SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            LOG_DEBUG("playerbots", "{} / {}: error sending thanks", guild->GetName().c_str(),
                      player->GetName().c_str());
        }
    }

    uint32 reward = GetTaskValue(owner, guildId, "reward");
    if (!reward)
    {
        LOG_DEBUG("playerbots", "{} / {}: sending reward", guild->GetName().c_str(), player->GetName().c_str());

        if (Reward(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            LOG_DEBUG("playerbots", "{} / {}: error sending reward", guild->GetName().c_str(),
                      player->GetName().c_str());
        }
    }

    CharacterDatabase.CommitTransaction(trans);
}

uint32 GuildTaskMgr::CreateTask(Player* owner, uint32 guildId)
{
    switch (urand(0, 1))
    {
        case 0:
            CreateItemTask(owner, guildId);
            return GUILD_TASK_TYPE_ITEM;
        default:
            CreateKillTask(owner, guildId);
            return GUILD_TASK_TYPE_KILL;
    }
}

class RandomItemBySkillGuildTaskPredicate : public RandomItemPredicate
{
public:
    RandomItemBySkillGuildTaskPredicate(Player* player) : RandomItemPredicate(), player(player) {}

    bool Apply(ItemTemplate const* proto) override
    {
        uint32* tradeSkills = PlayerbotFactory::tradeSkills;

        for (uint32 i = 0; i < 13; ++i)
        {
            uint32 skillId = PlayerbotFactory::tradeSkills[i];
            if (player->HasSkill(skillId) && RandomItemMgr::IsUsedBySkill(proto, skillId))
                return true;
        }

        return false;
    }

private:
    Player* player;
};

bool GuildTaskMgr::CreateItemTask(Player* player, uint32 guildId)
{
    if (!player || player->GetLevel() < 5)
        return false;

    RandomItemBySkillGuildTaskPredicate predicate(player);
    uint32 itemId = sRandomItemMgr->GetRandomItem(player->GetLevel() - 5, RANDOM_ITEM_GUILD_TASK, &predicate);
    if (!itemId)
    {
        LOG_ERROR("playerbots", "{} / {}: no items avaible for item task",
                  sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str());
        return false;
    }

    uint32 count = GetMaxItemTaskCount(itemId);

    LOG_DEBUG("playerbots", "{} / {}: item task {} (x{})", sGuildMgr->GetGuildById(guildId)->GetName().c_str(),
              player->GetName().c_str(), itemId, count);

    SetTaskValue(player->GetGUID().GetCounter(), guildId, "itemCount", count,
                 sPlayerbotAIConfig->maxGuildTaskChangeTime);
    SetTaskValue(player->GetGUID().GetCounter(), guildId, "itemTask", itemId,
                 sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::CreateKillTask(Player* player, uint32 guildId)
{
    if (!player)
        return false;

    uint32 rank = !urand(0, 2) ? CREATURE_ELITE_RAREELITE : CREATURE_ELITE_RARE;

    std::vector<uint32> ids;

    uint32 level = player->GetLevel();
    QueryResult results = WorldDatabase.Query(
        "SELECT ct.Entry, c.map, c.position_x, c.position_y, ct.Name FROM creature_template ct "
        "JOIN creature c ON ct.Entry = c.id1 WHERE ct.MaxLevel < {} AND ct.MinLevel > {} AND ct.Rank = {} ",
        level + 4, level - 3, rank);
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 id = fields[0].Get<uint32>();
            uint32 map = fields[1].Get<uint32>();
            float x = fields[2].Get<float>();
            float y = fields[3].Get<float>();
            std::string const name = fields[4].Get<std::string>();

            if (strstr(name.c_str(), "UNUSED"))
                continue;

            float dist = sServerFacade->GetDistance2d(player, x, y);
            if (dist > sPlayerbotAIConfig->guildTaskKillTaskDistance || player->GetMapId() != map)
                continue;

            if (find(ids.begin(), ids.end(), id) == ids.end())
                ids.push_back(id);
        } while (results->NextRow());
    }

    if (ids.empty())
    {
        LOG_ERROR("playerbots", "{} / {}: no rare creatures available for kill task",
                  sGuildMgr->GetGuildById(guildId)->GetName().c_str(), player->GetName().c_str());
        return false;
    }

    uint32 index = urand(0, ids.size() - 1);
    uint32 creatureId = ids[index];

    LOG_DEBUG("playerbots", "{} / {}: kill task {}", sGuildMgr->GetGuildById(guildId)->GetName().c_str(),
              player->GetName().c_str(), creatureId);

    SetTaskValue(player->GetGUID().GetCounter(), guildId, "killTask", creatureId,
                 sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::SendAdvertisement(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 validIn;
    GetTaskValue(owner, guildId, "activeTask", &validIn);

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
        return SendItemAdvertisement(trans, itemTask, owner, guildId, validIn);

    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (killTask)
        return SendKillAdvertisement(trans, killTask, owner, guildId, validIn);

    return false;
}

std::string const formatTime(uint32 secs)
{
    std::ostringstream out;
    if (secs < 3600)
    {
        out << secs / 60 << " min";
    }
    else if (secs < 7200)
    {
        out << "1 hr " << (secs - 3600) / 60 << " min";
    }
    else if (secs < 3600 * 24)
    {
        out << secs / 3600 << " hr";
    }
    else
    {
        out << secs / 3600 / 24 << " days";
    }

    return out.str();
}

std::string const formatDateTime(uint32 secs)
{
    time_t rawtime = time(nullptr) + secs;
    tm* timeinfo = localtime(&rawtime);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "%b %d, %H:%M", timeinfo);
    return std::string(buffer);
}

std::string const GetHelloText(uint32 owner)
{
    ObjectGuid ownerGUID = ObjectGuid::Create<HighGuid::Player>(owner);

    std::ostringstream body;
    body << "Hello";

    std::string playerName;
    sCharacterCache->GetCharacterNameByGuid(ownerGUID, playerName);
    if (!playerName.empty())
        body << ", " << playerName;

    body << ",\n\n";

    return body.str();
}

bool GuildTaskMgr::SendItemAdvertisement(CharacterDatabaseTransaction& trans, uint32 itemId, uint32 owner,
                                         uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "We are in a great need of " << proto->Name1 << ". If you could sell us ";
    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (count > 1)
        body << "at least " << count << " of them ";
    else
        body << "some ";
    body << "we'd really appreciate that and pay a high price.\n\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: " << proto->Name1;
    if (count > 1)
        subject << " (x" << count << ")";

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendKillAdvertisement(CharacterDatabaseTransaction& trans, uint32 creatureId, uint32 owner,
                                         uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId);
    if (!proto)
        return false;

    QueryResult result =
        WorldDatabase.Query("SELECT map, position_x, position_y, position_z FROM creature WHERE id1 = {}", creatureId);
    if (!result)
        return false;

    std::string location;
    do
    {
        Field* fields = result->Fetch();
        uint32 mapid = fields[0].Get<uint32>();
        float x = fields[1].Get<float>();
        float y = fields[2].Get<float>();
        float z = fields[3].Get<float>();
        Map* map = sMapMgr->FindMap(mapid, 0);
        if (!map)
            continue;

        AreaTableEntry const* entry = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
        if (!entry)
            continue;

        location = entry->area_name[0];
        break;
    } while (result->NextRow());

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "As you probably know " << proto->Name
         << " is wanted dead for the crimes it did against our guild. If you should kill it ";
    body << "we'd really appreciate that.\n\n";
    if (!location.empty())
        body << proto->Name << "'s the last known location was " << location << ".\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: ";
    if (proto->rank == CREATURE_ELITE_ELITE || proto->rank == CREATURE_ELITE_RAREELITE ||
        proto->rank == CREATURE_ELITE_WORLDBOSS)
        subject << "(Elite) ";
    subject << proto->Name;
    if (!location.empty())
        subject << ", " << location;

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendThanks(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId, uint32 payment)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        std::ostringstream body;
        body << GetHelloText(owner);
        body << "One of our guild members wishes to thank you for the " << proto->Name1 << "!";
        uint32 count = GetTaskValue(owner, guildId, "itemCount");
        if (count)
        {
            body << " If we have another ";
            body << count << " of them that would help us tremendously.\n";
        }
        body << "\n";
        body << "Thanks again,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";

        MailDraft("Thank You", body.str()).AddMoney(payment).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

        Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
        if (player)
            SendCompletionMessage(player, "payed for");

        return true;
    }

    return false;
}

uint32 GuildTaskMgr::GetMaxItemTaskCount(uint32 itemId)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return 0;

    if (!proto->Stackable || proto->GetMaxStackSize() == 1)
        return 1;

    if (proto->Quality == ITEM_QUALITY_NORMAL)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return urand(1, 3) * proto->GetMaxStackSize();
            case 10:
                return urand(2, 6) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(4, 12) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    if (proto->Quality < ITEM_QUALITY_RARE)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return proto->GetMaxStackSize();
            case 10:
                return urand(1, 2) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(1, 4) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    return 1;
}

bool GuildTaskMgr::IsGuildTaskItem(uint32 itemId, uint32 guildId)
{
    uint32 value = 0;

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_VALUE);
    stmt->SetData(0, itemId);
    stmt->SetData(1, guildId);
    stmt->SetData(2, "itemTask");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        value = fields[0].Get<uint32>();
        uint32 lastChangeTime = fields[1].Get<uint32>();
        uint32 validIn = fields[2].Get<uint32>();
        if ((time(nullptr) - lastChangeTime) >= validIn)
            value = 0;
    }

    return value;
}

std::map<uint32, uint32> GuildTaskMgr::GetTaskValues(uint32 owner, std::string const type,
                                                     uint32* validIn /* = nullptr */)
{
    std::map<uint32, uint32> results;

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER);
    stmt->SetData(0, owner);
    stmt->SetData(1, type);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 value = fields[0].Get<uint32>();
            uint32 lastChangeTime = fields[1].Get<uint32>();
            uint32 secs = fields[2].Get<uint32>();
            uint32 guildId = fields[3].Get<uint32>();
            if ((time(nullptr) - lastChangeTime) >= secs)
                value = 0;

            results[guildId] = value;

        } while (result->NextRow());
    }

    return std::move(results);
}

uint32 GuildTaskMgr::GetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32* validIn /* = nullptr */)
{
    uint32 value = 0;

    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_AND_TYPE);
    stmt->SetData(0, owner);
    stmt->SetData(1, guildId);
    stmt->SetData(2, type);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        value = fields[0].Get<uint32>();
        uint32 lastChangeTime = fields[1].Get<uint32>();
        uint32 secs = fields[2].Get<uint32>();
        if ((time(nullptr) - lastChangeTime) >= secs)
            value = 0;

        if (validIn)
            *validIn = secs;
    }

    return value;
}

uint32 GuildTaskMgr::SetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32 value, uint32 validIn)
{
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_GUILD_TASKS);
    stmt->SetData(0, owner);
    stmt->SetData(1, guildId);
    stmt->SetData(2, type);
    PlayerbotsDatabase.Execute(stmt);

    if (value)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_GUILD_TASKS);
        stmt->SetData(0, owner);
        stmt->SetData(1, guildId);
        stmt->SetData(2, (uint32)time(nullptr));
        stmt->SetData(3, validIn);
        stmt->SetData(4, type);
        stmt->SetData(5, value);
        PlayerbotsDatabase.Execute(stmt);
    }

    return value;
}

bool GuildTaskMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        LOG_ERROR("playerbots", "Guild task system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        LOG_ERROR("playerbots", "Usage: gtask stats/reset");
        return false;
    }

    std::string const cmd = args;

    if (cmd == "reset")
    {
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_guild_tasks");
        LOG_INFO("playerbots", "Guild tasks were reset for all players");
        return true;
    }

    if (cmd == "stats")
    {
        LOG_INFO("playerbots", "Usage: gtask stats <player name>");
        return true;
    }

    if (cmd.find("stats ") != std::string::npos)
    {
        std::string const charName = cmd.substr(cmd.find("stats ") + 6);
        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(charName);
        if (!guid)
        {
            LOG_ERROR("playerbots", "Player {} not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();

        PlayerbotsDatabasePreparedStatement* stmt =
            PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_ORDERED);
        stmt->SetData(0, owner);
        stmt->SetData(1, "activeTask");
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 value = fields[0].Get<uint32>();
                uint32 lastChangeTime = fields[1].Get<uint32>();
                uint32 validIn = fields[2].Get<uint32>();
                if ((time(nullptr) - lastChangeTime) >= validIn)
                    value = 0;

                uint32 guildId = fields[3].Get<uint32>();

                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                std::ostringstream name;
                if (value == GUILD_TASK_TYPE_ITEM)
                {
                    name << "ItemTask";
                    uint32 itemId = sGuildTaskMgr->GetTaskValue(owner, guildId, "itemTask");
                    uint32 itemCount = sGuildTaskMgr->GetTaskValue(owner, guildId, "itemCount");

                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
                    {
                        name << " (" << proto->Name1 << " x" << itemCount << ",";

                        switch (proto->Quality)
                        {
                            case ITEM_QUALITY_UNCOMMON:
                                name << "green";
                                break;
                            case ITEM_QUALITY_NORMAL:
                                name << "white";
                                break;
                            case ITEM_QUALITY_RARE:
                                name << "blue";
                                break;
                            case ITEM_QUALITY_EPIC:
                                name << "epic";
                                break;
                            case ITEM_QUALITY_LEGENDARY:
                                name << "yellow";
                                break;
                        }

                        name << ")";
                    }
                }
                else if (value == GUILD_TASK_TYPE_KILL)
                {
                    name << "KillTask";
                    uint32 creatureId = sGuildTaskMgr->GetTaskValue(owner, guildId, "killTask");

                    if (CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId))
                    {
                        name << " (" << proto->Name << ",";

                        switch (proto->rank)
                        {
                            case CREATURE_ELITE_RARE:
                                name << "rare";
                                break;
                            case CREATURE_ELITE_RAREELITE:
                                name << "rare elite";
                                break;
                        }

                        name << ")";
                    }
                }
                else
                    continue;

                uint32 advertValidIn = 0;
                uint32 advert = sGuildTaskMgr->GetTaskValue(owner, guildId, "advertisement", &advertValidIn);
                if (advert && advertValidIn < validIn)
                    name << " advert in " << formatTime(advertValidIn);

                uint32 thanksValidIn = 0;
                uint32 thanks = sGuildTaskMgr->GetTaskValue(owner, guildId, "thanks", &thanksValidIn);
                if (thanks && thanksValidIn < validIn)
                    name << " thanks in " << formatTime(thanksValidIn);

                uint32 rewardValidIn = 0;
                uint32 reward = sGuildTaskMgr->GetTaskValue(owner, guildId, "reward", &rewardValidIn);
                if (reward && rewardValidIn < validIn)
                    name << " reward in " << formatTime(rewardValidIn);

                uint32 paymentValidIn = 0;
                uint32 payment = sGuildTaskMgr->GetTaskValue(owner, guildId, "payment", &paymentValidIn);
                if (payment && paymentValidIn < validIn)
                    name << " payment " << ChatHelper::formatMoney(payment) << " in " << formatTime(paymentValidIn);

                LOG_INFO("playerbots", "{}: {} valid in {} [{}]", charName.c_str(), name.str().c_str(),
                         formatTime(validIn).c_str(), guild->GetName().c_str());

            } while (result->NextRow());
        }

        return true;
    }

    if (cmd == "cleanup")
    {
        sGuildTaskMgr->CleanupAdverts();
        sGuildTaskMgr->RemoveDuplicatedAdverts();
        return true;
    }

    if (cmd == "reward")
    {
        LOG_INFO("playerbots", "Usage: gtask reward <player name>");
        return true;
    }

    if (cmd == "advert")
    {
        LOG_INFO("playerbots", "Usage: gtask advert <player name>");
        return true;
    }

    bool reward = cmd.find("reward ") != std::string::npos;
    bool advert = cmd.find("advert ") != std::string::npos;
    if (reward || advert)
    {
        std::string charName;
        if (reward)
            charName = cmd.substr(cmd.find("reward ") + 7);

        if (advert)
            charName = cmd.substr(cmd.find("advert ") + 7);

        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(charName);
        if (!guid)
        {
            LOG_ERROR("playerbots", "Player {} not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();

        PlayerbotsDatabasePreparedStatement* stmt =
            PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_DISTINCT);
        stmt->SetData(0, owner);
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].Get<uint32>();
                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                if (reward)
                    sGuildTaskMgr->Reward(trans, owner, guildId);

                if (advert)
                    sGuildTaskMgr->SendAdvertisement(trans, owner, guildId);
            } while (result->NextRow());

            CharacterDatabase.CommitTransaction(trans);
            return true;
        }
    }

    return false;
}

bool GuildTaskMgr::CheckItemTask(uint32 itemId, uint32 obtained, Player* ownerPlayer, Player* bot, bool byMail)
{
    if (!bot)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    uint32 owner = ownerPlayer->GetGUID().GetCounter();
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    LOG_DEBUG("playerbots", "{} / {}: checking guild task", guild->GetName().c_str(), ownerPlayer->GetName().c_str());

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask != itemId)
    {
        LOG_DEBUG("playerbots", "{} / {}: item {} is not guild task item ({})", guild->GetName().c_str(),
                  ownerPlayer->GetName().c_str(), itemId, itemTask);

        if (byMail)
            SendCompletionMessage(ownerPlayer, "made a mistake with");

        return false;
    }

    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (!count)
    {
        return false;
    }

    uint32 rewardTime = urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime);
    if (byMail)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            return false;

        uint32 money = GetTaskValue(owner, guildId, "payment");
        SetTaskValue(owner, guildId, "payment", money + proto->BuyPrice * obtained, rewardTime + 300);
    }

    if (obtained >= count)
    {
        LOG_DEBUG("playerbots", "{} / {}: guild task complete", guild->GetName().c_str(),
                  ownerPlayer->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1, rewardTime - 15);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 0, 0);
        SendCompletionMessage(ownerPlayer, "completed");
    }
    else
    {
        LOG_DEBUG("playerbots", "{} / {}: guild task progress {}/{}", guild->GetName().c_str(),
                  ownerPlayer->GetName().c_str(), obtained, count);
        SetTaskValue(owner, guildId, "itemCount", count - obtained, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "thanks", 1, rewardTime - 30);
        SendCompletionMessage(ownerPlayer, "made a progress with");
    }
    return true;
}

bool GuildTaskMgr::Reward(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (!player)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (!itemTask && !killTask)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);

    RandomItemType rewardType;
    uint32 itemId = 0;
    uint32 itemCount = 1;
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name1
             << " you provided so kindly. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->Quality > ITEM_QUALITY_NORMAL ? RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE
                                                          : RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN;
        itemId = sRandomItemMgr->GetRandomItem(player->GetLevel() - 5, rewardType);
    }
    else if (killTask)
    {
        CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(killTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name
             << " you've killed recently. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->rank == CREATURE_ELITE_RARE ? RANDOM_ITEM_GUILD_TASK_REWARD_TRADE
                                                        : RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE;
        itemId = sRandomItemMgr->GetRandomItem(player->GetLevel(), rewardType);
        if (itemId)
        {
            ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(itemId);
            if (proto)
            {
                if (itemProto->Quality == ITEM_QUALITY_NORMAL)
                    itemCount = itemProto->GetMaxStackSize();

                if (proto->rank != CREATURE_ELITE_RARE && itemProto->Quality > ITEM_QUALITY_NORMAL)
                    itemCount = urand(1, itemProto->GetMaxStackSize());
            }
        }
    }

    uint32 payment = GetTaskValue(owner, guildId, "payment");
    if (payment)
        SendThanks(trans, owner, guildId, payment);

    MailDraft draft("Thank You", body.str());

    if (itemId)
    {
        Item* item = Item::CreateItem(itemId, itemCount);
        if (item)
        {
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
    }

    draft.SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (player)
        SendCompletionMessage(player, "rewarded for");

    SetTaskValue(owner, guildId, "activeTask", 0, 0);
    SetTaskValue(owner, guildId, "payment", 0, 0);

    return true;
}

void GuildTaskMgr::CheckKillTask(Player* player, Unit* victim)
{
    if (!player)
        return;

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            CheckKillTaskInternal(gr->GetSource(), victim);
        }
    }
    else
    {
        CheckKillTaskInternal(player, victim);
    }
}

void GuildTaskMgr::SendCompletionMessage(Player* player, std::string const verb)
{
    std::ostringstream out;
    out << player->GetName() << " has " << verb << " a guild task";

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            Player* member = gr->GetSource();
            if (member != player)
                ChatHandler(member->GetSession()).PSendSysMessage(out.str().c_str());
        }
    }
    else
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            if (Player* master = botAI->GetMaster())
                ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }

    std::ostringstream self;
    self << "You have " << verb << " a guild task";
    ChatHandler(player->GetSession()).PSendSysMessage(self.str().c_str());
}

void GuildTaskMgr::CheckKillTaskInternal(Player* player, Unit* victim)
{
    ObjectGuid::LowType owner = player->GetGUID().GetCounter();
    if (victim->GetTypeId() != TYPEID_UNIT)
        return;

    Creature* creature = reinterpret_cast<Creature*>(victim);
    if (!creature)
        return;

    std::map<uint32, uint32> tasks = GetTaskValues(owner, "killTask");
    for (std::map<uint32, uint32>::iterator i = tasks.begin(); i != tasks.end(); ++i)
    {
        uint32 guildId = i->first;
        uint32 value = i->second;
        Guild* guild = sGuildMgr->GetGuildById(guildId);

        if (value != creature->GetEntry())
            continue;

        LOG_DEBUG("playerbots", "{} / {}: guild task complete", guild->GetName().c_str(), player->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1,
                     urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime));

        SendCompletionMessage(player, "completed");
    }
}

void GuildTaskMgr::CleanupAdverts()
{
    uint32 deliverTime = time(nullptr) - sPlayerbotAIConfig->minGuildTaskChangeTime;
    QueryResult result = CharacterDatabase.Query(
        "SELECT id, receiver FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= {}", deliverTime);
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].Get<uint32>();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].Get<uint32>())))
            player->RemoveMail(id);

        ++count;
    } while (result->NextRow());

    if (count > 0)
    {
        CharacterDatabase.Execute("DELETE FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= {}",
                                  deliverTime);
        LOG_INFO("playerbots", "{} old gtask adverts removed", count);
    }
}

void GuildTaskMgr::RemoveDuplicatedAdverts()
{
    uint32 deliverTime = time(nullptr);
    QueryResult result = CharacterDatabase.Query(
        "SELECT m.id, m.receiver FROM (SELECT MAX(id) AS id, subject, receiver FROM mail WHERE subject LIKE 'Guild "
        "Task%%' "
        "AND deliver_time <= {} GROUP BY subject, receiver) q JOIN mail m ON m.subject = q.subject WHERE m.id <> q.id "
        "AND m.deliver_time <= {}",
        deliverTime, deliverTime);

    if (!result)
        return;

    std::vector<uint32> ids;
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].Get<uint32>();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].Get<uint32>())))
            player->RemoveMail(id);

        ++count;
        ids.push_back(id);
    } while (result->NextRow());

    if (count > 0)
    {
        std::vector<uint32> buffer;
        for (std::vector<uint32>::iterator i = ids.begin(); i != ids.end(); ++i)
        {
            buffer.push_back(*i);
            if (buffer.size() > 50)
            {
                DeleteMail(buffer);
                buffer.clear();
            }
        }

        DeleteMail(buffer);
        LOG_INFO("playerbots", "{} duplicated gtask adverts removed", count);
    }
}

void GuildTaskMgr::DeleteMail(std::vector<uint32> buffer)
{
    std::ostringstream sql;
    sql << "DELETE FROM mail WHERE id IN (";

    bool first = true;
    for (std::vector<uint32>::iterator j = buffer.begin(); j != buffer.end(); ++j)
    {
        if (first)
            first = false;
        else
            sql << ",";

        sql << *j;
    }

    sql << ")";

    CharacterDatabase.Execute(sql.str().c_str());
}

bool GuildTaskMgr::CheckTaskTransfer(std::string const text, Player* ownerPlayer, Player* bot)
{
    if (!bot)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    ObjectGuid::LowType owner = ownerPlayer->GetGUID().GetCounter();
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (text.empty())
        return false;

    LOG_DEBUG("playerbots", "{} / {}: checking guild task transfer", guild->GetName().c_str(),
              ownerPlayer->GetName().c_str());

    uint32 account = ownerPlayer->GetSession()->GetAccountId();

    if (QueryResult results = CharacterDatabase.Query("SELECT guid, name FROM characters WHERE account = {}", account))
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 newOwner = fields[0].Get<uint32>();
            std::string const name = fields[1].Get<std::string>();
            if (newOwner == bot->GetGUID().GetCounter())
                continue;

            std::wstring wnamepart;
            if (!Utf8toWStr(name, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (Utf8FitTo(text, wnamepart))
            {
                uint32 validIn;
                uint32 activeTask = GetTaskValue(owner, guildId, "activeTask", &validIn);
                uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
                uint32 killTask = GetTaskValue(owner, guildId, "killTask");

                if (itemTask || killTask)
                {
                    SetTaskValue(newOwner, guildId, "activeTask", activeTask, validIn);
                    SetTaskValue(newOwner, guildId, "advertisement", 1, 15);

                    if (itemTask)
                        SetTaskValue(newOwner, guildId, "itemTask", itemTask, validIn);

                    if (killTask)
                        SetTaskValue(newOwner, guildId, "killTask", killTask, validIn);

                    SetTaskValue(owner, guildId, "activeTask", 0, 0);
                    SetTaskValue(owner, guildId, "payment", 0, 0);

                    SendCompletionMessage(ownerPlayer, "transfered");
                }
            }
        } while (results->NextRow());
    }

    return true;
}
