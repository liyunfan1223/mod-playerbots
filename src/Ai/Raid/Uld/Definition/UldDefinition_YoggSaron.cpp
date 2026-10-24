/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"
#include "RaidTankDefensive.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_YoggSaron.h"
#include "UldEncounterGate.h"
#include "UldEncounter_YoggSaron.h"
#include "UldMultipliers_YoggSaron.h"
#include "UldTriggers_YoggSaron.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// Generic picker uses GetDistance, which takes off Yogg's 30 yd combat reach, so Yogg always reads nearest.
// 70 yd covers the Guardian spawn ring (38-48 yd) from the melee spot 18.5 yd behind him.
bool YoggSaronGuardianToHold(PlayerbotAI* botAI)
{
    constexpr float guardianSearchRadius = 70.0f;
    return YoggSaronInPhase3(botAI) && YoggSaronHoldableGuardianWithin(botAI->GetBot(), guardianSearchRadius);
}

// Must match YoggSaronSetDpsPriorityTrigger's window or a tank has no target source.
// Phase 2 is in it: the generic picker walks a tank into a Crusher's reach and Crush is a sure proc.
bool YoggSaronTankTargetOwned(PlayerbotAI* botAI)
{
    return YoggSaronInPhase2(botAI) || YoggSaronGuardianToHold(botAI);
}

// The Brain sits 25 yd above its room's floor, so a leap at it strands the bot in the air (Feral Charge).
// Warrior Charge and Intercept land on the floor and the brain charge node casts those. Tentacle charges stay.
bool YoggSaronBrainIsTarget(PlayerbotAI* botAI)
{
    if (!YoggSaronOnBrainLevel(botAI->GetBot()))
        return false;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return target && target->GetEntry() == NPC_BRAIN;
}

// A Crusher that can swing, with no stun holding it.
bool YoggSaronCrusherIsTarget(PlayerbotAI* botAI)
{
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return target && target->IsAlive() && target->GetEntry() == NPC_CRUSHER_TENTACLE &&
           !YoggSaronCrusherStunHolds(botAI, target);
}

// Eating sits a bot down with no AI for up to 18 s, so with no Guardian alive the raid drops combat and eats in place.
// The back line on the 21.5 yd station keeps eating.
bool YoggSaronInsideRing(PlayerbotAI* botAI)
{
    Position const& middle = ULDUAR_YOGG_SARON_MIDDLE;
    return botAI->GetBot()->GetDistance2d(middle.GetPositionX(), middle.GetPositionY()) <
               ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS &&
           YoggSaronEncounterActive(botAI);
}

void DefineYoggSaron(EncounterBuilder& e)
{
    e.Node(
        "sara shadow resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossShadowResistanceTrigger(botAI, "sara"); },
        "sara shadow resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossShadowResistanceAction(botAI, "sara"); }, ACTION_RAID);
    e.Node(
        "yogg-saron shadow resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossShadowResistanceTrigger(botAI, "yogg-saron"); },
        "yogg-saron shadow resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossShadowResistanceAction(botAI, "yogg-saron"); }, ACTION_RAID);
    // These two take the first paladin that answers; this one picks the paladin that can double the aura.
    // Prayer of Shadow Protection already gives everyone the same 130.
    e.Node<YoggSaronShadowAuraTrigger, YoggSaronShadowAuraAction>(ACTION_RAID);
    e.Node<YoggSaronGuardianPositioningTrigger, YoggSaronGuardianPositioningAction>(ACTION_RAID);
    e.Node<YoggSaronPhase1SpacingTrigger, YoggSaronPhase1SpacingAction>(ACTION_RAID + 2);
    // Under the spacing node, so a nova still wins the tick. It never contends with guardian
    // positioning at the same relevance because that one only fires on melee and tanks.
    e.Node<YoggSaronPhase1StationTrigger, YoggSaronPhase1StationAction>(ACTION_RAID);
    e.Node<YoggSaronDarkVolleyTrigger, YoggSaronDarkVolleyInterruptAction>(ACTION_EMERGENCY + 1);
    e.Node<YoggSaronSanityTrigger, YoggSaronSanityAction>(ACTION_RAID + 1);
    e.Node<YoggSaronMaladyOfTheMindTrigger, YoggSaronMaladyOfTheMindAction>(ACTION_RAID);
    e.Node<YoggSaronAntiFearTrigger, YoggSaronAntiFearAction>(ACTION_RAID + 2);
    e.Node<YoggSaronPhase3ControlTrigger, YoggSaronPhase3ControlAction>(ACTION_RAID);
    e.Node<YoggSaronSetDpsPriorityTrigger, YoggSaronSetDpsPriorityAction>(ACTION_RAID + 1);
    e.Node<YoggSaronPhase2SpacingTrigger, YoggSaronPhase2SpacingAction>(ACTION_RAID + 3);
    // Over the dps resolver and the Sanity Well walk, under the hazard dodges and the fear counter: a
    // link costs 2 Sanity and a shared hit a second, a Death Ray costs the bot.
    e.Node<YoggSaronBrainLinkTrigger, YoggSaronBrainLinkAction>(ACTION_RAID + 1.5f);
    e.Node<YoggSaronMoveToEnterPortalTrigger, YoggSaronMoveToEnterPortalAction>(ACTION_RAID);
    e.Node<YoggSaronUsePortalTrigger, YoggSaronUsePortalAction>(ACTION_RAID);
    e.Node<YoggSaronFallFromFloorTrigger, YoggSaronFallFromFloorAction>(ACTION_RAID);
    e.Node<YoggSaronStopFollowingTrigger, YoggSaronStopFollowingAction>(ACTION_RAID);
    e.Node<YoggSaronIllusionRoomTrigger, YoggSaronIllusionRoomAction>(ACTION_RAID);
    e.Node<YoggSaronMoveToExitPortalTrigger, YoggSaronMoveToExitPortalAction>(ACTION_RAID + 4);
    e.Node<YoggSaronLunaticGazeTrigger, YoggSaronLunaticGazeAction>(ACTION_EMERGENCY);
    // Same relevance as the gaze node above and they cannot share a bot, which costs nothing: Yogg's
    // own Lunatic Gaze is a phase 3 self aura on the platform and the skulls only exist below it.
    e.Node<YoggSaronLaughingSkullTrigger, YoggSaronLaughingSkullAction>(ACTION_EMERGENCY);
    e.Node<YoggSaronPhase3PositioningTrigger, YoggSaronPhase3PositioningAction>(ACTION_RAID);
    e.Node<YoggSaronGuardianControlTrigger, YoggSaronGuardianControlAction>(ACTION_RAID + 3);
    // Over guardian control: a bot tank only drags with no unmarked Guardian of its own, and guardian
    // control would walk it straight back to the pile.
    e.Node<YoggSaronBeaconDragTrigger, YoggSaronBeaconDragAction>(ACTION_RAID + 4);
    // Under the drag, which takes over at the first mark, and over guardian control for the same reason.
    e.Node<YoggSaronBeaconStageTrigger, YoggSaronBeaconStageAction>(ACTION_RAID + 3.5f);
    // Over the drag and the stage: the drag's Growl fails until the shift is in.
    e.Node<YoggSaronBeaconBearFormTrigger, YoggSaronBeaconBearFormAction>(ACTION_RAID + 4.5f);
    // Over the dps resolver and the rotation, under every dodge. One instant cast.
    e.Node<YoggSaronPhase3LustTrigger, YoggSaronPhase3LustAction>(ACTION_RAID + 2);
    e.Node<YoggSaronSanityConservationTrigger, YoggSaronSanityConservationAction>(ACTION_RAID + 5);
    e.Node<YoggSaronSqueezeEscapeTrigger, YoggSaronSqueezeEscapeAction>(ACTION_RAID + 1);
    // Above the sanity conservation retreat: cutting somebody out of 7.5k a second beats walking
    // somebody else to a well.
    e.Node<YoggSaronSqueezeRescueTrigger, YoggSaronSqueezeRescueAction>(ACTION_RAID + 6);

    // Shadow Nova goes off in the pile, so these mitigate instead of stepping out. Above everything, it lands in 1-2 s.
    // Raid cooldown first (Divine Sacrifice covers 25), externals last: a bot's own button is cheaper than a 3 min one.
    e.Node<YoggSaronNovaRaidCooldownTrigger, YoggSaronNovaRaidCooldownAction>(ACTION_EMERGENCY + 7);
    e.Node<YoggSaronNovaDefensiveTrigger, YoggSaronNovaDefensiveAction>(ACTION_EMERGENCY + 6);
    e.Node<YoggSaronNovaExternalTrigger, YoggSaronNovaExternalAction>(ACTION_EMERGENCY + 5);

    // Phase 3's same pair against a beacon-healed Guardian, which hits a non-tank for 25-36k a few
    // seconds after the heal. Level with the nova pair, which only runs in phase 1.
    e.Node<YoggSaronBeaconDefensiveTrigger, YoggSaronBeaconDefensiveAction>(ACTION_EMERGENCY + 6);
    e.Node<YoggSaronBeaconExternalTrigger, YoggSaronBeaconExternalAction>(ACTION_EMERGENCY + 5);

    // Under the beacon pair, which a tank's own button and the externals answer first.
    e.Node<YoggSaronTankLayOnHandsTrigger, YoggSaronTankLayOnHandsAction>(ACTION_EMERGENCY + 4);

    // Over everything: the roar's cast is 2.3 s and Aura Mastery is off the global cooldown. The aura it
    // needs goes up ahead of time, as a raid node.
    e.Node<YoggSaronRoarAuraMasteryTrigger, YoggSaronRoarAuraMasteryAction>(ACTION_EMERGENCY + 8);

    // Above the roar press, so the engine is right before anything else runs. Only does anything on the
    // non-combat engine, which a healer with no target always falls back to.
    e.Node<YoggSaronHealerCombatEngineTrigger, YoggSaronHealerCombatEngineAction>(ACTION_EMERGENCY + 9);
    e.Node<YoggSaronRoarAuraTrigger, YoggSaronRoarAuraAction>(ACTION_RAID + 1);
    // Level with the phase 2 dodge and above the dps resolver: where the bot stands has to be settled
    // before the reach node is asked to close on the target from there.
    e.Node<YoggSaronIllusionFacingTrigger, YoggSaronIllusionFacingAction>(ACTION_RAID + 3);
    // Above the dps resolver for the same reason, though it never claims the tick: the pet has to be
    // pulled off before it swings again, not after the bot has picked its own target.
    e.Node<YoggSaronPetGuardTrigger, YoggSaronPetGuardAction>(ACTION_RAID + 2);
    // Under every raid node and over reach melee, reach spell and charge: it is the last thing asked
    // before the plain walk it exists to replace, and it stands down the moment the route is clear.
    e.Node<YoggSaronBodyDetourTrigger, YoggSaronBodyDetourAction>(ACTION_RAID - 1);
    // An instant cast, so under every raid walk and dodge, and well over the paladin's own rotation,
    // which would only ever judge its current target.
    e.Node<YoggSaronDiminishPowerJudgementTrigger, YoggSaronDiminishPowerJudgementAction>(ACTION_RAID - 0.5f);
    // Same reasoning for the warrior's Mortal Strike on a healing Guardian, over its own rotation, which
    // would only ever strike Yogg.
    e.Node<YoggSaronMortalStrikeTrigger, YoggSaronMortalStrikeAction>(ACTION_RAID - 0.5f);
    // Same slot for the warlock's summon, so it waits for the station walk and beats the rotation.
    e.Node<YoggSaronWarlockPetTrigger, YoggSaronWarlockPetAction>(ACTION_RAID - 0.5f);
    // Level with the room walk, above heal reach (38-40): the station puts the whole room in range.
    // The trigger stands down inside 10 yd of the station and heal reach takes over.
    e.Node<YoggSaronIllusionHealerStationTrigger, YoggSaronIllusionHealerStationAction>(ACTION_RAID);
    // Level with the room walk, which a held bot never runs: its room state reads reprisal, not walkingin.
    e.Node<YoggSaronReprisalRegroupTrigger, YoggSaronReprisalRegroupAction>(ACTION_RAID);
    // Over reach melee and set behind, so a bot arriving in front of the Brain goes round to the spot
    // rather than being walked out of range and back. Under the exit walk.
    e.Node<YoggSaronBrainSpotTrigger, YoggSaronBrainSpotAction>(ACTION_RAID);
    // Just over the brain spot walk it speeds up. It never claims the tick, so the walk still runs.
    e.Node<YoggSaronNitroBoostsTrigger, YoggSaronNitroBoostsAction>(ACTION_RAID + 0.5f);
    e.Node<YoggSaronBrainDashTrigger, YoggSaronBrainDashAction>(ACTION_RAID + 0.5f);
    // Over the brain spot walk, which outranks the class charge nodes, so they never fire on it.
    e.Node<YoggSaronBrainChargeTrigger, YoggSaronBrainChargeAction>(ACTION_RAID + 0.25f);

    // The dps priority row owns every non-tank's target, so the generic picker stands down on the row's exact test.
    // A wider window leaves a bot with no target source. "attack rti target" stays: a player's mark wins.
    e.OwnTargeting("yogg-saron dps target guard multiplier", Role::NonTank, YoggSaronEncounterActive,
                   Family::DpsAssist);
    e.OwnTargeting("yogg-saron dps target guard multiplier", Role::Tank, YoggSaronTankTargetOwned,
                   Family::TankAssist);

    // Phase 1 is six fixed cloud orbits, so any jump lands on a ring. Blink and Disengage stay off all fight.
    // Gap-closers return in phases 2 and 3, where no orbit is in the way. Reach melee survives either way.
    e.Block("yogg-saron displacement guard multiplier", Role::Any, YoggSaronEncounterActive,
            Family::Blink | Family::Disengage);
    e.Block("yogg-saron displacement guard multiplier", Role::Any, YoggSaronInPhase1, Family::Charge,
            {"killing spree"});
    // Killing Spree on the Brain lands the rogue on its model, 21 yd above the floor, where nothing walks it out.
    e.Block("yogg-saron displacement guard multiplier", Role::Any, YoggSaronBrainIsTarget, Family::Charge,
            {"killing spree"});

    // Everything else that could put a held bot back on a tentacle. Not the whole Attack family: the dps
    // resolver is an AttackAction itself, and it is what drops the target. The assist is already blocked.
    e.Block("yogg-saron reprisal hold", Role::Any, YoggSaronReprisalHold,
            Family::Melee | Family::DpsAoe | Family::AggressiveTarget | Family::AttackAnything |
                Family::AttackLeastHp | Family::AttackRti | Family::DebuffOnAttacker | Family::Charge);

    // A bot waiting at the door stays there. A player's mark still wins, which the share already allows.
    e.Block("yogg-saron illusion stage hold", Role::Any, YoggSaronIllusionStaged,
            Family::Melee | Family::DpsAoe | Family::AggressiveTarget | Family::AttackAnything |
                Family::AttackLeastHp | Family::DebuffOnAttacker | Family::Charge);

    // Off the Brain while the brain team holds it, the same families as the reprisal hold. DoTs on
    // attackers stop at the earlier line, since they keep ticking after the bot stops.
    e.Block("yogg-saron brain hold", Role::Any, YoggSaronBrainHeld,
            Family::Melee | Family::DpsAoe | Family::AggressiveTarget | Family::AttackAnything |
                Family::AttackLeastHp | Family::AttackRti | Family::Charge | Family::PetAttack);
    e.Block("yogg-saron brain dot hold", Role::Any, YoggSaronBrainDotHeld, Family::DebuffOnAttacker);

    // A bot on its portal spot waits there, or reach takes it to its target and the portal walk fetches it again.
    // The Yogg dodges are none of these and still move it.
    e.Block("yogg-saron portal spot hold", Role::Any, YoggSaronHoldingPortalSpot,
            Family::Reach | Family::ReachHeal | Family::SetBehind | Family::RearFlank |
                Family::CombatFormationMove | Family::Charge | Family::Follow | Family::MoveRandom);

    // Shadowfiend spawns 3 yd from its target, which puts it in the Crusher's reach past the pet guard. The
    // priest saves it for a non-Crusher.
    e.Block("yogg-saron crusher pet spell hold", Role::Any, YoggSaronCrusherIsTarget, 0, {"shadowfiend"});

    e.Multiplier<YoggSaronMovementGuardMultiplier>(Family::Flee | Family::SetBehind | Family::Reach |
                                                   Family::ReachHeal);

    // Whether an action is AoE is its threat type, which any action can override.
    e.Multiplier<YoggSaronPhase1AoeHoldMultiplier>(Family::AnyAction);
    e.Multiplier<YoggSaronIllusionMultiTargetHoldMultiplier>(Family::AnyAction);
    e.Multiplier<YoggSaronExitHoldCastMultiplier>(Family::Spell);
    e.Multiplier<YoggSaronHeldTargetCastMultiplier>(Family::Spell);
    e.Multiplier<YoggSaronPhase3AreaMultiplier>(Family::AnyAction);

    e.Block("yogg-saron stack food guard multiplier", Role::Any, YoggSaronInsideRing, 0, {"food", "drink"});

    // These buttons otherwise fire on "I am already low", which is after the nova. Melee only: the tank keeps its own
    // against the Guardians it holds and the back line against Dark Volley.
    e.Block("yogg-saron nova defensive hold", Role::Melee, YoggSaronNovaHold, 0, HeldMagicDefensiveNames());

    // The externals and the raid cooldown, held for the same reason.
    e.Block("yogg-saron nova cooldown hold", Role::Any, YoggSaronNovaHold, 0,
            {"pain suppression", "pain suppression on party", "guardian spirit on party", "divine sacrifice"});

    // The aura slot is exclusive, so the paladin's own buff strategy trades it back on the next GCD.
    e.Block("yogg-saron shadow aura hold", Role::Any, YoggSaronShadowAuraHeld, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "shadow resistance aura", "fire resistance aura", "frost resistance aura"});

    // Every other paladin keeps off it so the elected one owns the slot. The two generic nodes above are named
    // too, since their action adds a strategy that re-casts the aura.
    e.Block("yogg-saron shadow aura other hold", Role::Any, YoggSaronShadowAuraOtherHeld, 0,
            {"shadow resistance aura", "sara shadow resistance action", "yogg-saron shadow resistance action"});

    // Not back into the pile: a battle rez strips every raid buff, so the bot returns on a smaller bar and dies again.
    // Only Rebirth, since Reincarnation runs from the dead engine where no encounter rule reaches.
    e.Block("yogg-saron battle rez hold", Role::Any, YoggSaronBattleRezHeld, 0, {"rebirth"});

    // The roar paladin keeps Concentration Aura until it has pressed, or the buff strategy trades it back next GCD.
    // The generic shadow resistance nodes add a strategy that would re-cast it.
    e.Block("yogg-saron roar aura hold", Role::Any, YoggSaronRoarAuraHeld, 0,
            {"devotion aura", "retribution aura", "crusader aura", "sanctity aura", "shadow resistance aura",
             "fire resistance aura", "frost resistance aura", "sara shadow resistance action",
             "yogg-saron shadow resistance action"});

    // An immunity on a healed Guardian's victim sheds it onto somebody else, at full power.
    e.Block("yogg-saron beacon immunity hold", Role::Any, YoggSaronBeaconImmunityHeld, 0,
            {"divine shield", "ice block"});

    // A hunter's drag dies with Feign Death, and Deterrence pacifies the shot.
    e.Block("yogg-saron beacon hunter hold", Role::Any, YoggSaronBeaconHunterHold, 0,
            {"feign death", "deterrence"});

    // A druid dragging stays a bear: the cat strategy recasts Cat Form, through Caster Form, whenever it isn't
    // in it. That same trigger shifts it back once the drag is over.
    e.Block("yogg-saron beacon bear hold", Role::Any, YoggSaronBeaconBearHeld, 0, {"cat form", "caster form"});

    // Its hop answers a ray at combat priority, which a forced walk refuses, and it outranks the Yogg dodge,
    // so near a ray nothing would move the bot. Its spots can also sit beside the ray.
    e.Block("yogg-saron death ray avoid aoe hold", Role::Any, YoggSaronDeathRayNear, Family::AvoidAoe);

    // The holder of a tank swap keeps its threat off the pile so the Guardians stay with the taker. Holy
    // Shield and Shield of Righteousness stay.
    e.Block("yogg-saron tank swap hold", Role::Tank, YoggSaronTankShedding, 0,
            {"consecration", "hammer of the righteous", "avenger's shield", "hand of reckoning",
             "righteous defense"});

    // Avenging Wrath locks Divine Protection for 30 s, so a tank that pressed it has no button left against the pile.
    e.Block("yogg-saron tank wrath hold", Role::Tank, YoggSaronTankWrathHeld, 0, {"avenging wrath"});

    // Kept for the tanks, not spent on dps in phase 1. The tank node casts it directly.
    e.Block("yogg-saron lay on hands hold", Role::Any, YoggSaronEncounterActive, 0, {"lay on hands on party"});
    e.Block("yogg-saron lay on hands hold", Role::NonTank, YoggSaronEncounterActive, 0, {"lay on hands"});
    e.Block("yogg-saron lay on hands hold", Role::Tank, YoggSaronBeforePhase3, 0, {"lay on hands"});

    // A healer with no target is dropped straight back to the non-combat engine.
    e.Block("yogg-saron healer target hold", Role::Heal, YoggSaronHealerTargetless, 0, {"drop target"});

    e.Multiplier<YoggSaronPhase1WalkGuardMultiplier>(Family::AnyMovement);
    e.Multiplier<YoggSaronAntiFearTotemGuardMultiplier>(Family::Spell);

    e.Tick(YoggSaronTick);
}
}  // namespace

EncounterDefinition const& UldYoggSaronDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_YOGGSARON, &BossStateGateOpen, &DefineYoggSaron);
    return definition;
}
