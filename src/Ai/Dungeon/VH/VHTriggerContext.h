/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHTRIGGERCONTEXT_H
#define PLAYERBOTS_VHTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "VHTriggers.h"

class WotlkDungeonVHTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonVHTriggerContext()
        {
            creators["erekem target"] = &WotlkDungeonVHTriggerContext::erekem_target;
            creators["ichoron target"] = &WotlkDungeonVHTriggerContext::ichoron_target;
            creators["void shift"] = &WotlkDungeonVHTriggerContext::void_shift;
            creators["shroud of darkness"] = &WotlkDungeonVHTriggerContext::shroud_of_darkness;
            creators["cyanigosa positioning"] = &WotlkDungeonVHTriggerContext::cyanigosa_positioning;
        }
    private:
        static Trigger* erekem_target(PlayerbotAI* botAI) { return new ErekemTargetTrigger(botAI); }
        static Trigger* ichoron_target(PlayerbotAI* botAI) { return new IchoronTargetTrigger(botAI); }
        static Trigger* void_shift(PlayerbotAI* botAI) { return new VoidShiftTrigger(botAI); }
        static Trigger* shroud_of_darkness(PlayerbotAI* botAI) { return new ShroudOfDarknessTrigger(botAI); }
        static Trigger* cyanigosa_positioning(PlayerbotAI* botAI) { return new CyanigosaPositioningTrigger(botAI); }
};

#endif
