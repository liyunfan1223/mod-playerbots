/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_IronAssembly.h"

#include "GameObject.h"
#include "Group.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "UldData.h"
#include "UldEncounter_IronAssembly.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"

using namespace EncounterHelpers;

bool IronAssemblyResetEncounterStateTrigger::IsActive()
{
    return IronAssemblyBotHasEncounterState(bot) && IronAssemblyEncounterStateIsStale(botAI);
}

bool IronAssemblyLightningTendrilsTrigger::IsActive()
{
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir || !IronAssemblyTendrilsActive(brundir))
        return false;

    // Gate on the hazard itself rather than a wider round number, so the reaction distance and the
    // radius it is protecting against can never drift apart.
    return bot->GetDistance2d(brundir) < ULDUAR_IRON_ASSEMBLY_TENDRILS_CLEARANCE;
}

bool IronAssemblyOverloadTrigger::IsActive()
{
    // Tanks hold through it: 20,000 nature is survivable in plate, and leaving costs uptime and lets
    // Brundir drift toward the raid.
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir || !IronAssemblyOverloadActive(brundir))
        return false;

    return bot->GetDistance2d(brundir) < ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE;
}

bool IronAssemblyRuneOfDeathTrigger::IsActive()
{
    if (!IronAssemblyEncounterActive(botAI))
        return false;

    std::vector<Position> const runes = GetIronAssemblyScan(botAI).RunesOfDeath();
    if (runes.empty())
        return false;

    // DANGER, not CLEARANCE: this asks whether the bot is being ticked, and the escape action it
    // fires takes it the extra yards. Testing at the clearance would re-fire on every yard of drift.
    return !IsIronAssemblyPositionClearOfRunes(bot->GetPosition(), runes,
                                               ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_DANGER_RADIUS);
}

bool IronAssemblyInterruptTrigger::IsActive()
{
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir)
        return false;

    // Which cast is up and who owes it are one decision, made in the encounter helper so the trigger
    // and the action cannot end up describing different elections.
    return IronAssemblyInterruptDuty(botAI, bot, brundir) != nullptr;
}

bool IronAssemblyTankAssignmentTrigger::IsActive()
{
    if (!IronAssemblyFormationActive(botAI))
        return false;

    Unit* boss = IronAssemblyAssignedBoss(botAI, bot);
    if (!boss)
        return false;

    if (boss->GetVictim() != bot)
        return true;

    // The boss above, not a second assignment: deriving it ranks the bot tanks and walks the group
    // again, and both answers come from the same tick anyway.
    Position spot;
    if (!TryGetIronAssemblyBossTankSpot(bot, boss, spot))
        return false;

    return bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) >
           ULDUAR_IRON_ASSEMBLY_TANK_SPOT_TOLERANCE;
}

bool IronAssemblyShieldOfRunesTrigger::IsActive()
{
    Unit* molgeim = GetIronAssemblyMember(botAI, NPC_MOLGEIM);
    if (!molgeim || !IronAssemblyShieldOfRunesUp(molgeim))
        return false;

    static std::vector<std::string> const dispels = {"spellsteal", "purge", "dispel magic"};
    for (std::string const& dispel : dispels)
        if (botAI->CanCastSpell(dispel, molgeim))
            return true;

    return false;
}

bool IronAssemblyFusionPunchDispelTrigger::IsActive()
{
    if (!GetIronAssemblyMember(botAI, NPC_STEELBREAKER))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return IronAssemblyHasFusionPunch(bot);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != ULDUAR_MAP_ID)
            continue;

        if (IronAssemblyHasFusionPunch(member))
            return true;
    }

    return false;
}

bool IronAssemblyRedirectThreatTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    return IronAssemblyFocusTarget(botAI) != nullptr;
}

bool IronAssemblyRuneOfPowerSoakTrigger::IsActive()
{
    Position rune;
    if (!TryGetIronAssemblyRuneOfPowerSoakSpot(botAI, bot, rune))
        return false;

    return bot->GetExactDist2d(rune.GetPositionX(), rune.GetPositionY()) >
           ULDUAR_IRON_ASSEMBLY_RUNE_OF_POWER_RADIUS;
}

bool IronAssemblySetDpsPriorityTrigger::IsActive()
{
    // Tanks have their own assignment node; this one owns every other target in the fight.
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!IronAssemblyFormationActive(botAI))
        return false;

    return IronAssemblyFocusTarget(botAI) != nullptr;
}

bool IronAssemblyRaidPositionTrigger::IsActive()
{
    if (!IronAssemblyFormationActive(botAI))
        return false;

    Position spot;
    return TryGetIronAssemblyRaidSpot(botAI, bot, spot);
}
