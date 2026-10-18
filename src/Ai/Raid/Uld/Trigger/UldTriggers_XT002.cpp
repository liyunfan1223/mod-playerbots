/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_XT002.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "RangeTriggers.h"
#include "UldEncounter_XT002.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"

// XT-002 Deconstructor

namespace
{
// Centre to centre against the raw radii, which is how the hazard dodge scores its candidates.
// XT002AvoidHazardTrigger asks FindNearestCreature instead, and that subtracts both object sizes.
bool InsideXT002Hazard(PlayerbotAI* botAI, Player* bot)
{
    GuidVector const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == PB_NPC_XT002_VOID_ZONE &&
            unit->GetExactDist2d(bot) < ULDUAR_XT002_VOID_ZONE_RADIUS)
            return true;

        if (unit->GetEntry() == PB_NPC_XT002_BOOMBOT && PlayerbotAI::IsMelee(bot) &&
            unit->GetExactDist2d(bot) < ULDUAR_XT002_BOOMBOT_AVOID_RADIUS)
            return true;
    }

    return false;
}
}  // namespace

bool XT002DebuffCarrierTrigger::IsActive()
{
    // No proximity check: nobody steps aside, so the carrier leaves anyway. Auras first, XT's lookup is a scan.
    if (!bot->HasAura(GetXT002GravityBombSpellId(bot)) && !bot->HasAura(GetXT002SearingLightSpellId(bot)))
        return false;

    return GetXT002(botAI) != nullptr;
}

bool XT002AvoidHazardTrigger::IsActive()
{
    // Two small-radius searches before XT's lookup, which is the expensive one.
    bool hazardNear = false;

    // Ranged already stand outside the blast; pulling them out too would only break their casts.
    if (PlayerbotAI::IsMelee(bot) && bot->FindNearestCreature(PB_NPC_XT002_BOOMBOT, ULDUAR_XT002_BOOMBOT_AVOID_RADIUS))
        hazardNear = true;
    // Must match the action's gate or the tick is wasted: a carrier's puddles belong to the carrier action.
    else if (!bot->HasAura(GetXT002SearingLightSpellId(bot)) && !bot->HasAura(GetXT002GravityBombSpellId(bot)) &&
             bot->FindNearestCreature(PB_NPC_XT002_VOID_ZONE, ULDUAR_XT002_VOID_ZONE_RADIUS))
        hazardNear = true;

    return hazardNear && GetXT002(botAI) != nullptr;
}

bool XT002RaidPositionTrigger::IsActive()
{
    // Melee dps and off-tanks never get a spot, so this is always false for them. Same role calls
    // BuildXT002RingMembers makes, answered before XT's lookup.
    if (!PlayerbotAI::IsMainTank(bot) && (PlayerbotAI::IsTank(bot) || (!PlayerbotAI::IsRangedDps(bot) && !PlayerbotAI::IsHeal(bot))))
        return false;

    Unit* xt002 = GetXT002(botAI);

    // Combat-gated, unlike the rest of the encounter's triggers: an anchor that fires on sight has
    // the raid pre-positioning before anyone pulls.
    if (!xt002 || !xt002->IsInCombat())
        return false;

    // Anything the bot has to dodge outranks standing on a spot, and the carriers have destinations of
    // their own, so the anchor stands down rather than fighting them for the tick.
    if (bot->HasAura(GetXT002SearingLightSpellId(bot)) || bot->HasAura(GetXT002GravityBombSpellId(bot)))
        return false;

    // Stand down for the dodge only while really in a puddle by its centre to centre measure, or the dodge fails
    // every tick in the FindNearestCreature size gap while this keeps yielding.
    XT002AvoidHazardTrigger avoidHazard(botAI);
    if (avoidHazard.IsActive() && InsideXT002Hazard(botAI, bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        // Only while holding XT: the anchor outranks "reach melee", and XT is a vehicle, taunt-immune through
        // Vehicle::ApplyAllImmunities. No victim means the Heart window, where the spot is right.
        Unit* victim = xt002->GetVictim();
        if (victim && victim != bot)
            return false;

        return bot->GetExactDist(ULDUAR_XT002_MAINTANK_SPOT) > ULDUAR_XT002_MAINTANK_SPOT_TOLERANCE;
    }

    if (PlayerbotAI::IsHeal(bot))
    {
        // Stands down while anyone is out of heal range: this outranks "reach party member to heal", and the far
        // cells are 55 yd from the anchor.
        PartyMemberToHealOutOfSpellRangeTrigger outOfHealRange(botAI);
        if (outOfHealRange.IsActive())
            return false;
    }

    // Same call the action makes, so the two cannot disagree about which slot is this bot's.
    Position slot;
    if (!GetXT002RangedSlot(bot, xt002, slot))
        return false;

    return bot->GetExactDist(slot) > ULDUAR_XT002_RANGED_SPOT_TOLERANCE;
}

bool XT002SetDpsPriorityTrigger::IsActive()
{
    Unit* xt002 = GetXT002(botAI);

    // The action calls Attack() directly, so without the combat gate the first bot to see XT pulls
    // him. Whoever pulls flips this for everyone, which is what makes the raid engage together.
    return xt002 && xt002->IsInCombat();
}

bool XT002PummellerTauntTrigger::IsActive()
{
    // Only a tank can own the Pummeller. Own role first, it is a flag test and XT's lookup is a scan.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetXT002(botAI))
        return false;

    if (!IsXT002PummellerTank(botAI, bot))
        return false;

    // Taunt range, not the leash: this node sits at ACTION_RAID + 4, so firing it against an add the
    // cast cannot reach would burn the tick while a nearer one goes untaunted.
    Unit* pummeller = GetXT002EngageableAdd(botAI, bot, PB_NPC_XT002_PUMMELLER, ULDUAR_XT002_TAUNT_RANGE);
    if (!pummeller)
        return false;

    return botAI->GetPlayer(pummeller->GetTarget()) != bot;
}

bool XT002RedirectThreatTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* xt002 = GetXT002(botAI);
    if (!xt002 || !xt002->IsAlive())
        return false;

    // Submerged XT is REACT_PASSIVE and off everyone's threat list, so a redirect fired there would
    // burn its charges on nothing.
    return !IsXT002Submerged(xt002);
}
