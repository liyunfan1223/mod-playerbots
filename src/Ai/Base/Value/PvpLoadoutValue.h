/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTVALUE_H
#define PLAYERBOTS_PVPLOADOUTVALUE_H

#include "PvpLoadoutRules.h"
#include "Value.h"
#include <optional>

class PlayerbotAI;

// The bot's stored PvE snapshot while it wears a PvP loadout; empty otherwise. Starts from playerbots_pvp_loadout and
// is kept in step with it by whoever changes it, so trigger checks never touch the repository's lock.
class PvpLoadoutValue : public ManualSetValue<std::optional<PvpLoadout::Snapshot>>
{
public:
    PvpLoadoutValue(PlayerbotAI* botAI, std::string const name = "pvp loadout");
};

#endif
