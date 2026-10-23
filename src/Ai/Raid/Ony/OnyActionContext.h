/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ONYACTIONCONTEXT_H
#define PLAYERBOTS_ONYACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "OnyActions.h"

class RaidOnyxiaActionContext : public NamedObjectContext<Action>
{
public:
    RaidOnyxiaActionContext()
    {
        creators["ony move to side"] = &RaidOnyxiaActionContext::move_to_side;
        creators["ony spread out"] = &RaidOnyxiaActionContext::spread_out;
        creators["ony move to safe zone"] = &RaidOnyxiaActionContext::move_to_safe_zone;
        creators["ony kill whelps"] = &RaidOnyxiaActionContext::kill_whelps;
        creators["ony avoid eggs move"] = &RaidOnyxiaActionContext::avoid_eggs;
    }

private:
    static Action* move_to_side(PlayerbotAI* botAI) { return new RaidOnyxiaMoveToSideAction(botAI); }
    static Action* spread_out(PlayerbotAI* botAI) { return new RaidOnyxiaSpreadOutAction(botAI); }
    static Action* move_to_safe_zone(PlayerbotAI* botAI) { return new RaidOnyxiaMoveToSafeZoneAction(botAI); }
    static Action* kill_whelps(PlayerbotAI* botAI) { return new RaidOnyxiaKillWhelpsAction(botAI); }
    static Action* avoid_eggs(PlayerbotAI* botAI) { return new OnyxiaAvoidEggsAction(botAI); }
};

#endif
