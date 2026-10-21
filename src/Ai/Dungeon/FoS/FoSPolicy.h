// Pure rules for the Forge of Souls bosses: Bronjahm's souls and Soulstorm, Devourer's wells, wail and debuffs.

#ifndef _PLAYERBOTS_FOS_POLICY_H
#define _PLAYERBOTS_FOS_POLICY_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

#include "DungeonTacticsPolicy.h"

namespace ForgeOfSouls
{
using DungeonTactics::ActionShape;
using DungeonTactics::CastKind;
using DungeonTactics::Pi;

// Bronjahm

constexpr float CorruptSoulFleeRange = 10.0f;
constexpr float CorruptSoulFleeDistance = 15.0f;
constexpr float SoulNearBossRange = 10.0f;
constexpr float SoulFleeDistance = 13.0f;
constexpr float TankSpotTolerance = 5.0f;
// The tank steps this far behind the boss when a soul sits between them
constexpr float BehindBossStep = 5.0f;
constexpr float SoulstormMaxStep = 10.0f;

constexpr bool SoulBetweenTankAndBoss(float tankToBoss, float soulToBoss, float soulToTank)
{
    return tankToBoss > soulToBoss && soulToTank < tankToBoss;
}

// Soulstorm hits outside its ring: close in from beyond 10 yd, ranged to 6.5 yd inside, melee to 2; 0 = stay.
constexpr float SoulstormStep(bool ranged, float distanceToBoss, bool soulAlive, bool soulstorm)
{
    if (distanceToBoss <= SoulstormMaxStep || soulAlive || !soulstorm)
        return 0.0f;
    return std::min(distanceToBoss - (ranged ? 6.5f : 2.0f), SoulstormMaxStep);
}

// Tank Assist would drag the tank off Bronjahm; with Corrupt Soul only the step away may move.
constexpr bool BronjahmAllows(bool tankAssist, bool fleeing, bool movement, bool corruptSoul)
{
    if (tankAssist)
        return false;
    return !corruptSoul || !movement || fleeing;
}

// Devourer of Souls

// Well of Souls damage (68863) reaches 4 yards; the rest is margin for the bot's size and the well's drift
constexpr float WellAvoidRadius = 6.0f;
// The sweep reaches 52 degrees either side of his facing and turns up to 90 degrees from where it started,
// so a bot within 35 degrees of straight behind stays clear for the whole wail
constexpr float WailingSafeArc = 70.0f * Pi / 180.0f;
constexpr std::array<float, 5> BehindOffsets = {0.0f, 0.25f, -0.25f, 0.45f, -0.45f};
// Wider rings only when wells cluster: the multiplier blocks every other action while the bot stands in one
constexpr std::array<float, 3> WellExitRadii = {WellAvoidRadius + 1.5f, WellAvoidRadius + 4.0f,
                                                WellAvoidRadius + 7.0f};
constexpr int WellExitPoints = 16;
// Inside the ring the well action steps to, so a bot that stepped out can still chase
constexpr float WellGuardRadius = WellAvoidRadius + 1.0f;
constexpr float MaxClimb = 3.0f;
constexpr uint32_t LongInterruptCooldownMs = 15000;
constexpr int32_t LongInterruptHoldMs = 1000;
constexpr std::array<std::string_view, 7> PhantomBlastInterrupts = {
    "kick", "pummel", "shield bash", "wind shear", "mind freeze", "counterspell", "arcane torrent"};

// relative: the bot's angle from his facing, as DungeonTactics::RelativeAngle gives it.
inline bool InSafeWedge(float relative) { return std::fabs(relative - Pi) < WailingSafeArc / 2.0f; }

inline bool InFrontHalf(float relative) { return relative < Pi / 2.0f || relative > 3.0f * Pi / 2.0f; }

// From the front a straight line behind runs through the cone, so step out to this side first.
inline float SideStepAngle(float relative) { return relative < Pi ? 0.6f * Pi : 1.4f * Pi; }

// Melee keep hitting from behind; ranged keep their distance but move into the same safe wedge.
inline float WailingStandDistance(bool ranged, float distanceToBoss, float combatReach)
{
    return ranged ? std::clamp(distanceToBoss, 8.0f, 20.0f) : std::max(combatReach, 2.0f);
}

// The tank drags him this far from the well under him, so melee can follow without stepping in.
constexpr float WellDragDistance(float combatReach) { return WellAvoidRadius + combatReach + 3.0f; }

// During a wail only spots behind him count; stepping out of a well into the sweep is worse.
inline bool WellExitAllowed(bool wailing, float relative) { return !wailing || InSafeWedge(relative); }

// Long-cooldown interrupts wait for the second half of the 2 s cast so a 10 s Kick can take it first.
constexpr bool HoldForQuickerInterrupt(uint32_t cooldownMs, int32_t castRemainingMs)
{
    return cooldownMs > LongInterruptCooldownMs && castRemainingMs > LongInterruptHoldMs;
}

struct DevourerFight
{
    bool phantomBlast = false;
    bool mirroredSoul = false;
    bool wailingSouls = false;
};

// Whether an action outside the Devourer tactics may run, once the bot is out of any well.
constexpr bool DevourerAllows(ActionShape action, DevourerFight fight)
{
    // Class interrupts fire on any cast: keep them for Phantom Blast (a kicked Wailing Souls leaves him rooted)
    if (action.cast == CastKind::Interrupt)
        return fight.phantomBlast;

    // Damage on him is mirrored onto the debuffed player: heal, buff and cure only
    if (fight.mirroredSoul)
    {
        if (action.attack || action.cast == CastKind::DamagingTotem || action.cast == CastKind::Other)
            return false;
    }

    // Any other movement could walk a bot into the sweep
    return !(fight.wailingSouls && action.movement);
}
}  // namespace ForgeOfSouls

#endif
