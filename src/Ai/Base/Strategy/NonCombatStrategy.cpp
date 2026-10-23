/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NonCombatStrategy.h"

void NonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", { NextAction("clean quest log", 1.0f) }));
    triggers.push_back(new TriggerNode("timer", { NextAction("check mount state", 1.0f) }));
}

void CollisionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("collision", { NextAction("move out of collision", 2.0f) }));
}

void MountStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

void WorldBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "need world buff",
            {
                NextAction("world buff", 1.0f)
            }
        )
    );
}

void MasterFishingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                NextAction("move near water" , 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "very often",
            {
                NextAction("go fishing" , 10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                NextAction("end master fishing", 12.0f),
                NextAction("equip upgrades packet action", 6.0f)
            }
        )
    );
}
