/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldPacketHandlerStrategy.h"

void WorldPacketHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassThroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("group invite", { NextAction("accept invitation", relevance) }));
    triggers.push_back(
        new TriggerNode("uninvite", { NextAction("uninvite", relevance) }));
    triggers.push_back(
        new TriggerNode("uninvite guid", { NextAction("uninvite", relevance) }));
    triggers.push_back(
        new TriggerNode("group set leader", { NextAction("reset botAI", relevance) }));
    triggers.push_back(new TriggerNode(
        "not enough money", { NextAction("tell not enough money", relevance) }));
    triggers.push_back(
        new TriggerNode("not enough reputation",
                        { NextAction("tell not enough reputation", relevance) }));
    triggers.push_back(
        new TriggerNode("cannot equip", { NextAction("tell cannot equip", relevance) }));
    triggers.push_back(
        new TriggerNode("use game object", { NextAction("add loot", relevance),
                                                             NextAction("use meeting stone", relevance) }));
    triggers.push_back(
        new TriggerNode("gossip hello", { NextAction("trainer", relevance) }));
    triggers.push_back(new TriggerNode("activate taxi", { NextAction("remember taxi", relevance),
                                                                          NextAction("taxi", relevance) }));
    triggers.push_back(new TriggerNode("taxi done", { NextAction("taxi", relevance) }));
    triggers.push_back(new TriggerNode("trade status", { NextAction("accept trade", relevance), NextAction("equip upgrades packet action", relevance) }));
    triggers.push_back(new TriggerNode("trade status extended", { NextAction("trade status extended", relevance) }));
    triggers.push_back(new TriggerNode("area trigger", { NextAction("reach area trigger", relevance) }));
    triggers.push_back(new TriggerNode("within area trigger", { NextAction("area trigger", relevance) }));
    triggers.push_back(new TriggerNode("loot response", { NextAction("store loot", relevance) }));
    triggers.push_back(new TriggerNode("item push result", { NextAction("unlock items", relevance),
                                                                                NextAction("open items", relevance),
                                                                                NextAction("query item usage", relevance),
                                                                                NextAction("equip upgrades packet action", relevance) }));
    triggers.push_back(new TriggerNode("item push result", { NextAction("quest item push result", relevance) }));
    triggers.push_back(new TriggerNode("loot roll won", { NextAction("equip upgrades packet action", relevance) }));
    triggers.push_back(new TriggerNode("ready check finished", { NextAction("ready check finished", relevance) }));
    // triggers.push_back(new TriggerNode("often", { NextAction("security check", relevance), NextAction("check mail", relevance) }));
    triggers.push_back(new TriggerNode("guild invite", { NextAction("guild accept", relevance) }));
    triggers.push_back(new TriggerNode("petition offer", { NextAction("petition sign", relevance) }));
    triggers.push_back(new TriggerNode("lfg proposal", { NextAction("lfg accept", relevance) }));
    triggers.push_back(new TriggerNode("lfg proposal active", { NextAction("lfg accept", relevance) }));
    triggers.push_back(new TriggerNode("arena team invite", { NextAction("arena team accept", relevance) }));
    //triggers.push_back(new TriggerNode("no non bot players around", { NextAction("delay", relevance) }));
    triggers.push_back(new TriggerNode("bg status", { NextAction("bg status", relevance) }));
    triggers.push_back(new TriggerNode("xpgain", { NextAction("xp gain", relevance) }));
    triggers.push_back(
        new TriggerNode("levelup", { NextAction("auto maintenance on levelup", relevance + 3) }));
    // triggers.push_back(new TriggerNode("group destroyed", { NextAction("reset botAI",
    // relevance) }));
    triggers.push_back(new TriggerNode("group list", { NextAction("reset botAI", relevance) }));
    triggers.push_back(new TriggerNode("see spell", { NextAction("see spell", relevance) }));
    triggers.push_back(new TriggerNode("release spirit", { NextAction("release", relevance) }));
    triggers.push_back(new TriggerNode("revive from corpse", { NextAction("revive from corpse", relevance) }));
    triggers.push_back(new TriggerNode("master loot roll", { NextAction("master loot roll", relevance) }));

    // quest ?
    //triggers.push_back(new TriggerNode("quest confirm", { NextAction("quest confirm", relevance) }));
    triggers.push_back(new TriggerNode("questgiver quest details", { NextAction("turn in query quest", relevance) }));

    // loot roll
    triggers.push_back(new TriggerNode("very often", { NextAction("loot roll", relevance) }));
}

WorldPacketHandlerStrategy::WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassThroughStrategy(botAI)
{
    supported.push_back("loot roll");
    supported.push_back("check mount state");
    supported.push_back("party command");
    supported.push_back("ready check");
    supported.push_back("uninvite");
    supported.push_back("lfg role check");
    supported.push_back("lfg teleport");
    supported.push_back("random bot update");
    supported.push_back("inventory change failure");
    supported.push_back("bg status");

    // quests
    supported.push_back("quest update add kill");
    // supported.push_back("quest update add item");
    supported.push_back("quest update failed");
    supported.push_back("quest update failed timer");
    supported.push_back("quest update complete");
    supported.push_back("confirm quest");
}

void ReadyCheckStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("timer", { NextAction("ready check", relevance) }));
}
