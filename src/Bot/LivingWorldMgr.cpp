/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingWorldMgr.h"

#include "Log.h"
#include "Random.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotsDatabase.h"
#include "QueryResult.h"
#include "RandomPlayerbotMgr.h"
#include "WorldSession.h"

LivingWorldMgr& LivingWorldMgr::instance()
{
    static LivingWorldMgr instance;
    return instance;
}

void LivingWorldMgr::Load()
{
    if (!sPlayerbotAIConfig.LivingWorldEnabled)
        return;

    std::lock_guard<std::mutex> lock(_mutex);
    _profiles.clear();
    auto* statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_LIVING_PROFILES);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(statement))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const guid = fields[0].Get<uint32>();
            LivingWorld::Profile profile;
            profile.motivation = static_cast<LivingWorld::Motivation>(fields[1].Get<uint8>());
            profile.temperament = static_cast<LivingWorld::Temperament>(fields[2].Get<uint8>());
            profile.levelCap = fields[3].Get<uint8>();
            profile.initialized = fields[4].Get<bool>();
            if (profile.motivation >= LivingWorld::Motivation::Count ||
                profile.temperament >= LivingWorld::Temperament::Count || profile.levelCap > 80)
            {
                LOG_ERROR("playerbots", "Invalid living profile for bot {}; preserving progress with neutral traits",
                          guid);
                profile = {};
                profile.initialized = true;
            }
            // Never repeat a starting grant after a crash or an interrupted first session.
            if (!profile.initialized)
            {
                profile.initialized = true;
                Save(guid, profile);
            }
            _profiles.emplace(guid, profile);
        } while (result->NextRow());
    }
    LOG_INFO("playerbots", "Loaded {} persistent living-world bot profiles", _profiles.size());
}

bool LivingWorldMgr::IsEnabled(Player* player) const
{
    return sPlayerbotAIConfig.LivingWorldEnabled && player && player->GetSession() &&
           player->GetSession()->IsHeadless() && sRandomPlayerbotMgr.IsRandomBot(player);
}

LivingWorld::Profile LivingWorldMgr::GetProfile(Player* player)
{
    std::lock_guard<std::mutex> lock(_mutex);
    uint32 const guid = player->GetGUID().GetCounter();
    bool const established =
        player->GetTotalPlayedTime() > 0 || player->GetLevel() > (player->getClass() == CLASS_DEATH_KNIGHT ? 55 : 1);
    auto const found = _profiles.find(guid);
    if (found != _profiles.end())
        return found->second;

    // Played characters, including level-one characters, are adopted without regeneration.
    auto const profile =
        LivingWorld::Create(guid, player->GetLevel(), sPlayerbotAIConfig.RandomBotMaxLevel, established);
    _profiles.emplace(guid, profile);
    Save(guid, profile);
    return profile;
}

void LivingWorldMgr::Save(uint32 guid, LivingWorld::Profile const& profile)
{
    auto* statement = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPSERT_LIVING_PROFILE);
    statement->SetData(0, guid);
    statement->SetData(1, static_cast<uint8>(profile.motivation));
    statement->SetData(2, static_cast<uint8>(profile.temperament));
    statement->SetData(3, profile.levelCap);
    statement->SetData(4, profile.initialized);
    PlayerbotsDatabase.Execute(statement);
}

bool LivingWorldMgr::Preserve(Player* player) { return IsEnabled(player) && GetProfile(player).initialized; }

void LivingWorldMgr::MarkInitialized(Player* player)
{
    if (!IsEnabled(player))
        return;
    GetProfile(player);
    std::lock_guard<std::mutex> lock(_mutex);
    uint32 const guid = player->GetGUID().GetCounter();
    auto& profile = _profiles.at(guid);
    profile.initialized = true;
    Save(guid, profile);
}

uint32 LivingWorldMgr::GetActivityWeight(Player* player, NewRpgStatus status, uint32 baseline)
{
    if (!IsEnabled(player))
        return baseline;
    LivingWorld::Activity activity;
    switch (status)
    {
        case RPG_GO_GRIND:
            activity = LivingWorld::Activity::Grind;
            break;
        case RPG_GO_CAMP:
            activity = LivingWorld::Activity::Camp;
            break;
        case RPG_WANDER_RANDOM:
            activity = LivingWorld::Activity::Wander;
            break;
        case RPG_WANDER_NPC:
            activity = LivingWorld::Activity::Npc;
            break;
        case RPG_DO_QUEST:
            activity = LivingWorld::Activity::Quest;
            break;
        case RPG_TRAVEL_FLIGHT:
            activity = LivingWorld::Activity::Flight;
            break;
        case RPG_REST:
            activity = LivingWorld::Activity::Rest;
            break;
        case RPG_OUTDOOR_PVP:
            activity = LivingWorld::Activity::Pvp;
            break;
        case RPG_DO_GATHER:
            activity = LivingWorld::Activity::Gather;
            break;
        default:
            return baseline;
    }
    return LivingWorld::Weight(GetProfile(player).motivation, activity, baseline);
}

uint32 LivingWorldMgr::GetBattlegroundRole(Player* player, uint32 roll)
{
    if (!IsEnabled(player))
        return roll;
    return LivingWorld::BattlegroundRole(GetProfile(player).temperament, roll);
}

uint32 LivingWorldMgr::GetLevelCap(Player* player)
{
    uint32 const eraCap = sPlayerbotAIConfig.RandomBotMaxLevel;
    auto const profile = GetProfile(player);
    return profile.levelCap ? std::min<uint32>(eraCap, profile.levelCap) : eraCap;
}

void LivingWorldMgr::LimitExperience(Player* player, uint32& amount)
{
    if (!IsEnabled(player))
        return;
    uint32 const cap = GetLevelCap(player);
    uint64 allowed = 0;
    for (uint32 level = player->GetLevel(); level < cap; ++level)
        allowed += sObjectMgr->GetXPForLevel(static_cast<uint8>(level));
    uint32 const current = player->GetUInt32Value(PLAYER_XP);
    allowed = allowed > current ? allowed - current : 0;
    amount = static_cast<uint32>(std::min<uint64>(amount, allowed));
}

uint32 LivingWorldMgr::GetSessionTime(uint32 guid, uint32 baseline)
{
    if (!sPlayerbotAIConfig.LivingWorldEnabled)
        return baseline;
    std::lock_guard<std::mutex> lock(_mutex);
    auto const found = _profiles.find(guid);
    if (found == _profiles.end())
        return baseline;
    return static_cast<uint32>(uint64(baseline) * LivingWorld::SessionPercent(found->second.motivation) / 100);
}

void LivingWorldMgr::RollCityStart(Player* player)
{
    if (!IsEnabled(player) || player->GetLevel() < LivingWorld::CityStartMinLevel)
        return;
    if (urand(1, 100) > LivingWorld::CityStartPercent(GetProfile(player).motivation))
        return;
    std::lock_guard<std::mutex> lock(_mutex);
    _cityStarts.insert(player->GetGUID().GetCounter());
}

bool LivingWorldMgr::TakeCityStart(Player* player)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _cityStarts.erase(player->GetGUID().GetCounter()) > 0;
}
