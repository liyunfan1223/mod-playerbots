/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AKACTIONCONTEXT_H
#define PLAYERBOTS_AKACTIONCONTEXT_H

#include "AKActions.h"
#include "Action.h"
#include "NamedObjectContext.h"

class WotlkDungeonOKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonOKActionContext() {
            creators["attack nadox guardian"] = &WotlkDungeonOKActionContext::attack_nadox_guardian;
            creators["attack jedoga volunteer"] = &WotlkDungeonOKActionContext::attack_jedoga_volunteer;
            creators["avoid shadow crash"] = &WotlkDungeonOKActionContext::avoid_shadow_crash;
        }
    private:
        static Action* attack_nadox_guardian(PlayerbotAI* botAI) { return new AttackNadoxGuardianAction(botAI); }
        static Action* attack_jedoga_volunteer(PlayerbotAI* botAI) { return new AttackJedogaVolunteerAction(botAI); }
        static Action* avoid_shadow_crash(PlayerbotAI* botAI) { return new AvoidShadowCrashAction(botAI); }
};

#endif
