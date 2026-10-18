/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Hodir.h"

#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellMgr.h"
#include "UldEncounter_Hodir.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"

// The nearest icicle of an entry that has not detonated yet, or nullptr. Both icicle entries are
// non-selectable, so they never reach "possible targets" and have to be found by a direct search.
static Creature* NearestHodirIcicle(Player* bot, uint32 entry, float radius)
{
    Creature* icicle = bot->FindNearestCreature(entry, radius);
    return IsHodirIcicleLethal(icicle) ? icicle : nullptr;
}

bool HodirBitingColdTrigger::IsActive()
{
    if (!IsHodirEngaged(botAI))
        return false;

    if (bot->HasAura(SPELL_HODIR_FLASH_FREEZE_TRAPPED))
        return false;

    // A Toasty Fire counts as movement on every tick, so a bot standing in one is already shedding a
    // stack every two seconds without going anywhere.
    if (bot->HasAura(SPELL_HODIR_TOASTY_FIRE_AURA))
        return false;

    // Stateless on purpose. The action owns the arm threshold and the shed-to-floor latch, because it
    // is the cached instance and anything held here is lost by a stack-allocated copy.
    return bot->HasAura(SPELL_BITING_COLD_PLAYER_AURA);
}

bool HodirNearSnowpackedIcicleTrigger::IsActive()
{
    if (!GetHodir(botAI))
        return false;

    // Only while the cast is up. A target outlives the freeze by ~7s, and running to it after the landing
    // buys nothing and flips the tank between it and his spot, dragging Hodir off the centre.
    if (!IsHodirFlashFreezeIncoming(botAI))
        return false;

    // A drift counts, at its wider radius: it blows for 14000 in 7 yd, so the raid stands on the edge of
    // the blast instead of waiting it out and then crossing the room in what's left of the cast.
    Creature* shelter = GetHodirShelter(botAI, bot);
    if (!shelter)
        return false;

    // Release, not the park distance: one number at both ends stands this down the tick the bot arrives,
    // and the position anchor walks it straight back out of the shelter.
    return bot->GetExactDist2d(shelter) > GetHodirShelterRelease(shelter);
}

bool HodirFrostResistanceTrigger::IsActive()
{
    if (!IsHodirEngaged(botAI))
        return false;

    if (botAI->HasAura("frost resistance aura", bot))
        return false;

    return GetHodirResistancePaladin(botAI, bot) == bot;
}

bool HodirIcicleDodgeTrigger::IsActive()
{
    if (!GetHodir(botAI))
        return false;

    // Tanks eat the icicle: Hodir follows, so a dodging tank drags him into the ranged. The Biting Cold
    // shuttle already moves them without leaving the spot.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Leaving is decided on the radius that kills; the clear is margin for where the bot lands, and
    // testing it at both ends has bots stepping out of pools they were never standing in.
    if (NearestHodirIcicle(bot, NPC_HODIR_ICICLE_SMALL,
                           ULDUAR_HODIR_ICE_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN))
        return true;

    // A drift icicle is worth dodging only until it lands. After that the Snowpacked Icicle Target
    // it leaves behind is the shelter, and dodging would push the bot out of the one safe spot.
    Creature* drift = NearestHodirIcicle(bot, NPC_HODIR_ICICLE_DRIFT,
                                         ULDUAR_HODIR_BIG_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN);
    if (!drift)
        return false;

    return drift->FindNearestCreature(NPC_SNOWPACKED_ICICLE, ULDUAR_HODIR_SAFE_AREA_RADIUS) == nullptr;
}

bool HodirRaidPositionTrigger::IsActive()
{
    // Combat-gated: an anchor that fires on sight has the raid walking to its spots before anyone
    // has pulled.
    if (!IsHodirEngaged(botAI))
        return false;

    if (bot->HasAura(SPELL_HODIR_FLASH_FREEZE_TRAPPED))
        return false;

    // Surviving beats standing on a spot, and letting the anchor fight a dodge is what makes bots
    // step out and get walked straight back in.
    HodirNearSnowpackedIcicleTrigger shelter(botAI);
    if (shelter.IsActive())
        return false;

    HodirIcicleDodgeTrigger dodge(botAI);
    if (dodge.IsActive())
        return false;

    // The arm threshold, not HodirBitingColdTrigger: that fires on any stack, and at one stack the shuttle
    // does nothing, so standing down for it keeps ranged out of Starlight.
    if (IsHodirBitingColdShedArmed(bot))
        return false;

    // With a spot in hand (tank hold or buff stand) the only question is walk there or stay. Both finders
    // re-check their latch against the band every tick, so the rules below are only for bots with none.
    Position anchor;
    float tolerance = 0.0f;
    if (GetHodirAnchor(botAI, bot, anchor, tolerance))
    {
        // The dodge stands down once the bot's clear, but the icicle stays lethal till it blows. Test the
        // whole walk back, not just its end, or the anchor and the dodge trade the bot back and forth.
        if (IsHodirWalkThroughLiveIcicle(bot, anchor, ULDUAR_HODIR_ICE_SHARDS_CLEAR,
                                         ULDUAR_HODIR_BIG_SHARDS_CLEAR))
            return false;

        return bot->GetExactDist2d(&anchor) > tolerance;
    }

    // Ranged and healers only: a tank stepping back drags him off the fire, and melee doing their job sit
    // inside the gap rule every tick. Same predicate the anchor uses, so the two can't drift apart.
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // Reactive, not restoring: fire only on a broken constraint. A home spot makes the anchor a spring,
    // since every dodge displaces past any tolerance and buys a return trip.

    // In among the melee, where an icicle or a Freeze on either catches both.
    Unit* hodir = GetHodir(botAI);
    if (hodir && bot->GetExactDist2d(hodir) < ULDUAR_HODIR_RANGED_MIN_BOSS_GAP)
        return true;

    // Or out of his range entirely: nothing walks a bot home, so one that dodged to the far wall would
    // stand there shooting nothing.
    if (hodir && bot->GetExactDist2d(hodir) > ULDUAR_HODIR_CASTER_MAX_BOSS_GAP)
        return true;

    // No Starlight or fire rule here: both reach a few yards, so a bot is outside every zone nearly always
    // and the rule would just walk. GetHodirAnchor offers one when it's worth the walk.

    // Clumped. One icicle splashes 4 yd, so neighbours inside the declump radius mean one landing
    // catches both.
    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest friendly players"))
    {
        Unit* ally = botAI->GetUnit(guid);
        if (!ally || ally == bot || !ally->IsAlive() || !ally->IsPlayer())
            continue;

        if (!PlayerbotAI::IsRanged(ally->ToPlayer()) && !PlayerbotAI::IsHeal(ally->ToPlayer()))
            continue;

        if (bot->GetExactDist2d(ally) < ULDUAR_HODIR_DECLUMP_RADIUS)
            return true;
    }

    return false;
}

bool HodirSetDpsPriorityTrigger::IsActive()
{
    // The action calls Attack() directly, so without the combat gate the first bot to lay eyes on him
    // pulls. Whoever pulls flips this for everyone, which is what makes the raid engage together.
    if (!IsHodirEngaged(botAI))
        return false;

    // Tanks are on this node too, but the action hands them the boss and nothing else.
    // Healers keep the generic picker so nothing pulls them off healing.
    return !PlayerbotAI::IsHeal(bot);
}

bool HodirFrozenBlowsSwapTrigger::IsActive()
{
    Unit* boss = GetHodir(botAI);
    if (!boss)
        return false;

    bool const frozenBlows = HodirFrozenBlowsActive(botAI, bot);
    bool const holding = boss->GetVictim() == bot;

    // Off-tank holds him for the 20s window, main tank takes him back after. Under the floor the taunt is a
    // death, not a swap: two 63511 hits land 2.4s apart and either is most of a tank's pool.
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return frozenBlows && !holding && bot->GetHealthPct() >= ULDUAR_HODIR_TAUNT_HEALTH_FLOOR;

    if (PlayerbotAI::IsMainTank(bot))
        return !frozenBlows && !holding;

    return false;
}

bool HodirRedirectThreatTrigger::IsActive()
{
    if (!IsHodirEngaged(botAI))
        return false;

    return bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE;
}

bool HodirSpreadStormCloudTrigger::IsActive()
{
    if (!GetHodir(botAI))
        return false;

    // A tank leaving its spot mid-Frozen-Blows costs more than the buff is worth.
    if (PlayerbotAI::IsTank(bot))
        return false;

    return bot->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_HODIR_STORM_CLOUD, bot));
}

bool HodirCollectStormPowerTrigger::IsActive()
{
    if (!GetHodir(botAI))
        return false;

    // Tanks never leave the boss for a damage buff, and a charge spent on a healer is wasted.
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    // Already holding it: the pulse would be spent on a bot that can't use it, and the walk is
    // downtime for nothing.
    if (bot->HasAura(sSpellMgr->GetSpellIdForDifficulty(SPELL_HODIR_STORM_POWER, bot)))
        return false;

    Player* carrier = GetHodirStormCloudCarrier(botAI, bot);
    if (!carrier || carrier == bot)
        return false;

    Position rally;
    if (!GetHodirStormCloudRally(botAI, bot, carrier, rally))
        return false;

    float const gap = bot->GetExactDist2d(&rally);
    if (gap > ULDUAR_HODIR_STORM_CLOUD_COLLECT_LEASH)
        return false;

    // Wait out an icicle between the bot and the rally rather than walk back into the pool the dodge
    // just stepped it out of.
    if (IsHodirWalkThroughLiveIcicle(bot, rally, ULDUAR_HODIR_ICE_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN,
                                     ULDUAR_HODIR_BIG_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN))
        return false;

    // Release, not the park distance: one number at both ends stands this down the tick the bot arrives,
    // and the position anchor walks it straight back out.
    return gap > ULDUAR_HODIR_STORM_CLOUD_COLLECT_RELEASE;
}
