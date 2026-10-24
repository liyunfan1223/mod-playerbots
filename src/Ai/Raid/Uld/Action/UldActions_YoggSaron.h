/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_YOGGSARON_H
#define PLAYERBOTS_ULDACTIONS_YOGGSARON_H

#include "Action.h"
#include "AttackAction.h"
#include "EncounterHelpers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidAntiFear.h"
#include "UldEncounter_YoggSaron.h"
#include "UldTriggers.h"
#include "Vehicle.h"

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

class YoggSaronGuardianPositioningAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron guardian positioning action";

    YoggSaronGuardianPositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronSanityAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron sanity action";

    YoggSaronSanityAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronMaladyOfTheMindAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    static constexpr char const* Name = "yogg-saron malady of the mind action";

    YoggSaronMaladyOfTheMindAction(PlayerbotAI* botAI)
        : MoveAwayFromPlayerWithDebuffAction(botAI, Name, SPELL_MALADY_OF_THE_MIND,
                                             ULDUAR_YOGG_SARON_MALADY_SPREAD_RADIUS)
    {
    }
};

// Latch and sweep shared by the two spacing nodes. One node per phase: two at the same relevance cannot share
// a bot, the engine ends the tick at the first action returning true, so a pair would trade ticks.
class YoggSaronSpacingAction : public MovementAction
{
public:
    YoggSaronSpacingAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    struct HazardSet
    {
        std::vector<EncounterHelpers::HazardCircle> hazards;

        // The retry, swept when nothing satisfies everything at once. Empty means no retry, the bot stays put.
        // fallbackClear is the retry's own shape test, null to drop it.
        std::vector<EncounterHelpers::HazardCircle> fallback;
        std::function<bool(float, float)> fallbackClear;

        // Shapes a circle cannot describe, like the Crush wedge. True means the spot is safe.
        std::function<bool(float, float)> clear;

        // Where the bot would rather end up. Tried first, dropped when nothing satisfies both, so it can never leave
        // a bot in a hazard. Null means the first spot that clears everything will do.
        std::function<bool(float, float)> preferred;
    };

    // False skips the tick outright.
    virtual bool Collect(HazardSet& set) = 0;
    virtual float SearchRadius() const = 0;

    // How far out a destination may sit from the body. The load-bearing half of the dodge: too tight
    // and a bot needing a sidestep has every outward candidate rejected.
    virtual float MaxFromMiddle() const { return ULDUAR_YOGG_SARON_SPACING_MAX_FROM_MIDDLE; }

    // What MaxFromMiddle is measured against, and what the sweep biases toward. The boss platform for
    // both phases that happen on it; an illusion room is 93 yd below it and its own middle away.
    virtual Position Anchor() const { return ULDUAR_YOGG_SARON_MIDDLE; }

    // Whether the walk to a candidate is acceptable, not just the candidate itself. No opinion by
    // default: for most hazards crossing one to leave another still beats standing still.
    virtual bool RouteAcceptable(float /*x*/, float /*y*/) const { return true; }

    // Whether this tick's dodge may cut into a walk in flight. A forced walk holds off every other forced move
    // until it arrives. Asked after Collect.
    virtual bool OverridesWalkInFlight() const { return false; }

    // Whether the rest of the walk to the held spot is still fine, asked each tick the spot is held.
    // Only for hazards that move onto the route rather than the spot.
    virtual bool HeldRouteAcceptable(float /*x*/, float /*y*/) const { return true; }

private:
    // Held destination. The hazards move (clouds orbit at 3 yd/s, a Crusher re-faces its victim), so a fresh
    // sweep every tick answers a different question and the bot never arrives.
    Position heldSpot;
    uint32 heldSpotMs = 0;
};

class YoggSaronPhase1SpacingAction : public YoggSaronSpacingAction
{
public:
    static constexpr char const* Name = "yogg-saron phase 1 spacing action";

    YoggSaronPhase1SpacingAction(PlayerbotAI* botAI) : YoggSaronSpacingAction(botAI, Name) {}

protected:
    bool Collect(HazardSet& set) override;
    float SearchRadius() const override { return ULDUAR_YOGG_SARON_SPACING_SEARCH_RADIUS; }
    bool RouteAcceptable(float x, float y) const override;

private:
    // Where the clouds stood on this tick's Collect. Their 14 yd clear circles are already in the
    // hazard set; this is for the tighter summon radius the walk itself has to miss.
    std::vector<Position> clouds;
};

// Ranged and healers hold one spot on the second orbit. Everyone walks to the same point, as the
// phase 3 stations do; the trigger's stack radius is what stops the re-issue and shapes the blob.
class YoggSaronPhase1StationAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron phase 1 station action";

    YoggSaronPhase1StationAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronPhase2SpacingAction : public YoggSaronSpacingAction
{
public:
    static constexpr char const* Name = "yogg-saron phase 2 spacing action";

    YoggSaronPhase2SpacingAction(PlayerbotAI* botAI) : YoggSaronSpacingAction(botAI, Name) {}

protected:
    bool Collect(HazardSet& set) override;
    float SearchRadius() const override { return ULDUAR_YOGG_SARON_P2_SPACING_SEARCH_RADIUS; }
    float MaxFromMiddle() const override { return ULDUAR_YOGG_SARON_P2_SPACING_MAX_FROM_MIDDLE; }
    bool RouteAcceptable(float x, float y) const override;
    bool OverridesWalkInFlight() const override { return inDeathRay; }
    bool HeldRouteAcceptable(float x, float y) const override;

private:
    // Death Rays only: they move onto a bot and kill in three ticks. Letting a Crush wedge or the body
    // ring cut in as well would have the detour and the dodge trade every tick.
    bool inDeathRay = false;

    // Where the live rays stood on this tick's Collect, for the route check.
    std::vector<Position> rays;
};

// Stand so facing the tentacle faces away from the skulls. Facing can't be held (set facing, AttackAction and
// CastSpell turn the bot back in the same tick), so the lever is which side of the tentacle it fights from.
class YoggSaronIllusionFacingAction : public YoggSaronSpacingAction
{
public:
    static constexpr char const* Name = "yogg-saron illusion facing action";

    YoggSaronIllusionFacingAction(PlayerbotAI* botAI)
        : YoggSaronSpacingAction(botAI, Name)
    {
    }

protected:
    bool Collect(HazardSet& set) override;
    float SearchRadius() const override { return ULDUAR_YOGG_SARON_ILLUSION_FACING_SEARCH_RADIUS; }
    float MaxFromMiddle() const override { return ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS; }
    Position Anchor() const override { return roomMiddle; }

private:
    // Filled by Collect, which is what decides the room the bot is standing in. Anchor() is called
    // after it and is const, so it cannot work that out for itself.
    Position roomMiddle;
};

// Walk around the body, not through it. ReachCombatTo shortens its path to halfway, so a target on Yogg's
// far side puts the destination on top of him.
class YoggSaronBodyDetourAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron body detour action";

    YoggSaronBodyDetourAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Keep pets and guardians out of a Crusher Tentacle's melee range. Melee bots can't target a Crusher,
// so a pet is the only thing that can put one there.
class YoggSaronPetGuardAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron pet guard action";

    YoggSaronPetGuardAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;

private:
    void Hush(Creature* pet);

    // Give a pet its stance back, once it is clear and only if this node took it away.
    void Unhush(Creature* pet);

    // Pets held passive while a Crusher is in play for them, with the stance each had before.
    std::unordered_map<ObjectGuid, ReactStates> hushed;
};

// One owner of every non-tank's target for the whole encounter, so no raid icon is needed.
class YoggSaronSetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "yogg-saron set dps priority action";

    YoggSaronSetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    // Kill order for wherever the bot is standing, as a tier index. npos means "not a target here".
    // `beaconHealFirst` puts a leftover a mark's heal reaches at the top, see YoggSaronBeaconHealReaches.
    static size_t TierOf(Unit* unit, bool brainLevel, bool phaseThreeRanged = false, bool beaconHealFirst = false);

    // Whether the Brain is open to hit. Only worked out once a Brain turns up, most calls never meet one.
    struct BrainApproach
    {
        bool brainLevel = false;
        bool phaseThreeRanged = false;
        std::optional<bool> approachable;
        // Filled on first use: most resolves never test a Guardian or a leftover tentacle.
        std::optional<bool> phaseThreeMelee;
        std::optional<bool> thorim;
        std::optional<bool> gaze;
    };

    bool IsAllowedTarget(Unit* candidate, BrainApproach& brain) const;
    Unit* ResolveTarget(Unit* currentTarget);

    // An illusion room's tentacles, one bot each while any is untaken. Nearest-first sends the whole
    // team, which walks in as one pack, to the same tentacle every time. nullptr when none is allowed.
    Unit* SpreadTarget(std::vector<Unit*> const& candidates, Unit* currentTarget, BrainApproach& brain);

    // Stops the bot, and any of its pets, hitting `target`. The healer's cast in progress is a heal,
    // so it keeps it.
    void DropTarget(Unit* target, bool interruptCasts = true);
};

class YoggSaronDarkVolleyInterruptAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron dark volley interrupt action";

    YoggSaronDarkVolleyInterruptAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;

private:
    bool CastClassInterrupt(Unit* target);
    int32 GetInterrupterIndex();
};

// A paladin's Judgement breaks a Crusher's Diminish Power channel from outside its swing. Each break
// buys 1.5-3 s without the raid-wide -21%, once per 10 s cooldown.
class YoggSaronDiminishPowerJudgementAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron diminish power judgement action";

    YoggSaronDiminishPowerJudgementAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Mortal Strike's -50% halves both Guardian heals, the beacon's 37.5k ticks and Drain Life's leech, and on
// Yogg it cuts nothing.
class YoggSaronMortalStrikeAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron mortal strike action";

    YoggSaronMortalStrikeAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Warlock pets are only summoned out of combat, so a warlock back from a death before phase 3 has none for the rest
// of the fight. A felhunter's Spell Lock is one of the raid's Drain Life kicks.
class YoggSaronWarlockPetAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron warlock pet action";

    YoggSaronWarlockPetAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// The TankAssist strategy swap one designated bot tank does for the raid at the start of phase 3.
// TankFace goes with it: it would turn the tank back into Lunatic Gaze, which the face-away answers.
class YoggSaronPhase3ControlAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 control action";

    YoggSaronPhase3ControlAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronBrainLinkAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron brain link action";

    YoggSaronBrainLinkAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronMoveToEnterPortalAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron move to enter portal action";

    YoggSaronMoveToEnterPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronFallFromFloorAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron fall from floor action";

    YoggSaronFallFromFloorAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Following a master is wrong in every part of this fight, and a bot that follows a human down to the brain
// level idles behind them there.
class YoggSaronStopFollowingAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron stop following action";

    YoggSaronStopFollowingAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronUsePortalAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron use portal action";

    YoggSaronUsePortalAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronIllusionRoomAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron illusion room action";

    YoggSaronIllusionRoomAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    bool SetRtiMark(YoggSaronTrigger yoggSaronTrigger);
    bool GoToBrainRoom(YoggSaronTrigger yoggSaronTrigger);
    bool WalkIntoRoom();
    bool WalkToStage();
};

class YoggSaronIllusionHealerStationAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron illusion healer station action";

    YoggSaronIllusionHealerStationAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronReprisalRegroupAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron reprisal regroup action";

    YoggSaronReprisalRegroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronBrainSpotAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron brain spot action";

    YoggSaronBrainSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Uses the boots and lets the walk go on in the same tick.
class YoggSaronNitroBoostsAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron nitro boosts action";

    YoggSaronNitroBoostsAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Dash, and the walk goes on in the same tick.
class YoggSaronBrainDashAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron brain dash action";

    YoggSaronBrainDashAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronBrainChargeAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron brain charge action";

    YoggSaronBrainChargeAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronMoveToExitPortalAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron move to exit portal action";

    YoggSaronMoveToExitPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Face away from the Laughing Skulls in the bot's front arc. The skull cannot be killed and its gaze
// picks targets by facing alone, so this is the only defence the room has.
class YoggSaronLaughingSkullAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron laughing skull action";

    YoggSaronLaughingSkullAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronLunaticGazeAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron lunatic gaze action";

    YoggSaronLunaticGazeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Taunts its Shadow Beacon marked Guardian off the pile and walks it to the nearer drag spot, holding
// it there until the heal has landed. Attacks only once standing: the turn to face would end the walk.
class YoggSaronBeaconDragAction : public AttackAction
{
public:
    static constexpr char const* Name = "yogg-saron beacon drag action";

    YoggSaronBeaconDragAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    // A hunter pulls with Distracting Shot instead of a taunt and a walk, and holds its fire.
    bool HunterDrag(Unit* guardian);
    // After the heal: walk the Guardian back toward the tank, or a hunter feigns it off.
    bool ReturnHealed();
};

// Walks a dragger staged for the next Shadow Beacon to its staging spot round the body and holds it there
// until the first mark. No swing: reach melee would walk it straight back to Yogg.
class YoggSaronBeaconStageAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron beacon stage action";

    YoggSaronBeaconStageAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Dire Bear Form, or Bear Form for a druid that hasn't learned it.
class YoggSaronBeaconBearFormAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron beacon bear form action";

    YoggSaronBeaconBearFormAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronPhase3LustAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 lust action";

    YoggSaronPhase3LustAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronPhase3PositioningAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 positioning action";

    YoggSaronPhase3PositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Tank holds a stack spot and taunts loose Guardians onto it. Phase 3: the raid cleaves Immortal Guardians to
// Weakened for Titanic Storm. Phase 1: a Guardian's death location is the phase.
class YoggSaronGuardianControlAction : public AttackAction
{
public:
    static constexpr char const* Name = "yogg-saron guardian control action";

    YoggSaronGuardianControlAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}

    bool Execute(Event event) override;

private:
    bool ControlPhaseOne();
    bool ControlPhaseThree();
    bool Taunt(Unit* guardian);
    // Righteous Defense on whoever the Guardian is hitting, for when the taunt is down.
    bool Defend(Unit* guardian);
    // The swap: Righteous Defense on the holder, then a taunt on the highest Guardian still on it.
    bool TakeFrom(Player* holder);
};

// Reduced-Keeper hard mode: retreat to a safe ranged spot and face away from Yogg to conserve sanity.
class YoggSaronSanityConservationAction : public MovementAction
{
public:
    static constexpr char const* Name = "yogg-saron sanity conservation action";

    YoggSaronSanityConservationAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
};

// Hand of Protection on somebody else's Constrictor. The main tank is rescued too (held, he holds nothing), and
// Forbearance locking his own Divine Shield and Lay on Hands for 2 min is the deliberate price.
class YoggSaronSqueezeRescueAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron squeeze rescue action";

    YoggSaronSqueezeRescueAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Mitigate the next Shadow Nova instead of running from it: melee have to stand in the blast to kill a
// Guardian at all, and one nova is most of a melee bot's health bar.
class YoggSaronNovaDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron nova defensive action";

    YoggSaronNovaDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// A healer's external on the bot the nova still kills, after that bot's own button.
class YoggSaronNovaExternalAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron nova external action";

    YoggSaronNovaExternalAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;

    // Shared with the trigger, so a healer with nothing in range never claims an emergency tick.
    static char const* PickExternal(PlayerbotAI* botAI, Player* victim);

private:
    // In spend order. Guardian Spirit absorbs the whole hit, the only answer to a Fervored one-shot. Hand of
    // Protection is absent on purpose: physical only, so a shadow blast goes through it, and it sheds threat.
    static constexpr char const* HEALER_EXTERNALS[] = {"pain suppression", "guardian spirit",
                                                       "hand of sacrifice"};
};

// The roar paladin's own Concentration Aura, ahead of the roar: Aura Mastery's immunity only reaches
// raiders carrying the presser's own copy.
class YoggSaronRoarAuraAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron roar aura action";

    YoggSaronRoarAuraAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Aura Mastery during Deafening Roar's 2.3 s cast, one press per roar.
class YoggSaronRoarAuraMasteryAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron roar aura mastery action";

    YoggSaronRoarAuraMasteryAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// The bot's own physical cut while a beacon-healed Guardian is hitting it.
class YoggSaronBeaconDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron beacon defensive action";

    YoggSaronBeaconDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// A healer's or paladin's external on whoever a healed Guardian is hitting with no button of their own.
// None of them is an immunity, which would hand the Guardian to somebody else.
class YoggSaronBeaconExternalAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron beacon external action";

    YoggSaronBeaconExternalAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Lay on Hands on a tank about to die in phase 3. Straight through CastSpell, past the holds that keep it
// off everyone else for the whole fight.
class YoggSaronTankLayOnHandsAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron tank lay on hands action";

    YoggSaronTankLayOnHandsAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Puts a healer back on the combat engine, which carries its mana cooldowns, potions and heal bands.
class YoggSaronHealerCombatEngineAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron healer combat engine action";

    YoggSaronHealerCombatEngineAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Puts Shadow Resistance Aura up and leaves it there, so Aura Mastery has something worth doubling.
class YoggSaronShadowAuraAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron shadow aura action";

    YoggSaronShadowAuraAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// One cover per nova window: Aura Mastery, or Divine Sacrifice for the 20% Divine Guardian hangs off it.
// Its split on the paladin is survivable: the pool is 40% of his health times raid size, broken under 20% health.
class YoggSaronNovaRaidCooldownAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron nova raid cooldown action";

    YoggSaronNovaRaidCooldownAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

// Pop an immunity to shed Squeeze, which kills the Constrictor Tentacle holding the bot.
class YoggSaronSqueezeEscapeAction : public Action
{
public:
    static constexpr char const* Name = "yogg-saron squeeze escape action";

    YoggSaronSqueezeEscapeAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
};

class YoggSaronAntiFearAction : public RaidAntiFearAction
{
public:
    static constexpr char const* Name = "yogg-saron anti fear action";

    YoggSaronAntiFearAction(PlayerbotAI* botAI) : RaidAntiFearAction(botAI, Name) {}

protected:
    bool FearWindowActive() override { return YoggSaronFearWindowActive(botAI); }
};

#endif
