/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Hodir.h"
#include "UldActions_Shared.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldEncounter_Hodir.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "Unit.h"

using namespace EncounterHelpers;

namespace
{
bool IsEmptyPosition(Position const& position)
{
    return !position.GetPositionX() && !position.GetPositionY();
}

bool IsClearOfHazards(Position const& position, std::vector<HazardCircle> const& hazards)
{
    for (HazardCircle const& hazard : hazards)
        if (hazard.first.GetExactDist2d(&position) < hazard.second)
            return false;

    return true;
}

// Any cast in flight blocks the spline: MoveTo still succeeds and the bot just stands there. Only nodes
// already in danger call this: a cast is worth more than a shed leg, less than a 14000 hit.
void BreakCastPinningTheFeet(PlayerbotAI* botAI, Player* bot)
{
    if (bot->IsMovementPreventedByCasting())
        botAI->RequestSpellInterrupt();
}

// MoveTo refuses a point it issued for MaxWaitForMove (5s), moving or not, so a walk stopped short is never
// reissued. Clear the booking once the bot stands still past the stall time, which a fresh spline never does.
void ReleaseStalledWalk(PlayerbotAI* botAI, Player* bot)
{
    LastMovement& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    if (!bot->isMoving() && last.msTime && getMSTimeDiff(last.msTime, getMSTime()) > ULDUAR_HODIR_STALL_MS)
        last.clear();
}

}  // namespace

bool HodirMoveSnowpackedIcicleAction::Execute(Event /*event*/)
{
    // The trigger fires only outside the release ring with somewhere to shelter on the floor, so
    // reaching here always means the bot has to cross ground it is not standing on yet.
    BreakCastPinningTheFeet(botAI, bot);

    Creature* shelter = GetHodirShelter(botAI, bot);
    if (!shelter)
        return false;

    // MoveInside answers false inside the radius, so a bot caught in a falling drift's blast falls through
    // to the dodge, which walks it out at the same clear.
    ReleaseStalledWalk(botAI, bot);
    return MoveInside(bot->GetMapId(), shelter->GetPositionX(), shelter->GetPositionY(),
                      shelter->GetPositionZ(), GetHodirShelterPark(shelter),
                      MovementPriority::MOVEMENT_COMBAT);
}

bool HodirFrostResistanceAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("frost resistance aura", Event(), true);
}

bool HodirIcicleDodgeAction::Execute(Event /*event*/)
{
    // The trigger fires only inside a lethal radius, and it stands tanks down, so every bot that
    // reaches here is standing somewhere that kills and has about 3s of icicle fuse to leave it.
    BreakCastPinningTheFeet(botAI, bot);

    std::vector<HazardCircle> const hazards = CollectHodirIcicleHazards(
        bot, ULDUAR_HODIR_ROOM_SEARCH_RADIUS, ULDUAR_HODIR_ICE_SHARDS_CLEAR, ULDUAR_HODIR_BIG_SHARDS_CLEAR);
    if (hazards.empty())
    {
        _dest = Position();
        return false;
    }

    // Hold the spot through arrival, re-checked against live hazards: the sweep rings out from the bot, so
    // re-deriving is another 2 yd hop. A Duplicate false here is fine, the forced walk still holds the slot.
    if (!IsEmptyPosition(_dest) && IsClearOfHazards(_dest, hazards))
    {
        float const remaining = bot->GetExactDist2d(&_dest);
        if (remaining <= ULDUAR_HODIR_DODGE_ARRIVE)
            return false;

        if (remaining <= _destDist + ULDUAR_HODIR_DODGE_SLIP)
        {
            _destDist = std::min(_destDist, remaining);
            ReleaseStalledWalk(botAI, bot);
            return MoveTo(bot->GetMapId(), _dest.GetPositionX(), _dest.GetPositionY(), _dest.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED);
        }
    }

    Position preference;
    Position const* preferNear = GetHodirDodgePreference(botAI, bot, preference) ? &preference : nullptr;

    // First hit is the shortest walk MoveTo will accept. Not FleePosition: it clamps travel to
    // Playerbots.FleeDistance and reads one hazard, so a second icicle leaves the bot in the blast.
    Position dest = FindNearestPositionClearOfHazards(bot, hazards, ULDUAR_HODIR_DODGE_LEASH, ULDUAR_HODIR_SWEEP_STEP,
                                                      ULDUAR_HODIR_SWEEP_ANGLE_STEP, preferNear);

    // Nothing fully clear: the clears above were margin, these are the radii that actually kill, and
    // moving to the least bad spot beats holding still because the sweep found no perfect one.
    if (IsEmptyPosition(dest))
        dest = FindNearestPositionClearOfHazards(
            bot,
            CollectHodirIcicleHazards(bot, ULDUAR_HODIR_ROOM_SEARCH_RADIUS,
                                      ULDUAR_HODIR_ICE_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN,
                                      ULDUAR_HODIR_BIG_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN),
            ULDUAR_HODIR_DODGE_LEASH * 2.0f, ULDUAR_HODIR_SWEEP_STEP, ULDUAR_HODIR_SWEEP_ANGLE_STEP, preferNear);

    if (IsEmptyPosition(dest))
    {
        _dest = Position();
        return false;
    }

    _dest = dest;
    _destDist = bot->GetExactDist2d(&_dest);

    // FORCED to outrank the anchor walking the bot back. A second icicle can't preempt it (IsWaitingForLastMove
    // wants strictly higher), which is what the 4.5 yd declump spacing is for. Don't escalate further.
    return MoveTo(bot->GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false,
                  false, false, false, MovementPriority::MOVEMENT_FORCED);
}

bool HodirBitingColdShedAction::Execute(Event /*event*/)
{
    Aura* cold = bot->GetAura(SPELL_BITING_COLD_PLAYER_AURA);
    if (!cold)
    {
        _shedding = false;
        _leg = Position();
        return false;
    }

    // Same check everything that steps aside for the shed uses, so they can't disagree. It reads the buff
    // auras, not a zone lookup: that's what the walk would lose, and it needs no grid sweep.
    if (!_shedding && !IsHodirBitingColdShedArmed(bot))
        return false;

    // Once started, go down to the floor: a stack only drops on the second moving tick in a row, so
    // stopping between stacks wastes the walk already done.
    if (_shedding && cold->GetStackAmount() <= ULDUAR_HODIR_BITING_COLD_SHED_FLOOR)
    {
        _shedding = false;
        _leg = Position();
        return false;
    }

    _shedding = true;

    // A channel pins the feet for its whole length, so legs get accepted and never walked. A normal cast
    // ends and then walks, so only a channel is broken.
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) && bot->IsMovementPreventedByCasting())
        botAI->RequestSpellInterrupt();

    // Hold one leg till it's walked instead of re-deriving under the walk. On arrival derive the next leg,
    // don't stop: only the chain covers consecutive aura ticks.
    if (!IsEmptyPosition(_leg))
    {
        float const remaining = bot->GetExactDist2d(&_leg);
        bool const clear = IsClearOfHazards(
            _leg, CollectHodirIcicleHazards(bot, ULDUAR_HODIR_ROOM_SEARCH_RADIUS, ULDUAR_HODIR_ICE_SHARDS_CLEAR,
                                            ULDUAR_HODIR_BIG_SHARDS_CLEAR));

        if (clear && remaining > ULDUAR_HODIR_DODGE_ARRIVE && remaining <= _legDist + ULDUAR_HODIR_DODGE_SLIP)
        {
            _legDist = std::min(_legDist, remaining);

            ReleaseStalledWalk(botAI, bot);
            return MoveTo(bot->GetMapId(), _leg.GetPositionX(), _leg.GetPositionY(), _leg.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    Position leg;
    if (!GetHodirShuttleLeg(botAI, bot, leg))
    {
        _leg = Position();
        return false;
    }

    _leg = leg;
    _legDist = bot->GetExactDist2d(&_leg);

    return MoveTo(bot->GetMapId(), leg.GetPositionX(), leg.GetPositionY(), leg.GetPositionZ(), false,
                  false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool HodirRaidPositionAction::Execute(Event /*event*/)
{
    Position anchor;
    float tolerance = 0.0f;
    if (GetHodirAnchor(botAI, bot, anchor, tolerance))
    {
        // No arrival latch: the trigger already stands down inside the tolerance, and a latch would swallow
        // a re-anchor, since a bot Hodir walked onto is usually still inside twice the tolerance.
        if (bot->GetExactDist2d(&anchor) <= tolerance)
            return false;

        // The exact point, not MoveInside: that one offsets the destination by the tolerance at the
        // bot's follow angle, which parks the bot a couple of yards outside the zone it was sent to.
        return MoveTo(bot->GetMapId(), anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    // No stand, so a constraint broke: too close, too far, or sharing an icicle splash. Step to the nearest
    // spot that fixes all three, never a home spot, which walks bots back out of the buffs they're in.
    std::vector<HazardCircle> hazards;

    // Only the neighbours the trigger's declump rule counts. Melee are stacked on him by design, so
    // making them hazards walks the bot out over a clump nothing ever complained about.
    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest friendly players"))
    {
        Unit* ally = botAI->GetUnit(guid);
        if (!ally || ally == bot || !ally->IsAlive() || !ally->IsPlayer())
            continue;

        if (!PlayerbotAI::IsRanged(ally->ToPlayer()) && !PlayerbotAI::IsHeal(ally->ToPlayer()))
            continue;

        if (bot->GetExactDist2d(ally) <= ULDUAR_HODIR_DODGE_LEASH)
            hazards.emplace_back(ally->GetPosition(), ULDUAR_HODIR_DECLUMP_RADIUS);
    }

    for (HazardCircle const& icicle : CollectHodirIcicleHazards(bot, ULDUAR_HODIR_ROOM_SEARCH_RADIUS,
                                                               ULDUAR_HODIR_ICE_SHARDS_CLEAR,
                                                               ULDUAR_HODIR_BIG_SHARDS_CLEAR))
        hazards.push_back(icicle);

    Unit* hodir = GetHodir(botAI);
    if (hodir)
        hazards.emplace_back(hodir->GetPosition(), ULDUAR_HODIR_RANGED_MIN_BOSS_GAP);

    // The band's far end is a cap, not a circle to stand outside, so it goes in as an accept rule. Otherwise
    // the outward sweep can answer further out than the spot that fired the trigger.
    auto inBand = [&](float x, float y)
    { return !hodir || hodir->GetExactDist2d(x, y) <= ULDUAR_HODIR_CASTER_MAX_BOSS_GAP; };

    Position const dest = FindNearestPositionClearOfHazards(
        bot, hazards, ULDUAR_HODIR_CASTER_MAX_BOSS_GAP, ULDUAR_HODIR_SWEEP_STEP, ULDUAR_HODIR_SWEEP_ANGLE_STEP, nullptr,
        inBand);
    if (IsEmptyPosition(dest))
        return false;

    ReleaseStalledWalk(botAI, bot);
    return MoveTo(bot->GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT);
}

Unit* HodirSetDpsPriorityAction::ResolveTarget()
{
    Unit* hodir = GetHodir(botAI);
    if (!hodir)
        return nullptr;

    // Tanks never break ice. He follows whoever holds him, so a tank that walks 20 yd to a block puts
    // him on the ranged formation, and a freed helper is worth a lot less than that costs.
    if (PlayerbotAI::IsTank(bot))
        return hodir;

    Unit* trappedAlly = nullptr;

    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest npcs"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() != NPC_HODIR_FLASH_FREEZE_PLAYER)
            continue;

        if (bot->GetExactDist2d(unit) > ULDUAR_HODIR_TRAPPED_ALLY_RANGE)
            continue;

        if (!IsHodirTrappedAllyBreaker(botAI, bot, unit))
            continue;

        // Sticky: switch only when another is clearly closer, or two bots standing either side of a
        // pair ping-pong between them.
        if (!trappedAlly ||
            bot->GetExactDist2d(unit) + ULDUAR_HODIR_TRAPPED_ALLY_SWITCH_MARGIN < bot->GetExactDist2d(trappedAlly))
            trappedAlly = unit;
    }

    // Helper blocks come from a raid-wide breaker budget, not a per-block cap: one freeze puts up eight.
    // Skipped with a trapped raider in hand, which outranks a helper anyway, to save the grid pass.
    Unit* helperBlock = trappedAlly ? nullptr : GetHodirAssignedHelperBlock(botAI, bot);

    std::vector<std::pair<uint32, Unit*>> const priority = {
        {static_cast<uint32>(NPC_HODIR_FLASH_FREEZE_PLAYER), trappedAlly},
        {static_cast<uint32>(NPC_HODIR_FLASH_FREEZE_BLOCK), helperBlock},
        {static_cast<uint32>(NPC_HODIR), hodir}};

    Unit* target = nullptr;
    for (auto const& candidate : priority)
    {
        if (candidate.second)
        {
            target = candidate.second;
            break;
        }
    }

    // No stickiness across entries: a block outranks Hodir while it lives, assigned to this bot or not, so
    // holding one parks the bot on helper ice. The 10 yd margin above is all the hysteresis needed.
    if (!target)
        target = AI_VALUE(Unit*, "dps target");

    return target;
}

bool HodirSetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();
    Unit* target = ResolveTarget();
    if (!target)
        return false;

    // No threat wipe in this fight and it's a race, so nobody should be throttling.
    if (target == GetHodir(botAI))
        context->GetValue<bool>("neglect threat")->Set(true);

    bool needsAttack = currentTarget != target;
    if (!needsAttack && PlayerbotAI::IsMelee(bot))
        needsAttack = !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);

    if (needsAttack)
        return Attack(target);

    return false;
}

bool HodirFrozenBlowsSwapAction::Execute(Event /*event*/)
{
    // Class taunts are on an 8s cooldown, so a swap waiting on it retries at tick rate until it lands.
    return CastClassTaunt(botAI, GetHodir(botAI));
}

Player* HodirRedirectThreatAction::GetRedirectTank()
{
    // Follow the swap rather than mirroring its state: whoever is holding him is the one who needs
    // the lead, and that is the off-tank for the length of a Frozen Blows window.
    if (Player* holding = GetTankHolding(GetHodir(botAI)))
        return holding;

    // Hodir is on somebody who is not a tank, which is the case the redirect exists for. Feed the
    // main tank so the taunt that follows has something to hold.
    return GetGroupMainTank(bot);
}

Unit* HodirRedirectThreatAction::GetThreatDumpTarget() { return GetHodir(botAI); }

bool HodirSpreadStormCloudAction::Execute(Event /*event*/)
{
    Position rally;
    if (!GetHodirStormCloudRally(botAI, bot, bot, rally))
        return false;

    // Holding still is the job: Storm Power pulses 3 yd round the carrier, so a walking carrier spends its
    // charges on whoever it passes. The rally is where it stood, so this only walks once pushed off it.
    return MoveInside(bot->GetMapId(), rally.GetPositionX(), rally.GetPositionY(), rally.GetPositionZ(),
                      ULDUAR_HODIR_STORM_CLOUD_COLLECT_PARK, MovementPriority::MOVEMENT_COMBAT);
}

bool HodirCollectStormPowerAction::Execute(Event /*event*/)
{
    Player* carrier = GetHodirStormCloudCarrier(botAI, bot);
    if (!carrier || carrier == bot)
        return false;

    Position rally;
    if (!GetHodirStormCloudRally(botAI, bot, carrier, rally))
        return false;

    // No cast break here, unlike the dodge and the shelter run: dropping a cast to walk 3 yd for a buff
    // costs more than the pulse is worth.
    return MoveInside(bot->GetMapId(), rally.GetPositionX(), rally.GetPositionY(), rally.GetPositionZ(),
                      ULDUAR_HODIR_STORM_CLOUD_COLLECT_PARK, MovementPriority::MOVEMENT_COMBAT);
}
