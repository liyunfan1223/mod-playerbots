/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_YOGGSARON_H
#define PLAYERBOTS_ULDTRIGGERS_YOGGSARON_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "RaidAntiFear.h"
#include "UldEncounter_YoggSaron.h"
#include "Trigger.h"

// Yogg-Saron
class YoggSaronTrigger : public Trigger
{
public:
    YoggSaronTrigger(PlayerbotAI* botAI, std::string const name = "yogg saron trigger", int32 checkInteval = 1)
        : Trigger(botAI, name, checkInteval) {}

    bool IsYoggSaronFight();
    bool IsPhase2();
    bool IsPhase3();
    bool IsInBrainLevel();
    bool IsInIllusionRoom();
    bool IsInStormwindKeeperIllusion();
    bool IsInIcecrownKeeperIllusion();
    bool IsInChamberOfTheAspectsIllusion();
    bool IsMasterIsInIllusionGroup();
    bool IsMasterIsInBrainRoom();
    bool IsBrainRoomApproachable();
    bool IsDesignatedBotTank();

    // Whether the phase 3 station still reaches a target: a fixed spot is a fence when the target sits beyond radius
    // plus range. The ranged spot is tuned so Yogg is reachable by 4.4 yd and nothing else is.
    bool PhaseThreeStationReaches(Unit* target);
};

// Clouds and Guardians in one node: two nodes at the same relevance cannot share a bot (the engine ends the tick at the
// first action returning true), so they would trade ticks and walk the bot down the line between them.
class YoggSaronPhase1SpacingTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 1 spacing trigger";

    YoggSaronPhase1SpacingTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Ranged and healers hold one spot on the second orbit instead of dodging clouds: no radius clears every orbit and the
// cloud outruns any sidestep. It costs the back line one Guardian per 45 s orbit.
class YoggSaronPhase1StationTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 1 station trigger";

    YoggSaronPhase1StationTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;

private:
    // Per bot, like the guardian positioning trigger's leash: trigger instances belong to one bot's
    // context.
    bool parked = false;
};

// Dark Volley is a 1.5 s cast hitting everything within 35 yd, so it can't be walked out of, and Drain Life gives an
// Immortal Guardian back three Empowered stacks. The always-on class interrupts only look at the bot's current target.
class YoggSaronDarkVolleyTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron dark volley trigger";

    YoggSaronDarkVolleyTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A paladin with Judgement ready and a Crusher in reach that is mid Diminish Power channel.
class YoggSaronDiminishPowerJudgementTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron diminish power judgement trigger";

    YoggSaronDiminishPowerJudgementTrigger(PlayerbotAI* botAI)
        : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A warrior with Mortal Strike ready and the rage for it, and a Guardian for it, see
// YoggSaronMortalStrikeTarget.
class YoggSaronMortalStrikeTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron mortal strike trigger";

    YoggSaronMortalStrikeTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A phase 3 warlock with no demon and the summon its pet strategy names castable, see YoggSaronWarlockSummon.
class YoggSaronWarlockPetTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron warlock pet trigger";

    YoggSaronWarlockPetTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Melee and tanks are leashed to Sara, not stationed on her: a Guardian walks to whoever holds threat, so where melee
// stand is where it dies, and only a death inside 15 yd of Sara advances the phase.
class YoggSaronGuardianPositioningTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron guardian positioning trigger";

    YoggSaronGuardianPositioningTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;

private:
    // Per bot, like the spacing action's held spot: trigger instances belong to one bot's context.
    bool returning = false;
};

class YoggSaronSanityTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron sanity trigger";

    YoggSaronSanityTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronMaladyOfTheMindTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron malady of the mind trigger";

    YoggSaronMaladyOfTheMindTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The strategy swap onto TankAssist the single designated bot tank does for the raid at the start of
// phase 3.
class YoggSaronPhase3ControlTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 control trigger";

    YoggSaronPhase3ControlTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// One owner of every non-tank's target for the whole encounter, replacing raid icons: an icon is a sticky override
// (RtiTargetValue hands it back before the smart picker, IsHighPriority pins it), so a wrong mark can't be fixed.
class YoggSaronSetDpsPriorityTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron set dps priority trigger";

    YoggSaronSetDpsPriorityTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Crush wedges and Death Rays in one node: two nodes at one relevance trade ticks and walk the bot down the line
// between them.
class YoggSaronPhase2SpacingTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 2 spacing trigger";

    YoggSaronPhase2SpacingTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronBrainLinkTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron brain link trigger";

    YoggSaronBrainLinkTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronMoveToEnterPortalTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron move to enter portal trigger";

    YoggSaronMoveToEnterPortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronFallFromFloorTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron fall from floor trigger";

    YoggSaronFallFromFloorTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronStopFollowingTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron stop following trigger";

    YoggSaronStopFollowingTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronUsePortalTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron use portal trigger";

    YoggSaronUsePortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronIllusionRoomTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron illusion room trigger";

    YoggSaronIllusionRoomTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;

private:
    bool GoToBrainRoomRequired();
    bool SetRtiMarkRequired();

    // Nothing else moves a bot once a portal drops it, and the dps resolver won't pick a target it has no line of sight
    // to. Two of the three rooms put their tentacles behind a doorway.
    bool WalkIntoRoomRequired();

    // Done with its share while the rest of the room is still alive: to the door, so the walk to the
    // Brain starts there rather than 106-130 yd out.
    bool StageRequired();
};

// The brain team's healer, back to its illusion room's middle while tentacles live. Chasing its own
// target takes it out of heal range of most of its mates.
class YoggSaronIllusionHealerStationTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron illusion healer station trigger";

    YoggSaronIllusionHealerStationTrigger(PlayerbotAI* botAI)
        : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A damage dealer the reprisal hold took off the tentacles, out of its room healer's reach. Nothing heals
// it there, so it would sit out the rest of the room.
class YoggSaronReprisalRegroupTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron reprisal regroup trigger";

    YoggSaronReprisalRegroupTrigger(PlayerbotAI* botAI)
        : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Once the room's door is open, onto the spot the bot hits the Brain from. A single walk dies on the
// healer's first heal, and melee arriving in front of the Brain get walked out of range by set behind.
class YoggSaronBrainSpotTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron brain spot trigger";

    YoggSaronBrainSpotTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Nitro Boosts on the walk from a cleared illusion room to the Brain, the longest walk of the wave and
// all of it time off the Brain. Never on the way out.
class YoggSaronNitroBoostsTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron nitro boosts trigger";

    YoggSaronNitroBoostsTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A cat druid's Dash on the same walk, outside the Chamber unless it has no Nitro boots. Its own Feral
// Charge stays blocked on the Brain: it leaps to a spot behind the target, which is mid-air there.
class YoggSaronBrainDashTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron brain dash trigger";

    YoggSaronBrainDashTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A warrior's Charge or Intercept at the Brain on the walk in, never on the way out. Both land on the
// floor ring round the Brain, where its oversized combat reach sends them, so they shorten the walk.
class YoggSaronBrainChargeTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron brain charge trigger";

    YoggSaronBrainChargeTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronMoveToExitPortalTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron move to exit portal trigger";

    YoggSaronMoveToExitPortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The Laughing Skull cannot be targeted or killed (UNIT_FLAG_NOT_SELECTABLE, flags_extra 128), and its gaze picks
// targets by HasInArc(M_PI, caster), so turning around is the only defence.
class YoggSaronLaughingSkullTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron laughing skull trigger";

    YoggSaronLaughingSkullTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The facing node corrects a heading; this one moves the bot so the heading it will be forced into is already safe.
// Only while there is something to kill: clear of a fight, turning away is free and the facing node handles it.
class YoggSaronIllusionFacingTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron illusion facing trigger";

    YoggSaronIllusionFacingTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronPetGuardTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron pet guard trigger";

    YoggSaronPetGuardTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The walk to a target on the far side of Yogg. The rest of the body ring asks where the bot is; this asks where the
// walk is going, the only read that stops a bot entering it.
class YoggSaronBodyDetourTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron body detour trigger";

    YoggSaronBodyDetourTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronLunaticGazeTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron lunatic gaze trigger";

    YoggSaronLunaticGazeTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Phase 3 without Thorim: a beacon-marked Guardian heals everything within 20 yd of it once the mark ends, and the pile
// follows its holder. So a second bot taunts it off and walks it clear (YoggSaronBeaconDragGuardian picks who).
class YoggSaronBeaconDragTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron beacon drag trigger";

    YoggSaronBeaconDragTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// This bot is staged for the next Shadow Beacon and no mark is up yet. See YoggSaronTickBeacon.
class YoggSaronBeaconStageTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron beacon stage trigger";

    YoggSaronBeaconStageTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A shaman's own Heroism trigger wants a boss target, and in phase 3 shamans are on Guardians and tentacles. This one
// waits for the raid to be in place instead.
class YoggSaronPhase3LustTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 lust trigger";

    YoggSaronPhase3LustTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

class YoggSaronPhase3PositioningTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron phase 3 positioning trigger";

    YoggSaronPhase3PositioningTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The tank taunts loose Immortal Guardians (P3) to the melee stack so they get cleaved to Weakened, where
// Thorim's Titanic Storm executes them or, without him, they hit for no bonus.
class YoggSaronGuardianControlTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron guardian control trigger";

    YoggSaronGuardianControlTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// With no Sanity Wells to run to, a bot near the Insane cliff retreats to a safe ranged spot and faces
// away from Yogg to stop every avoidable sanity drain.
class YoggSaronSanityConservationTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron sanity conservation trigger";

    YoggSaronSanityConservationTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A Constrictor Tentacle drops its passenger when Squeeze is removed, so a grabbed paladin or mage
// can free itself with an immunity.
class YoggSaronSqueezeEscapeTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron squeeze escape trigger";

    YoggSaronSqueezeEscapeTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A paladin frees somebody else's Constrictor Tentacle with Hand of Protection: it grants physical
// school immunity, which purges Squeeze, and the Squeeze aura script kills the tentacle on removal.
class YoggSaronSqueezeRescueTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron squeeze rescue trigger";

    YoggSaronSqueezeRescueTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Shadow Nova is a death explosion in the melee pile, so these three nodes fire on a Guardian about to die. Own-button
// gates by length: a 12 s button at 20%, a 5 s one at 10%, matching the gate-to-nova gap (4.8 s vs 2.4 s).
class YoggSaronNovaDefensiveTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron nova defensive trigger";

    YoggSaronNovaDefensiveTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A healer covers whoever the nova still kills after their own button, worst off first. One claim per
// victim, so two healers do not spend two three-minute cooldowns on one bot.
class YoggSaronNovaExternalTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron nova external trigger";

    YoggSaronNovaExternalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Divine Sacrifice, held until the nova threatens enough of the stack to be worth one cast for the whole
// raid. One per window.
class YoggSaronNovaRaidCooldownTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron nova raid cooldown trigger";

    YoggSaronNovaRaidCooldownTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// The roar paladin is missing its own Concentration Aura with the roar close.
class YoggSaronRoarAuraTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron roar aura trigger";

    YoggSaronRoarAuraTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Deafening Roar is being cast and this paladin can answer it: its own Concentration Aura up and Aura
// Mastery ready.
class YoggSaronRoarAuraMasteryTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron roar aura mastery trigger";

    YoggSaronRoarAuraMasteryTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A beacon-healed Guardian past the danger line is hitting this bot, a dragger or a non-tank, or a failed
// drag has healed the pile on this tank, and it has a physical cut to press.
class YoggSaronBeaconDefensiveTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron beacon defensive trigger";

    YoggSaronBeaconDefensiveTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Somebody a healed Guardian is hitting has no button of their own, and this healer or paladin has an
// external in range for them.
class YoggSaronBeaconExternalTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron beacon external trigger";

    YoggSaronBeaconExternalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A tank under ULDUAR_YOGG_SARON_TANK_LOH_PCT in phase 3 and this paladin can Lay on Hands it.
class YoggSaronTankLayOnHandsTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron tank lay on hands trigger";

    YoggSaronTankLayOnHandsTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// A healer in combat on the non-combat engine during the encounter.
class YoggSaronHealerCombatEngineTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron healer combat engine trigger";

    YoggSaronHealerCombatEngineTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Shadow Resistance Aura on the one paladin whose Aura Mastery can double it, and its own copy of the
// aura: a human's sits on every raider and would otherwise read as already done.
class YoggSaronShadowAuraTrigger : public YoggSaronTrigger
{
public:
    static constexpr char const* Name = "yogg-saron shadow aura trigger";

    YoggSaronShadowAuraTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, Name) {}
    bool IsActive() override;
};

// Covers Malady of the Mind, the fight's only fear. Complements the malady spread node, which handles the
// bot who is already feared.
class YoggSaronAntiFearTrigger : public RaidAntiFearTrigger
{
public:
    static constexpr char const* Name = "yogg-saron anti fear trigger";

    YoggSaronAntiFearTrigger(PlayerbotAI* botAI) : RaidAntiFearTrigger(botAI, Name) {}

protected:
    bool FearWindowActive() override { return YoggSaronFearWindowActive(botAI); }
};

#endif
