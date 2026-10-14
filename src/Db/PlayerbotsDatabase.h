/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOTS_DATABASE_H
#define _PLAYERBOTS_DATABASE_H

#include "DatabaseEnvFwd.h"
#include "ModuleDatabasePool.h"
#include "MySQLConnection.h"
#include "PreparedStatement.h"
#include "Transaction.h"
#include <memory>

enum PlayerbotsDatabaseStatements : uint32
{
    /*  Naming standard for defines:
        {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER,
    PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER_AND_NAME,
    PLAYERBOTS_SEL_CUSTOM_STRATEGY_BY_OWNER_AND_NAME_AND_IDX,
    PLAYERBOTS_DEL_CUSTOM_STRATEGY,
    PLAYERBOTS_UPD_CUSTOM_STRATEGY,
    PLAYERBOTS_INS_CUSTOM_STRATEGY,

    PLAYERBOTS_SEL_DB_STORE,
    PLAYERBOTS_DEL_DB_STORE,
    PLAYERBOTS_INS_DB_STORE,

    PLAYERBOTS_SEL_ENCHANTS,

    PLAYERBOTS_SEL_EQUIP_CACHE,
    PLAYERBOTS_INS_EQUIP_CACHE,

    PLAYERBOTS_SEL_GUILD_TASKS_BY_VALUE,
    PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER,
    PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_AND_TYPE,
    PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_DISTINCT,
    PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_ORDERED,
    PLAYERBOTS_DEL_GUILD_TASKS,
    PLAYERBOTS_INS_GUILD_TASKS,

    PLAYERBOTS_SEL_RANDOM_BOTS_VALUE,
    PLAYERBOTS_SEL_RANDOM_BOTS_BOT,
    PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_EVENT,
    PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_BOT,
    PLAYERBOTS_SEL_RANDOM_BOTS_BY_EVENT_AND_VALUE,
    PLAYERBOTS_INS_RANDOM_BOTS,
    PLAYERBOTS_DEL_RANDOM_BOTS,
    PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER,
    PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER_AND_EVENT,
    PLAYERBOTS_UPD_RANDOM_BOTS,

    PLAYERBOTS_SEL_RARITY_CACHE,
    PLAYERBOTS_INS_RARITY_CACHE,

    PLAYERBOTS_SEL_RNDITEM_CACHE,
    PLAYERBOTS_INS_RNDITEM_CACHE,

    PLAYERBOTS_SEL_TELE_CACHE,
    PLAYERBOTS_INS_TELE_CACHE,

    PLAYERBOTS_SEL_TEXT,
    PLAYERBOTS_SEL_DUNGEON_SUGGESTION,

    PLAYERBOTS_SEL_TRAVELNODE,
    PLAYERBOTS_INS_TRAVELNODE,
    PLAYERBOTS_DEL_TRAVELNODE,

    PLAYERBOTS_SEL_TRAVELNODE_LINK,
    PLAYERBOTS_INS_TRAVELNODE_LINK,
    PLAYERBOTS_DEL_TRAVELNODE_LINK,

    PLAYERBOTS_SEL_TRAVELNODE_PATH,
    PLAYERBOTS_INS_TRAVELNODE_PATH,
    PLAYERBOTS_DEL_TRAVELNODE_PATH,

    PLAYERBOTS_SEL_WEIGHTSCALES,
    PLAYERBOTS_SEL_WEIGHTSCALE_DATA,

    PLAYERBOTS_INS_EQUIP_CACHE_NEW,
    PLAYERBOTS_DEL_EQUIP_CACHE_NEW,

    PLAYERBOTS_SEL_ACCOUNT_KEY,
    PLAYERBOTS_REP_ACCOUNT_KEY,

    PLAYERBOTS_SEL_ACCOUNT_LINK,
    PLAYERBOTS_SEL_ACCOUNT_LINKS_BY_ACCOUNT,
    PLAYERBOTS_INS_ACCOUNT_LINK,
    PLAYERBOTS_DEL_ACCOUNT_LINK,

    PLAYERBOTS_SEL_ACCOUNT_TYPE,
    PLAYERBOTS_SEL_ACCOUNT_TYPE_BY_ACCOUNT_AND_TYPE,
    PLAYERBOTS_SEL_ACCOUNT_TYPE_COUNT_BY_TYPE,
    PLAYERBOTS_SEL_ACCOUNT_TYPE_COUNT_BY_TYPES,
    PLAYERBOTS_INS_ACCOUNT_TYPE,
    PLAYERBOTS_UPD_ACCOUNT_TYPE,
    PLAYERBOTS_UPD_ACCOUNT_TYPE_UNASSIGN,
    PLAYERBOTS_DEL_ACCOUNT_TYPE,

    PLAYERBOTS_SEL_BIS_GEAR,

    PLAYERBOTS_DEL_GUILD_TASKS_ALL,

    PLAYERBOTS_SEL_PREFERRED_MOUNTS,

    PLAYERBOTS_DEL_RANDOM_BOTS_BY_EVENT,

    PLAYERBOTS_SEL_TEXT_CHANCE,

    MAX_PLAYERBOTS_STATEMENTS
};

class PlayerbotsDatabaseConnection : public MySQLConnection
{
public:
    typedef PlayerbotsDatabaseStatements Statements;

    PlayerbotsDatabaseConnection(MySQLConnectionInfo& connInfo);
    PlayerbotsDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* queue, MySQLConnectionInfo& connInfo);
    ~PlayerbotsDatabaseConnection() override;

    //! Loads database type specific prepared statements
    void DoPrepareStatements() override;
};

using PlayerbotsDatabasePreparedStatement = PreparedStatement<PlayerbotsDatabaseConnection>;
using PlayerbotsDatabaseTransaction = std::shared_ptr<Transaction<PlayerbotsDatabaseConnection>>;

// Module-owned pool (see core ModuleDatabasePool): typed sugar so call sites read
// exactly like the core DatabaseWorkerPool API.
class PlayerbotsDatabasePool : public ModuleDatabasePool
{
public:
    PlayerbotsDatabasePreparedStatement* GetPreparedStatement(PlayerbotsDatabaseStatements index)
    {
        return new PlayerbotsDatabasePreparedStatement(index, GetPreparedStatementParamCount(index));
    }

    using ModuleDatabasePool::DirectExecute;
    using ModuleDatabasePool::Execute;
    using ModuleDatabasePool::Query;

    PlayerbotsDatabaseTransaction BeginTransaction()
    {
        return std::make_shared<Transaction<PlayerbotsDatabaseConnection>>();
    }

    void CommitTransaction(PlayerbotsDatabaseTransaction transaction)
    {
        ModuleDatabasePool::CommitTransaction(transaction);
    }

    void DirectCommitTransaction(PlayerbotsDatabaseTransaction transaction)
    {
        ModuleDatabasePool::DirectCommitTransaction(transaction);
    }

protected:
    MySQLConnection* CreateConnection(MySQLConnectionInfo& connInfo) override
    {
        return new PlayerbotsDatabaseConnection(connInfo);
    }

    MySQLConnection* CreateConnection(ProducerConsumerQueue<SQLOperation*>* queue,
        MySQLConnectionInfo& connInfo) override
    {
        return new PlayerbotsDatabaseConnection(queue, connInfo);
    }
};

extern PlayerbotsDatabasePool PlayerbotsDatabase;

#endif
