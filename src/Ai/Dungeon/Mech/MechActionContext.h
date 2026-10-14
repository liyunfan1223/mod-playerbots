/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHACTIONCONTEXT_H
#define PLAYERBOTS_MECHACTIONCONTEXT_H

#include "Action.h"
#include "MechActions.h"
#include "NamedObjectContext.h"

class TbcDungeonMechanarActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonMechanarActionContext()
    {
        creators["sepethrea kite flame"] =
            &TbcDungeonMechanarActionContext::sepethrea_kite_flame;
        creators["sepethrea avoid flame"] =
            &TbcDungeonMechanarActionContext::sepethrea_avoid_flame;
        creators["sepethrea avoid trail"] =
            &TbcDungeonMechanarActionContext::sepethrea_avoid_trail;
        creators["sepethrea focus boss"] =
            &TbcDungeonMechanarActionContext::sepethrea_focus_boss;
    }

private:
    static Action* sepethrea_kite_flame(PlayerbotAI* botAI) { return new SepethreaKiteFlameAction(botAI); }
    static Action* sepethrea_avoid_flame(PlayerbotAI* botAI) { return new SepethreaAvoidFlameAction(botAI); }
    static Action* sepethrea_avoid_trail(PlayerbotAI* botAI) { return new SepethreaAvoidTrailAction(botAI); }
    static Action* sepethrea_focus_boss(PlayerbotAI* botAI) { return new SepethreaFocusBossAction(botAI); }
};

#endif
