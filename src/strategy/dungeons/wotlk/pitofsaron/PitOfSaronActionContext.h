#ifndef _PLAYERBOT_WOTLKDUNGEONPOSACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONPOSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "PitOfSaronActions.h"

class WotlkDungeonPoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonPoSActionContext()
        {
            creators["ick and krick"] = &WotlkDungeonPoSActionContext::ick_and_krick;
            creators["tyrannus"] = &WotlkDungeonPoSActionContext::tyrannus;
        }
    private:
        static Action* ick_and_krick(PlayerbotAI* ai) { return new IckAndKrickAction(ai); }
        static Action* tyrannus(PlayerbotAI* ai) { return new TyrannusAction(ai); }
};

#endif
