/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ANMultipliers.h"
#include "ANActions.h"
#include "ANTriggers.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

float KrikthirMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsDps(bot)) { return 1.0f; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    Unit* boss = nullptr;
    Unit* watcher = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (!unit) { continue; }

        switch (unit->GetEntry())
        {
            case NPC_KRIKTHIR:
                boss = unit;
                continue;
            case NPC_WATCHER_SILTHIK:
            case NPC_WATCHER_GASHRA:
            case NPC_WATCHER_NARJIL:
            case NPC_WATCHER_SKIRMISHER:
            case NPC_WATCHER_SHADOWCASTER:
            case NPC_WATCHER_WARRIOR:
                watcher = unit;
                continue;
        }
    }

    if (boss && watcher)
    {
        // Do not target swap
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }

        if (action->getThreatType() == Action::ActionThreatType::Aoe)
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
