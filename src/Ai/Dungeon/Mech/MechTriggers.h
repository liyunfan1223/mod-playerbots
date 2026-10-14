/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHTRIGGERS_H
#define PLAYERBOTS_MECHTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "Trigger.h"

enum class MechanarIDs : uint32
{
    NPC_NETHERMANCER_SEPETHREA = 19221,
    NPC_RAGING_FLAMES          = 20481,
    NPC_RAGING_FLAMES_HEROIC   = 21538,
};

class SepethreaKiteFlameTrigger : public Trigger
{
public:
    SepethreaKiteFlameTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea kite flame") {}
    bool IsActive() override;
};

class SepethreaAvoidFlameTrigger : public Trigger
{
public:
    SepethreaAvoidFlameTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea avoid flame") {}
    bool IsActive() override;
};

class SepethreaTrailTrigger : public Trigger
{
public:
    SepethreaTrailTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea trail") {}
    bool IsActive() override;
};

class SepethreaFocusBossTrigger : public Trigger
{
public:
    SepethreaFocusBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sepethrea focus boss") {}
    bool IsActive() override;
};

#endif
