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
            creators["garfrost take cover"] = &WotlkDungeonPoSActionContext::garfrost_take_cover;
            creators["garfrost dodge saronite"] = &WotlkDungeonPoSActionContext::garfrost_dodge_saronite;
            creators["garfrost tank position"] = &WotlkDungeonPoSActionContext::garfrost_tank_position;
            creators["pos avoid ground hazard"] = &WotlkDungeonPoSActionContext::pos_avoid_ground_hazard;
            creators["tyrannus brand stop"] = &WotlkDungeonPoSActionContext::tyrannus_brand_stop;
            creators["tyrannus brand restore pets"] = &WotlkDungeonPoSActionContext::tyrannus_brand_restore_pets;
            creators["tyrannus brand stop cast"] = &WotlkDungeonPoSActionContext::tyrannus_brand_stop_cast;
        }
    private:
        static Action* ick_and_krick(PlayerbotAI* botAI) { return new IckAndKrickAction(botAI); }
        static Action* tyrannus(PlayerbotAI* botAI) { return new TyrannusAction(botAI); }
        static Action* garfrost_take_cover(PlayerbotAI* botAI) { return new GarfrostTakeCoverAction(botAI); }
        static Action* garfrost_dodge_saronite(PlayerbotAI* botAI) { return new GarfrostDodgeSaroniteAction(botAI); }
        static Action* garfrost_tank_position(PlayerbotAI* botAI) { return new GarfrostTankPositionAction(botAI); }
        static Action* pos_avoid_ground_hazard(PlayerbotAI* botAI) { return new PoSAvoidGroundHazardAction(botAI); }
        static Action* tyrannus_brand_stop(PlayerbotAI* botAI) { return new TyrannusBrandStopAction(botAI); }
        static Action* tyrannus_brand_restore_pets(PlayerbotAI* botAI) { return new TyrannusBrandRestorePetsAction(botAI); }
        static Action* tyrannus_brand_stop_cast(PlayerbotAI* botAI) { return new TyrannusBrandStopCastAction(botAI); }
};

#endif
