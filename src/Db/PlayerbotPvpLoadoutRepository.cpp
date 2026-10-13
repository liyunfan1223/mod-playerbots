/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotPvpLoadoutRepository.h"
#include "PlayerbotsDatabase.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"

void PlayerbotPvpLoadoutRepository::LoadAll()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_loaded)
        return;

    _loaded = true;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_PVP_LOADOUT);
    PreparedQueryResult result = PlayerbotsDatabase.Query(stmt);
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 const guid = fields[0].Get<uint32>();

        PvpLoadout::Snapshot snapshot{};
        snapshot.talentLink = fields[1].Get<std::string>();
        snapshot.arenaInstanceId = fields[4].Get<uint32>();

        // A damaged row still loads, so the bot is restored as far as possible instead of staying in PvP state.
        bool const glyphsOk = PvpLoadout::ParseGlyphs(fields[2].Get<std::string>(), snapshot.glyphs);
        bool const matchItemsOk = PvpLoadout::ParseItemList(fields[3].Get<std::string>(), snapshot.matchItems);
        if (!glyphsOk || !matchItemsOk)
            LOG_ERROR("playerbots", "playerbots_pvp_loadout row for guid {} is malformed, restoring what parses", guid);

        _snapshots[guid] = std::move(snapshot);
    } while (result->NextRow());

    LoadItemCopies();
    LOG_INFO("server.loading", "Loaded {} PvP loadout snapshots", static_cast<uint32>(_snapshots.size()));
}

void PlayerbotPvpLoadoutRepository::LoadItemCopies()
{
    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_PVP_LOADOUT_ITEMS);
    PreparedQueryResult result = PlayerbotsDatabase.Query(stmt);
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 const guid = fields[0].Get<uint32>();
        auto const snapshot = _snapshots.find(guid);
        if (snapshot == _snapshots.end())
            continue;

        PvpLoadout::ItemCopy copy{};
        copy.slot = fields[1].Get<uint8>();
        copy.itemGuid = fields[2].Get<uint32>();
        copy.entry = fields[3].Get<uint32>();
        copy.creatorGuid = fields[4].Get<uint32>();
        copy.giftCreatorGuid = fields[5].Get<uint32>();
        copy.flags = fields[6].Get<uint32>();
        copy.duration = fields[7].Get<uint32>();
        copy.randomPropertyId = fields[10].Get<int16>();
        copy.durability = fields[11].Get<uint16>();
        copy.playedTime = fields[12].Get<uint32>();
        copy.text = fields[13].Get<std::string>();

        // Recreating without its charges or enchantments still gives the bot its item back.
        if (!PvpLoadout::ParseCharges(fields[8].Get<std::string>(), copy.charges) ||
            !PvpLoadout::ParseEnchantments(fields[9].Get<std::string>(), copy.enchantments))
            LOG_ERROR("playerbots",
                      "playerbots_pvp_loadout_item row for guid {} slot {} is malformed, restoring the item "
                      "without what does not parse",
                      guid, uint32(copy.slot));

        snapshot->second.pveCopies.push_back(std::move(copy));
    } while (result->NextRow());
}

std::optional<PvpLoadout::Snapshot> PlayerbotPvpLoadoutRepository::Find(uint32 guid) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto const it = _snapshots.find(guid);
    if (it == _snapshots.end())
        return std::nullopt;

    return it->second;
}

void PlayerbotPvpLoadoutRepository::Save(uint32 guid, PvpLoadout::Snapshot const& snapshot)
{
    uint64 save = 0;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _snapshots[guid] = snapshot;
        save = ++_lastSave;
        _unsaved[guid] = save;
        _latestSave[guid] = save;
    }

    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();
    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_REP_PVP_LOADOUT);
    stmt->SetData(0, guid);
    stmt->SetData(1, snapshot.talentLink);
    stmt->SetData(2, PvpLoadout::FormatGlyphs(snapshot.glyphs));
    stmt->SetData(3, PvpLoadout::FormatItemList(snapshot.matchItems));
    stmt->SetData(4, snapshot.arenaInstanceId);
    trans->Append(stmt);
    AppendDeleteItemCopies(trans, guid);

    for (PvpLoadout::ItemCopy const& copy : snapshot.pveCopies)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_PVP_LOADOUT_ITEM);
        stmt->SetData(0, guid);
        stmt->SetData(1, copy.slot);
        stmt->SetData(2, copy.itemGuid);
        stmt->SetData(3, copy.entry);
        stmt->SetData(4, copy.creatorGuid);
        stmt->SetData(5, copy.giftCreatorGuid);
        stmt->SetData(6, copy.flags);
        stmt->SetData(7, copy.duration);
        stmt->SetData(8, PvpLoadout::FormatCharges(copy.charges));
        stmt->SetData(9, PvpLoadout::FormatEnchantments(copy.enchantments));
        stmt->SetData(10, int16(copy.randomPropertyId));
        stmt->SetData(11, uint16(copy.durability));
        stmt->SetData(12, copy.playedTime);
        stmt->SetData(13, copy.text);
        trans->Append(stmt);
    }

    TransactionCallback callback = PlayerbotsDatabase.AsyncCommitTransaction(trans);
    callback.AfterComplete([this, guid, save](bool success) { OnSaved(guid, save, success); });

    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbacks.AddCallback(std::move(callback));
}

void PlayerbotPvpLoadoutRepository::OnSaved(uint32 guid, uint64 save, bool success)
{
    if (!success)
    {
        // The bot stays unsaved: it keeps its PvE items until it is restored, which needs nothing from the copies.
        LOG_ERROR("playerbots", "Saving the PvP loadout of bot guid {} failed, it keeps its PvE items", guid);
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    auto const unsaved = _unsaved.find(guid);
    if (unsaved != _unsaved.end() && unsaved->second == save)
        _unsaved.erase(unsaved);
}

bool PlayerbotPvpLoadoutRepository::IsSaved(uint32 guid) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _snapshots.count(guid) && !_unsaved.count(guid);
}

void PlayerbotPvpLoadoutRepository::ProcessCallbacks()
{
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbacks.ProcessReadyCallbacks();
}

void PlayerbotPvpLoadoutRepository::Forget(uint32 guid, TransactionCallback characterSave)
{
    uint64 forgottenSave = 0;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _snapshots.erase(guid);
        _unsaved.erase(guid);
        forgottenSave = _latestSave[guid];
    }

    characterSave.AfterComplete(
        [this, guid, forgottenSave](bool success)
        {
            if (!success)
            {
                LOG_ERROR("playerbots", "Saving bot guid {} after restoring its PvE loadout failed, keeping its copies",
                          guid);
                return;
            }

            {
                std::lock_guard<std::mutex> lock(_mutex);
                // The bot entered another arena meanwhile; its new rows replaced these.
                if (_latestSave[guid] != forgottenSave)
                    return;

                _latestSave.erase(guid);
            }

            PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();
            PlayerbotsDatabasePreparedStatement* stmt =
                PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_PVP_LOADOUT);
            stmt->SetData(0, guid);
            trans->Append(stmt);
            AppendDeleteItemCopies(trans, guid);
            PlayerbotsDatabase.CommitTransaction(trans);
        });

    std::lock_guard<std::mutex> lock(_callbackMutex);
    _callbacks.AddCallback(std::move(characterSave));
}

void PlayerbotPvpLoadoutRepository::AppendDeleteItemCopies(PlayerbotsDatabaseTransaction& trans, uint32 guid)
{
    PlayerbotsDatabasePreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_PVP_LOADOUT_ITEMS);
    stmt->SetData(0, guid);
    trans->Append(stmt);
}
