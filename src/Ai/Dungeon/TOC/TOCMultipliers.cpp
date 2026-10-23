/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TOCMultipliers.h"
#include "Action.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "TOCActions.h"
#include "TOCTriggers.h"
#include "Timer.h"
#include "WipeAction.h"

namespace
{
constexpr uint32 SPARE_MOUNT_CHECK_MS = 500;
}

float ToCJoustMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() != BOT_STATE_COMBAT)
        return 1.0f;

    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase)
    {
        if (dynamic_cast<ToCMountAction*>(action) || dynamic_cast<ToCLanceAction*>(action) ||
            dynamic_cast<ToCUELanceAction*>(action) || dynamic_cast<WipeAction*>(action))
            return 1.0f;

        return SpareMountNearby() ? 0.0f : 1.0f;
    }

    uint32 entry = vehicleBase->GetEntry();
    if (entry != NPC_ARGENT_BATTLEWORG && entry != NPC_ARGENT_WARHORSE)
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<ToCMountedAction*>(action))
        return 0.0f;

    return 1.0f;
}

bool ToCJoustMultiplier::SpareMountNearby()
{
    uint32 now = getMSTime();
    if (now >= mountCheckAt)
    {
        spareMount = ToCMountAction::FindMount(botAI) != nullptr;
        mountCheckAt = now + SPARE_MOUNT_CHECK_MS;
    }

    return spareMount;
}
