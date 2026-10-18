/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Algalon.h"

#include <cmath>
#include <vector>

#include "AiObjectContext.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaidTankDefensive.h"
#include "Timer.h"
#include "UldData.h"
#include "UldEncounter_Algalon.h"
#include "Unit.h"

using namespace EncounterHelpers;

// Algalon the Observer

namespace
{
// A walk accepted while a cast holds the feet books the movement slot and goes nowhere until the cast
// ends. Only the runs that have to land in time call this.
void BreakCastPinningTheFeet(PlayerbotAI* botAI, Player* bot)
{
    if (bot->IsMovementPreventedByCasting())
        botAI->RequestSpellInterrupt();
}

// MoveTo refuses a point it already issued for MaxWaitForMove, moving or not, so a walk stopped short
// (a mana gem's StopMoving, Disengage) is never issued again unless the booking is dropped.
LastMovement& Booking(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
}

void ReleaseStalledWalk(PlayerbotAI* botAI, Player* bot)
{
    LastMovement& last = Booking(botAI);
    if (!bot->isMoving() && last.msTime && getMSTimeDiff(last.msTime, getMSTime()) > ULDUAR_ALGALON_STALL_MS)
        last.clear();
}

// A walk waits behind a booking of equal priority until it runs out. The walks that must land within a
// second drop one they didn't make.
void DropForeignBooking(PlayerbotAI* botAI, Position const& own, MovementPriority upTo)
{
    LastMovement& last = Booking(botAI);
    if (last.msTime && last.priority <= upTo &&
        std::hypot(last.lastMoveToX - own.GetPositionX(), last.lastMoveToY - own.GetPositionY()) > 0.5f)
        last.clear();
}

// How close a walk may pass another hole: with margin, then just outside its 6 yd field, then anywhere.
// Callers fall back down the list: the void Dark Matter still beats a meteor or staying in the field.
enum class AlgalonPathRule
{
    Clear,
    Field,
    Any,
};

// Nearest ground clear of every marker, hole and dying star, near the bot's own slot when it has one. Searches
// chained in one Execute share `sweep`: they walk the same grid, so each point's collision check is paid once.
Position FindAlgalonClearSpot(Player* bot, float markerClearance, AlgalonPathRule rule, HazardSweepCache& sweep)
{
    Position anchor;
    Position const* prefer = GetAlgalonSlotAnchor(bot, anchor) ? &anchor : nullptr;
    auto const reachable = [bot, rule](float x, float y)
    {
        if (!AlgalonSpotInRoom(x, y))
            return false;

        switch (rule)
        {
            case AlgalonPathRule::Clear:
                return !AlgalonPathCrossesHole(bot, x, y);
            case AlgalonPathRule::Field:
                return !AlgalonPathCrossesHole(bot, x, y, ULDUAR_ALGALON_FIELD_PATH_CLEARANCE);
            default:
                return true;
        }
    };
    return FindNearestPositionClearOfHazards(bot, GetAlgalonDodgeHazards(bot, markerClearance),
                                             ULDUAR_ALGALON_DODGE_SEARCH_RADIUS, ULDUAR_ALGALON_SWEEP_STEP,
                                             ULDUAR_ALGALON_SWEEP_ANGLE_STEP, prefer, reachable, &sweep);
}

// The class's second taunt, for when the first missed or is cooling down while he is off a swap tank.
// Righteous Defense goes on the friend he is hitting; the shout and the roar taunt everything near the tank.
bool CastBackupTaunt(PlayerbotAI* botAI, Player* bot, Unit* boss)
{
    Unit* victim = boss->GetVictim();
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            return victim && victim != bot && victim->IsPlayer() &&
                   botAI->CanCastSpell("righteous defense", victim) && botAI->CastSpell("righteous defense", victim);
        case CLASS_WARRIOR:
            return bot->IsWithinDistInMap(boss, ULDUAR_ALGALON_AREA_TAUNT_RANGE) &&
                   botAI->CanCastSpell("challenging shout", bot) && botAI->CastSpell("challenging shout", bot);
        case CLASS_DRUID:
            return bot->IsWithinDistInMap(boss, ULDUAR_ALGALON_AREA_TAUNT_RANGE) &&
                   botAI->CanCastSpell("challenging roar", bot) && botAI->CastSpell("challenging roar", bot);
        default:
            return false;
    }
}
}  // namespace

bool AlgalonBigBangSoakAction::Execute(Event /*event*/)
{
    // Dispersion's 90% survives it outright; the tank's own armour and a physical defensive do the rest.
    if (AlgalonCanDisperse(bot))
    {
        if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            bot->InterruptSpell(CURRENT_CHANNELED_SPELL);

        return botAI->CastSpell("dispersion", bot);
    }

    uint32 const defensive = NextTankDefensive(botAI, bot, true);
    return defensive && botAI->CastSpell(defensive, bot);
}

bool AlgalonBigBangExternalAction::Execute(Event /*event*/)
{
    Player* soaker = GetAlgalonBigBangSoaker(botAI);
    char const* external = GetAlgalonSoakerExternal(botAI, soaker);
    return external && botAI->CastSpell(external, soaker);
}

bool AlgalonSoakerPrepAction::Execute(Event /*event*/)
{
    Player* soaker = GetAlgalonBigBangSoaker(botAI);
    char const* layer = GetAlgalonSoakerPrep(botAI, soaker);
    return layer && botAI->CastSpell(layer, soaker);
}

bool AlgalonSoakerRescueAction::Execute(Event /*event*/)
{
    Player* soaker = GetAlgalonRescueTarget(botAI);
    char const* heal = GetAlgalonRescueSpell(botAI, soaker);
    return heal && botAI->CastSpell(heal, soaker);
}

bool AlgalonTankHealAction::Execute(Event /*event*/)
{
    AlgalonTankHealPick const pick = GetAlgalonTankHealPick(botAI);
    char const* heal = GetAlgalonTankHealSpell(botAI, pick);
    return heal && botAI->CastSpell(heal, pick.target);
}

bool AlgalonTankBeaconAction::Execute(Event /*event*/)
{
    Player* target = GetAlgalonBeaconTarget(botAI);
    return target && botAI->CastSpell("beacon of light", target);
}

bool AlgalonTankDefensiveAction::Execute(Event /*event*/)
{
    AlgalonTankCut const cut = GetAlgalonTankCut(botAI);
    if (!cut.spellId)
        return false;

    // A heal half cast is worth less than the tank.
    if (bot->IsNonMeleeSpellCast(false, true))
        bot->InterruptNonMeleeSpells(false);

    if (!botAI->CastSpell(cut.spellId, cut.target))
        return false;

    ClaimAlgalonTankCut(bot, cut);
    return true;
}

bool AlgalonExplosionCoverAction::Execute(Event /*event*/)
{
    char const* cover = GetAlgalonExplosionCover(botAI);
    if (!cover || !botAI->CastSpell(cover, bot))
        return false;

    ClaimAlgalonExplosionCover(bot, cover);
    return true;
}

bool AlgalonExplosionPersonalAction::Execute(Event /*event*/)
{
    // Before the pick: a channel fails every button's cast check and the item use alike.
    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    AlgalonPersonalPick const pick = GetAlgalonExplosionPersonal(botAI);
    if (!pick)
        return false;

    bool const used =
        pick.consumable ? botAI->DoSpecificAction(pick.consumable, Event(), true) : botAI->CastSpell(pick.spellId, bot);
    if (!used)
        return false;

    ClaimAlgalonExplosionPersonal(bot);
    return true;
}

bool AlgalonShadowAuraAction::Execute(Event /*event*/) { return botAI->CastSpell("shadow resistance aura", bot); }

bool AlgalonBigBangHideAction::Execute(Event /*event*/)
{
    Position shelter;
    if (!GetAlgalonShelter(bot, shelter))
        return false;

    // Anywhere inside the field phases the bot on its next pulse.
    if (bot->GetExactDist2d(shelter) <= ULDUAR_ALGALON_SHELTER_IN_RADIUS)
        return false;

    // Big Bang goes through all four. Magic immunity keeps the field's phase off, and Ice Block also holds the mage
    // in place until the hit.
    for (uint32 const immunity : {SPELL_ALGALON_ICE_BLOCK, SPELL_ALGALON_DIVINE_SHIELD, SPELL_ALGALON_CLOAK_OF_SHADOWS,
                                  SPELL_ALGALON_ANTI_MAGIC_SHELL})
        if (bot->HasAura(immunity))
            bot->RemoveAurasDueToSpell(immunity);

    BreakCastPinningTheFeet(botAI, bot);
    ReleaseStalledWalk(botAI, bot);
    AlgalonBoostHideRun(bot, bot->GetExactDist2d(shelter) - ULDUAR_ALGALON_SHELTER_IN_RADIUS);
    // Not a dodge still under way: the hole isn't checked against the meteor.
    if (!AlgalonCosmicSmashThreatens(bot))
        DropForeignBooking(botAI, _walk, MovementPriority::MOVEMENT_FORCED);
    _walk = shelter;

    if (MoveTo(bot->GetMapId(), shelter.GetPositionX(), shelter.GetPositionY(), shelter.GetPositionZ(), false,
               false, false, false, MovementPriority::MOVEMENT_FORCED, true))
        return true;

    // MoveTo answers Duplicate for the rest of the run. Returning false there hands the tick to a heal
    // whose cast then pins the feet.
    return bot->isMoving();
}

bool AlgalonCosmicSmashAction::Execute(Event /*event*/)
{
    // FleePosition clamps to Playerbots.FleeDistance (5 yd), inside the doubled damage band.
    HazardSweepCache sweep;
    Position clear = FindAlgalonClearSpot(bot, ULDUAR_ALGALON_COSMIC_SMASH_CLEARANCE, AlgalonPathRule::Clear, sweep);
    if (clear == Position())
        clear = FindAlgalonClearSpot(bot, ULDUAR_ALGALON_COSMIC_SMASH_MIN_CLEARANCE, AlgalonPathRule::Clear, sweep);

    if (clear == Position())
        clear = FindAlgalonClearSpot(bot, ULDUAR_ALGALON_COSMIC_SMASH_MIN_CLEARANCE, AlgalonPathRule::Field, sweep);

    if (clear == Position())
        clear = FindAlgalonClearSpot(bot, ULDUAR_ALGALON_COSMIC_SMASH_MIN_CLEARANCE, AlgalonPathRule::Any, sweep);

    if (clear == Position())
        return false;

    BreakCastPinningTheFeet(botAI, bot);
    ReleaseStalledWalk(botAI, bot);
    return MoveTo(bot->GetMapId(), clear.GetPositionX(), clear.GetPositionY(), clear.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED, true);
}

bool AlgalonLeaveBlackHoleAction::Execute(Event /*event*/)
{
    Position clear;
    HazardSweepCache sweep;
    for (AlgalonPathRule rule : {AlgalonPathRule::Clear, AlgalonPathRule::Field, AlgalonPathRule::Any})
    {
        clear = FindAlgalonClearSpot(bot, ULDUAR_ALGALON_COSMIC_SMASH_TRIGGER_RADIUS, rule, sweep);
        if (clear != Position())
            break;
    }

    if (clear == Position())
        return false;

    BreakCastPinningTheFeet(botAI, bot);
    ReleaseStalledWalk(botAI, bot);
    DropForeignBooking(botAI, _walk, MovementPriority::MOVEMENT_FORCED);
    if (MoveTo(bot->GetMapId(), clear.GetPositionX(), clear.GetPositionY(), clear.GetPositionZ(), false, false,
               false, false, MovementPriority::MOVEMENT_FORCED, true))
    {
        _walk = clear;
        return true;
    }

    // Its own walk still running answers Waiting; giving the tick away would let a cast pin the feet.
    return bot->isMoving();
}

bool AlgalonYieldBossAction::Execute(Event /*event*/)
{
    Unit* boss = GetAlgalon(botAI);
    if (!boss)
        return false;

    if (bot->GetVictim() == boss)
        bot->AttackStop();

    if (AI_VALUE(Unit*, "current target") == boss)
        context->GetValue<Unit*>("current target")->Set(nullptr);

    return true;
}

Player* AlgalonThreatRedirectAction::GetRedirectTank() { return GetAlgalonOpenerTank(botAI); }

Unit* AlgalonThreatRedirectAction::GetThreatDumpTarget()
{
    Unit* boss = GetAlgalon(botAI);
    return boss && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ? boss : nullptr;
}

bool AlgalonTakeBossAction::Execute(Event /*event*/)
{
    Unit* boss = GetAlgalon(botAI);
    if (!boss)
        return false;

    // Handed the soak on its way into a hole: stop, or the run carries it into the field while the walk
    // back waits behind its booking.
    bool const soaking = AlgalonBigBangLatched(bot) && GetAlgalonBigBangSoaker(botAI) == bot;
    LastMovement& last = Booking(botAI);
    if (soaking && last.msTime && AlgalonWalkEntersField(bot, last.lastMoveToX, last.lastMoveToY,
                                                         ULDUAR_ALGALON_SHELTER_RADIUS))
    {
        bot->StopMoving();
        last.clear();
    }

    // After a taunt the walk to the tank slot goes out next tick, not once the last walk runs out. The taunt
    // moves him, not the bot's own target: a former handler would keep chasing its constellation.
    if (boss->GetVictim() != bot && CastClassTaunt(botAI, boss))
    {
        // Before the clear: Attack stops a walk booked below combat, and a dodge under way is forced.
        Attack(boss);
        last.clear();
        return true;
    }

    Unit* victim = boss->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    if (victim != bot && (!holder || !IsAlgalonSwapTank(holder) || soaking) && CastBackupTaunt(botAI, bot, boss))
    {
        Attack(boss);
        last.clear();
        return true;
    }

    // Taunts on cooldown or out of reach: walk in and hit him.
    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    return false;
}

bool AlgalonConstellationTauntAction::Execute(Event /*event*/)
{
    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    if (!constellation)
        return false;

    // From the far side of a spare hole it comes straight across and is eaten on the way.
    Position behind;
    bool const fromBehind = AlgalonKiteBehind(bot, constellation, behind);
    if (fromBehind && bot->GetExactDist2d(behind) > ULDUAR_ALGALON_SLOT_TOLERANCE)
    {
        Position next;
        if (!AlgalonWalkAround(bot, behind, next))
            return false;

        ReleaseStalledWalk(botAI, bot);
        return MoveTo(bot->GetMapId(), next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true);
    }

    if (CastClassTaunt(botAI, constellation))
        return true;

    // On the spot with the taunt still cooling: hitting it would walk the handler off the line.
    if (fromBehind)
        return false;

    // Hits hold it until the taunt is back.
    if (AI_VALUE(Unit*, "current target") != constellation)
        return Attack(constellation);

    return false;
}

bool AlgalonDarkMatterTankAction::Execute(Event /*event*/)
{
    Unit* darkMatter = GetAlgalonLooseDarkMatter(bot);
    if (!darkMatter)
        return false;

    if (CastClassTaunt(botAI, darkMatter))
        return true;

    if (AI_VALUE(Unit*, "current target") != darkMatter)
        return Attack(darkMatter);

    return false;
}

bool AlgalonConstellationKiteAction::Execute(Event /*event*/)
{
    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    Position spot;
    if (!constellation)
        return false;

    if (!GetAlgalonKiteSpot(bot, constellation, spot))
        return false;

    // Waiting for their cast or for them to come across: the tick goes back to everything else.
    if (bot->GetExactDist2d(spot) <= ULDUAR_ALGALON_SLOT_TOLERANCE)
        return false;

    // The far spot is past the hole, so the straight walk there crosses it.
    Position next;
    if (!AlgalonWalkAround(bot, spot, next))
        return false;

    // Combat, not forced, so the exit and the meteor dodge always win over it.
    ReleaseStalledWalk(botAI, bot);
    DropForeignBooking(botAI, _walk, MovementPriority::MOVEMENT_COMBAT);
    if (!MoveTo(bot->GetMapId(), next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false, false,
                false, MovementPriority::MOVEMENT_COMBAT, true))
        return false;

    _walk = next;
    return true;
}

bool AlgalonStarTeamAction::Execute(Event /*event*/)
{
    Unit* star = GetAlgalonFocusStar(botAI);
    return star && Attack(star);
}

bool AlgalonStarMarkAction::Execute(Event /*event*/)
{
    Unit* star = GetAlgalonFocusStar(botAI);
    return star && MarkTargetWithStar(bot, star);
}

bool AlgalonRaidPositionAction::Execute(Event /*event*/)
{
    Position slot;
    bool waypoint = false;
    if (!TryGetAlgalonSlot(bot, slot, &waypoint))
    {
        _slotReached = false;
        return false;
    }

    float const distance = bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY());

    // A waypoint round a hole is never somewhere to hold: the next check hands out the next leg.
    if (waypoint)
    {
        _slotReached = false;
        if (distance <= ULDUAR_ALGALON_SLOT_TOLERANCE)
            return false;

        return MoveTo(bot->GetMapId(), slot.GetPositionX(), slot.GetPositionY(), slot.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true);
    }

    // Reach then hold, with a deadband. Re-issuing on every yard of drift restarts the spline, and a
    // moving bot can't start a cast.
    if (_slotReached && distance > ULDUAR_ALGALON_SLOT_TOLERANCE * 2.0f)
        _slotReached = false;

    if (_slotReached || distance <= ULDUAR_ALGALON_SLOT_TOLERANCE)
    {
        _slotReached = true;
        return false;
    }

    return MoveTo(bot->GetMapId(), slot.GetPositionX(), slot.GetPositionY(), slot.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}
