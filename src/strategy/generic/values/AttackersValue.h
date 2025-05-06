/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ATTACKERSVALUE_H
#define _PLAYERBOT_ATTACKERSVALUE_H

#include "PlayerbotAIConfig.h"
#include "Value.h"

class Group;
class Player;
class PlayerbotAI;
class Unit;

class AttackersValue : public ObjectGuidListCalculatedValue
{
public:
    AttackersValue(PlayerbotAI* botAI) : ObjectGuidListCalculatedValue(botAI, "attackers", 1 * 1000) {}

    GuidVector Calculate();
    static bool IsPossibleTarget(Unit* attacker, Player* bot, float range = sPlayerbotAIConfig->sightDistance);
    static bool IsValidTarget(Unit* attacker, Player* bot);

private:
    void AddAttackersOf(Group* group, std::unordered_set<Unit*>& targets);
    void AddAttackersOf(Player* player, std::unordered_set<Unit*>& targets);
    void RemoveNonThreating(std::unordered_set<Unit*>& targets);
    bool hasRealThreat(Unit* attacker);
};

class PossibleAddsValue : public BoolCalculatedValue
{
public:
    PossibleAddsValue(PlayerbotAI* botAI, std::string const name = "possible adds") : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

class PrioritizedTargetsValue : public ManualSetValue<GuidVector>
{
public:
    PrioritizedTargetsValue(PlayerbotAI* botAI, std::string const name = "prioritized targets")
        : ManualSetValue(botAI, GuidVector(), name)
    {
    }
};

#endif
