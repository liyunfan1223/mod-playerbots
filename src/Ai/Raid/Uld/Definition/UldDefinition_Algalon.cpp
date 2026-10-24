/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_Algalon.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Algalon.h"
#include "UldMultipliers_Algalon.h"
#include "UldTriggers_Algalon.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool AlgalonCooldownsHeld(PlayerbotAI* botAI)
{
    return AlgalonEngaged(botAI) &&
           (AlgalonBigBangCasting(botAI) || AlgalonBigBangWithin(botAI, ULDUAR_ALGALON_COOLDOWN_HOLD_SECONDS) ||
            IsAlgalonPhased(botAI->GetBot()));
}

// The explosion node presses these by id against the star that would kill the bot. The class nodes would spend
// Barkskin on a meteor or at a cast and have it cooling for the star. Phased, no explosion reaches the bot.
bool AlgalonExplosionButtonsHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return AlgalonEngaged(botAI) && bot->GetHealthPct() > ULDUAR_ALGALON_BUTTON_HOLD_PCT &&
           (IsAlgalonPhased(bot) || (AlgalonInPhaseOne(botAI) && !IsAlgalonSwapTank(bot)));
}

// Phased bots are left out: inside the void they are safe, and phase-16 Dark Matter is on them.
bool AlgalonHidingNow(PlayerbotAI* botAI) { return AlgalonShouldRunForShelter(botAI->GetBot()); }

bool AlgalonHiderCasting(PlayerbotAI* botAI) { return AlgalonHiderInCast(botAI->GetBot()); }

// A star isn't an attacker until something hits it, so "dps assist" would pull the team straight back
// to Algalon between the star node's checks.
bool AlgalonStarTeamFocused(PlayerbotAI* botAI)
{
    return IsAlgalonStarTeam(botAI->GetBot()) && GetAlgalonFocusStar(botAI) && AlgalonEngaged(botAI);
}

bool AlgalonYielding(PlayerbotAI* botAI) { return AlgalonYieldsBoss(botAI->GetBot()); }

bool AlgalonTankHealing(PlayerbotAI* botAI) { return IsAlgalonTankHealer(botAI->GetBot()); }

bool AlgalonShadowAuraHeld(PlayerbotAI* botAI)
{
    return AlgalonInPhaseOne(botAI) && IsAlgalonShadowAuraPaladin(botAI->GetBot());
}

// Not with a pickup or swap due: the walk back to him runs on the movers this would hold.
bool AlgalonHandlerKiting(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!AlgalonEngaged(botAI) || !IsAlgalonHandler(bot) || AlgalonTakesBossNow(bot))
        return false;

    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    return constellation && (constellation->GetVictim() == bot || AlgalonKitingBehind(bot, constellation));
}

// Only the rings and the swap tanks. Melee keep the generic movers once released, and so does the swap tank
// not holding him while it works an add; idle, it keeps its spot in his reach.
bool AlgalonFormationHolds(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!AlgalonPresent(botAI))
        return false;

    if (AlgalonTakesRingSlot(bot))
        return true;

    if (!IsAlgalonSwapTank(bot))
        return false;

    return !AlgalonEngaged(botAI) || !AlgalonOffTankWorking(bot);
}

bool AlgalonMeleeHeldBack(PlayerbotAI* botAI) { return AlgalonMeleeStaged(botAI->GetBot()); }

bool AlgalonGoingToFarHole(PlayerbotAI* botAI) { return AlgalonHeadsForFarHole(botAI->GetBot()); }

bool AlgalonSmashHold(PlayerbotAI* botAI) { return AlgalonEngaged(botAI) && AlgalonSmashHoldsGround(botAI->GetBot()); }

// Where the charge lands too, not just where the bot stands: he can stand on the impact, and Feral Charge
// lands behind him.
bool AlgalonSmashChargeHold(PlayerbotAI* botAI)
{
    if (AlgalonSmashHold(botAI))
        return true;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return AlgalonEngaged(botAI) && target &&
           AlgalonCosmicSmashMarkerNear(botAI, target->GetPosition(),
                                        ULDUAR_ALGALON_COSMIC_SMASH_CLEARANCE + target->GetCombatReach());
}

void DefineAlgalon(EncounterBuilder& e)
{
    // A survival ranking: a missed Big Bang is 76-112k or a reset, Cosmic Smash lands ~4.8s after its marker,
    // and a bot inside a hole when its phase ends is phased again. Position is last and yields once parked.
    e.Node<AlgalonBigBangSoakTrigger, AlgalonBigBangSoakAction>(ACTION_EMERGENCY + 9);
    e.Node<AlgalonBigBangExternalTrigger, AlgalonBigBangExternalAction>(ACTION_EMERGENCY + 8);
    e.Node<AlgalonBigBangHideTrigger, AlgalonBigBangHideAction>(ACTION_EMERGENCY + 7, EncounterRow::Mover);
    e.Node<AlgalonCosmicSmashTrigger, AlgalonCosmicSmashAction>(ACTION_EMERGENCY + 6, EncounterRow::Mover);
    e.Node<AlgalonSoakerPrepTrigger, AlgalonSoakerPrepAction>(ACTION_EMERGENCY + 5);
    e.Node<AlgalonLeaveBlackHoleTrigger, AlgalonLeaveBlackHoleAction>(ACTION_EMERGENCY + 4, EncounterRow::Mover);
    // Below the exit: a healer still inside the field when the phase drops is phased again.
    e.Node<AlgalonSoakerRescueTrigger, AlgalonSoakerRescueAction>(ACTION_EMERGENCY + 3);
    // Instants, below the hide, the dodge and the exit: those come first, and the cast follows on the next tick.
    e.Node<AlgalonTankDefensiveTrigger, AlgalonTankDefensiveAction>(ACTION_EMERGENCY + 2);
    e.Node<AlgalonExplosionCoverTrigger, AlgalonExplosionCoverAction>(ACTION_EMERGENCY + 1);
    // After the cover: a bot that survives the hit under it keeps its button.
    e.Node<AlgalonExplosionPersonalTrigger, AlgalonExplosionPersonalAction>(ACTION_EMERGENCY);
    // Ahead of the slot walk: the tank he's hitting from 70%, the partner and the healer itself from half.
    e.Node<AlgalonTankHealCriticalTrigger, AlgalonTankHealAction>(ACTION_RAID + 9);
    e.Node<AlgalonTankPickupTrigger, AlgalonTankPickupAction>(ACTION_RAID + 8);
    e.Node<AlgalonPhasePunchSwapTrigger, AlgalonPhasePunchSwapAction>(ACTION_RAID + 7);
    e.Node<AlgalonYieldBossTrigger, AlgalonYieldBossAction>(ACTION_RAID + 7);
    e.Node<AlgalonConstellationTauntTrigger, AlgalonConstellationTauntAction>(ACTION_RAID + 6);
    e.Node<AlgalonDarkMatterTankTrigger, AlgalonDarkMatterTankAction>(ACTION_RAID + 5);
    e.Node<AlgalonConstellationKiteTrigger, AlgalonConstellationKiteAction>(ACTION_RAID + 4, EncounterRow::Mover);
    e.Node<AlgalonStarTeamTrigger, AlgalonStarTeamAction>(ACTION_RAID + 3);
    e.Node<AlgalonStarMarkTrigger, AlgalonStarMarkAction>(ACTION_RAID + 2);
    e.Node<AlgalonThreatRedirectTrigger, AlgalonThreatRedirectAction>(ACTION_RAID + 1);
    e.Node<AlgalonRaidPositionTrigger, AlgalonRaidPositionAction>(ACTION_RAID, EncounterRow::Mover);
    // One global cooldown a pull, above the party heal bands so the heals don't starve it until the first star.
    e.Node<AlgalonShadowAuraTrigger, AlgalonShadowAuraAction>(ACTION_MEDIUM_HEAL + 2.0f);
    e.Node<AlgalonTankBeaconTrigger, AlgalonTankBeaconAction>(ACTION_MEDIUM_HEAL + 1.0f);
    // Under the party heal bands, which run when they land on a tank and are vetoed otherwise, and under
    // Judgement of Light on purpose: its procs are about a quarter of a holy paladin's healing here.
    e.Node<AlgalonTankHealTrigger, AlgalonTankHealAction>(ACTION_MEDIUM_HEAL + 0.75f);

    e.Multiplier<AlgalonLeaveFieldMultiplier>(Family::AnyAction);
    e.Multiplier<AlgalonCooldownReserveMultiplier>(Family::Spell);
    e.Multiplier<AlgalonTankHealerMultiplier>(Family::Spell | Family::ReachHeal);
    e.Multiplier<AlgalonHideCastMultiplier>(Family::Spell);
    e.Block("algalon phase immunity hold", Role::Any, AlgalonCooldownsHeld, 0,
            {"ice block", "divine shield", "cloak of shadows", "anti magic shell"});
    e.Block("algalon explosion button hold", Role::Any, AlgalonExplosionButtonsHeld, 0,
            {"barkskin", "shadow ward", "icebound fortitude", "shamanistic rage", "divine protection"});
    // The cover node spends these on the star about to die. The class nodes fire on a hurt raid, after it.
    e.Block("algalon explosion cover hold", Role::Any, AlgalonInPhaseOne, 0, {"divine sacrifice"});
    // Eating stops the bot thinking: one back from Reincarnation would sit down and eat through Big Bang.
    e.Block("algalon food guard", Role::Any, AlgalonEngaged, 0, {"food", "drink"});
    // The aura slot is exclusive, so the paladin's own buff strategy would trade it back on the next GCD.
    e.Block("algalon shadow aura hold", Role::Any, AlgalonShadowAuraHeld, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "shadow resistance aura", "fire resistance aura", "frost resistance aura"});
    // The encounter places these on the swap pair itself; the class nodes would move them to the main tank.
    e.Block("algalon tank healer buffs", Role::Heal, AlgalonTankHealing, 0,
            {"beacon of light on main tank", "sacred shield on main tank", "earth shield on main tank",
             "earth shield on party", "divine hymn"});

    // Every other mover would walk a hider back out of its hole, and MoveTo answers Duplicate from the
    // run's second tick, so the run can't defend itself.
    e.Exclusive("algalon big bang hide", Role::Any, AlgalonHidingNow, 0,
                {AlgalonBigBangHideAction::Name, AlgalonLeaveBlackHoleAction::Name, AlgalonCosmicSmashAction::Name});
    // The whole cast, not just the run: a charge or Killing Spree while waiting near the hole ends at him.
    e.Block("algalon big bang spell movers", Role::Any, AlgalonHiderCasting,
            Family::Charge | Family::Blink | Family::Disengage, {"killing spree"});

    // The class "lose aggro" nodes taunt whenever someone else holds him, 4 stacks or not, and an Arms warrior's
    // snare chain ends in Mocking Blow. The encounter's own taunt nodes go through CastClassTaunt instead.
    e.Block("algalon taunt guard", Role::Any, AlgalonEngaged, Family::Taunt);
    // Tricks goes to the holder from the redirect node or nowhere: the class value hands it to the strongest
    // melee dps when the tank is out of 20 yd or the rogue is low on threat, and that dps pulls him.
    e.Block("algalon class tricks", Role::Any, AlgalonEngaged, 0,
            {"tricks of the trade on main tank"});
    // The target exclusions keep every picker off him for the yielding tank except "attack rti target",
    // which follows a skull the raid put on him.
    e.Block("algalon yield skull", Role::Tank, AlgalonYielding, Family::AttackRti);

    e.OwnTargeting("algalon star team", Role::Dps, AlgalonStarTeamFocused);

    e.Multiplier<AlgalonSplashHoldMultiplier>(Family::AnyAction);
    e.Multiplier<AlgalonTargetGuardMultiplier>(Family::Melee | Family::Spell | Family::PetAttack);

    e.Multiplier<AlgalonWalkGuardMultiplier>(Family::Reach | Family::ReachHeal | Family::SetBehind |
                                             Family::RearFlank);

    // Attack would chase the constellation and Reach would walk back to Algalon while the handler leads
    // one to a hole. Its own taunts are AttackActions too, and a swap it can't make phases the holder out.
    e.OwnMovement("algalon kite movement", Role::Tank, AlgalonHandlerKiting, 0,
                  {AlgalonConstellationKiteAction::Name, AlgalonBigBangHideAction::Name, AlgalonCosmicSmashAction::Name,
                   AlgalonLeaveBlackHoleAction::Name, AlgalonTankPickupAction::Name, AlgalonPhasePunchSwapAction::Name,
                   AlgalonConstellationTauntAction::Name, AlgalonDarkMatterTankAction::Name});

    // Otherwise the generic movers walk the ranged off their slots and the slot node walks them back every tick.
    // ReachHeal still walks a healer to someone the rings can't reach.
    e.OwnMovement("algalon control movement", Role::Ranged | Role::Tank, AlgalonFormationHolds,
                  Family::Attack | Family::Reach | Family::ReachHeal);

    // At the end of the intro he swings at a random player, so melee wait outside his reach until a swap
    // tank has him: Reach and Charge would take them straight in the moment he turns selectable.
    e.OwnMovement("algalon melee staging", Role::Melee, AlgalonMeleeHeldBack, Family::Attack);
    e.Block("algalon melee staging charges", Role::Melee, AlgalonMeleeHeldBack, Family::Charge);

    // Otherwise "reach melee" walks a dodged melee bot straight back to 6-12 yd of the impact, inside the
    // doubled damage band. The encounter's own walks still run and already avoid a live marker.
    e.OwnMovement("algalon smash hold", Role::Any, AlgalonSmashHold, Family::Attack);
    e.Block("algalon smash hold charges", Role::Melee, AlgalonSmashChargeHold, Family::Charge);

    // A bot walking towards a hole it couldn't reach in the cast would be walked back to him by reach.
    e.OwnMovement("algalon far hole", Role::Any, AlgalonGoingToFarHole, Family::Attack);

    e.Tick(AlgalonTickEncounterState);
}
}  // namespace

EncounterDefinition const& UldAlgalonDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_ALGALON, &BossStateGateOpen, &DefineAlgalon);
    return definition;
}
