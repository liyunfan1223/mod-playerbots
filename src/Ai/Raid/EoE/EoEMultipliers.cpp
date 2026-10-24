/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEMultipliers.h"
#include "Action.h"
#include "EoEData.h"
#include "EoEEncounter_Malygos.h"

float MalygosScionTankAssistMultiplier::GetValue(Action* action)
{
    if (GetMalygosPhase(bot) != MalygosPhase::P2)
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->GetEntry() == NPC_SCION_OF_ETERNITY ? 0.0f : 1.0f;
}
