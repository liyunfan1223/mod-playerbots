/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOTS_LIVING_WORLD_POLICY_H
#define _PLAYERBOTS_LIVING_WORLD_POLICY_H

#include <algorithm>
#include <array>
#include <cstdint>

namespace LivingWorld
{
enum class Motivation : uint8_t
{
    Trader,
    Profession,
    Leveller,
    Explorer,
    Gear,
    Pvp,
    Count
};

enum class Temperament : uint8_t
{
    Objective,
    Defender,
    Diver,
    Skirmisher,
    Twink,
    Turtle,  // never leaves its own base/flag room (appended: stored values stay valid)
    Count
};

enum class Activity : uint8_t
{
    Grind,
    Camp,
    Wander,
    Npc,
    Quest,
    Flight,
    Rest,
    Pvp,
    Gather,
    Count
};

struct Profile
{
    Motivation motivation = Motivation::Explorer;
    Temperament temperament = Temperament::Objective;
    uint8_t levelCap = 0;
    bool initialized = false;
};

// Version 1 assignment is deliberately independent of the standard library's hash implementation.
constexpr uint32_t Mix(uint32_t value)
{
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    return value ^ (value >> 16);
}

constexpr Profile Create(uint32_t guid, uint32_t level, uint32_t eraCap, bool initialized)
{
    uint32_t const motivation = Mix(guid ^ 0x9e3779b9U) % 100;
    uint32_t const temperament = Mix(guid ^ 0x85ebca6bU) % 100;
    Profile profile;
    profile.motivation = motivation < 15   ? Motivation::Trader
                         : motivation < 35 ? Motivation::Profession
                         : motivation < 50 ? Motivation::Leveller
                         : motivation < 70 ? Motivation::Explorer
                         : motivation < 90 ? Motivation::Gear
                                           : Motivation::Pvp;
    profile.temperament = temperament < 25   ? Temperament::Objective
                          : temperament < 35 ? Temperament::Turtle
                          : temperament < 55 ? Temperament::Defender
                          : temperament < 75 ? Temperament::Diver
                          : temperament < 90 ? Temperament::Skirmisher
                                             : Temperament::Twink;
    if (profile.temperament == Temperament::Twink)
    {
        uint32_t const preferred = 19 + 10 * (Mix(guid ^ 0xc2b2ae35U) % 5);
        uint32_t const nextBracket = level < 10 ? 19 : (level / 10) * 10 + 9;
        profile.levelCap = static_cast<uint8_t>(std::min(eraCap, std::max(preferred, nextBracket)));
    }
    profile.initialized = initialized;
    return profile;
}

constexpr uint32_t Weight(Motivation motivation, Activity activity, uint32_t baseline)
{
    // Percentages modify administrator weights; zero always remains disabled.
    constexpr std::array<std::array<uint16_t, 9>, 6> multipliers = {{{{80, 100, 60, 200, 80, 100, 100, 70, 240}},
                                                                     {{100, 100, 70, 160, 100, 100, 80, 70, 300}},
                                                                     {{180, 70, 40, 60, 200, 130, 40, 70, 70}},
                                                                     {{70, 130, 220, 140, 100, 170, 180, 80, 130}},
                                                                     {{160, 140, 50, 100, 150, 100, 70, 90, 100}},
                                                                     {{100, 80, 80, 80, 80, 100, 70, 350, 70}}}};
    if (!baseline)
        return 0;
    auto const row = static_cast<uint8_t>(motivation);
    auto const column = static_cast<uint8_t>(activity);
    if (row >= multipliers.size() || column >= multipliers[0].size())
        return baseline;
    uint64_t const weighted = uint64_t(baseline) * multipliers[row][column] / 100;
    return static_cast<uint32_t>(std::clamp<uint64_t>(weighted, 1, 1000000));
}

// Levelling pace by personality, as a percentage of the bot XP rate: power levellers race
// ahead, explorers and crafters take their time. Applied only to bots not grouped with a
// real player (a player's party levels together at the normal rate).
constexpr uint32_t XpPacePercent(Motivation motivation)
{
    switch (motivation)
    {
        case Motivation::Leveller: return 160;
        case Motivation::Explorer: return 60;
        case Motivation::Profession: return 80;
        case Motivation::Trader: return 85;
        default: return 100;
    }
}

// Play-session length by personality, as a percentage of the configured in-world time: power
// levellers and gear farmers put in long sessions, explorers drop in briefly. Weighted by the
// motivation mix the average stays near 100%, so the online population is unchanged.
constexpr uint32_t SessionPercent(Motivation motivation)
{
    switch (motivation)
    {
        case Motivation::Leveller: return 150;
        case Motivation::Gear: return 120;
        case Motivation::Pvp: return 110;
        case Motivation::Explorer: return 60;
        default: return 100;
    }
}

constexpr uint32_t BattlegroundRole(Temperament temperament, uint32_t roll)
{
    roll %= 10;
    switch (temperament)
    {
        case Temperament::Defender:
            return roll < 7 ? roll % 3 : roll;
        case Temperament::Turtle:
            return roll % 3;
        case Temperament::Diver:
            return roll < 7 ? 8 + roll % 2 : roll;
        case Temperament::Skirmisher:
            return roll < 7 ? 4 + roll % 4 : roll;
        default:
            return roll;
    }
}

// Battleground gear spread around the real players queued (their average gear score):
// most bots that fill the match are close to it, some clearly weaker or stronger, a few far
// off. Twinks are overgeared for their bracket by design and always qualify.
constexpr uint32_t GearMatchChance(uint32_t botGearScore, uint32_t playerGearScore, Temperament temperament)
{
    if (!playerGearScore || temperament == Temperament::Twink)
        return 100;
    uint64_t const diff = botGearScore > playerGearScore ? botGearScore - playerGearScore
                                                          : playerGearScore - botGearScore;
    uint64_t const percent = diff * 100 / playerGearScore;
    return percent <= 15 ? 100 : percent <= 30 ? 50 : 15;
}

// One stable decision per bot, queue and 5-minute window, so a poorly matched bot cannot
// simply retry every tick until it gets in.
constexpr bool GearMatchRoll(uint32_t guid, uint32_t queue, uint64_t gameTimeSeconds, uint32_t chance)
{
    uint32_t const window = static_cast<uint32_t>(gameTimeSeconds / 300);
    return Mix(guid ^ Mix(queue * 0x9e3779b9U ^ window)) % 100 < chance;
}
}  // namespace LivingWorld

#endif
