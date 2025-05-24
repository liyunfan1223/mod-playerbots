#ifndef _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "ForgeOfSoulsTriggers.h"

class WotlkDungeonFoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonFoSTriggerContext()
    {
        creators["bronjahm position"] = &WotlkDungeonFoSTriggerContext::bronjahm_position;
        creators["move from bronjahm"] = &WotlkDungeonFoSTriggerContext::move_from_bronjahm;
        creators["switch to soul fragment"] = &WotlkDungeonFoSTriggerContext::switch_to_soul_fragment;
        creators["devourer of souls"] = &WotlkDungeonFoSTriggerContext::devourer_of_souls;
    }

private:
    static Trigger* move_from_bronjahm(PlayerbotAI* ai) { return new MoveFromBronjahmTrigger(ai); }
    static Trigger* switch_to_soul_fragment(PlayerbotAI* ai) { return new SwitchToSoulFragment(ai); }
    static Trigger* bronjahm_position(PlayerbotAI* ai) { return new BronjahmPositionTrigger(ai); }
    static Trigger* devourer_of_souls(PlayerbotAI* ai) { return new DevourerOfSoulsTrigger(ai); }
};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
