/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GUILDTASKMGR_H
#define _PLAYERBOT_GUILDTASKMGR_H

#include <map>

#include "Common.h"
#include "Transaction.h"

class ChatHandler;
class Player;
class Unit;

class GuildTaskMgr
{
public:
    GuildTaskMgr(){};
    virtual ~GuildTaskMgr(){};

    static GuildTaskMgr* instance()
    {
        static GuildTaskMgr instance;
        return &instance;
    }

    void Update(Player* owner, Player* guildMaster);

    static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
    bool IsGuildTaskItem(uint32 itemId, uint32 guildId);
    bool CheckItemTask(uint32 itemId, uint32 obtained, Player* owner, Player* bot, bool byMail = false);
    void CheckKillTask(Player* owner, Unit* victim);
    void CheckKillTaskInternal(Player* owner, Unit* victim);
    bool CheckTaskTransfer(std::string const text, Player* owner, Player* bot);

private:
    std::map<uint32, uint32> GetTaskValues(uint32 owner, std::string const type, uint32* validIn = nullptr);
    uint32 GetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32* validIn = nullptr);
    uint32 SetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32 value, uint32 validIn);
    uint32 CreateTask(Player* owner, uint32 guildId);
    bool SendAdvertisement(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId);
    bool SendItemAdvertisement(CharacterDatabaseTransaction& trans, uint32 itemId, uint32 owner, uint32 guildId,
                               uint32 validIn);
    bool SendKillAdvertisement(CharacterDatabaseTransaction& trans, uint32 creatureId, uint32 owner, uint32 guildId,
                               uint32 validIn);
    bool SendThanks(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId, uint32 payment);
    bool Reward(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId);
    bool CreateItemTask(Player* owner, uint32 guildId);
    bool CreateKillTask(Player* owner, uint32 guildId);
    uint32 GetMaxItemTaskCount(uint32 itemId);
    void CleanupAdverts();
    void RemoveDuplicatedAdverts();
    void DeleteMail(std::vector<uint32> buffer);
    void SendCompletionMessage(Player* player, std::string const verb);
};

#define sGuildTaskMgr GuildTaskMgr::instance()

#endif
