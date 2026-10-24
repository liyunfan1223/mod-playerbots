/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDENCOUNTERRULES_H
#define PLAYERBOTS_RAIDENCOUNTERRULES_H

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace RaidEncounterRules
{
using FamilyMask = uint64_t;

// What an action is, as far as a rule cares. ClassifyAction (RaidEncounter.cpp) is the only place
// that maps an Action to these.
namespace Family
{
// Movement. Every MovementAction has AnyMovement, and every AttackAction is a MovementAction.
constexpr FamilyMask CombatFormationMove = 1ull << 0;  // exact type, TankFace and SetBehind derive from it
constexpr FamilyMask TankFace = 1ull << 1;
constexpr FamilyMask SetBehind = 1ull << 2;
constexpr FamilyMask RearFlank = 1ull << 3;
constexpr FamilyMask Reach = 1ull << 4;  // any ReachTargetAction except ReachHeal
constexpr FamilyMask ReachHeal = 1ull << 5;
constexpr FamilyMask Follow = 1ull << 6;
constexpr FamilyMask Flee = 1ull << 7;
constexpr FamilyMask RunAway = 1ull << 8;
constexpr FamilyMask MoveRandom = 1ull << 9;
constexpr FamilyMask MoveOutOfCollision = 1ull << 10;
constexpr FamilyMask MoveOutOfEnemyContact = 1ull << 11;
constexpr FamilyMask AvoidAoe = 1ull << 12;
constexpr FamilyMask AnyMovement = 1ull << 13;

// Spells that move the caster.
constexpr FamilyMask Charge = 1ull << 14;  // CastReachTargetSpellAction
constexpr FamilyMask Blink = 1ull << 15;
constexpr FamilyMask Disengage = 1ull << 16;

constexpr FamilyMask Attack = 1ull << 17;  // any AttackAction, so every picker below too
constexpr FamilyMask Melee = 1ull << 18;

// Generic target pickers.
constexpr FamilyMask DpsAssist = 1ull << 19;
constexpr FamilyMask TankAssist = 1ull << 20;
constexpr FamilyMask DpsAoe = 1ull << 21;
constexpr FamilyMask AggressiveTarget = 1ull << 22;
constexpr FamilyMask AttackAnything = 1ull << 23;
constexpr FamilyMask AttackLeastHp = 1ull << 24;
constexpr FamilyMask AttackRti = 1ull << 25;
constexpr FamilyMask DebuffOnAttacker = 1ull << 26;
constexpr FamilyMask DropTarget = 1ull << 27;
constexpr FamilyMask PetAttack = 1ull << 28;

// The single and area taunts, whatever the bot's role. Warriors cast Taunt from two actions, and Mocking
// Blow is one too: the Arms snare chain falls back to it.
constexpr FamilyMask Taunt = 1ull << 29;

constexpr FamilyMask Spell = 1ull << 30;  // any CastSpellAction

// Every action, items and plain Actions included, for a hand-written multiplier whose zero can land on
// anything. No rule should name it.
constexpr FamilyMask AnyAction = 1ull << 31;
}  // namespace Family

using RoleMask = uint8_t;

namespace Role
{
constexpr RoleMask Any = 0;
constexpr RoleMask Tank = 1 << 0;
constexpr RoleMask MainTank = 1 << 1;
constexpr RoleMask Heal = 1 << 2;
constexpr RoleMask Ranged = 1 << 3;
constexpr RoleMask Melee = 1 << 4;
constexpr RoleMask NonTank = 1 << 5;  // healers and dps alike: every bot but a tank
constexpr RoleMask Dps = 1 << 6;       // PlayerbotAI::IsDps, which reads the bot's dps strategy
}  // namespace Role

// A rule zeroes an action it cares about while the gate is open, the bot has one of its roles and the
// boss's predicate holds. A pass always beats a block.
struct Rule
{
    bool blockMovement = false;  // every AnyMovement action
    FamilyMask blockFamilies = 0;
    std::vector<std::string> blockNames;
    FamilyMask passFamilies = 0;
    std::vector<std::string> passNames;
};

// Blocks every movement action but the boss's movers and the kept families. Reach stays in the default
// keep because it's what holds a tank in melee.
inline Rule OwnMovement(std::vector<std::string> movers, FamilyMask keep = Family::Attack | Family::Reach)
{
    Rule rule;
    rule.blockMovement = true;
    rule.passFamilies = keep;
    rule.passNames = std::move(movers);
    return rule;
}

inline Rule OwnTargeting(FamilyMask pickers = Family::DpsAssist | Family::TankAssist)
{
    Rule rule;
    rule.blockFamilies = pickers;
    return rule;
}

inline Rule Block(FamilyMask families, std::vector<std::string> names = {})
{
    Rule rule;
    rule.blockFamilies = families;
    rule.blockNames = std::move(names);
    return rule;
}

// Like OwnMovement with nothing kept, attacks included: only what it names moves the bot.
inline Rule Exclusive(FamilyMask families = 0, std::vector<std::string> names = {})
{
    Rule rule;
    rule.blockMovement = true;
    rule.passFamilies = families;
    rule.passNames = std::move(names);
    return rule;
}

inline bool Listed(std::vector<std::string> const& names, std::string const& name)
{
    return std::find(names.begin(), names.end(), name) != names.end();
}

// Depends only on the action, so callers can cache it per action.
inline bool RuleCares(Rule const& rule, FamilyMask action, std::string const& name)
{
    bool const blocked = (rule.blockMovement && (action & Family::AnyMovement)) ||
                         (action & rule.blockFamilies) || Listed(rule.blockNames, name);

    return blocked && !(action & rule.passFamilies) && !Listed(rule.passNames, name);
}

// hasRole is asked one role at a time and only until one matches: IsMainTank walks the group.
template <class HasRole>
bool RoleMatches(RoleMask roles, HasRole&& hasRole)
{
    if (roles == Role::Any)
        return true;

    for (unsigned bit = 1; bit <= roles; bit <<= 1)
        if ((roles & bit) && hasRole(static_cast<RoleMask>(bit)))
            return true;

    return false;
}

// Cheapest first, and each step runs only if the one before it passed: the predicate can sweep the
// map, and every rule sees every action a bot considers.
template <class GateOpen, class RoleMatch, class Predicate>
float Evaluate(bool cares, GateOpen&& gateOpen, RoleMatch&& roleMatches, Predicate&& predicate)
{
    if (!cares || !gateOpen() || !roleMatches() || !predicate())
        return 1.0f;

    return 0.0f;
}

// One read of an instance's boss states.
struct BossStates
{
    bool inInstance = false;
    uint32_t encounterCount = 0;  // capped at MAX_BOSS_STATES
    uint64_t inProgress = 0;
    uint64_t done = 0;
};

constexpr uint32_t MAX_BOSS_STATES = 64;

// Closed once this encounter is done or while another one is in progress, open otherwise, so pre-pull
// steps with no boss engaged yet still run. Open outside an instance and for ids the script lacks.
inline bool GateOpen(BossStates const& states, uint32_t encounterId)
{
    if (!states.inInstance || encounterId >= states.encounterCount)
        return true;

    uint64_t const bit = uint64_t(1) << encounterId;
    if (states.done & bit)
        return false;

    if (states.inProgress & bit)
        return true;

    return (states.inProgress & ~bit) == 0;
}
}  // namespace RaidEncounterRules

#endif
