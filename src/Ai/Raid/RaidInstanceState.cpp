/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidInstanceState.h"

#include "Map.h"
#include "ScriptMgr.h"

class RaidInstanceStateMapScript : public AllMapScript
{
public:
    RaidInstanceStateMapScript() : AllMapScript("RaidInstanceStateMapScript", {ALLMAPHOOK_ON_DESTROY_MAP}) {}

    // An instance is only destroyed once it has no players, so none of its bots can still be holding
    // a reference into a store.
    void OnDestroyMap(Map* map) override
    {
        if (uint32 const instanceId = map->GetInstanceId())
            RaidInstanceStateDrop(instanceId);
    }
};

void AddSC_RaidInstanceStateScripts() { new RaidInstanceStateMapScript(); }
