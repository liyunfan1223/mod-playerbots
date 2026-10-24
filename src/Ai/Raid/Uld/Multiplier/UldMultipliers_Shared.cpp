/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_Shared.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RaidEncounter.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldEncounter_Algalon.h"
#include "UldEncounter_Freya.h"
#include "UldEncounter_Mimiron.h"
#include "UldEncounter_Razorscale.h"
#include "UldEncounter_Thorim.h"
#include "UldEncounter_YoggSaron.h"
#include "UldEncounterGate.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <set>
#include <string>

using namespace EncounterHelpers;

float UldThreatRedirectMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action) &&
        !dynamic_cast<CastTricksOfTheTradeOnMainTankAction*>(action))
        return 1.0f;

    static uint32 const noRedirectBosses[] = {
        // "freya redirect threat" aims at the add tank while it holds the Snaplasher or the Conservator
        NPC_FREYA,
        // Each phase is a new creature with a fresh threat table, and P3 splits VX-001 and the ACU over
        // two tanks. "mimiron redirect threat action" covers P1, the one phase a redirect helps
        NPC_LEVIATHAN_MKII, NPC_VX001, NPC_AERIAL_COMMAND_UNIT,
        // Arena and gauntlet squads each bring their own tank, then Unbalancing Strike swaps
        NPC_THORIM,
        // Phase Punch swaps main and assist tank on a stack timer
        PB_NPC_ALGALON};

    for (uint32 entry : noRedirectBosses)
    {
        if (GetUldUnitByEntry(botAI, entry))
            return 0.0f;
    }

    // The MT holds nothing until the permanent ground phase: the Dark Rune adds go to the assist tanks,
    // and a harpoon knockdown above 50% puts her on the floor with nobody tanking her
    if (Unit* razorscale = GetUldUnitByEntry(botAI, NPC_RAZORSCALE))
        return RazorscaleBossHelper::IsFlyingPhaseFor(razorscale) ? 0.0f : 1.0f;

    return 1.0f;
}

// Several Ulduar bosses put their real DPS check minutes after the pull, where the generic
// hold-until-the-tank-engages window spends everything on a phase that does not matter.
float UlduarBurstWindowMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string const name = action->getName();
    if (!IsBurstCooldownAction(name) || IsManaReturnCooldown(bot, name))
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    uint32 now = getMSTime();
    if (now != cachedAtMs || !cachedAtMs)
    {
        cachedAtMs = now;
        cachedValue = EvaluateWindow();
    }

    bool const allowed = (name == "bloodlust" || name == "heroism") ? cachedValue.allowLust : cachedValue.allowAll;

    return allowed ? 1.0f : 0.0f;
}

UlduarBurstWindowMultiplier::BurstWindow UlduarBurstWindowMultiplier::EvaluateWindow()
{
    Unit* razorscale = nullptr;
    Unit* leviathanMkII = nullptr;
    Unit* vx001 = nullptr;
    Unit* aerialCommandUnit = nullptr;
    Unit* steelbreaker = nullptr;
    Unit* molgeim = nullptr;
    Unit* brundir = nullptr;
    Unit* freya = nullptr;
    Unit* thorim = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_RAZORSCALE: razorscale = unit; break;
            case NPC_LEVIATHAN_MKII: leviathanMkII = unit; break;
            case NPC_VX001: vx001 = unit; break;
            case NPC_AERIAL_COMMAND_UNIT: aerialCommandUnit = unit; break;
            case NPC_STEELBREAKER: steelbreaker = unit; break;
            case NPC_MOLGEIM: molgeim = unit; break;
            case NPC_BRUNDIR: brundir = unit; break;
            case NPC_FREYA: freya = unit; break;
            case NPC_THORIM: thorim = unit; break;
            default: break;
        }
    }

    // The sweep lags the pull and skips unselectable units, which every mech is through a handover.
    // Missing all three falls through to the open default below and lusts, hence the grid search
    if (!leviathanMkII)
        leviathanMkII = bot->FindNearestCreature(NPC_LEVIATHAN_MKII, sPlayerbotAIConfig.SightDistance, true);
    if (!vx001)
        vx001 = bot->FindNearestCreature(NPC_VX001, sPlayerbotAIConfig.SightDistance, true);
    if (!aerialCommandUnit)
        aerialCommandUnit =
            bot->FindNearestCreature(NPC_AERIAL_COMMAND_UNIT, sPlayerbotAIConfig.SightDistance, true);

    // The sweep stops at SightDistance, and her second flight point is past that for most of the raid.
    // DoZoneInCombat puts the whole raid on her threat list, and "find target" never calls UpdateBossAI
    if (!razorscale)
        razorscale = AI_VALUE2(Unit*, "find target", "razorscale");

    // No damage taken while airborne, so every landing is a burn window, knockdowns included. Lust
    // waits for the permanent ground phase, a knockdown is too short for a 10 min cooldown
    if (razorscale)
    {
        bool const grounded = razorscale->GetPositionZ() <= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD;

        return {grounded, RazorscaleBossHelper::IsGroundPhaseFor(razorscale)};
    }

    // Only lust waits, for P4 plus a delay: the raid spends the P4 opening running from the Frost Bomb.
    // A beaten or self-repairing mech stays alive but unselectable. P4 is all three selectable, VX riding
    auto const fighting = [](Unit const* mech) { return mech && !mech->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE); };
    if (leviathanMkII || vx001 || aerialCommandUnit)
    {
        bool const phase4 = fighting(leviathanMkII) && fighting(vx001) && fighting(aerialCommandUnit) &&
                            vx001->GetVehicleBase();
        if (!phase4)
            mimironPhase4LiveMs = 0;
        else if (!mimironPhase4LiveMs)
            mimironPhase4LiveMs = getMSTime();

        return {true, phase4 && GetMSTimeDiffToNow(mimironPhase4LiveMs) >= ULDUAR_MIMIRON_PHASE4_LUST_DELAY_MS};
    }

    // The council members resurrect each other until one is left, so only the survivor is a real
    // kill. Covers the hard mode too, where that survivor is the empowered phase-3 Steelbreaker.
    if (steelbreaker || molgeim || brundir)
    {
        uint32 const aliveCount = (steelbreaker ? 1u : 0u) + (molgeim ? 1u : 0u) + (brundir ? 1u : 0u);

        return {true, aliveCount == 1};
    }

    // Attuned to Nature makes damage on Freya pointless until the final phase removes it. Personal
    // cooldowns wait too, the wave adds aren't boss-flagged so burst on them is zeroed anyway
    if (freya)
    {
        bool const finalPhase =
            !freya->HasAura(SPELL_ATTUNED_TO_NATURE) || freya->GetHealthPct() <= FREYA_LUST_FALLBACK_PCT;

        return {finalPhase, finalPhase};
    }

    if (thorim)
    {
        // He is immune on his balcony for the whole gauntlet.
        bool const onArenaFloor = thorim->GetPositionZ() <= ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD;

        return {onArenaFloor, onArenaFloor};
    }

    // P1 damage lands on Sara and is wasted, P3 is the body burn. Keyed on the boss state: Sara is
    // findable from instance creation, and the brain room is past SightDistance from Yogg
    if (bot->GetMapId() == ULDUAR_MAP_ID && BossStateGateLive(botAI, ULD_BOSS_YOGGSARON))
    {
        uint32 const phase = YoggSaronPhase(botAI);

        // Lust belongs to the phase 3 node, which waits for the raid to reach its spots
        return {phase == 2 || phase == 3, false};
    }

    return {};
}
