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
        creators["move from ick pursuit"] = &WotlkDungeonPoSActionContext::move_from_ick_pursuit;
        creators["move from poison nova"] = &WotlkDungeonPoSActionContext::move_from_poison_nova;
        creators["move from exploding orb"] = &WotlkDungeonPoSActionContext::move_from_exploding_orb;
    }
    private:
        static Action* move_from_ick_pursuit(PlayerbotAI* ai) { return new MoveFromIckPursuitAction(ai); }
        static Action* move_from_poison_nova(PlayerbotAI* ai) { return new MoveFromPoisonNovaAction(ai); }
        static Action* move_from_exploding_orb(PlayerbotAI* ai) { return new MoveFromExplodingOrbAction(ai); }
};

#endif
