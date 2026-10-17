/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHACTIONCONTEXT_H
#define PLAYERBOTS_VHACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "VHActions.h"

class WotlkDungeonVHActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonVHActionContext() {
            creators["attack erekem"] = &WotlkDungeonVHActionContext::attack_erekem;
            creators["attack ichor globule"] = &WotlkDungeonVHActionContext::attack_ichor_globule;
            creators["attack void sentry"] = &WotlkDungeonVHActionContext::attack_void_sentry;
        }
    private:
        static Action* attack_erekem(PlayerbotAI* botAI) { return new AttackErekemAction(botAI); }
        static Action* attack_ichor_globule(PlayerbotAI* botAI) { return new AttackIchorGlobuleAction(botAI); }
        static Action* attack_void_sentry(PlayerbotAI* botAI) { return new AttackVoidSentryAction(botAI); }
};

#endif
