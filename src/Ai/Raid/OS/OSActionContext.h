/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OSACTIONCONTEXT_H
#define PLAYERBOTS_OSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "OSActions.h"

class RaidOsActionContext : public NamedObjectContext<Action>
{
public:
    RaidOsActionContext()
    {
        creators["sartharion tank position"] = &RaidOsActionContext::tank_position;
        creators["avoid twilight fissure"] = &RaidOsActionContext::avoid_twilight_fissure;
        creators["avoid flame tsunami"] = &RaidOsActionContext::avoid_flame_tsunami;
        creators["sartharion attack priority"] = &RaidOsActionContext::attack_priority;
        creators["enter twilight portal"] = &RaidOsActionContext::enter_twilight_portal;
        creators["exit twilight portal"] = &RaidOsActionContext::exit_twilight_portal;
    }

private:
    static Action* tank_position(PlayerbotAI* botAI) { return new SartharionTankPositionAction(botAI); }
    static Action* avoid_twilight_fissure(PlayerbotAI* botAI) { return new AvoidTwilightFissureAction(botAI); }
    static Action* avoid_flame_tsunami(PlayerbotAI* botAI) { return new AvoidFlameTsunamiAction(botAI); }
    static Action* attack_priority(PlayerbotAI* botAI) { return new SartharionAttackPriorityAction(botAI); }
    static Action* enter_twilight_portal(PlayerbotAI* botAI) { return new EnterTwilightPortalAction(botAI); }
    static Action* exit_twilight_portal(PlayerbotAI* botAI) { return new ExitTwilightPortalAction(botAI); }
};

#endif
