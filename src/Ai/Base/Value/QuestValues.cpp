/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestValues.h"
#include "DatabaseEnv.h"
#include "GameObjectData.h"
#include "LootValues.h"
#include "MapMgr.h"
#include "Playerbots.h"
#include "SharedValueContext.h"
#include "SmartScriptMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// What kind of a relation does this entry have with this quest.
entryQuestRelationMap EntryQuestRelationMapValue::Calculate()
{
    entryQuestRelationMap rMap;

    for (auto relation : *sObjectMgr->GetCreatureQuestRelationMap())
        rMap[relation.first][relation.second] |= (int)QuestRelationFlag::questGiver;

    for (auto relation : *sObjectMgr->GetCreatureQuestInvolvedRelationMap())
        rMap[relation.first][relation.second] |= (int)QuestRelationFlag::questTaker;

    for (auto relation : *sObjectMgr->GetGOQuestRelationMap())
        rMap[-(int32)relation.first][relation.second] |= (int)QuestRelationFlag::questGiver;

    for (auto relation : *sObjectMgr->GetGOQuestInvolvedRelationMap())
        rMap[-(int32)relation.first][relation.second] |= (int)QuestRelationFlag::questGiver;

    // Quest objectives
    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();

    for (auto& questItr : questMap)
    {
        uint32 questId = questItr.first;
        Quest* quest = questItr.second;

        for (uint32 objective = 0; objective < QUEST_OBJECTIVES_COUNT; objective++)
        {
            uint32 relationFlag = 1 << objective;

            // Kill objective
            if (quest->RequiredNpcOrGo[objective])
                rMap[quest->RequiredNpcOrGo[objective]][questId] |= relationFlag;

            // Loot objective
            if (quest->RequiredItemId[objective])
            {
                for (auto& entry : GAI_VALUE2(std::vector<int32>, "item drop list", quest->RequiredItemId[objective]))
                    rMap[entry][questId] |= relationFlag;
            }
        }
    }

    return rMap;
}

// Get all the objective entries for a specific quest.
void FindQuestObjectData::GetObjectiveEntries()
{
    relationMap = GAI_VALUE(entryQuestRelationMap, "entry quest relation");
}

// Every entry a spawn point can hold: the base from `creature`, plus up to two
// `creature_multispawn` alternates rolled between on respawn. 0 = no alternate.
static std::array<uint32, 3> SpawnEntries(CreatureData const& creData)
{
    return { creData.id, creData.id2, creData.id3 };
}

// Data worker. Checks for a specific creature what quest they are needed for and puts them in the proper place in the
// quest map.
void FindQuestObjectData::operator()(CreatureData const& creData)
{
    // The GuidPosition keeps the spawn's base entry, which can disagree with the alternate it
    // is filed under. It resolves by spawnId, and consumers read the map key.
    for (uint32 entry : SpawnEntries(creData))
    {
        if (!entry)
            continue;

        auto relations = relationMap.find(int32(entry));
        if (relations == relationMap.end())
            continue;

        for (auto& relation : relations->second)
        {
            uint32 questId = relation.first;
            uint32 flag = relation.second;
            data[questId][flag][entry].push_back(GuidPosition(creData));
        }
    }
}

// GameObject data worker. Checks for a specific gameObject what quest they are needed for and puts them in the proper
// place in the quest map.
void FindQuestObjectData::operator()(GameObjectData const& goData)
{
    int32 entry = goData.id * -1;

    for (auto& relation : relationMap[entry])
    {
        uint32 questId = relation.first;
        uint32 flag = relation.second;
        data[questId][flag][entry].push_back(GuidPosition(goData));
    }
}

// Goes past all creatures and gameobjects and creatures the full quest guid map.
questGuidpMap QuestGuidpMapValue::Calculate()
{
    FindQuestObjectData worker;
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
        worker(itr.second);
    for (auto const& itr : sObjectMgr->GetAllGOData())
        worker(itr.second);

    return worker.GetResult();
}

// Entry keys: negative = gameobject.
static std::unordered_map<int32, std::unordered_map<uint32, std::vector<GuidPosition>>> g_entrySpawns;

static std::unordered_map<uint32, std::unordered_map<uint32, std::vector<int32>>> g_questObjectiveEntries;

static std::unordered_map<uint32, std::vector<int32>> g_questEnderEntries;

// Above this an item objective is a world drop, with no useful spawn to walk to.
static constexpr size_t MAX_ITEM_OBJECTIVE_SPAWNS = 500;

static std::unordered_map<uint32, std::unordered_set<uint32>> g_questTransformSources;

static std::unordered_map<uint32, std::array<QuestSummonAnchor, QUEST_OBJECTIVES_COUNT>> g_questSummonAnchors;

static std::unordered_map<uint32, std::array<std::vector<uint32>, QUEST_OBJECTIVES_COUNT>> g_questKillSources;

static std::unordered_map<uint32, std::vector<QuestCraftReagent>> g_questItemReagents;

static constexpr uint32 REAGENT_STOCK_CASTS = 5;

static std::unordered_map<uint32, std::array<std::vector<QuestCraftReagent>, QUEST_ITEM_OBJECTIVES_COUNT>>
    g_questCraftReagents;

static std::unordered_map<uint32, std::array<uint32, QUEST_ITEM_OBJECTIVES_COUNT>> g_questCraftItems;

static uint32 GetItemSpell(uint32 itemId)
{
    ItemTemplate const* proto = itemId ? sObjectMgr->GetItemTemplate(itemId) : nullptr;
    if (!proto)
        return 0;

    for (auto const& spell : proto->Spells)
        if (spell.SpellId > 0)
            return spell.SpellId;

    return 0;
}

static uint32 GetSrcItemSpell(Quest const* quest) { return GetItemSpell(quest->GetSrcItemId()); }

// SMART_EVENT_SPELLHIT (8) -> SMART_ACTION_UPDATE_TEMPLATE (36): spell -> (pre, post) entries.
static std::unordered_map<uint32, std::vector<std::pair<uint32, uint32>>> LoadSpellTransforms()
{
    std::unordered_map<uint32, std::vector<std::pair<uint32, uint32>>> transforms;
    QueryResult result = WorldDatabase.Query(
        "SELECT entryorguid, event_param1, action_param1 FROM smart_scripts "
        "WHERE source_type = 0 AND event_type = 8 AND action_type = 36 "
        "AND entryorguid > 0 AND event_param1 > 0 AND action_param1 > 0");
    if (!result)
        return transforms;

    do
    {
        Field* fields = result->Fetch();
        transforms[fields[1].Get<uint32>()].emplace_back(fields[0].Get<uint32>(), fields[2].Get<uint32>());
    } while (result->NextRow());

    return transforms;
}

// target_type 19 = SMART_TARGET_CLOSEST_CREATURE, which links a summoned trigger and the NPC
// reacting to it; either side is a creature the item has to be used next to.
static std::unordered_map<uint32, std::vector<uint32>> LoadSmartAnchorHints(
    std::unordered_map<uint32, std::vector<uint32>>& deathAnchors)
{
    std::unordered_map<uint32, std::vector<uint32>> hints;
    QueryResult result = WorldDatabase.Query(
        "SELECT entryorguid, target_param1, event_type FROM smart_scripts "
        "WHERE source_type = 0 AND target_type = 19 AND entryorguid > 0 AND target_param1 > 0 "
        "AND entryorguid <> target_param1");
    if (!result)
        return hints;

    do
    {
        Field* fields = result->Fetch();
        uint32 const owner = fields[0].Get<uint32>();
        uint32 const target = fields[1].Get<uint32>();
        hints[owner].push_back(target);
        hints[target].push_back(owner);

        // A reactor acting from its own death event only credits once killed.
        if (fields[2].Get<uint32>() == SMART_EVENT_DEATH)
            deathAnchors[target].push_back(owner);
    } while (result->NextRow());

    return hints;
}

static uint32 SpellKillCreditEntry(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return 0;

    for (uint8 e = 0; e < MAX_SPELL_EFFECTS; ++e)
        if ((info->Effects[e].Effect == SPELL_EFFECT_KILL_CREDIT ||
             info->Effects[e].Effect == SPELL_EFFECT_KILL_CREDIT2) &&
            info->Effects[e].MiscValue > 0)
            return uint32(info->Effects[e].MiscValue);

    return 0;
}

static uint32 SpellSummonEntry(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return 0;

    for (uint8 e = 0; e < MAX_SPELL_EFFECTS; ++e)
        if (info->Effects[e].Effect == SPELL_EFFECT_SUMMON && info->Effects[e].MiscValue > 0)
            return uint32(info->Effects[e].MiscValue);

    return 0;
}

// action 11 = SMART_ACTION_CAST, 33 = SMART_ACTION_CALL_KILLEDMONSTER. Spells go through
// sSpellMgr, not Spell.dbc: many of them are server-side.
static std::unordered_map<uint32, std::vector<uint32>> LoadObjectiveKillSources()
{
    std::unordered_map<uint32, std::vector<uint32>> sources;
    QueryResult result = WorldDatabase.Query(
        "SELECT entryorguid, action_type, action_param1 FROM smart_scripts "
        "WHERE source_type = 0 AND action_type IN (11, 33) AND entryorguid > 0 "
        "AND action_param1 > 0");
    if (!result)
        return sources;

    do
    {
        Field* fields = result->Fetch();
        uint32 const owner = fields[0].Get<uint32>();
        uint32 const param = fields[2].Get<uint32>();

        uint32 wanted = 0;
        if (fields[1].Get<uint32>() == 33)
            wanted = param;
        else if (uint32 credited = SpellKillCreditEntry(param))
            wanted = credited;
        else
            wanted = SpellSummonEntry(param);

        if (wanted && wanted != owner)
            sources[wanted].push_back(owner);
    } while (result->NextRow());

    return sources;
}

static bool SpellCraftsItem(uint32 spellId, uint32 itemId, std::vector<QuestCraftReagent>& reagents)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;

    bool creates = false;
    for (uint8 e = 0; e < MAX_SPELL_EFFECTS; ++e)
        if (info->Effects[e].Effect == SPELL_EFFECT_CREATE_ITEM && info->Effects[e].ItemType == itemId)
            creates = true;

    if (!creates)
        return false;

    for (uint8 r = 0; r < MAX_SPELL_REAGENTS; ++r)
        if (info->Reagent[r] > 0 && info->ReagentCount[r] > 0)
            reagents.push_back({uint32(info->Reagent[r]), info->ReagentCount[r]});

    return !reagents.empty();
}

static uint32 FindCraftReagents(Quest const* quest, uint32 itemId, std::vector<QuestCraftReagent>& reagents)
{
    if (SpellCraftsItem(GetItemSpell(quest->GetSrcItemId()), itemId, reagents))
        return quest->GetSrcItemId();

    for (uint8 i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
        if (SpellCraftsItem(GetItemSpell(quest->ItemDrop[i]), itemId, reagents))
            return quest->ItemDrop[i];

    return 0;
}

// 17 = CONDITION_SOURCE_TYPE_SPELL, 29 = CONDITION_NEAR_CREATURE.
static std::unordered_map<uint32, uint32> LoadSpellNearCreature()
{
    std::unordered_map<uint32, uint32> nearCreature;
    QueryResult result = WorldDatabase.Query(
        "SELECT SourceEntry, ConditionValue1 FROM conditions "
        "WHERE SourceTypeOrReferenceId = 17 AND ConditionTypeOrReference = 29 "
        "AND NegativeCondition = 0 AND ConditionValue1 > 0");
    if (!result)
        return nearCreature;

    do
    {
        Field* fields = result->Fetch();
        nearCreature[fields[0].Get<uint32>()] = fields[1].Get<uint32>();
    } while (result->NextRow());

    return nearCreature;
}

static void CollectWantedDrops(LootTemplateAccess const* loot, std::unordered_set<uint32> const& wanted, int32 entry,
                               std::unordered_map<uint32, std::vector<int32>>& dropSources, uint32 depth = 0)
{
    // References nest a level or two in practice; the guard is only against a cyclic table.
    if (!loot || depth > 3)
        return;

    for (auto const& item : loot->Entries)
    {
        // Negative references are ordinary in the shipped tables; the core uses the absolute value.
        if (item->reference)
        {
            CollectWantedDrops(reinterpret_cast<LootTemplateAccess const*>(
                                   LootTemplates_Reference.GetLootFor(uint32(std::abs(item->reference)))),
                               wanted, entry, dropSources, depth + 1);
            continue;
        }

        if (wanted.count(item->itemid))
            dropSources[item->itemid].push_back(entry);
    }
}

static void FileSpawnPositions(std::unordered_map<int32, std::vector<std::pair<uint32, uint32>>> const& entryMap)
{
    for (auto const& [spawnId, creData] : sObjectMgr->GetAllCreatureData())
    {
        // Filed under each wanted alternate entry: the respawn decides which one appears.
        for (uint32 entry : SpawnEntries(creData))
        {
            if (entry && entryMap.count(int32(entry)))
                g_entrySpawns[int32(entry)][creData.mapid].push_back(GuidPosition(creData));
        }
    }

    for (auto const& [spawnId, goData] : sObjectMgr->GetAllGOData())
    {
        if (entryMap.count(-int32(goData.id)))
            g_entrySpawns[-int32(goData.id)][goData.mapid].push_back(GuidPosition(goData));
    }
}

static size_t GetEntrySpawnCount(int32 entry)
{
    auto it = g_entrySpawns.find(entry);
    if (it == g_entrySpawns.end())
        return 0;

    size_t total = 0;
    for (auto const& [mapId, positions] : it->second)
        total += positions.size();

    return total;
}

static void FileObjectiveEntries(std::unordered_map<int32, std::vector<std::pair<uint32, uint32>>> const& entryMap)
{
    uint32 constexpr firstItemFlag = 1u << QUEST_OBJECTIVES_COUNT;

    std::unordered_map<int32, std::vector<std::pair<uint32, uint32>>> spawned;
    for (auto const& [entry, wants] : entryMap)
    {
        if (!g_entrySpawns.count(entry))
            continue;

        std::vector<std::pair<uint32, uint32>> objectives = wants;
        std::sort(objectives.begin(), objectives.end());
        objectives.erase(std::unique(objectives.begin(), objectives.end()), objectives.end());
        spawned.emplace(entry, std::move(objectives));
    }

    // The world-drop cap applies to the objective's total spawns, not to each dropper.
    std::unordered_map<uint32, std::unordered_map<uint32, size_t>> spawnTotals;
    for (auto const& [entry, wants] : spawned)
        for (auto const& [questId, flag] : wants)
            spawnTotals[questId][flag] += GetEntrySpawnCount(entry);

    for (auto const& [entry, wants] : spawned)
    {
        for (auto const& [questId, flag] : wants)
        {
            if (flag >= firstItemFlag && spawnTotals[questId][flag] > MAX_ITEM_OBJECTIVE_SPAWNS)
                continue;

            g_questObjectiveEntries[questId][flag].push_back(entry);
        }
    }
}

void BuildQuestObjectiveSpawns()
{
    // Appends to the globals below, so a second call would double every spawn vector.
    if (!g_entrySpawns.empty())
        return;

    std::unordered_map<int32, std::vector<std::pair<uint32, uint32>>> entryMap;
    std::unordered_set<uint32> questItems;

    // Enders get an empty want-list: spawn positions only, no objective.
    for (auto const& [entry, questId] : *sObjectMgr->GetCreatureQuestInvolvedRelationMap())
    {
        g_questEnderEntries[questId].push_back(int32(entry));
        entryMap[int32(entry)];
    }

    for (auto const& [entry, questId] : *sObjectMgr->GetGOQuestInvolvedRelationMap())
    {
        g_questEnderEntries[questId].push_back(-int32(entry));
        entryMap[-int32(entry)];
    }
    for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
    {
        for (uint32 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
        {
            int32 const entry = quest->RequiredNpcOrGo[idx];
            if (entry && quest->RequiredNpcOrGoCount[idx])
                entryMap[entry].emplace_back(questId, 1u << idx);
        }

        uint32 const srcSpellId = GetSrcItemSpell(quest);

        if (SpellInfo const* srcSpell = sSpellMgr->GetSpellInfo(srcSpellId))
            for (uint8 r = 0; r < MAX_SPELL_REAGENTS; ++r)
                if (srcSpell->Reagent[r] > 0 && srcSpell->ReagentCount[r] > 0)
                    g_questItemReagents[questId].push_back({uint32(srcSpell->Reagent[r]), srcSpell->ReagentCount[r]});

        for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (!quest->RequiredItemId[i] || !quest->RequiredItemCount[i])
                continue;

            questItems.insert(quest->RequiredItemId[i]);

            std::vector<QuestCraftReagent> reagents;
            uint32 const crafter = FindCraftReagents(quest, quest->RequiredItemId[i], reagents);
            if (reagents.empty())
                continue;

            for (QuestCraftReagent const& reagent : reagents)
                questItems.insert(reagent.item);

            questItems.insert(crafter);

            g_questCraftReagents[questId][i] = std::move(reagents);
            g_questCraftItems[questId][i] = crafter;
        }
    }

    std::unordered_map<uint32, std::vector<int32>> dropSources;
    if (!questItems.empty())
    {
        if (CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates())
        {
            for (auto const& [entry, tmpl] : *creatures)
                CollectWantedDrops(
                    DropMapValue::GetLootTemplate(ObjectGuid::Create<HighGuid::Unit>(entry, uint32(1)), LOOT_CORPSE),
                    questItems, int32(entry), dropSources);
        }

        if (GameObjectTemplateContainer const* gos = sObjectMgr->GetGameObjectTemplates())
        {
            for (auto const& [entry, tmpl] : *gos)
                CollectWantedDrops(DropMapValue::GetLootTemplate(
                                       ObjectGuid::Create<HighGuid::GameObject>(entry, uint32(1)), LOOT_CORPSE),
                                   questItems, -int32(entry), dropSources);
        }
    }

    std::unordered_set<uint32> spawnedEntries;
    for (auto const& [spawnId, creData] : sObjectMgr->GetAllCreatureData())
        for (uint32 entry : SpawnEntries(creData))
            if (entry)
                spawnedEntries.insert(entry);

    auto const spellTransforms = LoadSpellTransforms();

    std::unordered_map<uint32, std::vector<uint32>> focusObjects;
    if (GameObjectTemplateContainer const* gos = sObjectMgr->GetGameObjectTemplates())
        for (auto const& [goEntry, goInfo] : *gos)
            if (goInfo.type == GAMEOBJECT_TYPE_SPELL_FOCUS && goInfo.spellFocus.focusId)
                focusObjects[goInfo.spellFocus.focusId].push_back(goEntry);

    std::unordered_map<uint32, std::vector<uint32>> deathAnchors;
    auto const smartAnchorHints = LoadSmartAnchorHints(deathAnchors);
    auto const objectiveKillSources = LoadObjectiveKillSources();
    auto const spellNearCreature = LoadSpellNearCreature();

    for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
    {
        uint32 const srcSpellId = GetSrcItemSpell(quest);
        SpellInfo const* srcSpell = sSpellMgr->GetSpellInfo(srcSpellId);
        auto const craftIt = g_questCraftReagents.find(questId);

        for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (!quest->RequiredItemId[i] || !quest->RequiredItemCount[i])
                continue;

            std::vector<uint32> wanted{quest->RequiredItemId[i]};
            if (craftIt != g_questCraftReagents.end() && !craftIt->second[i].empty())
            {
                wanted.clear();
                for (QuestCraftReagent const& reagent : craftIt->second[i])
                    wanted.push_back(reagent.item);

                if (uint32 const crafter = GetQuestCraftItem(questId, i); crafter && crafter != quest->GetSrcItemId())
                    wanted.push_back(crafter);
            }

            uint32 const flag = 1u << (QUEST_OBJECTIVES_COUNT + i);
            for (uint32 wantedItem : wanted)
            {
                auto src = dropSources.find(wantedItem);
                if (src == dropSources.end())
                    continue;

                for (int32 entry : src->second)
                {
                    // An unspawned dropper only exists after a SmartAI transform by the quest's item, so index the
                    // spawned pre-transform creature instead.
                    if (entry > 0 && !spawnedEntries.count(uint32(entry)))
                    {
                        auto tf = spellTransforms.find(srcSpellId);
                        if (tf == spellTransforms.end())
                            continue;

                        for (auto const& [pre, post] : tf->second)
                        {
                            if (post != uint32(entry) || !spawnedEntries.count(pre))
                                continue;

                            entryMap[int32(pre)].emplace_back(questId, flag);
                            g_questTransformSources[questId].insert(pre);
                        }
                        continue;
                    }

                    entryMap[entry].emplace_back(questId, flag);
                }
            }
        }

        for (uint32 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
        {
            int32 const entry = quest->RequiredNpcOrGo[idx];
            if (entry <= 0 || !quest->RequiredNpcOrGoCount[idx])
                continue;

            uint32 const flag = 1u << idx;
            bool const objectiveSpawned = spawnedEntries.count(uint32(entry)) != 0;

            std::vector<uint32> credits;
            if (!objectiveSpawned)
            {
                auto addCredit = [&credits, &spawnedEntries](uint32 source)
                {
                    if (spawnedEntries.count(source) &&
                        std::find(credits.begin(), credits.end(), source) == credits.end())
                        credits.push_back(source);
                };

                if (auto it = objectiveKillSources.find(uint32(entry)); it != objectiveKillSources.end())
                    for (uint32 source : it->second)
                        addCredit(source);

                if (auto it = deathAnchors.find(uint32(entry)); it != deathAnchors.end())
                    for (uint32 source : it->second)
                        addCredit(source);

                for (uint32 source : credits)
                    entryMap[int32(source)].emplace_back(questId, flag);

                if (!credits.empty())
                    g_questKillSources[questId][idx] = credits;
            }

            if (!srcSpell)
                continue;

            bool summoned = false;
            for (uint8 e = 0; e < MAX_SPELL_EFFECTS; ++e)
                if (srcSpell->Effects[e].Effect == SPELL_EFFECT_SUMMON && srcSpell->Effects[e].MiscValue == entry)
                    summoned = true;

            // Spell focus wins: the core enforces it, even for an objective that is already spawned.
            if (srcSpell->RequiresSpellFocus)
            {
                g_questSummonAnchors[questId][idx].focusId = srcSpell->RequiresSpellFocus;
                g_questSummonAnchors[questId][idx].summonsObjective = summoned;
                if (!objectiveSpawned)
                    for (uint32 goEntry : focusObjects[srcSpell->RequiresSpellFocus])
                        entryMap[-int32(goEntry)].emplace_back(questId, flag);
                continue;
            }

            if (!summoned || objectiveSpawned)
                continue;

            uint32 anchor = 0;
            if (auto it = spellNearCreature.find(srcSpellId);
                it != spellNearCreature.end() && spawnedEntries.count(it->second))
                anchor = it->second;

            if (!anchor)
                if (auto it = smartAnchorHints.find(uint32(entry)); it != smartAnchorHints.end())
                    for (uint32 hint : it->second)
                        if (spawnedEntries.count(hint))
                        {
                            anchor = hint;
                            break;
                        }

            if (!anchor && !credits.empty())
                anchor = credits.front();

            if (!anchor)
                continue;

            bool const killAnchor = std::find(credits.begin(), credits.end(), anchor) != credits.end();
            g_questSummonAnchors[questId][idx].creature = anchor;
            g_questSummonAnchors[questId][idx].killAnchor = killAnchor;
            g_questSummonAnchors[questId][idx].summonsObjective = true;
            if (!killAnchor)
                entryMap[int32(anchor)].emplace_back(questId, flag);
        }
    }

    FileSpawnPositions(entryMap);
    FileObjectiveEntries(entryMap);
}

std::vector<int32> const& GetQuestObjectiveEntries(uint32 questId, uint32 objectiveFlag)
{
    static std::vector<int32> const empty;
    auto quest = g_questObjectiveEntries.find(questId);
    if (quest == g_questObjectiveEntries.end())
        return empty;

    auto flag = quest->second.find(objectiveFlag);
    return flag != quest->second.end() ? flag->second : empty;
}

std::vector<GuidPosition> const& GetEntrySpawns(int32 entry, uint32 mapId)
{
    static std::vector<GuidPosition> const empty;
    auto it = g_entrySpawns.find(entry);
    if (it == g_entrySpawns.end())
        return empty;

    auto onMap = it->second.find(mapId);
    return onMap != it->second.end() ? onMap->second : empty;
}

std::vector<int32> const& GetQuestEnderEntries(uint32 questId)
{
    static std::vector<int32> const empty;
    auto it = g_questEnderEntries.find(questId);
    return it != g_questEnderEntries.end() ? it->second : empty;
}

std::unordered_set<uint32> const& GetQuestTransformSources(uint32 questId)
{
    static std::unordered_set<uint32> const empty;
    auto it = g_questTransformSources.find(questId);
    return it != g_questTransformSources.end() ? it->second : empty;
}

QuestSummonAnchor GetQuestSummonAnchor(uint32 questId, uint32 objectiveIdx)
{
    if (objectiveIdx >= QUEST_OBJECTIVES_COUNT)
        return {};

    auto it = g_questSummonAnchors.find(questId);
    return it != g_questSummonAnchors.end() ? it->second[objectiveIdx] : QuestSummonAnchor{};
}

std::vector<QuestCraftReagent> const& GetQuestItemReagents(uint32 questId)
{
    static std::vector<QuestCraftReagent> const empty;
    auto it = g_questItemReagents.find(questId);
    return it != g_questItemReagents.end() ? it->second : empty;
}

bool IsQuestExtraItemWanted(Player* bot, Quest const* quest, uint32 itemId)
{
    if (!bot || !quest || !itemId)
        return false;

    for (uint8 i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
    {
        if (quest->ItemDrop[i] != itemId)
            continue;

        uint32 const need = quest->ItemDropQuantity[i];
        return !need || bot->GetItemCount(itemId, false) < need;
    }

    for (QuestCraftReagent const& reagent : GetQuestItemReagents(quest->GetQuestId()))
    {
        if (reagent.item != itemId)
            continue;

        return bot->GetItemCount(itemId, false) < reagent.count * REAGENT_STOCK_CASTS;
    }

    return false;
}

std::vector<QuestCraftReagent> const& GetQuestCraftReagents(uint32 questId, uint32 itemObjectiveIdx)
{
    static std::vector<QuestCraftReagent> const empty;
    if (itemObjectiveIdx >= QUEST_ITEM_OBJECTIVES_COUNT)
        return empty;

    auto it = g_questCraftReagents.find(questId);
    return it != g_questCraftReagents.end() ? it->second[itemObjectiveIdx] : empty;
}

uint32 GetQuestCraftItem(uint32 questId, uint32 itemObjectiveIdx)
{
    if (itemObjectiveIdx >= QUEST_ITEM_OBJECTIVES_COUNT)
        return 0;

    auto it = g_questCraftItems.find(questId);
    return it != g_questCraftItems.end() ? it->second[itemObjectiveIdx] : 0;
}

std::vector<uint32> const& GetQuestKillSources(uint32 questId, uint32 objectiveIdx)
{
    static std::vector<uint32> const empty;
    if (objectiveIdx >= QUEST_OBJECTIVES_COUNT)
        return empty;

    auto it = g_questKillSources.find(questId);
    return it != g_questKillSources.end() ? it->second[objectiveIdx] : empty;
}

// Selects all questgivers for a specific level (range).
questGiverMap QuestGiversValue::Calculate()
{
    uint32 level = 0;
    std::string const q = getQualifier();
    bool hasQualifier = !q.empty();

    if (hasQualifier)
        level = stoi(q);

    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    questGiverMap guidps;

    for (auto& qPair : questMap)
    {
        for (auto& entry : qPair.second[(int)QuestRelationFlag::questGiver])
        {
            for (auto& guidp : entry.second)
            {
                uint32 questId = qPair.first;

                if (hasQualifier)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

                    if (quest && (level < quest->GetMinLevel() || (int)level > quest->GetQuestLevel() + 10))
                        continue;
                }

                guidps[questId].push_back(guidp);
            }
        }
    }

    return guidps;
}

std::vector<GuidPosition> ActiveQuestGiversValue::Calculate()
{
    questGiverMap qGivers = GAI_VALUE2(questGiverMap, "quest givers", bot->GetLevel());

    std::vector<GuidPosition> retQuestGivers;

    for (auto& qGiver : qGivers)
    {
        uint32 questId = qGiver.first;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            continue;
        }

        if (!bot->CanTakeQuest(quest, false))
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);

        if (status != QUEST_STATUS_NONE)
            continue;

        for (auto& guidp : qGiver.second)
        {
            CreatureTemplate const* creatureTemplate = guidp.GetCreatureTemplate();

            if (creatureTemplate)
            {
                if (bot->GetFactionReactionTo(bot->GetFactionTemplateEntry(),
                                              sFactionTemplateStore.LookupEntry(creatureTemplate->faction)) <
                    REP_FRIENDLY)
                    continue;
            }

            if (!guidp.IsCreatureOrGOAccessible())
                continue;

            retQuestGivers.push_back(guidp);
        }
    }

    return retQuestGivers;
}

std::vector<GuidPosition> ActiveQuestTakersValue::Calculate()
{
    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    std::vector<GuidPosition> retQuestTakers;

    QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

    for (auto& questStatus : questStatusMap)
    {
        uint32 questId = questStatus.first;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

        if (!quest)
        {
            continue;
        }

        QuestStatus status = questStatus.second.Status;
        if ((status != QUEST_STATUS_COMPLETE || bot->GetQuestRewardStatus(questId)) &&
            (!quest->IsAutoComplete() || !bot->CanTakeQuest(quest, false)))
            continue;

        auto q = questMap.find(questId);

        if (q == questMap.end())
            continue;

        auto qt = q->second.find((int)QuestRelationFlag::questTaker);

        if (qt == q->second.end())
            continue;

        for (auto& entry : qt->second)
        {
            if (entry.first > 0)
            {
                if (CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(entry.first))
                {
                    if (bot->GetFactionReactionTo(bot->GetFactionTemplateEntry(),
                                                  sFactionTemplateStore.LookupEntry(info->faction)) < REP_FRIENDLY)
                        continue;
                }
            }

            for (auto& guidp : entry.second)
            {
                if (!guidp.IsCreatureOrGOAccessible())
                    continue;

                retQuestTakers.push_back(guidp);
            }
        }
    }

    return retQuestTakers;
}

std::vector<GuidPosition> ActiveQuestObjectivesValue::Calculate()
{
    questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

    std::vector<GuidPosition> retQuestObjectives;

    QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

    for (auto& questStatus : questStatusMap)
    {
        uint32 questId = questStatus.first;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            continue;
        }

        QuestStatusData statusData = questStatus.second;
        if (statusData.Status != QUEST_STATUS_INCOMPLETE)
            continue;

        for (uint32 objective = 0; objective < QUEST_OBJECTIVES_COUNT; objective++)
        {
            if (quest->RequiredItemCount[objective])
            {
                uint32 reqCount = quest->RequiredItemCount[objective];
                uint32 hasCount = statusData.ItemCount[objective];

                if (!reqCount || hasCount >= reqCount)
                    continue;
            }

            if (quest->RequiredNpcOrGoCount[objective])
            {
                uint32 reqCount = quest->RequiredNpcOrGoCount[objective];
                uint32 hasCount = statusData.CreatureOrGOCount[objective];

                if (!reqCount || hasCount >= reqCount)
                    continue;
            }

            auto q = questMap.find(questId);

            if (q == questMap.end())
                continue;

            auto qt = q->second.find((int)QuestRelationFlag(1 << objective));

            if (qt == q->second.end())
                continue;

            for (auto& entry : qt->second)
            {
                for (auto& guidp : entry.second)
                {
                    if (!guidp.IsCreatureOrGOAccessible())
                        continue;

                    retQuestObjectives.push_back(guidp);
                }
            }
        }
    }

    return retQuestObjectives;
}

uint8 FreeQuestLogSlotValue::Calculate()
{
    uint8 numQuest = 0;
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);

        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        numQuest++;
    }

    return MAX_QUEST_LOG_SIZE - numQuest;
}

uint32 DialogStatusValue::getDialogStatus(Player* bot, int32 questgiver, uint32 questId)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationBounds rbounds;   // QuestRelations (quest-giver)
    QuestRelationBounds irbounds;  // InvolvedRelations (quest-finisher)

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelationBounds(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelationBounds(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelations::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (questId && itr->second != questId)
            continue;

        Quest const* pQuest = sObjectMgr->GetQuestTemplate(itr->second);
        if (!pQuest)
        {
            continue;
        }

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        QuestStatus status = bot->GetQuestStatus(itr->second);

        if ((status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(itr->second)) ||
            (pQuest->IsAutoComplete() && bot->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelations::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (questId && itr->second != questId)
            continue;

        Quest const* pQuest = sObjectMgr->GetQuestTemplate(itr->second);
        if (!pQuest)
        {
            continue;
        }

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        QuestStatus status = bot->GetQuestStatus(itr->second);

        if (status == QUEST_STATUS_NONE)  // For all other cases the mark is handled either at some place else, or with
                                          // involved-relations already
        {
            if (bot->CanSeeStartQuest(pQuest))
            {
                if (bot->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() ||
                        (pQuest->IsRepeatable() &&
                         bot->getQuestStatusMap()[itr->second].Status == QUEST_STATUS_REWARDED))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 || bot->GetLevel() <= bot->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

uint32 DialogStatusValue::Calculate() { return getDialogStatus(bot, stoi(getQualifier())); }

uint32 DialogStatusQuestValue::Calculate()
{
    return getDialogStatus(bot, getMultiQualifier(getQualifier(), 0), getMultiQualifier(getQualifier(), 1));
}

bool CanAcceptQuestValue::Calculate()
{
    return AI_VALUE2(uint32, "dialog status", getQualifier()) == DIALOG_STATUS_AVAILABLE;
};

bool CanAcceptQuestLowLevelValue::Calculate()
{
    uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
    return dialogStatus == DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
};

bool CanTurnInQuestValue::Calculate()
{
    uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
    return dialogStatus == DIALOG_STATUS_REWARD2 || dialogStatus == DIALOG_STATUS_REWARD ||
           dialogStatus == DIALOG_STATUS_REWARD_REP;
};
