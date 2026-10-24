/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_Hodir.h"

#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Hodir.h"

float HodirBreakerAoeHoldMultiplier::GetValue(Action* action)
{
    if (!action || action->getThreatType() != Action::ActionThreatType::Aoe ||
        dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* const target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    uint32 const entry = target->GetEntry();
    return entry == NPC_HODIR_FLASH_FREEZE_BLOCK || entry == NPC_HODIR_FLASH_FREEZE_PLAYER ? 0.0f : 1.0f;
}
