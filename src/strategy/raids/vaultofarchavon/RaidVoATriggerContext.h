// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
//  and/or modify it under version 3 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDVOATRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDVOATRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidVoATriggers.h"

class RaidVoATriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidVoATriggerContext()
    {
        creators["emalon mark boss trigger"] = &RaidVoATriggerContext::emalon_mark_boss_trigger;
        creators["emalon lighting nova trigger"] = &RaidVoATriggerContext::emalon_lighting_nova_trigger;
        creators["emalon overcharge trigger"] = &RaidVoATriggerContext::emalon_overcharge_trigger;
        creators["emalon fall from floor trigger"] = &RaidVoATriggerContext::emalon_fall_from_floor_trigger;
        creators["emalon nature resistance trigger"] = &RaidVoATriggerContext::emalon_nature_resistance_trigger;
        creators["koralon fire resistance trigger"] = &RaidVoATriggerContext::koralon_fire_resistance_trigger;
    }

private:
    static Trigger* emalon_mark_boss_trigger(PlayerbotAI* ai) { return new EmalonMarkBossTrigger(ai); }
    static Trigger* emalon_lighting_nova_trigger(PlayerbotAI* ai) { return new EmalonLightingNovaTrigger(ai); }
    static Trigger* emalon_overcharge_trigger(PlayerbotAI* ai) { return new EmalonOverchargeTrigger(ai); }
    static Trigger* emalon_fall_from_floor_trigger(PlayerbotAI* ai) { return new EmalonFallFromFloorTrigger(ai); }
    static Trigger* emalon_nature_resistance_trigger(PlayerbotAI* ai) { return new BossNatureResistanceTrigger(ai, "emalon the storm watcher"); }
    static Trigger* koralon_fire_resistance_trigger(PlayerbotAI* ai) { return new BossFireResistanceTrigger(ai, "koralon the flame watcher"); }
};

#endif
