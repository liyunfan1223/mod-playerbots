/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BisListMgr.h"
#include "PlayerbotsDatabase.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"

void BisListMgr::LoadAll()
{
    _bis.clear();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_BIS_GEAR);
    PreparedQueryResult result = PlayerbotsDatabase.Query(stmt);
    if (!result)
    {
        LOG_INFO("server.loading", "playerbots_bis_gear table missing or empty");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint8  cls       = fields[0].Get<uint8>();
        uint8  tab       = fields[1].Get<uint8>();
        uint8  slot      = fields[2].Get<uint8>();
        uint8  faction   = fields[3].Get<uint8>();
        uint16 autoGearScoreLimit = fields[4].Get<uint16>();
        uint32 item      = fields[5].Get<uint32>();

        _bis[autoGearScoreLimit][MakeKey(cls, tab)][faction][slot] = item;
        ++count;
    } while (result->NextRow());

    LOG_INFO("server.loading", "Loaded {} BiS entries across {} item levels",
             count, static_cast<uint32>(_bis.size()));
}

std::map<uint8, uint32> BisListMgr::GetBisFor(uint16 autoGearScoreLimit, uint8 cls, uint8 tab, uint8 faction) const
{
    auto ilvlIt = _bis.find(autoGearScoreLimit);
    if (ilvlIt == _bis.end())
        return {};

    auto comboIt = ilvlIt->second.find(MakeKey(cls, tab));
    if (comboIt == ilvlIt->second.end())
        return {};

    std::map<uint8, uint32> result;

    // Base: faction=0 (Both).
    auto bothIt = comboIt->second.find(0);
    if (bothIt != comboIt->second.end())
        result = bothIt->second;

    // Faction-specific overrides Both.
    if (faction == 1 || faction == 2)
    {
        auto facIt = comboIt->second.find(faction);
        if (facIt != comboIt->second.end())
            for (auto const& kv : facIt->second)
                result[kv.first] = kv.second;
    }

    return result;
}

std::map<uint8, uint32> BisListMgr::GetBisForNearest(uint16 requestedIlvl, uint16 maxDrop, uint8 cls, uint8 tab,
                                                    uint8 faction, uint16* outResolved) const
{
    uint16 floor = requestedIlvl > maxDrop ? requestedIlvl - maxDrop : 1;
    for (uint16 try_ilvl = requestedIlvl; try_ilvl >= floor; --try_ilvl)
    {
        auto result = GetBisFor(try_ilvl, cls, tab, faction);
        if (!result.empty())
        {
            if (outResolved)
                *outResolved = try_ilvl;
            return result;
        }
        if (try_ilvl == 0)
            break;
    }
    if (outResolved)
        *outResolved = 0;
    return {};
}
