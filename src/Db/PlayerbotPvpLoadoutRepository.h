/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTPVPLOADOUTREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTPVPLOADOUTREPOSITORY_H

#include "AsyncCallbackProcessor.h"
#include "Define.h"
#include "PlayerbotsDatabase.h"
#include "PvpLoadoutRules.h"
#include "Transaction.h"
#include <mutex>
#include <optional>
#include <unordered_map>

// playerbots_pvp_loadout and its item copies (playerbots_pvp_loadout_item), read once at startup and written through:
// Save and Delete update memory and queue an async write, so map threads never wait on the database.
class PlayerbotPvpLoadoutRepository
{
public:
    static PlayerbotPvpLoadoutRepository& instance()
    {
        static PlayerbotPvpLoadoutRepository instance;

        return instance;
    }

    // World thread, startup only; later calls are ignored so a config reload cannot resurrect restored rows.
    void LoadAll();

    std::optional<PvpLoadout::Snapshot> Find(uint32 guid) const;
    // Queues the row and its item copies as one transaction; IsSaved is false until that commits.
    void Save(uint32 guid, PvpLoadout::Snapshot const& snapshot);
    // Whether the bot's latest Save is in the database. Nothing only the snapshot can restore, like a PvE item kept as
    // a copy, may be destroyed before.
    bool IsSaved(uint32 guid) const;
    // Forgets the bot's snapshot now and deletes its rows once characterSave, holding the restored PvE items, has
    // committed: until then the copies are the only record of those items. A Save in between keeps its rows.
    void Forget(uint32 guid, TransactionCallback characterSave);

    // World thread: runs the completion callbacks of committed saves.
    void ProcessCallbacks();

private:
    PlayerbotPvpLoadoutRepository() = default;
    ~PlayerbotPvpLoadoutRepository() = default;

    PlayerbotPvpLoadoutRepository(PlayerbotPvpLoadoutRepository const&) = delete;
    PlayerbotPvpLoadoutRepository& operator=(PlayerbotPvpLoadoutRepository const&) = delete;

    PlayerbotPvpLoadoutRepository(PlayerbotPvpLoadoutRepository&&) = delete;
    PlayerbotPvpLoadoutRepository& operator=(PlayerbotPvpLoadoutRepository&&) = delete;

    void LoadItemCopies();
    void OnSaved(uint32 guid, uint64 save, bool success);
    static void AppendDeleteItemCopies(PlayerbotsDatabaseTransaction& trans, uint32 guid);

    mutable std::mutex _mutex;
    std::unordered_map<uint32, PvpLoadout::Snapshot> _snapshots;
    std::unordered_map<uint32, uint64> _unsaved;     // bot guid -> its latest save still in flight
    std::unordered_map<uint32, uint64> _latestSave;  // bot guid -> its latest save, until its rows are deleted
    uint64 _lastSave = 0;
    bool _loaded = false;

    std::mutex _callbackMutex;
    AsyncCallbackProcessor<TransactionCallback> _callbacks;
};

#endif
