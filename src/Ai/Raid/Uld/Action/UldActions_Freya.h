/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_FREYA_H
#define PLAYERBOTS_ULDACTIONS_FREYA_H

#include <vector>

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Freya.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class FreyaMoveAwayNatureBombAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya move away nature bomb";

    FreyaMoveAwayNatureBombAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // The trigger fires at AVOID but the escape aims at CLEAR, so re-deriving each tick gives a short hop
    // that combat movement undoes. Hold the first answer until it's unsafe or the bomb is gone.
    Position bombSpot;
    uint32 bombSpotMs = 0;
    // Where the bot fled from. A melee bot keeps its spot while this is still inside a blast, which is
    // what stops reach melee marching it back with most of the fuse left to run.
    Position bombOrigin;
};

// Main tank only: walks Freya off the bombs so her melee ring is somewhere the rest of the melee can
// stand. Nothing else in the encounter ever moves the boss.
class FreyaTankNatureBombAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya tank nature bomb";

    FreyaTankNatureBombAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Position kiteSpot;
    uint32 kiteSpotMs = 0;
};

// Owns every DPS bot's target: Eonar's Gift > Ancient Conservator > trio slot > Detonating Lashers >
// Freya, with the trio moved up once leaving it would let a member revive.
class FreyaSetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "freya set dps priority";

    FreyaSetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Unit* ResolveFreyaDpsTarget(Unit* currentTarget);

    // Whether this bot is one of the few sent at Eonar's Gift while a lasher pack is up.
    bool TakesEonarsGift(FreyaWaveState const& state);

    // The trio pick, moved to a sibling while this bot is too close to the tank's threat on the Snaplasher.
    // Damage moves, none is held back: with no sibling left to hit the pick stands.
    Unit* ShedSnaplasherThreat(FreyaWaveState const& state, Unit* assigned);

    // Which Gift the share is timing, and when this bot first saw it.
    ObjectGuid giftGuid;
    uint32 giftSeenMs = 0;

    // Which Snaplasher the shed is timing, when this bot first saw it, and the last answer.
    ObjectGuid snaplasherGuid;
    uint32 snaplasherSeenMs = 0;
    bool snaplasherShed = false;
};

// Main tank holds Freya; assist tank 0 works down the add ladder and, on the Conservator, walks it onto
// a Healthy Spore so the melee sheltering there can still reach it.
class FreyaTankAddsAction : public AttackAction
{
public:
    static constexpr char const* Name = "freya tank adds";

    FreyaTankAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool ParkConservator(Unit* conservator);

    // Latched for the spore's whole life: new spores keep appearing 20 yd from wherever the Conservator
    // is, so re-deriving each tick can flip the destination mid-walk.
    ObjectGuid parkedSpore;
};

class FreyaMoveToHealingSporeAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya move to healing spore action";

    FreyaMoveToHealingSporeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Feeds Misdirection / Tricks to the tank holding what the raid is hitting. Useless on Detonating
// Lashers, which wipe their threat every 10s.
class FreyaRedirectThreatAction : public Action
{
public:
    static constexpr char const* Name = "freya redirect threat";

    FreyaRedirectThreatAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // dumpTarget is what that tank holds, so the redirected shots land on the table they are meant for.
    Player* GetRedirectTank(Unit*& dumpTarget);
};

// The walk out of the blasts of lashers about to detonate, shared by the melee bail and the back line's.
class FreyaLasherBailAction : public MovementAction
{
protected:
    FreyaLasherBailAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

    // Nearest spot `clearance` from every blast, settling for just outside them when the pile leaves no such spot.
    bool StepOutOfBlasts(std::vector<Position> const& blasts, float clearance);

private:
    // Lashers die one by one, so re-deriving the exit each tick moves it a yard and restarts the walk.
    // Hold the first answer until it stops being clear or the bot arrives.
    Position bailSpot;
    uint32 bailSpotMs = 0;
};

// Steps melee out of the blast of lashers about to detonate. Only those: clearing the whole pack parks
// melee outside the fight.
class FreyaLasherAboutToBlowAction : public FreyaLasherBailAction
{
public:
    static constexpr char const* Name = "freya lasher about to blow";

    FreyaLasherAboutToBlowAction(PlayerbotAI* botAI) : FreyaLasherBailAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Balls the ranged half and the healers up on the anchor bot so the lashers pile onto one spot the raid can
// AoE, and walks each of them out of the blasts once lashers get low. A player cannot outrun the healthy pack.
class FreyaRangedCampAction : public FreyaLasherBailAction
{
public:
    static constexpr char const* Name = "freya ranged camp";

    FreyaRangedCampAction(PlayerbotAI* botAI) : FreyaLasherBailAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Root whatever has closed on the mage. Above the trap so a hunter standing next to it drops the patch
// under an already-rooted pair rather than under one still moving.
class FreyaFrostNovaLashersAction : public Action
{
public:
    static constexpr char const* Name = "freya frost nova lashers";

    FreyaFrostNovaLashersAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Snare what is running at this hunter. -50% takes a lasher from 8.0 yd/s to 4.0, under a player's
// 7.0, which is the only thing on the encounter that makes one of them slower than the raid.
class FreyaTrapLashersAction : public Action
{
public:
    static constexpr char const* Name = "freya trap lashers";

    FreyaTrapLashersAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Eight ghouls with an AoE taunt, for the wave nobody can tank. Not named "army of the dead": that name
// is on IsBurstCooldownAction's list, which the Ulduar burst gate holds until Attuned to Nature drops.
class FreyaSummonArmyAction : public Action
{
public:
    static constexpr char const* Name = "freya summon army";

    FreyaSummonArmyAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode: stop a cast Ground Tremor would cut. EffectInterruptCast only applies the 10s school
// lockout when it finds a cast to cut, so stopping first dodges it.
class FreyaGroundTremorHoldCastAction : public Action
{
public:
    static constexpr char const* Name = "freya ground tremor hold cast";

    FreyaGroundTremorHoldCastAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode: kill the Iron Roots creature trapping the bot, its death removes the root DoT.
class FreyaBreakIronRootsAction : public AttackAction
{
public:
    static constexpr char const* Name = "freya break iron roots";

    FreyaBreakIronRootsAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Hard mode: step out of an Unstable Sun Beam before it detonates.
class FreyaDodgeUnstableSunBeamAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya dodge unstable sun beam";

    FreyaDodgeUnstableSunBeamAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // Beams keep spawning, so re-deriving each tick nudges the spot, and every MoveTo clears the motion
    // master: the bot never gets anywhere. Hold the first answer until it's unsafe or the bot arrives.
    Position dodgeSpot;
    uint32 dodgeSpotMs = 0;
};

// Carry Nature's Fury out of the raid, onto another Healthy Spore if one's free: shelter groups sit far
// enough apart to clear the 8 yd splash, and the carrier stays out of the Grip.
class FreyaNaturesFuryBailAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya nature fury bail";

    FreyaNaturesFuryBailAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    // Allies drift, so re-deriving every tick answers with a spot a yard along and clears the motion
    // master doing it. Hold the first answer until the raid catches up with it or the bot arrives.
    Position bailSpot;
    uint32 bailSpotMs = 0;
};

// Step out of the circle Freya's Sunbeam is about to drop on a neighbour.
class FreyaStepOutOfSunbeamAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya step out of sunbeam";

    FreyaStepOutOfSunbeamAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Position stepSpot;
    uint32 stepSpotMs = 0;
};

// Main tank only: walk Freya back to her anchor. She follows whoever holds her, so the tank's own feet
// are the only thing that moves her.
class FreyaTankHoldFreyaAction : public MovementAction
{
public:
    static constexpr char const* Name = "freya tank hold freya";

    FreyaTankHoldFreyaAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    uint32 holdMs = 0;
};

#endif
