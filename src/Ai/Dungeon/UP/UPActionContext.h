/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UPACTIONCONTEXT_H
#define PLAYERBOTS_UPACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "UPActions.h"

class WotlkDungeonUPActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonUPActionContext() {
            creators["avoid freezing cloud"] = &WotlkDungeonUPActionContext::avoid_freezing_cloud;
            creators["avoid skadi whirlwind"] = &WotlkDungeonUPActionContext::avoid_whirlwind;
        }
    private:
        static Action* avoid_freezing_cloud(PlayerbotAI* botAI) { return new AvoidFreezingCloudAction(botAI); }
        static Action* avoid_whirlwind(PlayerbotAI* botAI) { return new AvoidSkadiWhirlwindAction(botAI); }
};

#endif
