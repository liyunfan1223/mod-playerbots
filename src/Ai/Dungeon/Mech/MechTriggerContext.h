/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHTRIGGERCONTEXT_H
#define PLAYERBOTS_MECHTRIGGERCONTEXT_H

#include "MechTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class TbcDungeonMechanarTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonMechanarTriggerContext()
    {
        creators["sepethrea kite flame"] =
            &TbcDungeonMechanarTriggerContext::sepethrea_kite_flame;
        creators["sepethrea avoid flame"] =
            &TbcDungeonMechanarTriggerContext::sepethrea_avoid_flame;
        creators["sepethrea trail"] =
            &TbcDungeonMechanarTriggerContext::sepethrea_trail;
        creators["sepethrea focus boss"] =
            &TbcDungeonMechanarTriggerContext::sepethrea_focus_boss;
    }

private:
    static Trigger* sepethrea_kite_flame(PlayerbotAI* botAI) { return new SepethreaKiteFlameTrigger(botAI); }
    static Trigger* sepethrea_avoid_flame(PlayerbotAI* botAI) { return new SepethreaAvoidFlameTrigger(botAI); }
    static Trigger* sepethrea_trail(PlayerbotAI* botAI) { return new SepethreaTrailTrigger(botAI); }
    static Trigger* sepethrea_focus_boss(PlayerbotAI* botAI) { return new SepethreaFocusBossTrigger(botAI); }
};

#endif
