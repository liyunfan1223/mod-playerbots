/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEACTIONCONTEXT_H
#define PLAYERBOTS_EOEACTIONCONTEXT_H

#include "Action.h"
#include "EoEActions.h"
#include "NamedObjectContext.h"

class RaidEoEActionContext : public NamedObjectContext<Action>
{
public:
    RaidEoEActionContext()
    {
        creators["malygos position"] = &RaidEoEActionContext::position;
        creators["malygos target"] = &RaidEoEActionContext::target;
        // creators["pull power spark"] = &RaidEoEActionContext::pull_power_spark;
        // creators["kill power spark"] = &RaidEoEActionContext::kill_power_spark;
        creators["eoe fly drake"] = &RaidEoEActionContext::eoe_fly_drake;
        creators["eoe drake attack"] = &RaidEoEActionContext::eoe_drake_attack;
    }

private:
    static Action* position(PlayerbotAI* botAI) { return new MalygosPositionAction(botAI); }
    static Action* target(PlayerbotAI* botAI) { return new MalygosTargetAction(botAI); }
    // static Action* pull_power_spark(PlayerbotAI* botAI) { return new PullPowerSparkAction(botAI); }
    // static Action* kill_power_spark(PlayerbotAI* botAI) { return new KillPowerSparkAction(botAI); }
    static Action* eoe_fly_drake(PlayerbotAI* botAI) { return new EoEFlyDrakeAction(botAI); }
    static Action* eoe_drake_attack(PlayerbotAI* botAI) { return new EoEDrakeAttackAction(botAI); }
};

#endif
