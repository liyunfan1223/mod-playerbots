/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTTEXTMGR_H
#define PLAYERBOTS_PLAYERBOTTEXTMGR_H

#include "Common.h"
#include <map>
#include <vector>

struct BotTextEntry
{
    BotTextEntry(std::string name, std::map<uint32, std::string> text, uint32 say_type, uint32 reply_type)
        : m_name(name), m_text(text), m_sayType(say_type), m_replyType(reply_type)
    {
    }
    std::string m_name;
    std::map<uint32, std::string> m_text;
    uint32 m_sayType;
    uint32 m_replyType;
};

struct ChatReplyData
{
    ChatReplyData(uint32 guid, uint32 type, std::string chat) : m_type(type), m_guid(guid), m_chat(chat) {}
    uint32 m_type, m_guid = 0;
    std::string m_chat = "";
};

struct ChatQueuedReply
{
    ChatQueuedReply(uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name,
                    time_t time)
        : m_type(type), m_guid1(guid1), m_guid2(guid2), m_msg(msg), m_chanName(chanName), m_name(name), m_time(time)
    {
    }
    uint32 m_type;
    uint32 m_guid1;
    uint32 m_guid2;
    std::string m_msg;
    std::string m_chanName;
    std::string m_name;
    time_t m_time;
};

enum ChatReplyType
{
    REPLY_NOT_UNDERSTAND,
    REPLY_GRUDGE,
    REPLY_VICTIM,
    REPLY_ATTACKER,
    REPLY_HELLO,
    REPLY_NAME,
    REPLY_ADMIN_ABUSE
};

class PlayerbotTextMgr
{
public:
    static PlayerbotTextMgr& instance()
    {
        static PlayerbotTextMgr instance;

        return instance;
    }

    std::string GetBotText(std::string name, std::map<std::string, std::string> placeholders);
    std::string GetBotText(std::string name);
    std::string GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders);
    std::string GetBotText(ChatReplyType replyType, std::string name);
    bool GetBotText(std::string name, std::string& text);
    bool GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders);
    std::string GetBotTextOrDefault(std::string name, std::string defaultText,
                                    std::map<std::string, std::string> placeholders);
    void LoadBotTexts();
    void LoadBotTextChance();
    static void replaceAll(std::string& str, std::string const& from, std::string const& to);
    bool rollTextChance(std::string text);

    uint32 GetLocalePriority();
    void AddLocalePriority(uint32 locale);
    void ResetLocalePriority();

private:
    PlayerbotTextMgr()
    {
        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
        {
            botTextLocalePriority[i] = 0;
        }
    };
    ~PlayerbotTextMgr() = default;

    PlayerbotTextMgr(PlayerbotTextMgr const&) = delete;
    PlayerbotTextMgr& operator=(PlayerbotTextMgr const&) = delete;

    PlayerbotTextMgr(PlayerbotTextMgr&&) = delete;
    PlayerbotTextMgr& operator=(PlayerbotTextMgr&&) = delete;

    std::map<std::string, std::vector<BotTextEntry>> botTexts;
    std::map<std::string, uint32> botTextChance;
    uint32 botTextLocalePriority[TOTAL_LOCALES];
};

#endif
