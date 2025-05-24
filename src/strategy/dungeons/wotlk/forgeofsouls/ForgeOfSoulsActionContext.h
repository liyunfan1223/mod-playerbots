#ifndef _PLAYERBOT_WOTLKDUNGEONFOSACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONFOSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "ForgeOfSoulsActions.h"

class WotlkDungeonFoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonFoSActionContext()
        {
            creators["move from bronjahm"] = &WotlkDungeonFoSActionContext::move_from_bronjahm;
            creators["attack corrupted soul fragment"] = &WotlkDungeonFoSActionContext::attack_corrupted_soul_fragment;
            creators["bronjahm group position"] = &WotlkDungeonFoSActionContext::bronjahm_group_position;
            creators["devourer of souls"] = &WotlkDungeonFoSActionContext::devourer_of_souls;
        }
    private:
        static Action* move_from_bronjahm(PlayerbotAI* ai) { return new MoveFromBronjahmAction(ai); }
        static Action* attack_corrupted_soul_fragment(PlayerbotAI* ai) { return new AttackCorruptedSoulFragmentAction(ai); }
        static Action* bronjahm_group_position(PlayerbotAI* ai) { return new BronjahmGroupPositionAction(ai); }
        static Action* devourer_of_souls(PlayerbotAI* ai) { return new DevourerOfSoulsAction(ai); }
};

#endif
