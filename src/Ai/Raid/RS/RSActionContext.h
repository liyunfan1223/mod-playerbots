/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSACTIONCONTEXT_H
#define PLAYERBOTS_RSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RSActions.h"

class RaidRsActionContext : public NamedObjectContext<Action>
{
public:
    RaidRsActionContext()
    {
        creators["rs baltharus brand"] = &RaidRsActionContext::rs_baltharus_brand;
        creators["rs baltharus tank position"] = &RaidRsActionContext::rs_baltharus_tank_position;
        creators["rs baltharus avoid front"] = &RaidRsActionContext::rs_baltharus_avoid_front;
        creators["rs baltharus healer position"] = &RaidRsActionContext::rs_baltharus_healer_position;
        creators["rs saviana conflagration"] = &RaidRsActionContext::rs_saviana_conflagration;
        creators["rs saviana avoid front"] = &RaidRsActionContext::rs_saviana_avoid_front;
        creators["rs saviana tank position"] = &RaidRsActionContext::rs_saviana_tank_position;
        creators["rs saviana melee spread"] = &RaidRsActionContext::rs_saviana_melee_spread;
        creators["rs zarithrian adds"] = &RaidRsActionContext::rs_zarithrian_adds;
        creators["rs zarithrian tank"] = &RaidRsActionContext::rs_zarithrian_tank;
        creators["rs halion tank position"] = &RaidRsActionContext::rs_halion_tank_position;
        creators["rs halion avoid cones"] = &RaidRsActionContext::rs_halion_avoid_cones;
        creators["rs halion combustion"] = &RaidRsActionContext::rs_halion_combustion;
        creators["rs halion meteor"] = &RaidRsActionContext::rs_halion_meteor;
        creators["rs halion adds"] = &RaidRsActionContext::rs_halion_adds;
        creators["rs halion add tank"] = &RaidRsActionContext::rs_halion_add_tank;
        creators["rs halion start position"] = &RaidRsActionContext::rs_halion_start_position;
        creators["rs halion enter portal"] = &RaidRsActionContext::rs_halion_enter_portal;
        creators["rs halion p2 tank position"] = &RaidRsActionContext::rs_halion_p2_tank_position;
        creators["rs halion p2 avoid cones"] = &RaidRsActionContext::rs_halion_p2_avoid_cones;
        creators["rs halion consumption"] = &RaidRsActionContext::rs_halion_consumption;
        creators["rs halion cutter"] = &RaidRsActionContext::rs_halion_cutter;
        creators["rs halion heal consumption"] = &RaidRsActionContext::rs_halion_heal_consumption;
        creators["rs trash adds"] = &RaidRsActionContext::rs_trash_adds;
        creators["rs trash main tank"] = &RaidRsActionContext::rs_trash_main_tank;
        creators["rs trash assist tank"] = &RaidRsActionContext::rs_trash_assist_tank;
        creators["rs trash ranged"] = &RaidRsActionContext::rs_trash_ranged;
    }

private:
    static Action* rs_baltharus_brand(PlayerbotAI* botAI) { return new RsBaltharusBrandAction(botAI); }
    static Action* rs_baltharus_tank_position(PlayerbotAI* botAI) { return new RsBaltharusTankPositionAction(botAI); }
    static Action* rs_baltharus_avoid_front(PlayerbotAI* botAI) { return new RsBaltharusAvoidFrontAction(botAI); }
    static Action* rs_baltharus_healer_position(PlayerbotAI* botAI) { return new RsBaltharusHealerPositionAction(botAI); }
    static Action* rs_saviana_conflagration(PlayerbotAI* botAI) { return new RsSavianaConflagrationAction(botAI); }
    static Action* rs_saviana_avoid_front(PlayerbotAI* botAI) { return new RsSavianaAvoidFrontAction(botAI); }
    static Action* rs_saviana_tank_position(PlayerbotAI* botAI) { return new RsSavianaTankPositionAction(botAI); }
    static Action* rs_saviana_melee_spread(PlayerbotAI* botAI) { return new RsSavianaMeleeSpreadAction(botAI); }
    static Action* rs_zarithrian_adds(PlayerbotAI* botAI) { return new RsZarithrianAddsAction(botAI); }
    static Action* rs_zarithrian_tank(PlayerbotAI* botAI) { return new RsZarithrianTankAction(botAI); }
    static Action* rs_halion_tank_position(PlayerbotAI* botAI) { return new RsHalionTankPositionAction(botAI); }
    static Action* rs_halion_avoid_cones(PlayerbotAI* botAI) { return new RsHalionAvoidConesAction(botAI); }
    static Action* rs_halion_combustion(PlayerbotAI* botAI) { return new RsHalionCombustionAction(botAI); }
    static Action* rs_halion_meteor(PlayerbotAI* botAI) { return new RsHalionMeteorAction(botAI); }
    static Action* rs_halion_adds(PlayerbotAI* botAI) { return new RsHalionAddsAction(botAI); }
    static Action* rs_halion_add_tank(PlayerbotAI* botAI) { return new RsHalionAddTankAction(botAI); }
    static Action* rs_halion_start_position(PlayerbotAI* botAI) { return new RsHalionStartPositionAction(botAI); }
    static Action* rs_halion_enter_portal(PlayerbotAI* botAI) { return new RsHalionEnterPortalAction(botAI); }
    static Action* rs_halion_p2_tank_position(PlayerbotAI* botAI) { return new RsHalionP2TankPositionAction(botAI); }
    static Action* rs_halion_p2_avoid_cones(PlayerbotAI* botAI) { return new RsHalionP2AvoidConesAction(botAI); }
    static Action* rs_halion_consumption(PlayerbotAI* botAI) { return new RsHalionConsumptionAction(botAI); }
    static Action* rs_halion_cutter(PlayerbotAI* botAI) { return new RsHalionCutterAction(botAI); }
    static Action* rs_halion_heal_consumption(PlayerbotAI* botAI) { return new RsHalionHealConsumptionAction(botAI); }
    static Action* rs_trash_adds(PlayerbotAI* botAI) { return new RsTrashAddsAction(botAI); }
    static Action* rs_trash_main_tank(PlayerbotAI* botAI) { return new RsTrashMainTankAction(botAI); }
    static Action* rs_trash_assist_tank(PlayerbotAI* botAI) { return new RsTrashAssistTankAction(botAI); }
    static Action* rs_trash_ranged(PlayerbotAI* botAI) { return new RsTrashRangedAction(botAI); }
};

#endif
