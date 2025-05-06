#ifndef _PLAYERBOT_WOTLKDUNGEONUPACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONUPACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "UtgardePinnacleActions.h"

class WotlkDungeonUPActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonUPActionContext() {
            creators["avoid freezing cloud"] = &WotlkDungeonUPActionContext::avoid_freezing_cloud;
            creators["avoid skadi whirlwind"] = &WotlkDungeonUPActionContext::avoid_whirlwind;
            creators["stop attack"] = &WotlkDungeonUPActionContext::stop_attack;
        }
    private:
        static Action* avoid_freezing_cloud(PlayerbotAI* ai) { return new AvoidFreezingCloudAction(ai); }
        static Action* avoid_whirlwind(PlayerbotAI* ai) { return new AvoidSkadiWhirlwindAction(ai); }
        static Action* stop_attack(PlayerbotAI* ai) { return new DropTargetAction(ai); }
};

#endif
