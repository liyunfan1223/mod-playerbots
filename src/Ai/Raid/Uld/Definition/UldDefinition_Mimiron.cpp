/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "EncounterHelpers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "UldActions_Mimiron.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Mimiron.h"
#include "UldHardMode.h"
#include "UldMultipliers_Mimiron.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "UldTriggers_Mimiron.h"

using namespace EncounterHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// Windows where a hit kills the bot. Mines and Bomb Bots stay out on purpose: a mine hit is healable and its
// row ranks last, so letting it veto a charge would contradict that ranking.
bool MimironLethalWindowActive(PlayerbotAI* botAI)
{
    if (!GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) && !GetUldUnitByEntry(botAI, NPC_VX001) &&
        !GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return false;

    MimironShockBlastTrigger shockBlast(botAI);
    if (shockBlast.IsActive())
        return true;

    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    if (barrage.IsActive())
        return true;

    MimironRocketStrikeTrigger rocketStrike(botAI);
    if (rocketStrike.IsActive())
        return true;

    // Both of these check the hard-mode config first, so they cost nothing on a normal clear.
    MimironDodgeFlamesTrigger dodgeFlames(botAI);
    if (dodgeFlames.IsActive())
        return true;

    MimironFrostBombTrigger frostBomb(botAI);
    return frostBomb.IsActive();
}

// Whole barrage: the dodge hands the tick back at the band edge and phase 4 melee have no slot, so the
// unstacker would walk them back into the band. Otherwise only while the bot stands on its slot.
bool MimironFormationHeld(PlayerbotAI* botAI)
{
    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    if (barrage.IsActive())
        return true;

    Player* bot = botAI->GetBot();
    Position slot;
    return GetMimironSpreadSlot(botAI, bot, slot) &&
           bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY()) <= ULDUAR_MIMIRON_SPREAD_TOLERANCE;
}

// Only while the fire is there to be mishandled: the hard mode switch is a config read that holds all
// over Ulduar.
bool MimironFireFight(PlayerbotAI* botAI) { return IsMimironHardModeActive(botAI) && IsMimironEngaged(botAI); }

// Empty off hard mode, so a normal clear eats and drinks as before.
bool MimironNearFire(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    for (Position const& node : hazards.flames)
        if (bot->GetExactDist2d(node.GetPositionX(), node.GetPositionY()) < ULDUAR_MIMIRON_DRINK_FIRE_CLEARANCE)
            return true;

    return false;
}

// Room test before the grid scan: the hard mode switch holds all over Ulduar.
bool MimironNearFrostBomb(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsMimironHardModeActive(botAI) || !IsNearMimironRoom(bot))
        return false;

    float const hold = ULDUAR_MIMIRON_FROST_BOMB_CLEARANCE + ULDUAR_MIMIRON_FROST_BOMB_HOLD_MARGIN;
    Creature* bomb = bot->FindNearestCreature(NPC_FROST_BOMB, hold + ULDUAR_MIMIRON_HOLD_SEARCH_PAD);
    return bomb && bot->GetExactDist2d(bomb) < hold;
}

// Room test first, so the grid scan only runs at Mimiron. Rocket Strike is cast in normal mode too.
bool MimironNearRocket(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsNearMimironRoom(bot))
        return false;

    float const hold = ULDUAR_MIMIRON_ROCKET_CLEARANCE + ULDUAR_MIMIRON_ROCKET_HOLD_MARGIN;
    Creature* rocket = bot->FindNearestCreature(NPC_ROCKET_STRIKE_N, hold + ULDUAR_MIMIRON_HOLD_SEARCH_PAD);
    return rocket && bot->GetExactDist2d(rocket) < hold;
}

// Water Spray is an instant 15 yd line that one-shots, so nothing may walk a bot back into one.
bool MimironFireHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsMimironHardModeActive(botAI) || !IsNearMimironRoom(bot))
        return false;

    return IsMimironFireHoldActive(bot) ||
           IsMimironSpotInFireBotSpray(GetMimironFirefighterHazards(botAI), bot->GetPosition(),
                                       ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH);
}

bool MimironResistancePaladin(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_PALADIN && GetMimironFrostResistancePaladin(botAI, bot) == bot;
}

bool MimironRaidDropping(PlayerbotAI* botAI)
{
    return IsMimironPhase2(botAI) && botAI->GetAiObjectContext()->GetValue<uint8>("aoe heal", "low")->Get() <
                                         ULDUAR_MIMIRON_STORM_COOLDOWN_LOW_COUNT;
}

bool MimironMeleeSector(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsRanged(bot))
        return false;

    if (IsMimironHardModeActive(botAI) && (IsMimironPhase2(botAI) || IsMimironPhase4(bot)))
        return true;

    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    return barrage.IsActive();
}

bool MimironPhase4MainTank(PlayerbotAI* botAI) { return IsMimironPhase4(botAI->GetBot()); }

void DefineMimiron(EncounterBuilder& e)
{
    // Ranked by what a hit costs: the barrage cone one-shots, Shock Blast is 100000 in 15 yd, a mine is healable.
    // No Rapid Burst node: a step lands too late to save a tick. Destinations are screened against the live cone.
    e.Node<MimironP3Wx2LaserBarrageTrigger, MimironP3Wx2LaserBarrageAction>(ACTION_RAID + 7);
    e.Node<MimironShockBlastTrigger, MimironShockBlastAction>(ACTION_RAID + 5.5f);
    e.Node(
        "mimiron fire resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossFireResistanceTrigger(botAI, "mimiron"); },
        "mimiron fire resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossFireResistanceAction(botAI, "mimiron"); }, ACTION_RAID);
    e.Node<MimironFrostResistanceTrigger, MimironFrostResistanceAction>(ACTION_RAID);
    e.Node<MimironPhase1PositioningTrigger, MimironPhase1PositioningAction>(ACTION_RAID);
    e.Node<MimironArcSpreadTrigger, MimironArcSpreadAction>(ACTION_RAID);
    e.Node<MimironAerialCommandUnitTrigger, MimironAerialCommandUnitAction>(ACTION_RAID);
    e.Node<MimironRocketStrikeTrigger, MimironRocketStrikeAction>(ACTION_RAID + 5);
    e.Node<MimironPhase4FocusTrigger, MimironPhase4FocusAction>(ACTION_RAID);
    e.Node<MimironMagneticCoreTrigger, MimironMagneticCoreAction>(ACTION_RAID + 1);
    e.Node<MimironPlasmaBlastDefensiveTrigger, MimironPlasmaBlastDefensiveAction>(ACTION_RAID + 6);
    e.Node<MimironRedirectThreatTrigger, MimironRedirectThreatAction>(ACTION_RAID + 1);
    e.Node<MimironSetDpsPriorityTrigger, MimironSetDpsPriorityAction>(ACTION_RAID);
    // Last of the Mimiron nodes and tied with none of them, so it only ever takes a tick no other
    // mechanic wants. A mine can never cost the raid a dodge, a taunt or a core delivery.
    e.Node<MimironProximityMineTrigger, MimironProximityMineAction>(ACTION_RAID - 1);
    e.Node<MimironBombBotTrigger, MimironBombBotAction>(ACTION_RAID + 2);
    e.Node<MimironSlowBombBotTrigger, MimironSlowBombBotAction>(ACTION_RAID + 2);
    // The action always returns false, so this only ever redirects the pet and the bot keeps its tick.
    e.Node<MimironPetControlTrigger, MimironPetControlAction>(ACTION_RAID);
    // Hard mode only. Bomb above fire, never tied: a tie goes by push order, and the fire step would end the tick
    // first. Fire is 3000 a second and healable, the 30 yd blast one-shots.
    e.Node<MimironDodgeFlamesTrigger, MimironDodgeFlamesAction>(ACTION_RAID + 4);
    // Under every Mimiron node and over the generic reach nodes at ACTION_HIGH: only a bot the formation and
    // dodges left idle walks at its target, and this steers it round the fire.
    e.Node<MimironApproachTargetTrigger, MimironApproachTargetAction>(ACTION_RAID - 2);
    e.Node<MimironFrostBombTrigger, MimironFrostBombAction>(ACTION_RAID + 6);
    // Below the fire dodge, whose fan already refuses the spray line and the silence. A fire bot only
    // sprays when it reaches a flame; the fire ticks every second.
    e.Node<MimironFireBotTrigger, MimironFireBotAction>(ACTION_RAID + 3);
    e.Node<MimironResetEncounterStateTrigger, MimironResetEncounterStateAction>(ACTION_RAID);

    // The set dps priority row owns every non-tank's target, or the generic picker drags bots to the nearest.
    // Engaged only: that row is idle before the pull. "attack rti target" stays so a player's mark still wins.
    e.OwnTargeting("mimiron target guard", Role::NonTank, IsMimironEngaged, Family::DpsAssist);

    // Phase 4 focus row picks the main tank's target. VX-001 never takes a victim, so tank assist
    // reads it as loose and keeps pulling him off the MK II, so his rotation never runs.
    e.OwnTargeting("mimiron phase 4 tank target guard", Role::MainTank, MimironPhase4MainTank);

    // Gap-closers and "reach melee" go straight at the target, blind to the ground. Other reaches stay or ranged
    // and healers get stranded. Killing Spree teleports inside VX-001's barrage boundary, where beams ignore bearing.
    e.Block("mimiron charge guard", Role::Any, MimironLethalWindowActive, Family::Charge,
            {"reach melee", "killing spree"});

    // Generic avoid aoe flees only the aura's 3 yd, onto the next node of a field growing 7 yd a step, then
    // returns false, so the Mimiron dodge runs into the movement lock it still holds.
    e.Block("mimiron avoid aoe guard", Role::Any, MimironFireFight, 0, {"avoid aoe"});

    // The unstacker and the formation both move the same ranged bot, so on its slot only the formation may.
    // Tank face derives from the formation move but keeps the tank pointed away from the raid, so it isn't here.
    e.Block("mimiron formation guard", Role::Any, MimironFormationHeld, Family::CombatFormationMove);

    e.Multiplier<MimironTankAnchorGuardMultiplier>(Family::TankFace | Family::Reach);

    // Eating or drinking stops the bot thinking for up to 18 s, so one that sits down near the fire
    // doesn't dodge it. Near fire only: a handover drink on clean ground is worth the mana.
    e.Block("mimiron drink guard", Role::Any, MimironNearFire, 0, {"drink", "food"});

    // Every mover that aims at a unit rather than the floor. The bomb flee lets go just inside its clearance, so
    // any of these walks the bot back into the blast for the rest of the fuse.
    e.Block("mimiron frost bomb guard", Role::Any, MimironNearFrostBomb, 0,
            {"reach melee", "reach spell", "reach party member to heal", "set behind", "follow",
             MimironApproachTargetAction::Name});
    // A window after the dodge, not fire nearby: that's all of phase 3 and would strand melee and healers.
    e.Block("mimiron fire hold guard", Role::Any, MimironFireHold, 0,
            {"reach melee", "reach spell", "reach party member to heal", "set behind", "follow",
             MimironApproachTargetAction::Name});
    // Same for a Rocket Strike marker: 5000000 in 3 yd, and the dodge lets go at the clearance.
    e.Block("mimiron rocket guard", Role::Any, MimironNearRocket, 0,
            {"reach melee", "reach spell", "reach party member to heal", "set behind", "follow",
             MimironApproachTargetAction::Name});

    // Whether an action splashes is its spell, and the totem and trap summoners are named.
    e.Multiplier<MimironFireBotAoeGuardMultiplier>(Family::AnyAction);
    e.Multiplier<MimironPlasmaDefensiveHoldMultiplier>(Family::AnyAction);

    // Holds the paladin aura slot open for the phase 3 Frost Resistance Aura, including the shared fire
    // row, which would hand this bot "rfire" after its own pick dies. Lifts with phase 3.
    e.Block("mimiron paladin aura", Role::Any, MimironResistancePaladin, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "shadow resistance aura", "fire resistance aura", "mimiron fire resistance action"});

    // Their trigger fires on the walk-in Rapid Burst, which healers top off anyway, leaving all three down for
    // the storm after the first barrage. Holding Divine Sacrifice holds its cancel too.
    e.Block("mimiron storm cooldown hold", Role::Any, MimironRaidDropping, 0,
            {"divine sacrifice", "divine hymn", "power infusion"});

    // Firefighter phases 2 and 4: melee hold the sector opposite the wedge, and VX-001 turns to each Rapid Burst
    // and Hand Pulse target, so "behind" drifts off it. In any barrage it walks melee from the band edge into beams.
    e.Block("mimiron vx001 facing guard", Role::Any, MimironMeleeSector, Family::TankFace, {"set behind"});

    e.Tick(MimironTick);
}
}  // namespace

EncounterDefinition const& UldMimironDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_MIMIRON, &BossStateGateOpen, &DefineMimiron);
    return definition;
}
