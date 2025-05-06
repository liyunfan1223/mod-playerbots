/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANGETRIGGERS_H
#define _PLAYERBOT_RANGETRIGGERS_H

#include "PlayerbotAIConfig.h"
#include "Trigger.h"

class PlayerbotAI;

class EnemyTooCloseForSpellTrigger : public Trigger
{
public:
    EnemyTooCloseForSpellTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy too close for spell") {}

    bool IsActive() override;
};

class EnemyTooCloseForShootTrigger : public Trigger
{
public:
    EnemyTooCloseForShootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy too close for shoot") {}

    bool IsActive() override;
};

class EnemyTooCloseForAutoShotTrigger : public Trigger
{
public:
    EnemyTooCloseForAutoShotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy too close for auto shot") {}

    bool IsActive() override;
};

class EnemyTooCloseForMeleeTrigger : public Trigger
{
public:
    EnemyTooCloseForMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy too close for melee", 5) {}

    bool IsActive() override;
};

class EnemyIsCloseTrigger : public Trigger
{
public:
    EnemyIsCloseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy is close") {}

    bool IsActive() override;
};

class EnemyWithinMeleeTrigger : public Trigger
{
public:
    EnemyWithinMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "enemy within melee") {}

    bool IsActive() override;
};

class OutOfRangeTrigger : public Trigger
{
public:
    OutOfRangeTrigger(PlayerbotAI* botAI, std::string const name, float distance)
        : Trigger(botAI, name), distance(distance)
    {
    }

    bool IsActive() override;
    std::string const GetTargetName() override { return "current target"; }

protected:
    float distance;
};

class EnemyOutOfMeleeTrigger : public OutOfRangeTrigger
{
public:
    EnemyOutOfMeleeTrigger(PlayerbotAI* botAI)
        : OutOfRangeTrigger(botAI, "enemy out of melee range", sPlayerbotAIConfig->meleeDistance)
    {
    }

    // bool IsActive() override;
};

class EnemyOutOfSpellRangeTrigger : public OutOfRangeTrigger
{
public:
    EnemyOutOfSpellRangeTrigger(PlayerbotAI* botAI);
};

class PartyMemberToHealOutOfSpellRangeTrigger : public OutOfRangeTrigger
{
public:
    PartyMemberToHealOutOfSpellRangeTrigger(PlayerbotAI* botAI);

    bool IsActive() override;
    std::string const GetTargetName() override { return "party member to heal"; }
};

class FarFromMasterTrigger : public Trigger
{
public:
    FarFromMasterTrigger(PlayerbotAI* botAI, std::string const name = "far from master", float distance = 12.0f,
                         int32 checkInterval = 50)
        : Trigger(botAI, name, checkInterval), distance(distance)
    {
    }

    bool IsActive() override;

private:
    float distance;
};

class OutOfReactRangeTrigger : public FarFromMasterTrigger
{
public:
    OutOfReactRangeTrigger(PlayerbotAI* botAI) : FarFromMasterTrigger(botAI, "out of react range", 50.0f, 5) {}
};

#endif
