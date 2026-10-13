/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTEP_H
#define PLAYERBOTS_PVPLOADOUTEP_H

#include "Define.h"
#include <array>
#include <map>
#include <string>
#include <vector>

// Equivalence points for PvP gear: a role profile's stat targets turned into capped weights, and the loadout choices
// made with them. Pure, like PvpLoadoutRules, so it is unit-testable without a server.
namespace PvpLoadout
{
enum class Stat : uint8
{
    Hit,
    SpellPenetration,
    Resilience,
    Stamina,
    Strength,
    Agility,
    AttackPower,
    ArmorPenetration,
    SpellPower,
    Haste,
    Crit,
    Mp5,
    Spirit,
    Intellect,
    WeaponDps,
    Count
};

using StatVector = std::array<float, static_cast<size_t>(Stat::Count)>;

float& At(StatVector& stats, Stat stat);
float At(StatVector const& stats, Stat stat);
StatVector Add(StatVector lhs, StatVector const& rhs);
// lhs - rhs, never below 0.
StatVector Subtract(StatVector lhs, StatVector const& rhs);

enum class Role : uint8
{
    Melee,
    Hunter,
    Caster,
    Healer
};

struct Profile
{
    uint32 hitTarget;
    uint32 spellPenetrationTarget;  // 0 when the role does not need spell penetration
    uint32 resilienceTarget;
    std::vector<Stat> priority;  // stats worth spending on once the targets are met, most valuable first
    float weaponDpsWeight;       // per point of weapon DPS; 0 for roles whose damage is not weapon based
};

Profile DefaultProfile(Role role);

// "hit:164,spellpen:0,resilience:1000,priority:armorpen,strength,agility". Starts from the role's defaults, so the
// weapon DPS weight is kept. Returns false, leaving profile untouched, on any unknown key, stat or number.
bool ParseProfile(std::string const& text, Role role, Profile& profile);

// Capped weights: hit and spell penetration are worth nothing past their targets, resilience much less.
float Ep(StatVector const& totals, Profile const& profile);
float MarginalEp(StatVector const& totals, StatVector const& added, Profile const& profile);

// "hit 120/164, spellpen 0/130, resilience 900/1050" for the debug lines.
std::string FormatTargets(StatVector const& totals, Profile const& profile);

// One eligible set piece: its equipment slot, the set it belongs to, and its item level.
struct SetPiece
{
    uint8 slot;
    uint32 itemSet;
    uint32 itemId;
    uint32 itemLevel;
};

// Slot -> item id: the best `required` pieces of the set with the highest total item level (later season on ties);
// empty when no set has `required` eligible slots.
std::map<uint8, uint32> PickSetPieces(std::vector<SetPiece> const& pieces, uint32 required);

// One way to fill a group of slots, e.g. one ring, or a two-hander (main hand plus an empty off-hand).
struct Option
{
    std::vector<uint32> itemIds;  // one per item it equips; empty for "leave empty"
    StatVector stats;
    bool own;     // the bot already wears it, so nothing needs creating
    bool unique;  // at most one copy may be equipped across all groups
};

// Slots decided together, e.g. one ring slot, or main hand plus off-hand.
struct Group
{
    std::vector<Option> options;
};

// The chosen option per group (-1 when none fits), each round taking the biggest marginal EP over `base` and the
// choices so far; unique items are taken once and the bot's own option wins ties.
std::vector<int32> SelectBiggestGainFirst(StatVector const& base, std::vector<Group> const& groups,
                                          Profile const& profile);
}  // namespace PvpLoadout

#endif
