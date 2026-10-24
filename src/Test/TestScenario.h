/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_TESTSCENARIO_H
#define _PLAYERBOT_TESTSCENARIO_H

#include "ObjectGuid.h"
#include "Position.h"

#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>

class Player;
class PlayerbotAI;

enum class StepStatus
{
    InProgress,
    Done,
    Failed
};

class TestContext;

struct TestStep
{
    std::string name;
    uint32 timeoutMs;
    std::function<StepStatus(TestContext&)> tick;
    uint32 elapsedMs = 0;
};

struct BotDef
{
    uint8 cls;
    std::string spec;   // premade spec name from Playerbots.PremadeSpecName.* ("prot pve"); empty = random
    uint8 race = 0;     // RACE_* to force a specific race from the pool; 0 = any race of the faction
};

struct GearDef
{
    uint32 quality;     // ITEM_QUALITY_* cap for the factory
    uint16 ilvl;        // 0 = level-appropriate; else gearscore-capped at this ilvl
    bool bis;           // gear from the module's BiS list at ilvl ("autogear bis")
    GearDef(uint32 quality, uint16 ilvl = 0, bool bis = false) : quality(quality), ilvl(ilvl), bis(bis) { }
};

// mod-individual-progression tiers (hidden quests 66000+tier), applied via
// TestContext::SetProgression.
constexpr uint8 PROGRESSION_VANILLA_START = 0;
constexpr uint8 PROGRESSION_WOTLK_RAIDS = 13;

class TestContext
{
public:
    // Generic queued step; everything else is sugar over this.
    void Step(std::string name, uint32 timeoutMs, std::function<StepStatus(TestContext&)> tick);
    // One-shot action; returning false fails the scenario.
    void Do(std::string name, std::function<bool(TestContext&)> action);
    // Polled every world tick until true; timeout fails the scenario.
    void WaitUntil(std::string name, uint32 timeoutMs, std::function<bool(TestContext&)> pred);
    // One-shot check; false fails the scenario.
    void Assert(std::string name, std::function<bool(TestContext&)> pred);

    // Bot lifecycle primitives.
    void SpawnBots(uint32 level, std::vector<BotDef> const& bots, GearDef gear, bool alliance = true);
    void FormGroup(bool raid);
    void TeleportTo(uint32 mapId, float x, float y, float z, float o);
    void TeleportTo(uint32 mapId, Position const& pos)
    {
        TeleportTo(mapId, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
    }
    // Standard raid preamble: spawn, progression, group, teleport, clear, ready check.
    void PrepareRaid(uint32 level, std::vector<BotDef> const& comp, GearDef gear, uint8 progression,
                     uint32 mapId, Position const& pos, std::vector<uint32> const& spare = {});
    // Despawns all hostiles except spareEntries, map-wide (a radius sweep misses patrols).
    void ClearMap(std::vector<uint32> const& spareEntries);
    // mod-individual-progression tier via hidden quests 66000+tier; no-op if absent.
    void SetProgression(uint8 state);
    struct ReadinessSummary
    {
        uint32 unblessed = 0;          // raid members without any paladin blessing
        uint32 unstockedManaUsers = 0; // mana users without an energize potion
        uint32 unimbuedWeapons = 0;    // rogues/shamans missing a weapon imbue (poison/enchant)
        uint32 missingSoulstones = 0;  // warlocks with no soulstone aura anywhere in the raid
    };
    // Logs a per-bot readiness table (buffs, consumables, imbues) tagged @tag.
    ReadinessSummary NoteReadiness(std::string const& tag);
    // Rebuffs the raid via the ready-check machinery until nothing is pending.
    // Proceeds early on combat; releases stuck bots at the soft cap.
    void ReadyCheck(uint32 minMs = 15000, uint32 softCapMs = 140000);

    Player* GetBot(size_t index) const;   // nullptr if not connected/in world
    Player* FirstAliveBot() const;
    uint32 AliveBotCount() const;

    // Immediate helpers for use inside step ticks (not queued).
    // Every connected bot with AI attached; liveness checks are the caller's.
    void ForEachBotAI(std::function<void(Player*, PlayerbotAI*)> const& fn) const;
    // Leader's group puts the skull on the kill target.
    void MarkSkull(ObjectGuid target) const;
    void OrderAttack(size_t index) const;
    // Everyone attacks except off-tank indices 1..skipOffTanksBelow-1.
    void OrderAttackAll(size_t skipOffTanksBelow = 0) const;
    // Elapsed time of the currently executing step.
    uint32 StepElapsedMs() const { return steps.empty() ? 0 : steps.front().elapsedMs; }

    void SetMetric(std::string const& name, double value) { metrics[name] = value; }
    void AddNote(std::string note) { notes.push_back(std::move(note)); }

    std::vector<ObjectGuid> botGuids;
    std::vector<std::string> botSpecs;    // parallel to botGuids; the BotDef spec each bot was given
    ObjectGuid groupLeader;
    ObjectGuid bossGuid;                  // set by scenarios; watched by recorder/timeline
    uint32 elapsedMs = 0;                 // total scenario runtime so far
    std::map<std::string, double> metrics;
    std::vector<std::string> notes;
    std::deque<TestStep> steps;
};

class TestScenario
{
public:
    virtual ~TestScenario() = default;
    virtual std::string GetName() const = 0;
    virtual void Setup(TestContext& ctx) = 0;

    // True on a non-instanceable map: two copies would share the world and contend
    // for spawns, so the runner never runs such a scenario concurrently with itself.
    virtual bool SharesWorldWithCopies() const { return false; }

    // Runs sharing a non-empty key never run concurrently. Empty = ungrouped.
    virtual std::string ZoneKey() const { return ""; }
};

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
