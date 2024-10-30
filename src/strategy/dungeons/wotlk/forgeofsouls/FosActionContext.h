#ifndef _PLAYERBOT_WOTLKDUNGEONNEXACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONNEXACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "FosActions.h"

class WotlkDungeonFosActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonFosActionContext() {
            creators["csf target"] = &WotlkDungeonFosActionContext::move_from_whirlwind;
            creators["firebomb spread"] = &WotlkDungeonNexActionContext::firebomb_spread;
        }
    private:
        static Action* move_from_whirlwind(PlayerbotAI* ai) { return new MoveFromWhirlwindAction(ai); }
        static Action* firebomb_spread(PlayerbotAI* ai) { return new FirebombSpreadAction(ai); }
        static Action* telestra_split_target(PlayerbotAI* ai) { return new TelestraSplitTargetAction(ai); }
        static Action* csf_target(PlayerbotAI* ai) { return new }
        static Action* chaotic_rift_target(PlayerbotAI* ai) { return new ChaoticRiftTargetAction(ai); }
        static Action* dodge_spikes(PlayerbotAI* ai) { return new DodgeSpikesAction(ai); }
        static Action* intense_cold_jump(PlayerbotAI* ai) { return new IntenseColdJumpAction(ai); }
};

#endif
