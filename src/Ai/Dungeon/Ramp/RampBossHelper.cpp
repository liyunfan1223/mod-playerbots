/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampBossHelper.h"
#include "Playerbots.h"
#include "RampShared.h"

using namespace RampShared;

bool OmorBossHelper::HasTreacheryAura(Player const* bot) {
    return bot->HasAura(static_cast<uint32>(RampSpells::SPELL_BANE_OF_TREACHERY)) ||
           bot->HasAura(static_cast<uint32>(RampSpells::SPELL_TREACHEROUS_AURA));
}
