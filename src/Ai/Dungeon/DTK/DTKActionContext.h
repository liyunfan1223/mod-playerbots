/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKACTIONCONTEXT_H
#define PLAYERBOTS_DTKACTIONCONTEXT_H

#include "Action.h"
#include "DTKActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonDTKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonDTKActionContext() {
            creators["corpse explode spread"] = &WotlkDungeonDTKActionContext::corpse_explode_spread;
            creators["avoid arcane field"] = &WotlkDungeonDTKActionContext::avoid_arcane_field;
            creators["novos positioning"] = &WotlkDungeonDTKActionContext::novos_positioning;
            creators["novos target priority"] = &WotlkDungeonDTKActionContext::novos_target_priority;
            creators["slaying strike"] = &WotlkDungeonDTKActionContext::slaying_strike;
            creators["tharonja taunt"] = &WotlkDungeonDTKActionContext::taunt;
            creators["bone armor"] = &WotlkDungeonDTKActionContext::bone_armor;
            creators["touch of life"] = &WotlkDungeonDTKActionContext::touch_of_life;
        }
    private:
        static Action* corpse_explode_spread(PlayerbotAI* botAI) { return new CorpseExplodeSpreadAction(botAI); }
        static Action* avoid_arcane_field(PlayerbotAI* botAI) { return new AvoidArcaneFieldAction(botAI); }
        static Action* novos_positioning(PlayerbotAI* botAI) { return new NovosDefaultPositionAction(botAI); }
        static Action* novos_target_priority(PlayerbotAI* botAI) { return new NovosTargetPriorityAction(botAI); }
        static Action* slaying_strike(PlayerbotAI* botAI) { return new CastSlayingStrikeAction(botAI); }
        static Action* taunt(PlayerbotAI* botAI) { return new CastTauntAction(botAI); }
        static Action* bone_armor(PlayerbotAI* botAI) { return new CastBoneArmorAction(botAI); }
        static Action* touch_of_life(PlayerbotAI* botAI) { return new CastTouchOfLifeAction(botAI); }
};

#endif
