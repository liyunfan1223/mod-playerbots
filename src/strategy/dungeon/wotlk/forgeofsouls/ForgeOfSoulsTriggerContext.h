#ifndef _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "ForgeOfSoulsTriggers.h"

class WotlkDungeonFosTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonFosTriggerContext()
    {
        creators["bronjahm position"] = &WotlkDungeonFosTriggerContext::bronjahm_position;
        creators["move from bronjahm"] = &WotlkDungeonFosTriggerContext::move_from_bronjahm;
        creators["switch to soul fragment"] = &WotlkDungeonFosTriggerContext::switch_to_soul_fragment;
    }

private:
    static Trigger* move_from_bronjahm(PlayerbotAI* ai) { return new MoveFromBronjahmTrigger(ai); }
    static Trigger* switch_to_soul_fragment(PlayerbotAI* ai) { return new SwitchToSoulFragment(ai); }
    static Trigger* bronjahm_position(PlayerbotAI* ai) { return new BronjahmPositionTrigger(ai); }
};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
