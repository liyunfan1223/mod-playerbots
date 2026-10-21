/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOTS_LIVING_WORLD_MGR_H
#define _PLAYERBOTS_LIVING_WORLD_MGR_H

#include <mutex>
#include <unordered_map>

#include "LivingWorldPolicy.h"
#include "PlayerbotAIConfig.h"

class Player;

class LivingWorldMgr
{
public:
    static LivingWorldMgr& instance();
    void Load();
    bool IsEnabled(Player* player) const;
    bool Preserve(Player* player);
    LivingWorld::Profile GetProfile(Player* player);
    void MarkInitialized(Player* player);
    uint32 GetActivityWeight(Player* player, NewRpgStatus status, uint32 baseline);
    uint32 GetBattlegroundRole(Player* player, uint32 roll);
    uint32 GetLevelCap(Player* player);
    void LimitExperience(Player* player, uint32& amount);

private:
    void Save(uint32 guid, LivingWorld::Profile const& profile);
    std::mutex _mutex;
    std::unordered_map<uint32, LivingWorld::Profile> _profiles;
};

#endif
