/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40RULES_H
#define PLAYERBOTS_AQ40RULES_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace Aq40Rules
{
constexpr float Pi = 3.14159265358979323846f;

inline float AngleDelta(float to, float from) { return std::remainder(to - from, 2.0f * Pi); }

// Dark Glare hits anyone in front of the Eye less than 5 yd from its facing line (spell_cthun_dark_glare ->
// HasInLine), and the line turns pi/35 per 1-second tick. Direction 0 means not known yet.
constexpr float GlareStep = Pi / 35.0f;

inline bool InGlareBand(float bearing, float distance, float line, float halfWidth)
{
    float relative = AngleDelta(bearing, line);
    return std::abs(relative) < Pi / 2.0f && std::abs(std::sin(relative)) * distance < halfWidth;
}

// In the band now or within the next `ticks` ticks (both ways while the direction is unknown).
inline bool InGlarePath(float bearing, float distance, float facing, float direction, float halfWidth, int ticks)
{
    for (int tick = 0; tick <= ticks; ++tick)
    {
        if (direction != 0.0f && InGlareBand(bearing, distance, facing + direction * GlareStep * tick, halfWidth))
            return true;
        if (direction == 0.0f && (InGlareBand(bearing, distance, facing + GlareStep * tick, halfWidth) ||
                                  InGlareBand(bearing, distance, facing - GlareStep * tick, halfWidth)))
            return true;
    }
    return false;
}

// Bearing (around the Eye) to step to: ahead of the sweep for anyone it is coming toward, behind it for
// anyone it is leaving, and away from the line on the bot's own side while the direction is unknown.
inline float GlareEscapeBearing(float bearing, float distance, float facing, float direction, float halfWidth,
                                int ticks)
{
    float relative = AngleDelta(bearing, facing);
    float side = relative >= 0.0f ? 1.0f : -1.0f;
    bool ahead = direction != 0.0f && relative * direction > 0.0f;
    if (ahead)
        side = direction;
    float clear = std::asin(std::min(1.0f, halfWidth / std::max(distance, halfWidth)));
    float lead = direction == 0.0f || ahead ? GlareStep * float(ticks + 1) : GlareStep;
    return facing + side * (clear + lead);
}

inline bool InGlareDanger(float relative, float direction)
{
    relative = AngleDelta(relative, 0.0f);
    return std::abs(relative) < 0.65f ||
           (direction != 0.0f && relative * direction > 0.0f && relative * direction < 1.1f);
}

inline float GlareEscapeDirection(float relative, float direction)
{
    relative = AngleDelta(relative, 0.0f);
    return std::abs(relative) < 0.65f ? (relative >= 0.0f ? 1.0f : -1.0f) : direction;
}

// C'Thun raid spots, offsets (x = north, y = west) from the Eye's center. Eye Beam jumps to the closest player within
// ~13 yd (10 yd plus both combat reaches) at x1.5 damage, so the 40 spots fit the walkable floor (z 100.7) with no
// chain longer than two at 13.5 yd. Four melee stand on the Eye at N/W/S/E (inside its 17.8-yd melee range); the middle
// ring (30-42 yd) keeps ranged in reach (30-yd spells reach 46.5 yd from its center); healers take the outer ring.
enum class CthunSlotKind : uint8_t
{
    EyeMelee,
    Middle,
    Outer
};

struct CthunSlotSpot
{
    float x;
    float y;
    CthunSlotKind kind;
};

inline constexpr CthunSlotSpot CthunSlots[] = {
    {16.5f, 0.0f, CthunSlotKind::EyeMelee},  {0.0f, 16.5f, CthunSlotKind::EyeMelee},
    {-16.5f, 0.0f, CthunSlotKind::EyeMelee}, {0.0f, -16.5f, CthunSlotKind::EyeMelee},
    {-42.0f, -0.4f, CthunSlotKind::Middle},  {-28.1f, -13.1f, CthunSlotKind::Middle},
    {-28.9f, -30.5f, CthunSlotKind::Middle}, {-14.3f, -26.4f, CthunSlotKind::Middle},
    {-12.5f, -40.1f, CthunSlotKind::Middle}, {2.9f, -34.4f, CthunSlotKind::Middle},
    {16.7f, -38.6f, CthunSlotKind::Middle},  {20.6f, -21.8f, CthunSlotKind::Middle},
    {34.5f, -24.0f, CthunSlotKind::Middle},  {31.8f, -7.6f, CthunSlotKind::Middle},
    {42.0f, 0.1f, CthunSlotKind::Middle},    {26.7f, 13.7f, CthunSlotKind::Middle},
    {33.0f, 26.0f, CthunSlotKind::Middle},   {15.1f, 25.9f, CthunSlotKind::Middle},
    {12.5f, 40.1f, CthunSlotKind::Middle},   {-1.5f, 42.0f, CthunSlotKind::Middle},
    {-14.0f, 39.6f, CthunSlotKind::Middle},  {-20.7f, 21.8f, CthunSlotKind::Middle},
    {-30.7f, 28.7f, CthunSlotKind::Middle},  {-31.2f, 8.6f, CthunSlotKind::Middle},
    {-62.9f, -11.7f, CthunSlotKind::Outer},  {-48.3f, -16.7f, CthunSlotKind::Outer},
    {-45.0f, -30.7f, CthunSlotKind::Outer},  {10.0f, -53.0f, CthunSlotKind::Outer},
    {32.1f, -47.0f, CthunSlotKind::Outer},   {50.1f, -35.0f, CthunSlotKind::Outer},
    {61.5f, -17.7f, CthunSlotKind::Outer},   {55.5f, -5.0f, CthunSlotKind::Outer},
    {43.0f, 15.5f, CthunSlotKind::Outer},    {34.5f, 50.0f, CthunSlotKind::Outer},
    {12.8f, 62.7f, CthunSlotKind::Outer},    {-11.0f, 62.8f, CthunSlotKind::Outer},
    {-32.5f, 51.0f, CthunSlotKind::Outer},   {-45.5f, 33.0f, CthunSlotKind::Outer},
    {-47.0f, 20.6f, CthunSlotKind::Outer},   {-61.0f, 9.9f, CthunSlotKind::Outer},
};

// Leave the stomach at 6 stacks (9 with a healer inside) or below 55% health: leaving earlier lets the Flesh Tentacles
// live for minutes, leaving later kills bots on the exit pad during its 3-second wait.
inline bool LeaveStomach(uint32_t stacks, float healthPct, bool fleshAlive, bool healerInside)
{
    return stacks >= (healerInside ? 9u : 6u) || healthPct < 55.0f || !fleshAlive;
}
}  // namespace Aq40Rules

#endif
