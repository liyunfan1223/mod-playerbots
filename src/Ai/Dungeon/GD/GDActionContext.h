/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDACTIONCONTEXT_H
#define PLAYERBOTS_GDACTIONCONTEXT_H

#include "Action.h"
#include "GDActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonGDActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonGDActionContext() {
            creators["avoid poison nova"] = &WotlkDungeonGDActionContext::avoid_poison_nova;
            creators["attack snake wrap"] = &WotlkDungeonGDActionContext::attack_snake_wrap;
            creators["slad'ran stack on tank"] = &WotlkDungeonGDActionContext::sladran_stack_on_tank;
            creators["slad'ran tank hold"] = &WotlkDungeonGDActionContext::sladran_tank_hold;
            creators["avoid whirling slash"] = &WotlkDungeonGDActionContext::avoid_whirling_slash;
        }
    private:
        static Action* avoid_poison_nova(PlayerbotAI* botAI) { return new AvoidPoisonNovaAction(botAI); }
        static Action* attack_snake_wrap(PlayerbotAI* botAI) { return new AttackSnakeWrapAction(botAI); }
        static Action* sladran_stack_on_tank(PlayerbotAI* botAI) { return new SladranStackOnTankAction(botAI); }
        static Action* sladran_tank_hold(PlayerbotAI* botAI) { return new SladranTankHoldAction(botAI); }
        static Action* avoid_whirling_slash(PlayerbotAI* botAI) { return new AvoidWhirlingSlashAction(botAI); }
};

#endif
