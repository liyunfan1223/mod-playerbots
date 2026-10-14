/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AKStrategy.h"
#include "AKMultipliers.h"

void WotlkDungeonOKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Elder Nadox
    triggers.push_back(new TriggerNode("nadox guardian",
        { NextAction("attack nadox guardian", ACTION_RAID + 5) }));

    // Prince Taldaram
    // Flame Orb spawns in melee, doesn't have a clear direction until it starts moving.
    // Maybe not worth trying to avoid and just heal through. Only consideration is not to have ranged
    // players anywhere near melee when it spawns

    // Jedoga Shadowseeker
    triggers.push_back(new TriggerNode("jedoga volunteer",
        { NextAction("attack jedoga volunteer", ACTION_RAID + 5) }));

    // Herald Volazj
    // Trash mobs before him have a big telegraphed shadow crash spell,
    // this can be avoided and is intended to be dodged
    triggers.push_back(new TriggerNode("shadow crash",
        { NextAction("avoid shadow crash", ACTION_MOVE + 5) }));
    // Volazj is not implemented properly in AC, insanity phase does nothing.

    // Amanitar (Heroic Only)
    // TODO: once I get to heroics
}

void WotlkDungeonOKStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ElderNadoxMultiplier(botAI));
    multipliers.push_back(new JedogaShadowseekerMultiplier(botAI));
    multipliers.push_back(new ForgottenOneMultiplier(botAI));
}
