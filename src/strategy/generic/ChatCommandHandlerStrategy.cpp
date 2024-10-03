/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandlerStrategy.h"

#include "Playerbots.h"

class ChatCommandActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ChatCommandActionNodeFactoryInternal() { creators["tank attack chat shortcut"] = &tank_attack_chat_shortcut; }

private:
    static ActionNode* tank_attack_chat_shortcut(PlayerbotAI* botAI)
    {
        return new ActionNode("tank attack chat shortcut",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("attack my target", 100.0f), nullptr));
    }
};

void ChatCommandHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("rep", NextAction::array(0, new NextAction("reputation", relevance), nullptr)));
    triggers.push_back(new TriggerNode("q", NextAction::array(0, new NextAction("query quest", relevance),
                                                              new NextAction("query item usage", relevance), nullptr)));
    triggers.push_back(new TriggerNode("add all loot", NextAction::array(0, new NextAction("add all loot", relevance),
                                                                         new NextAction("loot", relevance), nullptr)));
    triggers.push_back(new TriggerNode("u", NextAction::array(0, new NextAction("use", relevance), nullptr)));
    triggers.push_back(new TriggerNode("c", NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("items", NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    triggers.push_back(new TriggerNode("inv", NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    triggers.push_back(new TriggerNode("e", NextAction::array(0, new NextAction("equip", relevance), nullptr)));
    triggers.push_back(new TriggerNode("ue", NextAction::array(0, new NextAction("unequip", relevance), nullptr)));
    triggers.push_back(new TriggerNode("t", NextAction::array(0, new NextAction("trade", relevance), nullptr)));
    triggers.push_back(new TriggerNode("nt", NextAction::array(0, new NextAction("trade", relevance), nullptr)));
    triggers.push_back(new TriggerNode("s", NextAction::array(0, new NextAction("sell", relevance), nullptr)));
    triggers.push_back(new TriggerNode("b", NextAction::array(0, new NextAction("buy", relevance), nullptr)));
    triggers.push_back(new TriggerNode("r", NextAction::array(0, new NextAction("reward", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("attack", NextAction::array(0, new NextAction("attack my target", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("accept", NextAction::array(0, new NextAction("accept quest", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("follow", NextAction::array(0, new NextAction("follow chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("stay", NextAction::array(0, new NextAction("stay chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("move from group", NextAction::array(0, new NextAction("move from group chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("flee", NextAction::array(0, new NextAction("flee chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode(
        "tank attack", NextAction::array(0, new NextAction("tank attack chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("grind", NextAction::array(0, new NextAction("grind chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("talk", NextAction::array(0, new NextAction("gossip hello", relevance),
                                                  new NextAction("talk to quest giver", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("enter vehicle", NextAction::array(0, new NextAction("enter vehicle", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("leave vehicle", NextAction::array(0, new NextAction("leave vehicle", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("cast", NextAction::array(0, new NextAction("cast custom spell", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("castnc", NextAction::array(0, new NextAction("cast custom nc spell", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("revive", NextAction::array(0, new NextAction("spirit healer", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("runaway", NextAction::array(0, new NextAction("runaway chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("warning", NextAction::array(0, new NextAction("runaway chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("max dps", NextAction::array(0, new NextAction("max dps chat shortcut", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("attackers", NextAction::array(0, new NextAction("tell attackers", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("target", NextAction::array(0, new NextAction("tell target", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("ready", NextAction::array(0, new NextAction("ready check", relevance), nullptr)));
    triggers.push_back(
        new TriggerNode("naxx", NextAction::array(0, new NextAction("naxx chat shortcut", relevance), NULL)));
    triggers.push_back(
        new TriggerNode("bwl", NextAction::array(0, new NextAction("bwl chat shortcut", relevance), NULL)));
    triggers.push_back(
        new TriggerNode("dps", NextAction::array(0, new NextAction("tell estimated dps", relevance), NULL)));
    triggers.push_back(
        new TriggerNode("disperse", NextAction::array(0, new NextAction("disperse set", relevance), NULL)));
    triggers.push_back(
	    new TriggerNode("open items", NextAction::array(0, new NextAction("open items", relevance), nullptr)));
}

ChatCommandHandlerStrategy::ChatCommandHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    actionNodeFactories.Add(new ChatCommandActionNodeFactoryInternal());

    supported.push_back("quests");
    supported.push_back("stats");
    supported.push_back("leave");
    supported.push_back("reputation");
    supported.push_back("log");
    supported.push_back("los");
    supported.push_back("aura");
    supported.push_back("drop");
    supported.push_back("share");
    supported.push_back("ll");
    supported.push_back("ss");
    supported.push_back("release");
    supported.push_back("teleport");
    supported.push_back("taxi");
    supported.push_back("repair");
    supported.push_back("talents");
    supported.push_back("spells");
    supported.push_back("co");
    supported.push_back("nc");
    supported.push_back("de");
    supported.push_back("trainer");
    supported.push_back("maintenance");
    supported.push_back("remove glyph");
    supported.push_back("autogear");
    supported.push_back("equip upgrade");
    supported.push_back("chat");
    supported.push_back("home");
    supported.push_back("destroy");
    supported.push_back("reset botAI");
    supported.push_back("emote");
    supported.push_back("buff");
    supported.push_back("help");
    supported.push_back("gb");
    supported.push_back("bank");
    supported.push_back("invite");
    supported.push_back("spell");
    supported.push_back("rti");
    supported.push_back("position");
    supported.push_back("summon");
    supported.push_back("who");
    supported.push_back("save mana");
    supported.push_back("formation");
    supported.push_back("stance");
    supported.push_back("sendmail");
    supported.push_back("mail");
    supported.push_back("outfit");
    supported.push_back("go");
    supported.push_back("debug");
    supported.push_back("cdebug");
    supported.push_back("cs");
    supported.push_back("wts");
    supported.push_back("hire");
    supported.push_back("craft");
    supported.push_back("flag");
    supported.push_back("range");
    supported.push_back("ra");
    supported.push_back("give leader");
    supported.push_back("cheat");
    supported.push_back("ginvite");
    supported.push_back("guild promote");
    supported.push_back("guild demote");
    supported.push_back("guild remove");
    supported.push_back("guild leave");
    supported.push_back("rtsc");
    supported.push_back("drink");
    supported.push_back("calc");
    supported.push_back("open items");
}
