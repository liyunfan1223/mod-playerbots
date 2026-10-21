/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSTRIGGERCONTEXT_H
#define PLAYERBOTS_FOSTRIGGERCONTEXT_H

#include "FoSTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonFoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonFoSTriggerContext()
    {
        creators["bronjahm position"] = &WotlkDungeonFoSTriggerContext::bronjahm_position;
        creators["move from bronjahm"] = &WotlkDungeonFoSTriggerContext::move_from_bronjahm;
        creators["switch to soul fragment"] = &WotlkDungeonFoSTriggerContext::switch_to_soul_fragment;
        creators["devourer wailing souls"] = &WotlkDungeonFoSTriggerContext::devourer_wailing_souls;
        creators["devourer well of souls"] = &WotlkDungeonFoSTriggerContext::devourer_well_of_souls;
        creators["devourer mirrored soul"] = &WotlkDungeonFoSTriggerContext::devourer_mirrored_soul;
        creators["devourer mirrored soul faded"] = &WotlkDungeonFoSTriggerContext::devourer_mirrored_soul_faded;
        creators["devourer phantom blast"] = &WotlkDungeonFoSTriggerContext::devourer_phantom_blast;
        creators["devourer stop cast"] = &WotlkDungeonFoSTriggerContext::devourer_stop_cast;
    }

private:
    static Trigger* move_from_bronjahm(PlayerbotAI* botAI) { return new MoveFromBronjahmTrigger(botAI); }
    static Trigger* switch_to_soul_fragment(PlayerbotAI* botAI) { return new SwitchToSoulFragment(botAI); }
    static Trigger* bronjahm_position(PlayerbotAI* botAI) { return new BronjahmPositionTrigger(botAI); }
    static Trigger* devourer_wailing_souls(PlayerbotAI* botAI) { return new DevourerWailingSoulsTrigger(botAI); }
    static Trigger* devourer_well_of_souls(PlayerbotAI* botAI) { return new DevourerWellOfSoulsTrigger(botAI); }
    static Trigger* devourer_mirrored_soul(PlayerbotAI* botAI) { return new DevourerMirroredSoulTrigger(botAI); }
    static Trigger* devourer_stop_cast(PlayerbotAI* botAI) { return new DevourerStopCastTrigger(botAI); }
    static Trigger* devourer_mirrored_soul_faded(PlayerbotAI* botAI) { return new DevourerMirroredSoulFadedTrigger(botAI); }
    static Trigger* devourer_phantom_blast(PlayerbotAI* botAI) { return new DevourerPhantomBlastTrigger(botAI); }
};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
