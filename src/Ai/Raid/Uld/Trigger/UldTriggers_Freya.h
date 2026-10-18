/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_FREYA_H
#define PLAYERBOTS_ULDTRIGGERS_FREYA_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

class FreyaNearNatureBombTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya near nature bomb";

    FreyaNearNatureBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The main tank, who walks Freya off a bomb field instead of standing in it. Bombs land at players'
// feet and the melee stack is on the boss, so volleys bury her melee ring.
class FreyaTankNatureBombTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya tank nature bomb";

    FreyaTankNatureBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Owns every DPS bot's target for the whole encounter, so the trio wave can be split three ways.
class FreyaSetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya set dps priority";

    FreyaSetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The main tank and assist tank 0, who both always have something on GetFreyaTankTarget's ladder.
class FreyaTankAddsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya tank adds";

    FreyaTankAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hunters and rogues, for as long as Freya is up. The action decides which tank the redirect goes to.
class FreyaRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya redirect threat";

    FreyaRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FreyaMoveToHealingSporeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya move to healing spore trigger";

    FreyaMoveToHealingSporeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode: bot is trapped by Iron Roots (Ironbranch or Freya-cast) and must break out.
class FreyaBreakIronRootsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya break iron roots";

    FreyaBreakIronRootsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A Detonating Lasher next to this bot is about to blow. Melee only: they're the ones on the pile, and
// a lasher under the bail line still leaves them time to clear its 15 yd blast.
class FreyaLasherAboutToBlowTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya lasher about to blow";

    FreyaLasherAboutToBlowTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Ranged and healers hold one camp a fixed standoff from the pack, so the lashers walk themselves into
// a pile to AoE. Nobody leads them: a lasher runs 8.0 yd/s against a player's 7.0.
class FreyaRangedCampTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya ranged camp";

    FreyaRangedCampTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Frost Nova holds the full 8s: no AURA_INTERRUPT_FLAG_TAKE_DAMAGE, so damage doesn't break it, and no
// CREATURE_FLAG_EXTRA_ALL_DIMINISH on the lasher, so no DR either.
class FreyaFrostNovaLashersTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya frost nova lashers";

    FreyaFrostNovaLashersTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Every hunter snares what is running at it. 30s patch on a 30s cooldown, so with two hunters the
// -50% is up for the whole wave.
class FreyaTrapLashersTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya trap lashers";

    FreyaTrapLashersTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Army of the Dead on the lasher wave: the ghouls AoE-taunt through 43263, which filters only world
// bosses, and that's worth more than their damage.
class FreyaSummonArmyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya summon army";

    FreyaSummonArmyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode: Freya is 2s into Ground Tremor and this bot has a cast in flight that it would eat, along
// with the 10s school lockout that comes with being cut.
class FreyaGroundTremorHoldCastTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya ground tremor hold cast";

    FreyaGroundTremorHoldCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Hard mode: bot is standing in an Unstable Sun Beam and must step out before it detonates.
class FreyaDodgeUnstableSunBeamTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya dodge unstable sun beam";

    FreyaDodgeUnstableSunBeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The Nature's Fury carrier, with raid members still inside the splash. Five ticks over 10s, 8 yd
// around the carrier each, so standing in the ball is five volleys into it.
class FreyaNaturesFuryBailTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya nature fury bail";

    FreyaNaturesFuryBailTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A ranged bot or healer next to Freya's Sunbeam target. The beam lands where the target stands when
// the cast ends, so only its neighbours can dodge it.
class FreyaStepOutOfSunbeamTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya step out of sunbeam";

    FreyaStepOutOfSunbeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Freya has been walked off her anchor. Whoever is holding her walks her back.
class FreyaTankHoldFreyaTrigger : public Trigger
{
public:
    static constexpr char const* Name = "freya tank hold freya";

    FreyaTankHoldFreyaTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
