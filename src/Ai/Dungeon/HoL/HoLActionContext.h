/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLACTIONCONTEXT_H
#define PLAYERBOTS_HOLACTIONCONTEXT_H

#include "Action.h"
#include "HoLActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoLActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonHoLActionContext() {
            creators["bjarngrim target"] = &WotlkDungeonHoLActionContext::bjarngrim_target;
            creators["avoid whirlwind"] = &WotlkDungeonHoLActionContext::avoid_whirlwind;
            creators["volkhan target"] = &WotlkDungeonHoLActionContext::volkhan_target;
            creators["static overload spread"] = &WotlkDungeonHoLActionContext::static_overload_spread;
            creators["ball lightning spread"] = &WotlkDungeonHoLActionContext::ball_lightning_spread;
            creators["ionar tank position"] = &WotlkDungeonHoLActionContext::ionar_tank_position;
            creators["disperse position"] = &WotlkDungeonHoLActionContext::disperse_position;
            creators["loken stack"] = &WotlkDungeonHoLActionContext::loken_stack;
            creators["avoid lightning nova"] = &WotlkDungeonHoLActionContext::avoid_lightning_nova;
        }
    private:
        static Action* bjarngrim_target(PlayerbotAI* botAI) { return new BjarngrimTargetAction(botAI); }
        static Action* avoid_whirlwind(PlayerbotAI* botAI) { return new AvoidWhirlwindAction(botAI); }
        static Action* volkhan_target(PlayerbotAI* botAI) { return new VolkhanTargetAction(botAI); }
        static Action* static_overload_spread(PlayerbotAI* botAI) { return new StaticOverloadSpreadAction(botAI); }
        static Action* ball_lightning_spread(PlayerbotAI* botAI) { return new BallLightningSpreadAction(botAI); }
        static Action* ionar_tank_position(PlayerbotAI* botAI) { return new IonarTankPositionAction(botAI); }
        static Action* disperse_position(PlayerbotAI* botAI) { return new DispersePositionAction(botAI); }
        static Action* loken_stack(PlayerbotAI* botAI) { return new LokenStackAction(botAI); }
        static Action* avoid_lightning_nova(PlayerbotAI* botAI) { return new AvoidLightningNovaAction(botAI); }
};

#endif
