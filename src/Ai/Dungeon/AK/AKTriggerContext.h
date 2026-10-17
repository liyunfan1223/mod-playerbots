/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AKTRIGGERCONTEXT_H
#define PLAYERBOTS_AKTRIGGERCONTEXT_H

#include "AKTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonOKTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonOKTriggerContext()
        {
            creators["nadox guardian"] = &WotlkDungeonOKTriggerContext::nadox_guardian;
            creators["jedoga volunteer"] = &WotlkDungeonOKTriggerContext::jedoga_volunteer;
            creators["shadow crash"] = &WotlkDungeonOKTriggerContext::shadow_crash;
        }
    private:
        static Trigger* nadox_guardian(PlayerbotAI* botAI) { return new NadoxGuardianTrigger(botAI); }
        static Trigger* jedoga_volunteer(PlayerbotAI* botAI) { return new JedogaVolunteerTrigger(botAI); }
        static Trigger* shadow_crash(PlayerbotAI* botAI) { return new ShadowCrashTrigger(botAI); }
};

#endif
