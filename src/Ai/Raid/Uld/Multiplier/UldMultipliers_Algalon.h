/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDMULTIPLIERS_ALGALON_H
#define PLAYERBOTS_ULDMULTIPLIERS_ALGALON_H

#include "Define.h"
#include "Multiplier.h"

#include <unordered_map>

// Algalon: catches damage already aimed at a star, constellation or Dark Matter that isn't the bot's job.
// Heals, movement and the pickers that would switch the bot back all pass.
class AlgalonTargetGuardMultiplier : public Multiplier
{
public:
    AlgalonTargetGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon target guard") {}
    float GetValue(Action* action) override;

private:
    std::unordered_map<Action*, bool> damagesCurrentTarget;
    // No DoT on a star: it keeps ticking once the team moves on, past an under-star's band or a paced one's gap.
    // A channel stops when it's broken, and a nuke with a small DoT rider (Fireball) is a nuke.
    std::unordered_map<Action*, bool> lingers;
};

// Algalon: keeps Dispersion, and with no disperser the tank soak buttons, for Big Bang unless back in time. The
// holder's own and the externals on him are the tank defensive node's; the encounter's own actions cast directly.
class AlgalonCooldownReserveMultiplier : public Multiplier
{
public:
    AlgalonCooldownReserveMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon cooldown reserve") {}
    float GetValue(Action* action) override;

private:
    struct Reserve
    {
        uint32 spellId = 0;
        bool bubble = false;
        bool dispersion = false;
        bool swapTanksOnly = false;
        // A bubble or a self Lay on Hands puts Forbearance on the tank, which blocks Divine Protection.
        bool alwaysForSwapTanks = false;
        // Released to a holder nobody can take him from: its own button, or one cast on it.
        bool holderOwn = false;
        bool holderExternal = false;
    };

    std::unordered_map<Action*, Reserve> reserves;
};

// Algalon: a bot only thinks again once its cast ends, so during Big Bang a cast or channel longer than its
// hide slack would start its run late.
class AlgalonHideCastMultiplier : public Multiplier
{
public:
    AlgalonHideCastMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon hide cast") {}
    float GetValue(Action* action) override;

private:
    std::unordered_map<Action*, uint32> spellIds;
};

// Algalon: a tank healer heals the swap pair and itself, nobody else. Its class rotation still runs whenever
// that's who it would heal; party heals and heals on anyone else are left to the raid's other healers.
class AlgalonTankHealerMultiplier : public Multiplier
{
public:
    AlgalonTankHealerMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon tank healer focus") {}
    float GetValue(Action* action) override;

private:
    enum class Heal : uint8
    {
        None,
        Party,
        Targeted,
    };

    std::unordered_map<Action*, Heal> heals;
};

// Algalon: stars die one at a time on purpose, each death 16-21k to the whole raid, and splash kills one on
// its own clock. The threat type misses most of it, so the spell and the name decide.
class AlgalonSplashHoldMultiplier : public Multiplier
{
public:
    AlgalonSplashHoldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon splash hold") {}
    float GetValue(Action* action) override;

private:
    enum class Splash : uint8
    {
        None,
        // Self auras that hit whatever comes near for their whole duration.
        Always,
        Near,
        // Ground AoE that sits there for 8-10 s while stars wander into it, dropped at the feet or at the target.
        GroundAtBot,
        GroundAtTarget,
    };

    std::unordered_map<Action*, Splash> splashes;
};

// Algalon: reach, set behind and the healers' reach know nothing of the black holes, and a hole beside him
// is where melee stand. A bot that would stop at a field's edge or walk through one waits instead.
class AlgalonWalkGuardMultiplier : public Multiplier
{
public:
    AlgalonWalkGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon walk guard") {}
    float GetValue(Action* action) override;
};

// Algalon: a phased bot still in a field when Big Bang strips the phase a second after the hit is phased again.
// A cast pins the feet and an item use stops the walk, so only the exit runs; Exclusive rules hold movement only.
class AlgalonLeaveFieldMultiplier : public Multiplier
{
public:
    AlgalonLeaveFieldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "algalon leave field") {}
    float GetValue(Action* action) override;
};

#endif
