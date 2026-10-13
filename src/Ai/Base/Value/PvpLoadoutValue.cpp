/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutValue.h"
#include "PlayerbotPvpLoadoutRepository.h"
#include "Playerbots.h"

PvpLoadoutValue::PvpLoadoutValue(PlayerbotAI* botAI, std::string const name)
    : ManualSetValue<std::optional<PvpLoadout::Snapshot>>(botAI, std::nullopt, name)
{
    value = PlayerbotPvpLoadoutRepository::instance().Find(botAI->GetBot()->GetGUID().GetCounter());
}
