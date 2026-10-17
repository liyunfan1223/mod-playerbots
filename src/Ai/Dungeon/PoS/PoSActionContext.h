/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSACTIONCONTEXT_H
#define PLAYERBOTS_POSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "PoSActions.h"

class WotlkDungeonPoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonPoSActionContext()
        {
            creators["ick and krick"] = &WotlkDungeonPoSActionContext::ick_and_krick;
            creators["tyrannus"] = &WotlkDungeonPoSActionContext::tyrannus;
        }
    private:
        static Action* ick_and_krick(PlayerbotAI* botAI) { return new IckAndKrickAction(botAI); }
        static Action* tyrannus(PlayerbotAI* botAI) { return new TyrannusAction(botAI); }
};

#endif
