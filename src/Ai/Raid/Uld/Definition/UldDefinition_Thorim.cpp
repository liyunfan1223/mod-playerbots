/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "BossResistanceMultipliers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "UldActions_Thorim.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Thorim.h"
#include "UldMultipliers_Thorim.h"
#include "UldTriggers_Thorim.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// The generic picker families. Picking a target isn't walking anywhere, and the encounter's own picker
// row is the one that drops a bad target, so no guard here may take these away.
constexpr RaidEncounterRules::FamilyMask PICKERS = Family::DpsAssist | Family::DpsAoe | Family::TankAssist |
                                                   Family::AggressiveTarget | Family::AttackAnything |
                                                   Family::AttackLeastHp;

constexpr RaidEncounterRules::FamilyMask DPS_PICKERS =
    Family::DpsAssist | Family::DpsAoe | Family::AggressiveTarget | Family::AttackAnything | Family::AttackLeastHp;

bool ThorimLeashBreached(PlayerbotAI* botAI) { return ThorimArenaLeashBreached(botAI, botAI->GetBot()); }

bool ThorimArenaSettled(PlayerbotAI* botAI) { return ThorimArenaAnchorSettled(botAI, botAI->GetBot()); }

bool ThorimRingSettled(PlayerbotAI* botAI) { return ThorimMeleeRingSettled(botAI, botAI->GetBot()); }

// A healer's target drives its wand and its offensive dispels, so it keeps the generic pickers.
bool ThorimPickingFor(PlayerbotAI* botAI)
{
    return botAI->GetState() == BOT_STATE_COMBAT && !PlayerbotAI::IsHeal(botAI->GetBot());
}

// Phase 2 only: the pickup row owns the tank's target there, and a voting "tank assist" would hold it on an add.
// Before that, "tank assist" holding whatever swings at the raid beats steering a tank onto the ranged pick.
bool ThorimTankPicked(PlayerbotAI* botAI) { return ThorimPickingFor(botAI) && ThorimPhase2Active(botAI); }

// Held for the whole split even with no pick, since nothing from the row means nothing legal to hit. Released,
// the generic pickers hand Thorim on his balcony straight back every tick the row drops him.
bool ThorimDpsPicked(PlayerbotAI* botAI)
{
    return ThorimPickingFor(botAI) && (ThorimHasDpsTarget(botAI, botAI->GetBot()) || ThorimSplitActive(botAI));
}

bool ThorimPhase2DpsPicked(PlayerbotAI* botAI) { return ThorimPhase2Active(botAI) && ThorimDpsPicked(botAI); }

// Another tank, human or bot, has him and he's this bot's target. Adds stay fair game for taunts. The pickup
// row's own taunts go through DoSpecificAction, which never sees a multiplier.
bool ThorimTauntWouldSteal(PlayerbotAI* botAI)
{
    if (!ThorimPhase2Active(botAI))
        return false;

    Unit* boss = GetThorim(botAI);
    if (!boss || botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() != boss)
        return false;

    Unit* victim = boss->GetVictim();
    if (!victim || victim == botAI->GetBot())
        return false;

    Player* holder = victim->ToPlayer();
    return holder && PlayerbotAI::IsTank(holder);
}

// Above the floor line, in the corridor half, and only while the walk has somewhere to go. The instant
// the hallway isn't the job any more this lets go, so nothing can leave a bot up there with no mover.
bool ThorimBalconyWalk(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD ||
        GetThorimSquad(botAI, bot) != ThorimSquad::Gauntlet)
        return false;

    ThorimBalconyAdvanceTrigger balconyAdvance(botAI);
    return balconyAdvance.IsActive();
}

bool ThorimFrostAuraHeld(PlayerbotAI* botAI) { return IsThorimFrostAuraPaladin(botAI->GetBot()); }

void TickThorim(PlayerbotAI* botAI)
{
    ThorimTickBarrierBail(botAI, botAI->GetBot());
    ThorimTickHumanTankCalls(botAI);
    ThorimTickFrostAuraPaladin(botAI);
}

void DefineThorim(EncounterBuilder& e)
{
    e.Node(
        "thorim nature resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossNatureResistanceTrigger(botAI, "thorim"); },
        "thorim nature resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossNatureResistanceAction(botAI, "thorim"); }, ACTION_RAID);
    e.Node(
        "thorim frost resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossFrostResistanceTrigger(botAI, "thorim"); },
        "thorim frost resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossFrostResistanceAction(botAI, "thorim"); }, ACTION_RAID);
    e.Node<ThorimUnbalancingStrikeSwapTrigger, ThorimUnbalancingStrikeSwapAction>(ACTION_RAID + 2);
    e.Node<ThorimTankPickupTrigger, ThorimTankPickupAction>(ACTION_RAID + 2);
    e.Node<ThorimDpsPriorityTrigger, ThorimDpsPriorityAction>(ACTION_RAID);

    e.Node<ThorimFrostAuraTrigger, ThorimFrostAuraAction>(ACTION_RAID);

    // Recalls a pet, not the bot, so it competes with nothing and sits at the base rank.
    e.Node<ThorimPetLeashTrigger, ThorimPetLeashAction>(ACTION_RAID);

    e.Node<ThorimGauntletPositioningTrigger, ThorimGauntletPositioningAction>(ACTION_RAID);

    // Above the corridor walk, because on the balcony that one has no waypoint to offer and the
    // hallway is where the two Paralytic Field bunnies are.
    e.Node<ThorimBalconyAdvanceTrigger, ThorimBalconyAdvanceAction>(ACTION_RAID + 2);

    e.Node<ThorimArenaPositioningTrigger, ThorimArenaPositioningAction>(ACTION_RAID);
    e.Node<ThorimFallFromFloorTrigger, ThorimFallFromFloorAction>(ACTION_RAID + 1);
    e.Node<ThorimPhase2PositioningTrigger, ThorimPhase2PositioningAction>(ACTION_RAID);
    e.Node<ThorimSifBlizzardTrigger, ThorimSifBlizzardAction>(ACTION_RAID + 3);

    // Charge Orb is phase 1's largest avoidable damage, so it beats the arena ring and the add chase. Lightning
    // Charge has no node: the phase 2 spot answers the cone, and a second mover would only fight it at the gate.
    e.Node<ThorimChargedOrbTrigger, ThorimChargedOrbAction>(ACTION_RAID + 4);

    e.Node<ThorimRunicSmashTrigger, ThorimRunicSmashAction>(ACTION_RAID + 3);
    e.Node<ThorimRunicBarrierBailTrigger, ThorimRunicBarrierBailAction>(ACTION_RAID + 2);

    // Top of the Thorim ladder. An arena squad member outside the box is not a positioning problem:
    // one 5 second scan finding nobody in there summons the Lightning Orb and kills the raid.
    e.Node<ThorimArenaLeashTrigger, ThorimArenaLeashAction>(ACTION_RAID + 5);

    e.Node<ThorimResetEncounterStateTrigger, ThorimResetEncounterStateAction>(ACTION_RAID);

    e.Multiplier<ThorimRunicBarrierMultiplier>(Family::Melee | Family::Reach | Family::ReachHeal);

    // Without this the generic pickers alternate with the row instead of losing: the row goes quiet once the bot
    // holds its pick, the engine falls through to "dps assist" at 50, and that retargets.
    e.OwnTargeting("thorim disable automatic targeting", Role::Tank, ThorimTankPicked,
                   DPS_PICKERS | Family::TankAssist);
    e.OwnTargeting("thorim disable automatic targeting", Role::NonTank, ThorimDpsPicked, DPS_PICKERS);
    e.OwnTargeting("thorim disable automatic targeting", Role::NonTank, ThorimPhase2DpsPicked, Family::TankAssist);

    // Stops "behind" (every melee DPS has it) walking the three stacks into one arc once the ring row yields.
    // Exempts the balcony walk, the one mover a bot that hasn't made it down yet has left.
    e.OwnMovement("thorim movement guard", Role::Any, ThorimRingSettled,
                  Family::Attack | Family::Reach | Family::ReachHeal | Family::AvoidAoe,
                  {ThorimPhase2PositioningAction::Name, ThorimBalconyAdvanceAction::Name,
                   ThorimSifBlizzardAction::Name});

    // No attack or reach exemption here: corridor mobs sit inside the sight cap from the arena, so the chase is
    // what walks a bot out. Open only while outside the leash, so it can't become a permanent freeze.
    e.Exclusive("thorim arena leash", Role::Any, ThorimLeashBreached, Family::AvoidAoe | PICKERS,
                {ThorimArenaLeashAction::Name, ThorimArenaPositioningAction::Name, ThorimChargedOrbAction::Name,
                 ThorimSifBlizzardAction::Name, ThorimDpsPriorityAction::Name});

    e.Multiplier<ThorimArenaTargetGuardMultiplier>(Family::Attack | Family::Reach | Family::ReachHeal);

    // Holds the arena formation once settled, or the generic movers smear the squad east into the corridor mouth.
    // Keeps the chase: an add can land out of range of an outer slot.
    e.OwnMovement("thorim arena anchor guard", Role::Any, ThorimArenaSettled,
                  Family::Attack | Family::Reach | Family::ReachHeal | Family::AvoidAoe,
                  {ThorimArenaPositioningAction::Name, ThorimArenaLeashAction::Name, ThorimChargedOrbAction::Name,
                   ThorimSifBlizzardAction::Name});

    // Up on the balcony, "follow" walks the corridor squad the only walkable way down to the master,
    // 300 yd back through the corridor. Rune Detonation lands up there, so the dodge stays.
    e.OwnMovement("thorim balcony guard", Role::Any, ThorimBalconyWalk,
                  Family::Attack | Family::Reach | Family::ReachHeal | Family::AvoidAoe,
                  {ThorimBalconyAdvanceAction::Name});
    e.Multiplier<ThorimBalconyGuardMultiplier>(Family::Attack | Family::Reach | Family::ReachHeal);

    // The class "lose aggro" taunt knows nothing about who should hold him and rips him off the human tank.
    // The pickup row owns the trade instead.
    e.Block("thorim taunt guard", Role::Any, ThorimTauntWouldSteal, Family::Taunt);

    e.Multiplier<BossNatureAspectHoldMultiplier>(Family::Spell, "thorim");

    // The aura slot is exclusive, so the paladin's own buff strategy trades it back on the next GCD.
    e.Block("thorim frost aura hold", Role::Any, ThorimFrostAuraHeld, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "shadow resistance aura", "fire resistance aura"});

    e.Tick(TickThorim);
}
}  // namespace

EncounterDefinition const& UldThorimDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_THORIM, &BossStateGateOpen, &DefineThorim);
    return definition;
}
