// Pure rules shared by dungeon encounter tactics: how a multiplier sees an action, angles and spell names.

#ifndef _PLAYERBOTS_DUNGEON_TACTICS_POLICY_H
#define _PLAYERBOTS_DUNGEON_TACTICS_POLICY_H

#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace DungeonTactics
{
constexpr float Pi = 3.14159265f;

// Same wrap as Position::NormalizeOrientation: [0, 2pi).
inline float NormalizeAngle(float angle)
{
    if (angle < 0.0f)
        return 2.0f * Pi - std::fmod(-angle, 2.0f * Pi);
    return std::fmod(angle, 2.0f * Pi);
}

// Angle of a point as seen from a unit's facing: 0 straight ahead, pi straight behind.
inline float RelativeAngle(float angleToPoint, float facing) { return NormalizeAngle(angleToPoint - facing); }

enum class CastKind : uint8_t
{
    None,  // not a spell cast
    Interrupt,
    DamagingTotem,
    Heal,
    Buff,
    Cure,
    Other
};

// What a multiplier needs to know about an action, without the engine types.
struct ActionShape
{
    bool movement = false;  // any movement action, attacks included
    bool attack = false;
    bool avoidAoe = false;
    CastKind cast = CastKind::None;
};

constexpr std::array<std::string_view, 9> InterruptSpells = {"kick",           "counterspell", "pummel",
                                                             "shield bash",    "wind shear",   "mind freeze",
                                                             "arcane torrent", "spell lock",   "silencing shot"};

constexpr std::array<std::string_view, 3> DamagingTotemSpells = {"searing totem", "magma totem",
                                                                 "fire elemental totem"};

constexpr bool IsInterruptSpell(std::string_view spell)
{
    for (std::string_view name : InterruptSpells)
        if (name == spell)
            return true;
    return false;
}

constexpr bool IsDamagingTotemSpell(std::string_view spell)
{
    for (std::string_view name : DamagingTotemSpells)
        if (name == spell)
            return true;
    return false;
}

// Spell names come first: totem drops are buff actions, and interrupts look like any other cast.
constexpr CastKind ClassifyCast(std::string_view spell, bool heal, bool buff, bool cure)
{
    if (IsDamagingTotemSpell(spell))
        return CastKind::DamagingTotem;
    if (IsInterruptSpell(spell))
        return CastKind::Interrupt;
    if (heal)
        return CastKind::Heal;
    if (buff)
        return CastKind::Buff;
    if (cure)
        return CastKind::Cure;
    return CastKind::Other;
}

// Chasing, tank positioning, reaching range: the moves that can walk a bot back into a hazard.
constexpr bool IsRepositioning(ActionShape action) { return action.movement && !action.attack && !action.avoidAoe; }
}  // namespace DungeonTactics

#endif
