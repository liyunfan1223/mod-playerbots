#ifndef _PLAYERBOT_WOTLKDUNGEONGDACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONGDACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "GundrakActions.h"

class WotlkDungeonGDActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonGDActionContext() {
            creators["avoid poison nova"] = &WotlkDungeonGDActionContext::avoid_poison_nova;
            creators["avoid whirling slash"] = &WotlkDungeonGDActionContext::avoid_whirling_slash;
        }
    private:
        static Action* avoid_poison_nova(PlayerbotAI* ai) { return new AvoidPoisonNovaAction(ai); }
        static Action* avoid_whirling_slash(PlayerbotAI* ai) { return new AvoidWhirlingSlashAction(ai); }
};

#endif
