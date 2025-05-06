#ifndef _PLAYERBOT_WOTLKDUNGEONDTKACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONDTKACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "DrakTharonKeepActions.h"

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
        static Action* corpse_explode_spread(PlayerbotAI* ai) { return new CorpseExplodeSpreadAction(ai); }
        static Action* avoid_arcane_field(PlayerbotAI* ai) { return new AvoidArcaneFieldAction(ai); }
        static Action* novos_positioning(PlayerbotAI* ai) { return new NovosDefaultPositionAction(ai); }
        static Action* novos_target_priority(PlayerbotAI* ai) { return new NovosTargetPriorityAction(ai); }
        static Action* slaying_strike(PlayerbotAI* ai) { return new CastSlayingStrikeAction(ai); }
        static Action* taunt(PlayerbotAI* ai) { return new CastTauntAction(ai); }
        static Action* bone_armor(PlayerbotAI* ai) { return new CastBoneArmorAction(ai); }
        static Action* touch_of_life(PlayerbotAI* ai) { return new CastTouchOfLifeAction(ai); }
};

#endif
