#ifndef PLAYERBOTS_RITUALACTIONS_HELPERS_H
#define PLAYERBOTS_RITUALACTIONS_HELPERS_H

#include "Player.h"
class PlayerbotAI;

// Shared helpers for ritual-related actions
float GetRitualSearchRange(Player* bot);
GameObject* FindNearestByRanks(Player* bot, uint32 rank1Id, uint32 rank2Id);
void FinalizeRitualInteraction(Player* bot, PlayerbotAI* botAI, uint32 delayMs);
bool CanUseRituals(Player* bot);
inline bool IsInRitualMap(Player* bot) { return bot->GetMap()->IsDungeon() || bot->GetMap()->IsRaid() || bot->GetMap()->IsBattleground(); }

// Utility: check if bot has at least 'limit' of any item in the list
bool HasAnyItemAtLeast(Player* bot, const std::vector<uint32>& itemIds, uint32 limit);
// Utility: check if bot has less than 'limit' of all items in the list
bool HasAllItemsBelow(Player* bot, const std::vector<uint32>& itemIds, uint32 limit);

#endif
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

// Ritual System Constants
#define RITUAL_BATTLEGROUND_SEARCH_RANGE 100.0f
#define RITUAL_DUNGEON_SEARCH_RANGE 30.0f
#define RITUAL_INTERACTION_DISTANCE 5.0f

#define RITUAL_MOVEMENT_OFFSET_MIN -10
#define RITUAL_MOVEMENT_OFFSET_MAX 10

#define RITUAL_HEALTHSTONE_LIMIT 1
#define RITUAL_FOOD_DRINK_LIMIT 20

#define RITUAL_DELAY_MS 3000
#define RITUAL_MAGE_DELAY_CALLS 10

#define RITUAL_SOUL_PORTAL_MOVEMENT_ID 4000
#define RITUAL_REFRESHMENT_PORTAL_MOVEMENT_ID 5000
#define RITUAL_REFRESHMENT_TABLE_MOVEMENT_ID 6000
#define RITUAL_SOUL_WELL_MOVEMENT_ID 7000

#define RITUAL_SOUL_PORTAL_RANK_1 181622
#define RITUAL_SOUL_PORTAL_RANK_2 193168
#define RITUAL_SOUL_WELL_RANK_1 181621
#define RITUAL_SOUL_WELL_RANK_2 193169
#define RITUAL_SOUL_WELL_RANK_2_VARIANT_1 193170
#define RITUAL_SOUL_WELL_RANK_2_VARIANT_2 193171
#define RITUAL_REFRESHMENT_PORTAL_RANK_1 186811
#define RITUAL_REFRESHMENT_PORTAL_RANK_2 193062
#define RITUAL_REFRESHMENT_TABLE_RANK_1 186812
#define RITUAL_REFRESHMENT_TABLE_RANK_2 193061

#define RITUAL_MINOR_HEALTHSTONE 5512
#define RITUAL_LESSER_HEALTHSTONE 5511
#define RITUAL_MAJOR_HEALTHSTONE 9421
#define RITUAL_MINOR_HEALTHSTONE_ALT 19004
#define RITUAL_LESSER_HEALTHSTONE_ALT 19005
#define RITUAL_FEL_HEALTHSTONE 36892
#define RITUAL_DEMONIC_HEALTHSTONE 22103

#define RITUAL_CONJURED_MANA_BISCUIT 43523
#define RITUAL_CONJURED_MANA_STRUDEL 43518
#define RITUAL_CONJURED_MANA_COOKIE 43517
#define RITUAL_CONJURED_MANA_CAKE 43516
#define RITUAL_CONJURED_MANA_PIE 43515
#define RITUAL_CONJURED_MANA_BREAD 43514
#define RITUAL_CONJURED_MANA_MUFFIN 43513
#define RITUAL_CONJURED_MANA_DONUT 43512
#define RITUAL_CONJURED_MANA_BAGEL 43511
#define RITUAL_CONJURED_MANA_PRETZEL 43510
#define RITUAL_CONJURED_MANA_WATER 43519
#define RITUAL_CONJURED_MANA_JUICE 43520
#define RITUAL_CONJURED_MANA_TEA 43521
#define RITUAL_CONJURED_MANA_COFFEE 43522

#define RITUAL_OF_REFRESHMENT_RANK_1 43987
#define RITUAL_OF_REFRESHMENT_RANK_2 58659

#define RITUAL_WARLOCK_NEARBY_DISTANCE 100.0f
#define RITUAL_NEARBY_PLAYER_SEARCH_RANGE 150.0f

#define RITUAL_COMPONENT_ITEM_1 29893
#define RITUAL_COMPONENT_ITEM_2 58887
#define RITUAL_COMPONENT_ITEM_3 6265
#define RITUAL_COMPONENT_ITEM_4 17020
#define RITUAL_LIGHT_FEATHER 17056

// Refreshment Table GameObject IDs
#define RITUAL_REFRESHMENT_TABLE_ID_1 181628
#define RITUAL_REFRESHMENT_TABLE_ID_2 181629
#define RITUAL_REFRESHMENT_TABLE_ID_3 181630

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
class CheckConjuredItemsAction : public MovementAction
{
public:
    CheckConjuredItemsAction(PlayerbotAI* botAI) : MovementAction(botAI, "check conjured items") {}
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
