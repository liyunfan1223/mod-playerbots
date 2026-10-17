/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OSTRIGGERCONTEXT_H
#define PLAYERBOTS_OSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "OSTriggers.h"

class RaidOsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidOsTriggerContext()
    {
        creators["sartharion tank"] = &RaidOsTriggerContext::sartharion_tank;
        creators["flame tsunami"] = &RaidOsTriggerContext::flame_tsunami;
        creators["twilight fissure"] = &RaidOsTriggerContext::twilight_fissure;
        creators["sartharion dps"] = &RaidOsTriggerContext::sartharion_dps;
        creators["sartharion melee positioning"] = &RaidOsTriggerContext::sartharion_melee;
        creators["twilight portal enter"] = &RaidOsTriggerContext::twilight_portal_enter;
        creators["twilight portal exit"] = &RaidOsTriggerContext::twilight_portal_exit;
    }

private:
    static Trigger* sartharion_tank(PlayerbotAI* botAI) { return new SartharionTankTrigger(botAI); }
    static Trigger* flame_tsunami(PlayerbotAI* botAI) { return new FlameTsunamiTrigger(botAI); }
    static Trigger* twilight_fissure(PlayerbotAI* botAI) { return new TwilightFissureTrigger(botAI); }
    static Trigger* sartharion_dps(PlayerbotAI* botAI) { return new SartharionDpsTrigger(botAI); }
    static Trigger* sartharion_melee(PlayerbotAI* botAI) { return new SartharionMeleePositioningTrigger(botAI); }
    static Trigger* twilight_portal_enter(PlayerbotAI* botAI) { return new TwilightPortalEnterTrigger(botAI); }
    static Trigger* twilight_portal_exit(PlayerbotAI* botAI) { return new TwilightPortalExitTrigger(botAI); }
};

#endif
