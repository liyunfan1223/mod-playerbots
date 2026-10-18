/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldHardMode.h"

#include "AiObjectContext.h"
#include "InstanceScript.h"
#include "Map.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"
#include "UldEncounter_FlameLeviathan.h"
#include "UldEncounter_IronAssembly.h"
#include "UldEncounter_XT002.h"
#include "UldScripts.h"
#include "Unit.h"

using namespace EncounterHelpers;

bool IsVezaxHardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarVezaxHardMode; }

bool IsIronAssemblyHardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarIronAssemblyHardMode; }

bool IsSteelbreakerEmpowered(PlayerbotAI* /*botAI*/, Unit* steelbreaker, Unit* molgeim, Unit* brundir)
{
    return steelbreaker != nullptr && molgeim == nullptr && brundir == nullptr;
}

bool IsSteelbreakerEmpowered(PlayerbotAI* botAI)
{
    // No hard-mode gate: phase 3 is a fact about the fight, not a raid setting
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    return IsSteelbreakerEmpowered(botAI, targets.steelbreaker, targets.molgeim, targets.brundir);
}

Unit* GetIronAssemblyNextKillTarget(PlayerbotAI* botAI)
{
    IronAssemblyTargets targets;
    GatherIronAssemblyTargets(botAI, targets);

    Unit* steelbreaker = targets.steelbreaker;
    Unit* molgeim = targets.molgeim;
    Unit* brundir = targets.brundir;

    // Hard mode keeps Steelbreaker for phase 3, else he dies first and his phase 3 kit never shows up.
    // Molgeim is second either way, so his Rune of Summoning never comes
    if (IsIronAssemblyHardModeActive(botAI))
    {
        if (brundir)
            return brundir;
        if (molgeim)
            return molgeim;

        return steelbreaker;
    }

    if (steelbreaker)
        return steelbreaker;
    if (molgeim)
        return molgeim;

    return brundir;
}

uint32 FlameLeviathanActiveTowerMask(PlayerbotAI* /*botAI*/)
{
    return sPlayerbotAIConfig.UlduarFlameLeviathanHardMode ? FL_TOWER_ALL : 0;
}

bool IsThorimHardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarThorimHardMode; }

bool IsFreyaHardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarFreyaHardMode; }

bool IsMimironHardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarMimironHardMode; }

bool IsXT002HardModeActive(PlayerbotAI* /*botAI*/) { return sPlayerbotAIConfig.UlduarXT002HardMode; }

bool IsXT002HeartbreakActive(Player* bot, Unit* xt002)
{
    return bot != nullptr && xt002 != nullptr && xt002->HasAura(GetXT002HeartbreakSpellId(bot));
}

bool YoggThorimKeeperActive(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Map* map = bot->GetMap();
    if (!map || !map->IsDungeon())
        return false;

    InstanceScript* instance = ((InstanceMap*)map)->GetInstanceScript();
    if (!instance)
        return false;

    // Same source the boss script reads: the freed-Keeper bitmask the raid set from the pre-pull
    // gossips. Which Keepers are up is a raid choice made in-instance, so it cannot come from config.
    return (instance->GetPersistentData(PERSISTENT_DATA_WATCHERS_MASK) & (1u << KEEPER_THORIM)) != 0;
}

uint32 YoggKeepersFreedCount(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return 0;

    Map* map = bot->GetMap();
    if (!map || !map->IsDungeon())
        return 0;

    InstanceScript* instance = ((InstanceMap*)map)->GetInstanceScript();
    if (!instance)
        return 0;

    // Bits 0-3, as Sara's DATA_GET_KEEPERS_COUNT counts them.
    uint32 const mask = instance->GetPersistentData(PERSISTENT_DATA_WATCHERS_MASK);
    uint32 count = 0;
    for (uint32 keeper = KEEPER_FREYA; keeper <= KEEPER_THORIM; ++keeper)
        count += (mask >> keeper) & 1u;

    return count;
}
