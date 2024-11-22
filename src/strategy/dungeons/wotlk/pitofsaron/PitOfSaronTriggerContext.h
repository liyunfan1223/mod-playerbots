#ifndef _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "PitOfSaronTriggers.h"

class WotlkDungeonPoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonPoSTriggerContext()
    {
        creators["move from ick pursuit"] = &WotlkDungeonPoSTriggerContext::move_from_ick_pursuit;
        creators["move from poison nova"] = &WotlkDungeonPoSTriggerContext::move_from_poison_nova;
        creators["move from exploding orb"] = &WotlkDungeonPoSTriggerContext::move_from_exploding_orb;
    }

private:
    static Trigger* move_from_ick_pursuit(PlayerbotAI* ai) { return new MoveFromIckPursuitTrigger(ai); }
    static Trigger* move_from_poison_nova(PlayerbotAI* ai) { return new MoveFromPoisonNovaTrigger(ai); }
    static Trigger* move_from_exploding_orb(PlayerbotAI* ai) { return new MoveFromExplodingOrbTrigger(ai); }
};

#endif  // !_PLAYERBOT_WOTLKDUNGEONPOSTRIGGERCONTEXT_H
