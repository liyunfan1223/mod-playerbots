// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
//  and/or modify it under version 2 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDVOAACTIONCONTEXT_H
#define _PLAYERBOT_RAIDVOAACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidVoAActions.h"
#include "PlayerbotAI.h"

class RaidVoAActionContext : public NamedObjectContext<Action>
{
public:
    RaidVoAActionContext()
    {
        creators["emalon mark boss action"] = &RaidVoAActionContext::emalon_mark_boss_action;
        creators["emalon lighting nova action"] = &RaidVoAActionContext::emalon_lighting_nova_action;
        creators["emalon overcharge action"] = &RaidVoAActionContext::emalon_overcharge_action;
        creators["emalon fall from floor action"] = &RaidVoAActionContext::emalon_fall_from_floor_action;
    }

private:
    static Action* emalon_mark_boss_action(PlayerbotAI* ai) { return new EmalonMarkBossAction(ai); }
    static Action* emalon_lighting_nova_action(PlayerbotAI* ai) { return new EmalonLightingNovaAction(ai); }
    static Action* emalon_overcharge_action(PlayerbotAI* ai) { return new EmalonOverchargeAction(ai); }
    static Action* emalon_fall_from_floor_action(PlayerbotAI* ai) { return new EmalonFallFromFloorAction(ai); }
};

#endif
