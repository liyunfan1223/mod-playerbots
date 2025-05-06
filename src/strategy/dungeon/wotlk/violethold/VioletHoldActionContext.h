#ifndef _PLAYERBOT_WOTLKDUNGEONVHACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONVHACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "VioletHoldActions.h"

class WotlkDungeonVHActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonVHActionContext() {
            creators["attack erekem"] = &WotlkDungeonVHActionContext::attack_erekem;
            creators["attack ichor globule"] = &WotlkDungeonVHActionContext::attack_ichor_globule;
            creators["attack void sentry"] = &WotlkDungeonVHActionContext::attack_void_sentry;
            creators["stop attack"] = &WotlkDungeonVHActionContext::stop_attack;
        }
    private:
        static Action* attack_erekem(PlayerbotAI* ai) { return new AttackErekemAction(ai); }
        static Action* attack_ichor_globule(PlayerbotAI* ai) { return new AttackIchorGlobuleAction(ai); }
        static Action* attack_void_sentry(PlayerbotAI* ai) { return new AttackVoidSentryAction(ai); }
        static Action* stop_attack(PlayerbotAI* ai) { return new StopAttackAction(ai); }
};

#endif
