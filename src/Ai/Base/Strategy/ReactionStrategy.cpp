/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from cmangos/playerbots (ReactionStrategy) with modifications.
 */

#include "ReactionStrategy.h"

#include "Playerbots.h"

void ReactionStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    // Upstream cmangos switches engines from here (combat start/end, death, resurrect). This
    // port keeps every engine switch on the existing paths (AttackAction, DropTargetAction,
    // PlayerbotAI::DoNextAction) because entering the combat engine without a current target
    // gets bounced straight back by "invalid target" -> "drop target". The reaction only wakes
    // the main AI when the group's attackers first appear, so a bot that is eating, drinking
    // or sitting out a long delay picks a target through "dps assist"/"tank assist" right away.
    triggers.push_back(
        new TriggerNode(
            "combat start",
            {
                NextAction("wake on combat start", ACTION_PASSTHROUGH)
            }
        )
    );
}
