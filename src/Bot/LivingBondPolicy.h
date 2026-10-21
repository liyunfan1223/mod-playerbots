/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef _PLAYERBOT_LIVING_BOND_POLICY_H
#define _PLAYERBOT_LIVING_BOND_POLICY_H

#include <cstdint>

namespace LivingWorld
{
// What a bot remembers about another character, from the everyday things players do to each other.
enum class BondEvent : uint8_t
{
    GroupTime,         // adventured in the same group (one event per 15 minutes together)
    Quest,             // completed a quest while grouped together
    DungeonBoss,       // killed a dungeon or raid boss together
    BattlegroundWin,   // won a battleground on the same side
    BattlegroundLoss,  // lost a battleground on the same side
    Resurrected,       // the other character resurrected this bot
    Healed,            // healed this bot without being in its group
    Buffed,            // gave this bot a long buff without being in its group
    Gift,              // offered this bot an item in trade
    Invited,           // invited this bot to a group
    Duel,              // fought a duel with this bot
    FriendlyEmote,     // /hug, /wave, /thank and the like, aimed at this bot
    RudeEmote,         // /rude, /spit, /slap and the like, aimed at this bot
    KilledBy,          // killed this bot in world PvP
    Ganked,            // killed it again within 10 minutes, or while 10+ levels higher
    Kicked,            // kicked this bot from a group
    Abandoned,         // left this bot's group in the middle of a dungeon
    Ninja,             // won a Need roll on gear it could not use or wear
    Count
};

enum class Attitude : uint8_t
{
    Rival,
    Wary,
    Neutral,
    Friendly,
    Friend
};

struct Bond
{
    int16_t score = 0;
    uint16_t quests = 0;
    uint16_t bosses = 0;
    uint16_t battlegrounds = 0;
    uint16_t minutes = 0;  // minutes grouped together (in 15-minute steps)
    uint16_t helps = 0;    // resurrections, heals, buffs, gifts, invites
    uint16_t duels = 0;
    uint16_t kills = 0;    // world PvP kills of this bot, ganks included
    uint16_t ninjas = 0;
    uint16_t slights = 0;  // kicks, abandoned dungeons, rude emotes
    uint32_t lastSeen = 0;
};

inline constexpr int16_t MaxBondScore = 100;
inline constexpr int16_t FriendScore = 30;
inline constexpr int16_t FriendlyScore = 10;
inline constexpr int16_t RivalScore = -25;
inline constexpr uint32_t GroupTimeStep = 15 * 60;  // seconds grouped per GroupTime event
inline constexpr uint32_t MaxGroupTimeEvents = 8;   // per group session (two hours)
inline constexpr uint32_t GankWindow = 10 * 60;     // a repeat kill within this window is a gank
inline constexpr uint8_t GankLevelGap = 10;

constexpr int16_t BondDelta(BondEvent event)
{
    switch (event)
    {
        case BondEvent::GroupTime: return 1;
        case BondEvent::Quest: return 2;
        case BondEvent::DungeonBoss: return 3;
        case BondEvent::BattlegroundWin: return 2;
        case BondEvent::BattlegroundLoss: return 1;
        case BondEvent::Resurrected: return 5;
        case BondEvent::Healed: return 1;
        case BondEvent::Buffed: return 1;
        case BondEvent::Gift: return 2;
        case BondEvent::Invited: return 1;
        case BondEvent::Duel: return 1;
        case BondEvent::FriendlyEmote: return 1;
        case BondEvent::RudeEmote: return -2;
        case BondEvent::KilledBy: return -3;
        case BondEvent::Ganked: return -5;
        case BondEvent::Kicked: return -5;
        case BondEvent::Abandoned: return -2;
        case BondEvent::Ninja: return -15;
        default: return 0;
    }
}

// Minimum seconds between two counted events of a kind for the same pair, so spam (buff, emote or
// trade-window farming) cannot buy friendship or bury it.
constexpr uint32_t BondCooldown(BondEvent event)
{
    switch (event)
    {
        case BondEvent::Resurrected: return 60;
        case BondEvent::Healed: return 10 * 60;
        case BondEvent::Buffed: return 30 * 60;
        case BondEvent::Gift: return 30 * 60;
        case BondEvent::Invited: return 30 * 60;
        case BondEvent::Duel: return 10 * 60;
        case BondEvent::FriendlyEmote: return 5 * 60;
        case BondEvent::RudeEmote: return 5 * 60;
        default: return 0;
    }
}

constexpr uint16_t Bump(uint16_t count, uint16_t by = 1)
{
    return count > UINT16_MAX - by ? UINT16_MAX : static_cast<uint16_t>(count + by);
}

constexpr void ApplyBond(Bond& bond, BondEvent event, uint32_t now)
{
    int32_t const score = int32_t(bond.score) + BondDelta(event);
    bond.score = static_cast<int16_t>(score > MaxBondScore ? MaxBondScore : score < -MaxBondScore ? -MaxBondScore : score);
    switch (event)
    {
        case BondEvent::GroupTime: bond.minutes = Bump(bond.minutes, GroupTimeStep / 60); break;
        case BondEvent::Quest: bond.quests = Bump(bond.quests); break;
        case BondEvent::DungeonBoss: bond.bosses = Bump(bond.bosses); break;
        case BondEvent::BattlegroundWin:
        case BondEvent::BattlegroundLoss: bond.battlegrounds = Bump(bond.battlegrounds); break;
        case BondEvent::Resurrected:
        case BondEvent::Healed:
        case BondEvent::Buffed:
        case BondEvent::Gift:
        case BondEvent::Invited: bond.helps = Bump(bond.helps); break;
        case BondEvent::Duel: bond.duels = Bump(bond.duels); break;
        case BondEvent::KilledBy: bond.kills = Bump(bond.kills); break;
        case BondEvent::Ninja: bond.ninjas = Bump(bond.ninjas); break;
        case BondEvent::RudeEmote:
        case BondEvent::Kicked:
        case BondEvent::Abandoned: bond.slights = Bump(bond.slights); break;
        default: break;  // friendly emotes and ganks only move the score
    }
    bond.lastSeen = now;
}

// GroupTime events earned by a stretch of time grouped together.
constexpr uint32_t GroupTimeEvents(uint32_t seconds)
{
    uint32_t const events = seconds / GroupTimeStep;
    return events > MaxGroupTimeEvents ? MaxGroupTimeEvents : events;
}

constexpr bool IsGank(uint32_t secondsSinceLastKill, bool killedBefore, uint8_t killerLevel, uint8_t victimLevel)
{
    return (killedBefore && secondsSinceLastKill <= GankWindow) || killerLevel >= victimLevel + GankLevelGap;
}

constexpr Attitude AttitudeFor(int16_t score)
{
    return score >= FriendScore     ? Attitude::Friend
           : score >= FriendlyScore ? Attitude::Friendly
           : score <= RivalScore    ? Attitude::Rival
           : score < 0              ? Attitude::Wary
                                    : Attitude::Neutral;
}

constexpr char const* AttitudeName(Attitude attitude)
{
    switch (attitude)
    {
        case Attitude::Rival: return "rival";
        case Attitude::Wary: return "wary";
        case Attitude::Friendly: return "friendly";
        case Attitude::Friend: return "friend";
        default: return "neutral";
    }
}

// Behaviour from bonds. Friendly bots treat the other character like a guildmate when invited (no
// level, gear score or battleground-queue refusals); rivals refuse and say why. Friends greet.
inline constexpr uint32_t GreetRange = 30;               // yards, for in-person greetings
inline constexpr uint32_t GreetCooldown = 60 * 60;       // per pair, in-person greetings
inline constexpr uint32_t WhisperCooldown = 6 * 60 * 60; // per pair, login greetings
inline constexpr uint32_t MaxLoginGreetings = 3;         // bots greeting one player at login
inline constexpr uint32_t LoginGreetDelay = 20;          // seconds, so the chat frame has loaded
inline constexpr uint32_t GuildInviteCooldown = 7 * 24 * 60 * 60; // per pair: a friend offers its guild once a week

constexpr bool TrustsLikeGuildmate(Attitude attitude) { return attitude >= Attitude::Friendly; }

enum class Topic : uint8_t
{
    General,
    Dungeons,
    Battlegrounds,
    Quests
};

// What friends have mostly done together, for what to talk about.
constexpr Topic SharedTopic(Bond const& bond)
{
    if (bond.bosses && bond.bosses >= bond.battlegrounds && bond.bosses >= bond.quests)
        return Topic::Dungeons;
    if (bond.battlegrounds && bond.battlegrounds >= bond.quests)
        return Topic::Battlegrounds;
    return bond.quests ? Topic::Quests : Topic::General;
}

enum class Grudge : uint8_t
{
    General,
    Ninja,
    Kills,
    Slights
};

// The main reason a rival holds a grudge.
constexpr Grudge MainGrudge(Bond const& bond)
{
    if (bond.ninjas)
        return Grudge::Ninja;
    if (bond.kills && bond.kills >= bond.slights)
        return Grudge::Kills;
    return bond.slights ? Grudge::Slights : Grudge::General;
}

// Text emote ids (EmotesText.dbc / TextEmotes in SharedDefines.h).
constexpr int8_t EmoteTone(uint32_t textEmote)
{
    switch (textEmote)
    {
        case 5:    // applaud
        case 17:   // bow
        case 21:   // cheer
        case 48:   // greet
        case 56:   // hug
        case 58:   // kiss
        case 78:   // salute
        case 97:   // thank
        case 101:  // wave
        case 102:  // welcome
        case 122:  // praise
        case 225:  // love
        case 364:  // pat
        case 380:  // high five
            return 1;
        case 13:   // bonk
        case 22:   // chicken
        case 46:   // glare
        case 77:   // rude
        case 89:   // spit
        case 98:   // threaten
        case 113:  // insult
        case 130:  // slap
        case 131:  // smirk
        case 133:  // snub
        case 136:  // taunt
        case 143:  // violin
        case 203:  // pity
            return -1;
        default:
            return 0;
    }
}

// Best body armour a class wears at a level (cloth 1, leather 2, mail 3, plate 4); warriors and
// paladins move to plate, hunters and shamans to mail, at level 40. Class ids as in ChrClasses.
constexpr uint8_t BestArmourTier(uint8_t classId, uint8_t level)
{
    switch (classId)
    {
        case 1:  // warrior
        case 2:  // paladin
            return level >= 40 ? 4 : 3;
        case 6:  // death knight
            return 4;
        case 3:  // hunter
        case 7:  // shaman
            return level >= 40 ? 3 : 2;
        case 4:   // rogue
        case 11:  // druid
            return 2;
        default:
            return 1;
    }
}

// A Need roll counts as ninja looting when the winner cannot use the item, or takes body armour
// below the best type its class wears (cloaks, jewellery and weapons it can use are fine).
constexpr bool IsNinjaNeed(bool canUse, bool bodyArmour, uint8_t armourTier, uint8_t bestTier)
{
    return !canUse || (bodyArmour && armourTier && armourTier < bestTier);
}
}  // namespace LivingWorld

#endif
