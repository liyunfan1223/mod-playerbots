/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VOAACTIONCONTEXT_H
#define PLAYERBOTS_VOAACTIONCONTEXT_H

#include "Action.h"
#include "BossAuraActions.h"
#include "NamedObjectContext.h"
#include "PlayerbotAI.h"
#include "VoAActions.h"

class RaidVoAActionContext : public NamedObjectContext<Action>
{
public:
    RaidVoAActionContext()
    {
        creators["emalon mark boss action"] = &RaidVoAActionContext::emalon_mark_boss_action;
        creators["emalon lighting nova action"] = &RaidVoAActionContext::emalon_lighting_nova_action;
        creators["emalon overcharge action"] = &RaidVoAActionContext::emalon_overcharge_action;
        creators["emalon fall from floor action"] = &RaidVoAActionContext::emalon_fall_from_floor_action;
        creators["emalon nature resistance action"] = &RaidVoAActionContext::emalon_nature_resistance_action;
        creators["koralon fire resistance action"] = &RaidVoAActionContext::koralon_fire_resistance_action;
    }

private:
    static Action* emalon_mark_boss_action(PlayerbotAI* botAI) { return new EmalonMarkBossAction(botAI); }
    static Action* emalon_lighting_nova_action(PlayerbotAI* botAI) { return new EmalonLightingNovaAction(botAI); }
    static Action* emalon_overcharge_action(PlayerbotAI* botAI) { return new EmalonOverchargeAction(botAI); }
    static Action* emalon_fall_from_floor_action(PlayerbotAI* botAI) { return new EmalonFallFromFloorAction(botAI); }
    static Action* emalon_nature_resistance_action(PlayerbotAI* botAI) { return new BossNatureResistanceAction(botAI, "emalon the storm watcher"); }
    static Action* koralon_fire_resistance_action(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "koralon the flame watcher"); }
};

#endif
