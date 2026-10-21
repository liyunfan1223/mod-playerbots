/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOTS_LIVING_BOND_MGR_H
#define _PLAYERBOTS_LIVING_BOND_MGR_H

#include <array>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Define.h"
#include "LivingBondPolicy.h"

class Player;

// Remembers, per living random bot, how it feels about each character it has played with or
// against. Hooks run on map threads, so every access is locked; saving is asynchronous.
class LivingBondMgr
{
public:
    static LivingBondMgr& instance();
    void Load();
    // Records an event in `bot`'s memory of `other`, unless that kind of event was counted for the
    // pair within its cooldown. Ignored unless `bot` is a living random bot. True when counted.
    bool Record(Player* bot, Player* other, LivingWorld::BondEvent event);
    // Seconds since `bot` last counted `event` for `other` (0 and false when never).
    bool SecondsSince(uint32 bot, uint32 other, LivingWorld::BondEvent event, uint32& seconds);
    LivingWorld::Bond Get(uint32 bot, uint32 other);
    // The bot's strongest bonds, best first then worst, at most `limit` of each.
    std::vector<std::pair<uint32, LivingWorld::Bond>> Strongest(uint32 bot, size_t limit);
    LivingWorld::Attitude AttitudeOf(uint32 bot, uint32 other);
    // Bots that have a bond with `other`, best first.
    std::vector<std::pair<uint32, LivingWorld::Bond>> KnownBy(uint32 other);
    // True (and starts the cooldown) when `bot` may greet `other` again in this channel.
    bool TryGreet(uint32 bot, uint32 other, bool whisper);
    // True (and starts the weekly cooldown) when `bot` may offer `other` its guild again.
    bool TryGuildInvite(uint32 bot, uint32 other);

    // Group membership times, for time spent adventuring together.
    void OnGroupJoin(uint32 group, uint32 member);
    // Credits time grouped between `member` and everyone still in the group, and forgets `member`.
    void OnGroupLeave(uint32 group, Player* member);
    void OnGroupDisband(uint32 group);

private:
    static uint64 Key(uint32 bot, uint32 other) { return (uint64(bot) << 32) | other; }
    void CreditGroupTime(Player* a, Player* b, uint32 seconds);

    std::mutex _mutex;
    std::unordered_map<uint64, LivingWorld::Bond> _bonds;
    std::unordered_map<uint64, std::array<uint32, size_t(LivingWorld::BondEvent::Count)>> _lastEvent;
    std::unordered_map<uint32, std::unordered_map<uint32, uint32>> _groupJoins;  // group -> member -> joined
    std::unordered_map<uint64, std::array<uint32, 3>> _lastGreet;               // pair -> [in person, whisper, guild invite]
};

#endif
