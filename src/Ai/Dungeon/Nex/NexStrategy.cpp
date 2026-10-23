/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NexStrategy.h"
#include "NexMultipliers.h"

void WotlkDungeonNexStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Horde Commander (Alliance N)/Commander Kolurg (Alliance H)
    // or
    // Alliance Commander (Horde N)/Commander Stoutbeard (Horde H)
    triggers.push_back(new TriggerNode("faction commander whirlwind",
        { NextAction("move from whirlwind", ACTION_MOVE + 5) }));
    // TODO: Handle fear? (tremor totems)

    // Grand Magus Telestra
    triggers.push_back(new TriggerNode("telestra firebomb",
        { NextAction("firebomb spread", ACTION_MOVE + 5) }));
    triggers.push_back(new TriggerNode("telestra split phase",
        { NextAction("telestra split target", ACTION_RAID + 1) }));
    // TODO: Add priority interrupt on the frost split's Blizzard casts

    // Anomalus
    triggers.push_back(new TriggerNode("chaotic rift",
        { NextAction("chaotic rift target", ACTION_RAID + 1) }));

    // Ormorok the Tree-Shaper
    // Tank trigger to stack inside boss. Can also add return action to prevent boss repositioning
    // if it becomes too much of a problem. He usually dies before he's up against a wall though
    triggers.push_back(new TriggerNode("ormorok spikes",
        { NextAction("dodge spikes", ACTION_MOVE + 5) }));
    // Non-tank trigger to stack. Avoiding the spikes at range is.. harder than it seems.
    // TODO: This turns hunters into melee marshmallows, have not come up with a better solution yet
    triggers.push_back(new TriggerNode("ormorok stack",
        { NextAction("dodge spikes", ACTION_MOVE + 5) }));
    // TODO: Add handling for spell reflect... best to spam low level/weak spells but don't want
    // to hardcode spells per class, might be difficult to dynamically generate this.
    // Will revisit if I find my altbots killing themselves in heroic, just heal through it for now

    // Keristrasza
    triggers.push_back(new TriggerNode("intense cold",
        { NextAction("intense cold jump", ACTION_MOVE + 5) }));
    // Flank dragon positioning
    triggers.push_back(new TriggerNode("keristrasza positioning",
        { NextAction("rear flank", ACTION_MOVE + 4) }));
    // TODO: Add frost resist aura for paladins?
}

void WotlkDungeonNexStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FactionCommanderMultiplier(botAI));
    multipliers.push_back(new TelestraMultiplier(botAI));
    multipliers.push_back(new AnomalusMultiplier(botAI));
    multipliers.push_back(new OrmorokMultiplier(botAI));
}
