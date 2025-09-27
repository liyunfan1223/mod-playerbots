/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RITUALACTIONS_H
#define _PLAYERBOT_RITUALACTIONS_H

#include "Action.h"
#include "Trigger.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "MovementActions.h"

class PlayerbotAI;

// Common Actions for Rituals

class MoveAwayFromSpawnAction : public MovementAction
{
public:
    MoveAwayFromSpawnAction(PlayerbotAI* botAI) : MovementAction(botAI, "move away from spawn") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Common Triggers for Rituals

class InDungeonOrRaidTrigger : public Trigger
{
public:
    InDungeonOrRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "in dungeon or raid") {}
    bool IsActive() override;
};

// Universal interaction actions for all classes
class InteractWithSoulPortalAction : public MovementAction
{
public:
    InteractWithSoulPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "interact with soul portal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InteractWithRefreshmentPortalAction : public MovementAction
{
public:
    InteractWithRefreshmentPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "interact with refreshment portal") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InteractWithRefreshmentTableAction : public MovementAction
{
public:
    InteractWithRefreshmentTableAction(PlayerbotAI* botAI) : MovementAction(botAI, "interact with refreshment table") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class InteractWithSoulwellAction : public MovementAction
{
public:
    InteractWithSoulwellAction(PlayerbotAI* botAI) : MovementAction(botAI, "interact with soulwell") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Generic action to check and re-interact with conjured objects if needed
class CheckConjuredItemsAction : public Action
{
public:
    CheckConjuredItemsAction(PlayerbotAI* botAI) : Action(botAI, "check conjured items") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Universal triggers for all classes
class SoulPortalAvailableTrigger : public Trigger
{
public:
    SoulPortalAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soul portal available") {}
    bool IsActive() override;
};

class RefreshmentPortalAvailableTrigger : public Trigger
{
public:
    RefreshmentPortalAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "refreshment portal available") {}
    bool IsActive() override;
};

class RefreshmentTableAvailableTrigger : public Trigger
{
public:
    RefreshmentTableAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "refreshment table available") {}
    bool IsActive() override;
};

class NeedsConjuredItemsTrigger : public Trigger
{
public:
    NeedsConjuredItemsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "needs conjured items") {}
    bool IsActive() override;
};

class SoulwellAvailableTrigger : public Trigger
{
public:
    SoulwellAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soulwell available") {}
    bool IsActive() override;
};

// Trigger for mage ritual with delay after soul portal detection
class MageRitualWithDelayTrigger : public Trigger
{
public:
    MageRitualWithDelayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mage ritual with delay") {}
    bool IsActive() override;
};

// Trigger to detect when a bot needs to restore follow behavior after ritual completion
class NeedsFollowRestorationTrigger : public Trigger
{
public:
    NeedsFollowRestorationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "needs follow restoration") {}
    bool IsActive() override;
};

// Action to restore follow behavior after ritual completion
class RestoreFollowBehaviorAction : public Action
{
public:
    RestoreFollowBehaviorAction(PlayerbotAI* botAI) : Action(botAI, "restore follow behavior") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};


// Helper functions for ritual components
bool HasRitualComponent(Player* bot, uint32 spellId);
bool GiveRitualComponent(Player* bot, uint32 spellId);
bool CanUseRituals(Player* bot);

#endif
