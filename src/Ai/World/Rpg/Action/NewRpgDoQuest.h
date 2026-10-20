/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGDOQUEST_H
#define PLAYERBOTS_NEWRPGDOQUEST_H

#include "AttackAction.h"
#include "NewRpgBaseAction.h"
#include "NewRpgInfo.h"
#include "PlayerbotAI.h"
#include "QuestValues.h"

class Creature;
class GameObject;
class Item;
class WorldObject;
struct QuestStatusData;

constexpr float NEW_RPG_ENGAGE_RANGE = 30.0f;

class StartRpgDoQuestAction : public Action
{
public:
    StartRpgDoQuestAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do quest") {}

    bool Execute(Event event) override;
};

class NewRpgAttackQuestTargetAction : public AttackAction
{
public:
    NewRpgAttackQuestTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "new rpg attack quest target") {}

    bool Execute(Event event) override;
    Unit* GetTarget() override;
};

class NewRpgDoQuestAction : public NewRpgBaseAction
{
public:
    NewRpgDoQuestAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do quest") {}
    bool Execute(Event event) override;

    // Warm the static caches at load: IsCastQuest runs a blocking query.
    static void BuildCaches();

protected:
    bool DoIncompleteQuest(NewRpgInfo::DoQuest& data);
    bool DoCompletedQuest(NewRpgInfo::DoQuest& data);

    // nullptr once the quest has left the log.
    QuestStatusData const* GetQuestStatus(uint32 questId) const;
    bool IsObjectiveCompleted(NewRpgInfo::DoQuest const& data) const;
    bool HasObjectiveProgress(NewRpgInfo::DoQuest const& data) const;
    void ResetObjectiveSpawn(NewRpgInfo::DoQuest& data);
    void ClearTarget(NewRpgInfo::DoQuest& data);

    bool SelectObjectiveSpawn(NewRpgInfo::DoQuest& data);
    bool SelectObjectivePoi(NewRpgInfo::DoQuest& data);
    bool SelectQuestEnder(NewRpgInfo::DoQuest& data);

    bool ScanForObjectiveTarget(NewRpgInfo::DoQuest& data, float maxDist);
    WorldObject* ScanEntryObjectives(NewRpgInfo::DoQuest const& data, float scanRange) const;
    WorldObject* ScanItemObjective(NewRpgInfo::DoQuest const& data, float scanRange) const;
    bool EngageTarget(NewRpgInfo::DoQuest& data);
    bool EngageCreature(NewRpgInfo::DoQuest& data, Creature* creature);
    bool EngageTalkTarget(NewRpgInfo::DoQuest& data, Creature* creature);
    bool EngageCastTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item);
    bool EngageTransformTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item);
    bool EngageGameObject(NewRpgInfo::DoQuest& data, GameObject* go);
    static uint32 GetItemUseSpell(Item* item);
    static uint32 QuestItemUseGrace(uint32 spellId);

    // The quest's own entry, not a kill or drop source standing in for it.
    bool IsIncompleteObjectiveEntry(NewRpgInfo::DoQuest const& data, uint32 entry) const;
    Item* GetCastQuestItem(NewRpgInfo::DoQuest const& data);
    // Raw quest_template_addon.SpecialFlags & 0x20: ObjectMgr ORs KILL|CAST|SPEAKTO onto
    // every creature-objective quest at load, so the runtime flag is unusable.
    static bool IsCastQuest(uint32 questId);

    bool DoCraftObjective(NewRpgInfo::DoQuest& data);

    bool DoSummonObjective(NewRpgInfo::DoQuest& data, QuestSummonAnchor const& anchor);
    WorldObject* SelectSummonAnchor(NewRpgInfo::DoQuest const& data, QuestSummonAnchor const& anchor) const;
    GameObject* FindFocusAnchor(uint32 focusId) const;
    Creature* FindCreatureAnchor(uint32 entry, float range) const;

    bool DoExploreObjective(NewRpgInfo::DoQuest& data);
    static uint32 GetQuestAreaTrigger(uint32 questId);
    void WriteOffTarget(NewRpgInfo::DoQuest& data);
    bool IsWrittenOff(NewRpgInfo::DoQuest const& data, ObjectGuid guid) const;
    bool IsValidCreatureTarget(NewRpgInfo::DoQuest const& data, Creature* creature) const;
    bool IsValidGoTarget(NewRpgInfo::DoQuest const& data, GameObject* go) const;
    bool HasNeededQuestItem(GameObject* go) const;
    bool HoldForLoot(NewRpgInfo::DoQuest& data);

    const uint32 lootHoldTime = 15 * IN_MILLISECONDS;
    const uint32 poiStayTime = 5 * MINUTE * IN_MILLISECONDS;
    // The questgiver search covers 80 yd every tick, so an ender that has not taken the quest by
    // now is not there.
    const uint32 enderWaitTime = 30 * IN_MILLISECONDS;
    const uint32 writeOffTime = poiStayTime;
    // Whole-quest budget: the spawn rotation never gives up on its own.
    const uint32 questStayTime = 30 * MINUTE * IN_MILLISECONDS;
    const uint32 scanInterval = 2 * IN_MILLISECONDS;
    const float objectiveScanRange = 150.0f;
    const uint32 targetEngageTimeout = 45 * IN_MILLISECONDS;
    const float engageRange = NEW_RPG_ENGAGE_RANGE;
    // Inside the shortest transform spell's range, outside the usual aggro pull.
    const float transformCastRange = 30.0f;
    // Past half of any spell-focus template's dist, which is the core's own casting rule.
    const float summonAnchorScanRange = 25.0f;
    const float summonCreatureAnchorRange = 5.0f;
    // The item has to land before the mob aggroes; combat pauses this engine.
    const float summonKillAnchorRange = 30.0f;
    const float summonTriggerNearRange = 12.0f;
};

#endif
