/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTLOG_H
#define PLAYERBOTS_PVPLOADOUTLOG_H

#include "Log.h"
#include "PlayerbotAIConfig.h"
#include <utility>

namespace PvpLoadout
{
// Whether the PvP loadout writes its debug lines (Playerbots.PvpLoadoutDebug).
inline bool DebugLogging() { return sPlayerbotAIConfig.PvpLoadoutDebug; }

// A PvP loadout debug line (logger "playerbots", debug level), written only with Playerbots.PvpLoadoutDebug. Its
// arguments are evaluated either way, so callers building costly ones check DebugLogging first.
template <typename... Args>
void LogDebug(Acore::FormatStringView fmt, Args&&... args)
{
    if (DebugLogging())
        LOG_DEBUG("playerbots", fmt, std::forward<Args>(args)...);
}
}  // namespace PvpLoadout

#endif
