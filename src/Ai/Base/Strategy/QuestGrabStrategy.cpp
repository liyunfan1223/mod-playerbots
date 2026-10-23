/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestGrabStrategy.h"

void QuestGrabStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Tried in order; one that finds nothing falls through to the next, then back to "follow".
    triggers.push_back(new TriggerNode("timer", {
        NextAction("quest grab giver", ACTION_NORMAL + 1.4f),
        NextAction("quest grab use item", ACTION_NORMAL + 1.3f),
        NextAction("quest grab create item", ACTION_NORMAL + 1.2f),
        NextAction("quest grab object", ACTION_NORMAL + 1.1f)
    }));
}
