/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BattleGroundTactics.h"
#include "Chat.h"
#include "GuildTaskMgr.h"
#include "PerformanceMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsAccountCommandTable = {
            {"set", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"link", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"linkedAccounts", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"unlink", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"debug", playerbotsDebugCommandTable},
            {"account", playerbotsAccountCommandTable, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable commandTable = {
            {"playerbots", playerbotsCommandTable},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* handler, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerformanceMonitor->Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerformanceMonitor->PrintStats(true, false);
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerformanceMonitor->PrintStats(false, true);
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig->perfMonEnabled = !sPlayerbotAIConfig->perfMonEnabled;
            if (sPlayerbotAIConfig->perfMonEnabled)
                LOG_INFO("playerbots", "Performance monitor enabled");
            else
                LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerformanceMonitor->PrintStats();
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        uint32 accountId = player->GetSession()->GetAccountId();
        CharacterDatabase.PExecute("REPLACE INTO playerbot_account_keys (account_id, security_key) VALUES ({}, '{}')", accountId, args);
        handler->PSendSysMessage("Security key set successfully.");
        return true;
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");

        if (!accountName || !key)
        {
            handler->PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
            return false;
        }

        QueryResult result = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '{}'", accountName);
        if (!result)
        {
            handler->PSendSysMessage("Account not found.");
            return false;
        }

        Field* fields = result->Fetch();
        uint32 linkedAccountId = fields[0].Get<uint32>();

        result = CharacterDatabase.PQuery("SELECT security_key FROM playerbot_account_keys WHERE account_id = {}", linkedAccountId);
        if (!result || result->Fetch()->Get<std::string>() != key)
        {
            handler->PSendSysMessage("Invalid security key.");
            return false;
        }

        uint32 accountId = handler->GetSession()->GetAccountId();
        CharacterDatabase.PExecute("INSERT IGNORE INTO playerbot_account_links (account_id, linked_account_id) VALUES ({}, {})", accountId, linkedAccountId);
        CharacterDatabase.PExecute("INSERT IGNORE INTO playerbot_account_links (account_id, linked_account_id) VALUES ({}, {})", linkedAccountId, accountId);

        handler->PSendSysMessage("Account linked successfully.");
        return true;
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint32 accountId = player->GetSession()->GetAccountId();

        // Query linked account IDs
        QueryResult result = CharacterDatabase.PQuery("SELECT linked_account_id FROM playerbot_account_links WHERE account_id = {}", accountId);

        if (!result)
        {
            handler->PSendSysMessage("No linked accounts.");
            return true;
        }

        handler->PSendSysMessage("Linked accounts:");

        do
        {
            Field* fields = result->Fetch();
            uint32 linkedAccountId = fields[0].Get<uint32>();

            // Query the username for the linked account ID
            QueryResult accountResult = LoginDatabase.PQuery("SELECT username FROM account WHERE id = {}", linkedAccountId);
            if (accountResult)
            {
                Field* accountFields = accountResult->Fetch();
                std::string username = accountFields[0].Get<std::string>();
                handler->PSendSysMessage("- %s", username.c_str());
            }
            else
            {
                handler->PSendSysMessage("- Unknown account");
            }
        } while (result->NextRow());

        return true;
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        if (!accountName)
        {
            handler->PSendSysMessage("Usage: .playerbots account unlink <accountName>");
            return false;
        }

        QueryResult result = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '{}'", accountName);
        if (!result)
        {
            handler->PSendSysMessage("Account not found.");
            return false;
        }

        Field* fields = result->Fetch();
        uint32 linkedAccountId = fields[0].Get<uint32>();
        uint32 accountId = handler->GetSession()->GetAccountId();

        CharacterDatabase.PExecute("DELETE FROM playerbot_account_links WHERE (account_id = {} AND linked_account_id = {}) OR (account_id = {} AND linked_account_id = {})",
                                    accountId, linkedAccountId, linkedAccountId, accountId);

        handler->PSendSysMessage("Account unlinked successfully.");
        return true;
    }
};

void AddSC_playerbots_commandscript() { new playerbots_commandscript(); }
