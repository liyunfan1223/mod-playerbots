/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXACTIONCONTEXT_H
#define PLAYERBOTS_NEXACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "NexActions.h"

class WotlkDungeonNexActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonNexActionContext() {
            creators["move from whirlwind"] = &WotlkDungeonNexActionContext::move_from_whirlwind;
            creators["firebomb spread"] = &WotlkDungeonNexActionContext::firebomb_spread;
            creators["telestra split target"] = &WotlkDungeonNexActionContext::telestra_split_target;
            creators["chaotic rift target"] = &WotlkDungeonNexActionContext::chaotic_rift_target;
            creators["dodge spikes"] = &WotlkDungeonNexActionContext::dodge_spikes;
            creators["intense cold jump"] = &WotlkDungeonNexActionContext::intense_cold_jump;
        }
    private:
        static Action* move_from_whirlwind(PlayerbotAI* botAI) { return new MoveFromWhirlwindAction(botAI); }
        static Action* firebomb_spread(PlayerbotAI* botAI) { return new FirebombSpreadAction(botAI); }
        static Action* telestra_split_target(PlayerbotAI* botAI) { return new TelestraSplitTargetAction(botAI); }
        static Action* chaotic_rift_target(PlayerbotAI* botAI) { return new ChaoticRiftTargetAction(botAI); }
        static Action* dodge_spikes(PlayerbotAI* botAI) { return new DodgeSpikesAction(botAI); }
        static Action* intense_cold_jump(PlayerbotAI* botAI) { return new IntenseColdJumpAction(botAI); }
};

#endif
