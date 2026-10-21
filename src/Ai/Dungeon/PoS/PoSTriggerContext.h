/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSTRIGGERCONTEXT_H
#define PLAYERBOTS_POSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "PoSTriggers.h"

class WotlkDungeonPoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonPoSTriggerContext()
    {
        creators["ick and krick"] = &WotlkDungeonPoSTriggerContext::ick_and_krick;
        creators["tyrannus"] = &WotlkDungeonPoSTriggerContext::tyrannus;
        creators["garfrost permafrost"] = &WotlkDungeonPoSTriggerContext::garfrost_permafrost;
        creators["garfrost saronite"] = &WotlkDungeonPoSTriggerContext::garfrost_saronite;
        creators["garfrost tank position"] = &WotlkDungeonPoSTriggerContext::garfrost_tank_position;
        creators["pos ground hazard"] = &WotlkDungeonPoSTriggerContext::pos_ground_hazard;
        creators["tyrannus overlords brand"] = &WotlkDungeonPoSTriggerContext::tyrannus_overlords_brand;
        creators["tyrannus overlords brand faded"] = &WotlkDungeonPoSTriggerContext::tyrannus_overlords_brand_faded;
        creators["tyrannus brand stop cast"] = &WotlkDungeonPoSTriggerContext::tyrannus_brand_stop_cast;
    }

private:
    static Trigger* ick_and_krick(PlayerbotAI* botAI) { return new IckAndKrickTrigger(botAI); }
    static Trigger* tyrannus(PlayerbotAI* botAI) { return new TyrannusTrigger(botAI); }
    static Trigger* garfrost_permafrost(PlayerbotAI* botAI) { return new GarfrostPermafrostTrigger(botAI); }
    static Trigger* garfrost_saronite(PlayerbotAI* botAI) { return new GarfrostSaroniteTrigger(botAI); }
    static Trigger* garfrost_tank_position(PlayerbotAI* botAI) { return new GarfrostTankPositionTrigger(botAI); }
    static Trigger* pos_ground_hazard(PlayerbotAI* botAI) { return new PoSGroundHazardTrigger(botAI); }
    static Trigger* tyrannus_overlords_brand(PlayerbotAI* botAI) { return new TyrannusOverlordsBrandTrigger(botAI); }
    static Trigger* tyrannus_overlords_brand_faded(PlayerbotAI* botAI) { return new TyrannusOverlordsBrandFadedTrigger(botAI); }
    static Trigger* tyrannus_brand_stop_cast(PlayerbotAI* botAI) { return new TyrannusBrandStopCastTrigger(botAI); }
};

#endif
