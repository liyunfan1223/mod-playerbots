/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_QUESTVALUES_H
#define _PLAYERBOT_QUESTVALUES_H

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class Player;
class PlayerbotAI;

struct CreatureData;
struct GameObjectData;

enum class QuestRelationFlag : uint32
{
    none = 0,
    objective1 = 1,
    objective2 = 2,
    objective3 = 4,
    objective4 = 8,
    questGiver = 16,
    questTaker = 32,
    maxQuestRelationFlag = 64
};

//                     questId, QuestRelationFlag
typedef std::unordered_map<uint32, uint32> questRelationMap;
//                     entry
typedef std::unordered_map<int32, questRelationMap> entryQuestRelationMap;

//                      entry
typedef std::unordered_map<int32, std::vector<GuidPosition>> questEntryGuidps;

//                      QuestRelationFlag
typedef std::unordered_map<uint32, questEntryGuidps> questRelationGuidps;

//                      questId
typedef std::unordered_map<uint32, questRelationGuidps> questGuidpMap;

//                      questId
typedef std::unordered_map<uint32, std::vector<GuidPosition>> questGiverMap;

// Returns the quest relation Flags for all entries and quests
class EntryQuestRelationMapValue : public SingleCalculatedValue<entryQuestRelationMap>
{
public:
    EntryQuestRelationMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "entry quest relation map") {}

    entryQuestRelationMap Calculate() override;
};

// Generic quest object finder
class FindQuestObjectData
{
public:
    FindQuestObjectData() { GetObjectiveEntries(); }

    void GetObjectiveEntries();
    void operator()(CreatureData const& creatureData);
    void operator()(GameObjectData const& gameobjectData);
    questGuidpMap GetResult() const { return data; };

private:
    std::unordered_map<int32, std::vector<std::pair<uint32, QuestRelationFlag>>> entryMap;
    std::unordered_map<uint32, std::vector<std::pair<uint32, QuestRelationFlag>>> itemMap;

    entryQuestRelationMap relationMap;

    questGuidpMap data;
};

// All objects to start, do or finish a quest.
class QuestGuidpMapValue : public SingleCalculatedValue<questGuidpMap>
{
public:
    QuestGuidpMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest guidp map") {}

    questGuidpMap Calculate() override;
};

// All questgivers and their quests that are Useful for a specific level
class QuestGiversValue : public SingleCalculatedValue<questGiverMap>, public Qualified
{
public:
    QuestGiversValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest givers") {}

    questGiverMap Calculate() override;
};

// All questgivers that have a quest for the bot.
class ActiveQuestGiversValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestGiversValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest givers", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// All quest takers that the bot has a quest for.
class ActiveQuestTakersValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestTakersValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest takers", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// All objectives that the bot still has to complete.
class ActiveQuestObjectivesValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestObjectivesValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest objectives", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// Free quest log slots
class FreeQuestLogSlotValue : public Uint8CalculatedValue
{
public:
    FreeQuestLogSlotValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI, "free quest log slots", 2) {}

    uint8 Calculate() override;
};

// Dialog status npc
class DialogStatusValue : public Uint32CalculatedValue, public Qualified
{
public:
    DialogStatusValue(PlayerbotAI* botAI, std::string const name = "dialog status")
        : Uint32CalculatedValue(botAI, name, 2)
    {
    }

    static uint32 getDialogStatus(Player* bot, int32 questgiver, uint32 questId = 0);

    uint32 Calculate() override;
};

// Dialog status npc quest
class DialogStatusQuestValue : public DialogStatusValue
{
public:
    DialogStatusQuestValue(PlayerbotAI* botAI) : DialogStatusValue(botAI, "dialog status quest") {}

    uint32 Calculate() override;
};

// Can accept quest from npc
class CanAcceptQuestValue : public BoolCalculatedValue, public Qualified
{
public:
    CanAcceptQuestValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can accept quest npc") {}

    bool Calculate() override;
};

// Can accept low level quest from npc
class CanAcceptQuestLowLevelValue : public BoolCalculatedValue, public Qualified
{
public:
    CanAcceptQuestLowLevelValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can accept quest low level npc") {}

    bool Calculate() override;
};

// Can hand in quest to npc
class CanTurnInQuestValue : public BoolCalculatedValue, public Qualified
{
public:
    CanTurnInQuestValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can turn in quest npc") {}

    bool Calculate() override;
};

#endif
