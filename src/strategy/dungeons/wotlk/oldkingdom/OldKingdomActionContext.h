#ifndef _PLAYERBOT_WOTLKDUNGEONOKACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONOKACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "OldKingdomActions.h"

class WotlkDungeonOKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonOKActionContext() {
            creators["attack nadox guardian"] = &WotlkDungeonOKActionContext::attack_nadox_guardian;
            creators["attack jedoga volunteer"] = &WotlkDungeonOKActionContext::attack_jedoga_volunteer;
            creators["avoid shadow crash"] = &WotlkDungeonOKActionContext::avoid_shadow_crash;
        }
    private:
        static Action* attack_nadox_guardian(PlayerbotAI* ai) { return new AttackNadoxGuardianAction(ai); }
        static Action* attack_jedoga_volunteer(PlayerbotAI* ai) { return new AttackJedogaVolunteerAction(ai); }
        static Action* avoid_shadow_crash(PlayerbotAI* ai) { return new AvoidShadowCrashAction(ai); }
};

#endif
