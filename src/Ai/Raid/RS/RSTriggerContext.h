/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSTRIGGERCONTEXT_H
#define PLAYERBOTS_RSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "RSTriggers.h"

class RaidRsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidRsTriggerContext()
    {
        creators["rs baltharus brand"] = &RaidRsTriggerContext::rs_baltharus_brand;
        creators["rs baltharus tank position"] = &RaidRsTriggerContext::rs_baltharus_tank_position;
        creators["rs baltharus avoid front"] = &RaidRsTriggerContext::rs_baltharus_avoid_front;
        creators["rs baltharus healer position"] = &RaidRsTriggerContext::rs_baltharus_healer_position;
        creators["rs saviana conflagration"] = &RaidRsTriggerContext::rs_saviana_conflagration;
        creators["rs saviana avoid front"] = &RaidRsTriggerContext::rs_saviana_avoid_front;
        creators["rs saviana tank position"] = &RaidRsTriggerContext::rs_saviana_tank_position;
        creators["rs saviana melee spread"] = &RaidRsTriggerContext::rs_saviana_melee_spread;
        creators["rs zarithrian adds"] = &RaidRsTriggerContext::rs_zarithrian_adds;
        creators["rs zarithrian tank"] = &RaidRsTriggerContext::rs_zarithrian_tank;
        creators["rs halion tank position"] = &RaidRsTriggerContext::rs_halion_tank_position;
        creators["rs halion avoid cones"] = &RaidRsTriggerContext::rs_halion_avoid_cones;
        creators["rs halion combustion"] = &RaidRsTriggerContext::rs_halion_combustion;
        creators["rs halion meteor"] = &RaidRsTriggerContext::rs_halion_meteor;
        creators["rs halion adds"] = &RaidRsTriggerContext::rs_halion_adds;
        creators["rs halion add tank"] = &RaidRsTriggerContext::rs_halion_add_tank;
        creators["rs halion start position"] = &RaidRsTriggerContext::rs_halion_start_position;
        creators["rs halion enter portal"] = &RaidRsTriggerContext::rs_halion_enter_portal;
        creators["rs halion p2 tank position"] = &RaidRsTriggerContext::rs_halion_p2_tank_position;
        creators["rs halion p2 avoid cones"] = &RaidRsTriggerContext::rs_halion_p2_avoid_cones;
        creators["rs halion consumption"] = &RaidRsTriggerContext::rs_halion_consumption;
        creators["rs halion cutter"] = &RaidRsTriggerContext::rs_halion_cutter;
        creators["rs halion heal consumption"] = &RaidRsTriggerContext::rs_halion_heal_consumption;
        creators["rs trash adds"] = &RaidRsTriggerContext::rs_trash_adds;
        creators["rs trash main tank"] = &RaidRsTriggerContext::rs_trash_main_tank;
        creators["rs trash assist tank"] = &RaidRsTriggerContext::rs_trash_assist_tank;
        creators["rs trash ranged"] = &RaidRsTriggerContext::rs_trash_ranged;
        creators["rs trash melee flank"] = &RaidRsTriggerContext::rs_trash_melee_flank;
    }

private:
    static Trigger* rs_baltharus_brand(PlayerbotAI* botAI) { return new RsBaltharusBrandTrigger(botAI); }
    static Trigger* rs_baltharus_tank_position(PlayerbotAI* botAI) { return new RsBaltharusTankPositionTrigger(botAI); }
    static Trigger* rs_baltharus_avoid_front(PlayerbotAI* botAI) { return new RsBaltharusAvoidFrontTrigger(botAI); }
    static Trigger* rs_baltharus_healer_position(PlayerbotAI* botAI) { return new RsBaltharusHealerPositionTrigger(botAI); }
    static Trigger* rs_saviana_conflagration(PlayerbotAI* botAI) { return new RsSavianaConflagrationTrigger(botAI); }
    static Trigger* rs_saviana_avoid_front(PlayerbotAI* botAI) { return new RsSavianaAvoidFrontTrigger(botAI); }
    static Trigger* rs_saviana_tank_position(PlayerbotAI* botAI) { return new RsSavianaTankPositionTrigger(botAI); }
    static Trigger* rs_saviana_melee_spread(PlayerbotAI* botAI) { return new RsSavianaMeleeSpreadTrigger(botAI); }
    static Trigger* rs_zarithrian_adds(PlayerbotAI* botAI) { return new RsZarithrianAddsTrigger(botAI); }
    static Trigger* rs_zarithrian_tank(PlayerbotAI* botAI) { return new RsZarithrianTankTrigger(botAI); }
    static Trigger* rs_halion_tank_position(PlayerbotAI* botAI) { return new RsHalionTankPositionTrigger(botAI); }
    static Trigger* rs_halion_avoid_cones(PlayerbotAI* botAI) { return new RsHalionAvoidConesTrigger(botAI); }
    static Trigger* rs_halion_combustion(PlayerbotAI* botAI) { return new RsHalionCombustionTrigger(botAI); }
    static Trigger* rs_halion_meteor(PlayerbotAI* botAI) { return new RsHalionMeteorTrigger(botAI); }
    static Trigger* rs_halion_adds(PlayerbotAI* botAI) { return new RsHalionAddsTrigger(botAI); }
    static Trigger* rs_halion_add_tank(PlayerbotAI* botAI) { return new RsHalionAddTankTrigger(botAI); }
    static Trigger* rs_halion_start_position(PlayerbotAI* botAI) { return new RsHalionStartPositionTrigger(botAI); }
    static Trigger* rs_halion_enter_portal(PlayerbotAI* botAI) { return new RsHalionEnterPortalTrigger(botAI); }
    static Trigger* rs_halion_p2_tank_position(PlayerbotAI* botAI) { return new RsHalionP2TankPositionTrigger(botAI); }
    static Trigger* rs_halion_p2_avoid_cones(PlayerbotAI* botAI) { return new RsHalionP2AvoidConesTrigger(botAI); }
    static Trigger* rs_halion_consumption(PlayerbotAI* botAI) { return new RsHalionConsumptionTrigger(botAI); }
    static Trigger* rs_halion_cutter(PlayerbotAI* botAI) { return new RsHalionCutterTrigger(botAI); }
    static Trigger* rs_halion_heal_consumption(PlayerbotAI* botAI) { return new RsHalionHealConsumptionTrigger(botAI); }
    static Trigger* rs_trash_adds(PlayerbotAI* botAI) { return new RsTrashAddsTrigger(botAI); }
    static Trigger* rs_trash_main_tank(PlayerbotAI* botAI) { return new RsTrashMainTankTrigger(botAI); }
    static Trigger* rs_trash_assist_tank(PlayerbotAI* botAI) { return new RsTrashAssistTankTrigger(botAI); }
    static Trigger* rs_trash_ranged(PlayerbotAI* botAI) { return new RsTrashRangedTrigger(botAI); }
    static Trigger* rs_trash_melee_flank(PlayerbotAI* botAI) { return new RsTrashMeleeFlankTrigger(botAI); }
};

#endif
