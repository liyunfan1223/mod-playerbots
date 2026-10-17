/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSACTIONCONTEXT_H
#define PLAYERBOTS_COSACTIONCONTEXT_H

#include "Action.h"
#include "CoSActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonCoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonCoSActionContext() {
            creators["explode ghoul spread"] = &WotlkDungeonCoSActionContext::explode_ghoul_spread;
            creators["epoch stack"] = &WotlkDungeonCoSActionContext::epoch_stack;
        }
    private:
        static Action* explode_ghoul_spread(PlayerbotAI* botAI) { return new ExplodeGhoulSpreadAction(botAI); }
        static Action* epoch_stack(PlayerbotAI* botAI) { return new EpochStackAction(botAI); }
};

#endif
