/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Freya.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Freya.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>

using namespace EncounterHelpers;

bool FreyaNearNatureBombTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Assist tank 0 eats the bomb: stepping out would lift the Conservator off the spore the melee are
    // sheltering on. The main tank has its own node, which moves Freya rather than just the bot.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Wider than the blast so the node still runs after the bot steps clear and can hold its spot.
    // isUseful narrows it again, so a bot with no blast and no escape held costs nothing.
    return bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS) != nullptr;
}

bool FreyaTankNatureBombTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    return bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, ULDUAR_FREYA_NATURE_BOMB_AVOID_RADIUS) != nullptr;
}

bool FreyaSetDpsPriorityTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    return PlayerbotAI::IsDps(bot);
}

bool FreyaTankAddsTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // The ladder ends at Freya, so these two always have something, no need to gather the wave here.
    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool FreyaRedirectThreatTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* boss = GetFreyaScan(botAI).Boss();

    return boss && boss->IsAlive();
}

bool FreyaMoveToHealingSporeTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // The Grip pacifies melee as much as ranged. Tanks are out: the add tank reaches the aura by parking
    // the Conservator, and the main tank would drag Freya off her anchor.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // The aura is what counts, and it's exact. Checked first: it's cheaper than the sweep, and most of
    // the raid is already sheltered by mid-wave.
    if (bot->HasAura(SPELL_POTENT_PHEROMONES))
        return false;

    // By entry, not "find target": that value walks only this bot's threat list, and a pacified bot that
    // has not hit the Conservator yet is exactly the bot that needs a spore.
    if (!GetFreyaScanUnitByEntry(botAI, NPC_ANCIENT_CONSERVATOR))
        return false;

    Unit* spore = GetFreyaTargetSpore(botAI);
    if (!spore)
        return false;

    // Distance stands the node down too: the aura lands a moment late, and every move ordered onto the
    // spore meanwhile clears the motion master under whatever else was walking.
    return bot->GetExactDist2d(spore) > ULDUAR_FREYA_SPORE_RADIUS - 1.0f;
}

bool FreyaBreakIronRootsTrigger::IsActive()
{
    // Trapped by either the Ironbranch or the Freya-cast Iron Roots (each leaves its own DoT), and both
    // ids of each: HasAura takes the exact spell, and a 25-man raid only ever applies the 25-man half.
    return bot->HasAura(SPELL_IRON_ROOTS_DAMAGE_10) || bot->HasAura(SPELL_IRON_ROOTS_DAMAGE_25) ||
           bot->HasAura(SPELL_IRON_ROOTS_FREYA_DAMAGE_10) || bot->HasAura(SPELL_IRON_ROOTS_FREYA_DAMAGE_25);
}

bool FreyaDodgeUnstableSunBeamTrigger::IsActive()
{
    // Beam stalkers are non-selectable, so walk the stalker scan: nearby npcs, entry tested before the LOS
    // ray. This runs every tick for every bot in Ulduar until Freya dies.
    for (ObjectGuid const& guid : GetFreyaScan(botAI).Stalkers())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() != NPC_FREYA_SUN_BEAM && unit->GetEntry() != NPC_FREYA_UNSTABLE_SUN_BEAM)
            continue;

        if (bot->GetExactDist2d(unit) < ULDUAR_FREYA_UNSTABLE_SUN_BEAM_RADIUS)
            return true;
    }

    return false;
}

bool FreyaLasherAboutToBlowTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Tanks stay: one that walked would take Freya or the Conservator with it, and it survives the
    // blasts anyway. Ranged and healers leave through the camp node, under the nova and the trap.
    if (PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsMelee(bot))
        return false;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    return !GetFreyaLowLasherPositions(botAI, state, ULDUAR_FREYA_LASHER_BAIL_PCT, ULDUAR_FREYA_DETONATE_RADIUS)
                .empty();
}

bool FreyaRangedCampTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Melee and tanks are already stacked on whatever they are hitting, and a tank that left the camp
    // would take the boss or the Conservator with it.
    if (PlayerbotAI::IsTank(bot) || !(PlayerbotAI::IsRangedDps(bot) || PlayerbotAI::IsHeal(bot)))
        return false;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);
    if (state.detonatingLashers.empty())
        return false;

    // A low lasher in reach ends the gather: inside its blast the bot leaves, outside it stays put, since the
    // walk to the ball would cross a blast. Only low ones count, the healthy pack is faster than the bot.
    std::vector<Position> const blasts =
        GetFreyaLowLasherPositions(botAI, state, ULDUAR_FREYA_LASHER_BAIL_PCT, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    if (!blasts.empty())
    {
        for (Position const& blast : blasts)
            if (bot->GetExactDist2d(blast.GetPositionX(), blast.GetPositionY()) <= ULDUAR_FREYA_DETONATE_RADIUS)
                return true;

        return false;
    }

    Position const camp = GetFreyaLasherCampSpot(botAI);
    if (camp == Position())
        return false;

    // Healers get the wider band: they also have to stay in range of the melee group and the tanks,
    // and pulling them all the way into the ball would leave the far half of the raid unhealed.
    float const tolerance =
        PlayerbotAI::IsHeal(bot) ? ULDUAR_FREYA_HEALER_CAMP_TOLERANCE : ULDUAR_FREYA_RANGED_CAMP_TOLERANCE;

    return bot->GetExactDist2d(camp.GetPositionX(), camp.GetPositionY()) > tolerance;
}

bool FreyaFrostNovaLashersTrigger::IsActive()
{
    // Class first: it rules out most of the raid for free, where the boss lookup and CanCastSpell both
    // cost real work. Nothing here has a side effect, so the order is free to be the cheap one.
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    // The nova is a sphere on the caster, so count lashers on the mage. Two, so one lasher the mage is
    // already killing doesn't spend the cooldown.
    if (CountFreyaLashersNear(bot->GetPosition(), state, ULDUAR_FREYA_FROST_NOVA_RADIUS) <
        ULDUAR_FREYA_FROST_NOVA_MIN_LASHERS)
        return false;

    // Last, because it builds a Spell on the heap and runs the whole cast check, and Frost Nova is off
    // cooldown for most of a fight that only wants it during a lasher wave.
    return botAI->CanCastSpell("frost nova", bot);
}

bool FreyaTrapLashersTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    // Wider than the trap's own 10 yd field, deliberately: the patch needs to be down and armed before
    // the lasher arrives, and one that has picked this bot is coming here at 8 yd/s whatever it does.
    if (!CountFreyaLashersNear(bot->GetPosition(), state, ULDUAR_FREYA_FROST_TRAP_ARM_RANGE))
        return false;

    return botAI->CanCastSpell("frost trap", bot);
}

bool FreyaSummonArmyTrigger::IsActive()
{
    if (bot->getClass() != CLASS_DEATH_KNIGHT)
        return false;

    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    if (state.detonatingLashers.empty())
        return false;

    return botAI->CanCastSpell("army of the dead", bot);
}

bool FreyaGroundTremorHoldCastTrigger::IsActive()
{
    // By entry rather than off the target list: this runs for every bot wherever the gate is open, and
    // on trash and in other fights nothing else needs that list built.
    if (!IsFreyaGroundTremorCasting(GetFreyaBossByEntry(botAI)))
        return false;

    return bot->HasUnitState(UNIT_STATE_CASTING);
}

bool FreyaNaturesFuryBailTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Tanks stay. Whatever they are holding walks after them, so a tank carrying the mark out of the
    // raid takes Freya or the Conservator into it instead.
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!bot->HasAura(SPELL_NATURES_FURY_10) && !bot->HasAura(SPELL_NATURES_FURY_25))
        return false;

    return CountFreyaRaidNear(botAI, bot->GetPosition(), ULDUAR_FREYA_NATURES_FURY_RADIUS, bot) > 0;
}

bool FreyaStepOutOfSunbeamTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Melee and tanks can't give up a second and a half of damage, and lose little by staying: beams
    // mostly go for ranged bots and their pets, away from the melee ball.
    if (PlayerbotAI::IsTank(bot) || !(PlayerbotAI::IsRangedDps(bot) || PlayerbotAI::IsHeal(bot)))
        return false;

    Unit* target = GetFreyaSunbeamTarget(boss);
    if (!target || target == bot)
        return false;

    return bot->GetExactDist2d(target) <= ULDUAR_FREYA_SUNBEAM_AVOID_RADIUS;
}

bool FreyaTankHoldFreyaTrigger::IsActive()
{
    Unit* boss = GetFreyaScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Only whoever actually has her: walking anyone else moves no boss, and a tank that has lost her
    // would drag the melee ring off to where she is not.
    if (!PlayerbotAI::IsMainTank(bot) || boss->GetVictim() != bot)
        return false;

    return boss->GetExactDist2d(&ULDUAR_FREYA_TANK_ANCHOR) > ULDUAR_FREYA_TANK_LEASH;
}
