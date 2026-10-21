// Pure rules for the Pit of Saron bosses: Garfrost's cover, Ick's nova and orbs, Tyrannus's brand, ground hazards.

#ifndef _PLAYERBOTS_POS_POLICY_H
#define _PLAYERBOTS_POS_POLICY_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include "DungeonTacticsPolicy.h"

namespace PitOfSaron
{
using DungeonTactics::ActionShape;
using DungeonTactics::CastKind;
using DungeonTactics::Pi;

// Ground hazards: Toxic Waste pools (Ick) and Icy Blast patches (Tyrannus)

// A persistent ground effect: centre and radius
struct Hazard
{
    float x, y, radius;
};

constexpr float HazardScanRange = 60.0f;
// Icy Blast lays three 7 yd patches every 15 s that last a minute, so the near ground fills up: search wide
constexpr std::array<float, 8> HazardSteps = {4.0f, 7.0f, 10.0f, 14.0f, 18.0f, 22.0f, 26.0f, 30.0f};
constexpr int HazardDirections = 24;
constexpr float MaxClimb = 3.0f;
constexpr float CastRange = 30.0f;

inline uint32_t HazardsCovering(std::vector<Hazard> const& hazards, float x, float y, float margin)
{
    uint32_t covering = 0;
    for (Hazard const& hazard : hazards)
    {
        float dx = hazard.x - x;
        float dy = hazard.y - y;
        float reach = hazard.radius + margin;
        if (dx * dx + dy * dy < reach * reach)
            ++covering;
    }
    return covering;
}

inline bool InHazard(std::vector<Hazard> const& hazards, float x, float y, float margin)
{
    return HazardsCovering(hazards, x, y, margin) > 0;
}

// The tank drags the boss far enough that his whole body leaves the patch.
constexpr float HazardClearance(bool dragBoss, float bossReach) { return dragBoss ? bossReach + 2.0f : 1.5f; }

// Fewest patches first, then melee stay close to the boss and everyone else takes the shortest step.
constexpr float HazardStepScore(uint32_t covering, bool stayNearBoss, float bossDistance, float step)
{
    float outOfRange = bossDistance > CastRange ? 500.0f : 0.0f;
    return float(covering) * 1000.0f + (stayNearBoss ? bossDistance : step) + outOfRange;
}

// Once out of a pool or patch, stay out: chasing and repositioning walked bots back in.
inline bool StaysOutOfHazards(std::vector<Hazard> const& hazards, float botX, float botY, float bossX, float bossY,
                              bool meleeNotTank)
{
    // A small margin only: with a dozen patches down, a wide one froze bots wherever they stood
    if (InHazard(hazards, botX, botY, 1.5f))
        return false;
    return !meleeNotTank || !InHazard(hazards, bossX, bossY, 0.0f);
}

// Ick and Krick

// Poison Nova reaches 15 yd; bots clear it to this distance
constexpr float PoisonNovaSafeDistance = 18.0f;
// Nova and Pursuit only take over from other actions this close to Ick
constexpr float IckCloseRange = 20.0f;
constexpr float PursuitKeepRunningRange = 18.0f;
constexpr std::array<float, 3> PursuitRadii = {20.0f, 16.0f, 24.0f};
constexpr std::array<float, 3> PursuitSteps = {0.9f, 1.3f, 0.6f};
constexpr float PursuitFallbackDistance = 10.0f;
constexpr float IckTankSpotTolerance = 7.0f;
constexpr float IckTankStep = 3.0f;
constexpr float OrbDangerRange = 7.0f;
constexpr float BarrageStepDistance = 7.0f;
constexpr float BarrageMaxBossDistance = 30.0f;
constexpr float BarrageNeighbourRange = 20.0f;
constexpr int BarrageDirections = 16;

// End up at 18 yd however close we start (a fixed 11 yd step left melee at 14); 0 = already clear.
constexpr float NovaStepOut(float distance)
{
    return distance < PoisonNovaSafeDistance ? PoisonNovaSafeDistance - distance + 1.0f : 0.0f;
}

constexpr bool StepClearsNova(bool nova, float bossDistance) { return !nova || bossDistance >= PoisonNovaSafeDistance; }

// Fixated: run on around the room, on the side away from Ick (angles around the room centre); +1 or -1.
inline float PursuitDirection(float botAngle, float bossAngle)
{
    return DungeonTactics::NormalizeAngle(botAngle - bossAngle) < Pi ? 1.0f : -1.0f;
}

// Spread from other players, capped so one far player cannot outweigh the orbs.
constexpr float PlayerSpacing(float distance) { return std::min(distance, 10.0f); }

// Away from the nearest orb counts double; closer to Ick counts only once clear of the orbs.
constexpr float BarrageSpotScore(float nearestOrb, float spacing, bool hasBoss, float bossDistance)
{
    float score = nearestOrb * 2.0f + spacing;
    if (hasBoss && nearestOrb > OrbDangerRange)
        score += (BarrageMaxBossDistance - bossDistance) * 0.5f;
    return score;
}

enum class IckAction : uint8_t
{
    Other,
    IckAndKrick,
    AvoidHazard
};

struct IckFight
{
    bool novaClose = false;
    bool pursuedClose = false;  // the bot itself is fixated and Ick is close
    bool barrage = false;
    bool healer = false;
};

constexpr bool IckAllows(IckAction action, IckFight fight)
{
    if (action == IckAction::IckAndKrick)
        return true;

    // Leaving a pool may go on through the nova: the hazard step keeps clear of it
    if (fight.novaClose)
        return action == IckAction::AvoidHazard;

    // A cast would stop the kite; pools still get dodged
    if (fight.pursuedClose && action != IckAction::AvoidHazard)
        return false;

    return !fight.barrage || fight.healer;
}

// Scourgelord Tyrannus

constexpr float SpreadRadius = 10.0f;
constexpr float SpreadMaxStep = 3.0f;
constexpr float BrandHealTankHealthPct = 50.0f;

constexpr float SpreadStep(float distance)
{
    return distance < SpreadRadius ? std::min(SpreadMaxStep, SpreadRadius - distance + 1.0f) : 0.0f;
}

// Guides split on a branded healer; the usual compromise: hold heals unless the tank is in danger.
constexpr bool BrandedHealerMayHeal(bool healer, bool hasTank, float tankHealthPct)
{
    return healer && hasTank && tankHealthPct < BrandHealTankHealthPct;
}

// Overlord's Brand copies the branded player's damage onto the tank and their healing onto Tyrannus x5.5.
constexpr bool BrandAllows(ActionShape action, bool mayHeal)
{
    if (action.attack)
        return false;
    if (action.cast == CastKind::Heal && mayHeal)
        return true;
    // Totem drops count as buffs, but a damaging one's hits would be copied onto the tank
    return action.cast == CastKind::None || action.cast == CastKind::Buff;
}

// A cast already under way when the brand lands: damage and held heals are stopped, plain buffs may finish.
constexpr bool BrandStopsCast(bool positive, bool heal, bool mayHeal) { return !positive || (heal && !mayHeal); }

// Forgemaster Garfrost

// Melee DPS duck behind a boulder at this many Permafrost stacks; ticks every 2 s, gone 3 s after the last one
constexpr uint32_t MeleeHideStacks = 8;
// Throw Saronite lands with a 6 yd impact
constexpr float SaroniteAvoidRadius = 7.5f;
constexpr int SaroniteDirections = 16;
constexpr std::array<float, 2> CoverBehindRock = {3.0f, 5.0f};
constexpr float CoverMaxClimb = 4.0f;
constexpr float HealerTankRange = 38.0f;
constexpr float HealerOutOfSightPenalty = 200.0f;
constexpr float TankSpotTolerance = 6.0f;
constexpr float TankWalkStep = 4.0f;

enum class Role : uint8_t
{
    Tank,
    Melee,
    Ranged  // healers included
};

enum class Cover : uint8_t
{
    No,
    Yes,
    WhileShielded  // stay hidden until the stacks are gone, but do not go looking for a boulder
};

constexpr Cover CoverNeed(Role role, uint32_t stacks, bool forging)
{
    // Stomp, leap to the anvil, forge: he is passive and untanked for these seconds
    if (forging)
        return stacks > 0 || role != Role::Tank ? Cover::Yes : Cover::No;
    if (role == Role::Tank)
        return Cover::No;
    if (role == Role::Ranged || stacks >= MeleeHideStacks)
        return Cover::Yes;
    return stacks > 0 ? Cover::WhileShielded : Cover::No;
}

constexpr bool CoverInCastRange(bool ranged, bool forging, float bossDistance)
{
    return !ranged || forging || bossDistance <= CastRange;
}

// Everyone gathers at the healer's boulder (or a close one in the healer's sight) so nobody drops out of heals.
constexpr float CoverScore(float botDistance, float middleDistance, bool hasHealer, float healerDistance,
                           bool healerInSight)
{
    float score = botDistance + middleDistance * 0.5f;
    if (!hasHealer)
        return score;
    return healerDistance * 2.0f + score * 0.5f + (healerInSight ? 0.0f : HealerOutOfSightPenalty);
}

// Walk him back to the middle in small steps so he stays in melee the whole way; 0 = close enough.
constexpr float WalkBackStep(float distanceToMiddle)
{
    return distanceToMiddle < TankSpotTolerance ? 0.0f : std::min(distanceToMiddle, TankWalkStep);
}

// The boulder follows its target: the target walks away from the others, everyone else away from the landing.
constexpr float SaroniteSpotScore(bool isTarget, float botDistance, float nearestOther)
{
    return isTarget ? -nearestOther : botDistance;
}

constexpr bool GarfrostAllows(ActionShape action, bool forging, bool hiding, bool ranged)
{
    if (action.avoidAoe)
        return true;

    // Nobody follows him to the anvil: he walks back to his tank in the middle once the weapon is done
    if (forging && action.movement)
        return false;

    // Behind a boulder: casters keep casting from there, melee and the tank wait out their stacks
    if (hiding && action.attack)
        return ranged;
    return !(hiding && action.movement);
}
}  // namespace PitOfSaron

#endif
