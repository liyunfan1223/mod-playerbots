/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingBondMgr.h"

#include <algorithm>

#include "GameTime.h"
#include "LivingWorldMgr.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotsDatabase.h"
#include "QueryResult.h"

namespace
{
uint32 Now() { return static_cast<uint32>(GameTime::GetGameTime().count()); }
}  // namespace

LivingBondMgr& LivingBondMgr::instance()
{
    static LivingBondMgr instance;
    return instance;
}

void LivingBondMgr::Load()
{
    if (!sPlayerbotAIConfig.LivingWorldEnabled)
        return;

    std::lock_guard<std::mutex> lock(_mutex);
    _bonds.clear();
    _lastEvent.clear();
    _groupJoins.clear();
    auto* statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_LIVING_BONDS);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(statement))
    {
        do
        {
            Field* fields = result->Fetch();
            LivingWorld::Bond bond;
            bond.score = fields[2].Get<int16>();
            bond.quests = fields[3].Get<uint16>();
            bond.bosses = fields[4].Get<uint16>();
            bond.battlegrounds = fields[5].Get<uint16>();
            bond.minutes = fields[6].Get<uint16>();
            bond.helps = fields[7].Get<uint16>();
            bond.duels = fields[8].Get<uint16>();
            bond.kills = fields[9].Get<uint16>();
            bond.ninjas = fields[10].Get<uint16>();
            bond.slights = fields[11].Get<uint16>();
            bond.lastSeen = fields[12].Get<uint32>();
            _bonds.emplace(Key(fields[0].Get<uint32>(), fields[1].Get<uint32>()), bond);
        } while (result->NextRow());
    }
    LOG_INFO("playerbots", "Loaded {} living-world bot bonds", _bonds.size());
}

bool LivingBondMgr::Record(Player* bot, Player* other, LivingWorld::BondEvent event)
{
    if (!bot || !other || bot == other || event >= LivingWorld::BondEvent::Count ||
        !LivingWorldMgr::instance().IsEnabled(bot))
        return false;

    uint32 const botGuid = bot->GetGUID().GetCounter();
    uint32 const otherGuid = other->GetGUID().GetCounter();
    uint32 const now = Now();
    LivingWorld::Bond bond;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        uint32& last = _lastEvent[Key(botGuid, otherGuid)][size_t(event)];
        if (last && now - last < LivingWorld::BondCooldown(event))
            return false;
        last = now;
        LivingWorld::Bond& stored = _bonds[Key(botGuid, otherGuid)];
        LivingWorld::ApplyBond(stored, event, now);
        bond = stored;
    }

    auto* statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPSERT_LIVING_BOND);
    statement->SetData(0, botGuid);
    statement->SetData(1, otherGuid);
    statement->SetData(2, bond.score);
    statement->SetData(3, bond.quests);
    statement->SetData(4, bond.bosses);
    statement->SetData(5, bond.battlegrounds);
    statement->SetData(6, bond.minutes);
    statement->SetData(7, bond.helps);
    statement->SetData(8, bond.duels);
    statement->SetData(9, bond.kills);
    statement->SetData(10, bond.ninjas);
    statement->SetData(11, bond.slights);
    statement->SetData(12, bond.lastSeen);
    PlayerbotsDatabase.Execute(statement);
    return true;
}

bool LivingBondMgr::SecondsSince(uint32 bot, uint32 other, LivingWorld::BondEvent event, uint32& seconds)
{
    seconds = 0;
    std::lock_guard<std::mutex> lock(_mutex);
    auto const found = _lastEvent.find(Key(bot, other));
    if (found == _lastEvent.end() || !found->second[size_t(event)])
        return false;
    seconds = Now() - found->second[size_t(event)];
    return true;
}

LivingWorld::Bond LivingBondMgr::Get(uint32 bot, uint32 other)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto const found = _bonds.find(Key(bot, other));
    return found == _bonds.end() ? LivingWorld::Bond{} : found->second;
}

std::vector<std::pair<uint32, LivingWorld::Bond>> LivingBondMgr::Strongest(uint32 bot, size_t limit)
{
    std::vector<std::pair<uint32, LivingWorld::Bond>> all;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto const& [key, bond] : _bonds)
            if (uint32(key >> 32) == bot)
                all.emplace_back(uint32(key & 0xFFFFFFFF), bond);
    }
    std::sort(all.begin(), all.end(), [](auto const& a, auto const& b) { return a.second.score > b.second.score; });
    if (all.size() <= limit * 2)
        return all;
    std::vector<std::pair<uint32, LivingWorld::Bond>> picked(all.begin(), all.begin() + limit);
    picked.insert(picked.end(), all.end() - limit, all.end());
    return picked;
}

LivingWorld::Attitude LivingBondMgr::AttitudeOf(uint32 bot, uint32 other)
{
    return LivingWorld::AttitudeFor(Get(bot, other).score);
}

std::vector<std::pair<uint32, LivingWorld::Bond>> LivingBondMgr::KnownBy(uint32 other)
{
    std::vector<std::pair<uint32, LivingWorld::Bond>> known;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto const& [key, bond] : _bonds)
            if (uint32(key & 0xFFFFFFFF) == other)
                known.emplace_back(uint32(key >> 32), bond);
    }
    std::sort(known.begin(), known.end(), [](auto const& a, auto const& b) { return a.second.score > b.second.score; });
    return known;
}

bool LivingBondMgr::TryGreet(uint32 bot, uint32 other, bool whisper)
{
    uint32 const now = Now();
    uint32 const cooldown = whisper ? LivingWorld::WhisperCooldown : LivingWorld::GreetCooldown;
    std::lock_guard<std::mutex> lock(_mutex);
    uint32& last = _lastGreet[Key(bot, other)][whisper ? 1 : 0];
    if (last && now - last < cooldown)
        return false;
    last = now;
    return true;
}

bool LivingBondMgr::TryGuildInvite(uint32 bot, uint32 other)
{
    uint32 const now = Now();
    std::lock_guard<std::mutex> lock(_mutex);
    uint32& last = _lastGreet[Key(bot, other)][2];
    if (last && now - last < LivingWorld::GuildInviteCooldown)
        return false;
    last = now;
    return true;
}

void LivingBondMgr::OnGroupJoin(uint32 group, uint32 member)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _groupJoins[group].emplace(member, Now());
}

void LivingBondMgr::CreditGroupTime(Player* a, Player* b, uint32 seconds)
{
    for (uint32 i = LivingWorld::GroupTimeEvents(seconds); i > 0; --i)
    {
        Record(a, b, LivingWorld::BondEvent::GroupTime);
        Record(b, a, LivingWorld::BondEvent::GroupTime);
    }
}

void LivingBondMgr::OnGroupLeave(uint32 group, Player* member)
{
    if (!member)
        return;
    uint32 const memberGuid = member->GetGUID().GetCounter();
    std::vector<std::pair<uint32, uint32>> together;  // other member, seconds together
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto const found = _groupJoins.find(group);
        if (found == _groupJoins.end())
            return;
        auto const self = found->second.find(memberGuid);
        if (self != found->second.end())
        {
            uint32 const now = Now();
            for (auto const& [other, joined] : found->second)
                if (other != memberGuid)
                    together.emplace_back(other, now - std::max(joined, self->second));
            found->second.erase(self);
        }
        if (found->second.empty())
            _groupJoins.erase(found);
    }
    for (auto const& [other, seconds] : together)
        if (Player* otherPlayer = ObjectAccessor::FindPlayerByLowGUID(other))
            CreditGroupTime(member, otherPlayer, seconds);
}

void LivingBondMgr::OnGroupDisband(uint32 group)
{
    std::vector<std::pair<uint32, uint32>> members;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto const found = _groupJoins.find(group);
        if (found == _groupJoins.end())
            return;
        members.assign(found->second.begin(), found->second.end());
        _groupJoins.erase(found);
    }
    uint32 const now = Now();
    for (size_t i = 0; i < members.size(); ++i)
        for (size_t j = i + 1; j < members.size(); ++j)
        {
            Player* a = ObjectAccessor::FindPlayerByLowGUID(members[i].first);
            Player* b = ObjectAccessor::FindPlayerByLowGUID(members[j].first);
            if (a && b)
                CreditGroupTime(a, b, now - std::max(members[i].second, members[j].second));
        }
}
