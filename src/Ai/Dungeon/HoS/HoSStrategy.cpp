/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoSStrategy.h"
#include "HoSMultipliers.h"

void WotlkDungeonHoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Maiden of Grief
    // TODO: Jump into damage during shock of sorrow?

    // Krystallus
    // TODO: I think bots need to dismiss pets on this, or they nuke players they are standing close to
    triggers.push_back(new TriggerNode("ground slam",
        { NextAction("shatter spread", ACTION_RAID + 5) }));

    // Tribunal of Ages
    // Seems fine, maybe add focus targeting strat if needed on heroic.
    // Main issue is dps will immediately rambo in and sometimes die before tank gets aggro,
    // this is mostly an issue with the bot AI as they do it on every fight

    // Sjonnir The Ironshaper
    // Possibly tank in place in the middle of the room, assign a dps to adds?
    triggers.push_back(new TriggerNode("lightning ring",
        { NextAction("avoid lightning ring", ACTION_RAID + 5) }));
}

void WotlkDungeonHoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KrystallusMultiplier(botAI));
    multipliers.push_back(new SjonnirMultiplier(botAI));
}
