/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GUILDTASKMGR_H
#define PLAYERBOTS_GUILDTASKMGR_H

#include "Chat.h"
#include "DatabaseEnvFwd.h"
#include "Player.h"
#include "Unit.h"
#include <cstdint>
#include <map>
#include <vector>

class GuildTaskMgr
{
public:
    static GuildTaskMgr& instance()
    {
        static GuildTaskMgr instance;

        return instance;
    }

    void Update(Player* owner, Player* guildMaster);

    static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
    bool IsGuildTaskItem(uint32_t itemId, uint32_t guildId);
    bool CheckItemTask(uint32_t itemId, uint32_t obtained, Player* owner, Player* bot, bool byMail = false);
    void CheckKillTask(Player* owner, Unit* victim);
    void CheckKillTaskInternal(Player* owner, Unit* victim);
    bool CheckTaskTransfer(std::string const text, Player* owner, Player* bot);

private:
    GuildTaskMgr() = default;
    ~GuildTaskMgr() = default;

    GuildTaskMgr(GuildTaskMgr const&) = delete;
    GuildTaskMgr& operator=(GuildTaskMgr const&) = delete;

    GuildTaskMgr(GuildTaskMgr&&) = delete;
    GuildTaskMgr& operator=(GuildTaskMgr&&) = delete;

    std::map<uint32_t, uint32_t> GetTaskValues(uint32_t owner, std::string const type, uint32_t* validIn = nullptr);
    uint32_t GetTaskValue(uint32_t owner, uint32_t guildId, std::string const type, uint32_t* validIn = nullptr);
    uint32_t SetTaskValue(uint32_t owner, uint32_t guildId, std::string const type, uint32_t value, uint32_t validIn);
    uint32_t CreateTask(Player* owner, uint32_t guildId);
    bool SendAdvertisement(CharacterDatabaseTransaction& trans, uint32_t owner, uint32_t guildId);
    bool SendItemAdvertisement(CharacterDatabaseTransaction& trans, uint32_t itemId, uint32_t owner, uint32_t guildId,
                               uint32_t validIn);
    bool SendKillAdvertisement(CharacterDatabaseTransaction& trans, uint32_t creatureId, uint32_t owner, uint32_t guildId,
                               uint32_t validIn);
    bool SendThanks(CharacterDatabaseTransaction& trans, uint32_t owner, uint32_t guildId, uint32_t payment);
    bool Reward(CharacterDatabaseTransaction& trans, uint32_t owner, uint32_t guildId);
    bool CreateItemTask(Player* owner, uint32_t guildId);
    bool CreateKillTask(Player* owner, uint32_t guildId);
    uint32_t GetMaxItemTaskCount(uint32_t itemId);
    void CleanupAdverts();
    void RemoveDuplicatedAdverts();
    void DeleteMail(std::vector<uint32_t> buffer);
    void SendCompletionMessage(Player* player, std::string const verb);
};

#endif
