/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSACTIONCONTEXT_H
#define PLAYERBOTS_HOSACTIONCONTEXT_H

#include "Action.h"
#include "HoSActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonHoSActionContext() {
            creators["shatter spread"] = &WotlkDungeonHoSActionContext::shatter_spread;
            creators["avoid lightning ring"] = &WotlkDungeonHoSActionContext::avoid_lightning_ring;
        }
    private:
        static Action* shatter_spread(PlayerbotAI* botAI) { return new ShatterSpreadAction(botAI); }
        static Action* avoid_lightning_ring(PlayerbotAI* botAI) { return new AvoidLightningRingAction(botAI); }
};

#endif
