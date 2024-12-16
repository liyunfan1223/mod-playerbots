#ifndef _PLAYERBOT_RAIDEOEACTIONCONTEXT_H
#define _PLAYERBOT_RAIDEOEACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidEoEActions.h"

class RaidEoEActionContext : public NamedObjectContext<Action>
{
public:
    RaidEoEActionContext()
    {
        creators["malygos position"] = &RaidEoEActionContext::position;
        creators["malygos target"] = &RaidEoEActionContext::target;
        // creators["pull power spark"] = &RaidEoEActionContext::pull_power_spark;
        // creators["kill power spark"] = &RaidEoEActionContext::kill_power_spark;
        creators["fly drake"] = &RaidEoEActionContext::fly_drake;
        creators["drake attack"] = &RaidEoEActionContext::drake_attack;
    }

private:
    static Action* position(PlayerbotAI* ai) { return new MalygosPositionAction(ai); }
    static Action* target(PlayerbotAI* ai) { return new MalygosTargetAction(ai); }
    // static Action* pull_power_spark(PlayerbotAI* ai) { return new PullPowerSparkAction(ai); }
    // static Action* kill_power_spark(PlayerbotAI* ai) { return new KillPowerSparkAction(ai); }
    static Action* fly_drake(PlayerbotAI* ai) { return new EoEFlyDrakeAction(ai); }
    static Action* drake_attack(PlayerbotAI* ai) { return new EoEDrakeAttackAction(ai); }
};

#endif
