/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_TESTRUNNER_H
#define _PLAYERBOT_TESTRUNNER_H

#include "TestScenario.h"

#include <array>
#include <deque>
#include <memory>
#include <mutex>
#include <set>

class Unit;

struct ScenarioResult
{
    std::string name;
    bool passed = false;
    std::string failedStep;
    uint32 elapsedMs = 0;
    std::map<std::string, double> metrics;
    std::vector<std::string> notes;
    std::vector<std::array<double, 3>> timeline;  // [tSec, bossHpPct, aliveBots]
};

struct RecordedHit
{
    uint32 atMs;
    std::string attacker;
    uint32 amount;
    uint32 healthAfter;
};

// Per-run combat telemetry, mutated from map-update threads under
// IntegrationTestMgr's participant lock.
struct RunCombatData
{
    uint32 deaths = 0;
    std::map<ObjectGuid, std::deque<RecordedHit>> lastHits;  // per victim, capped at 10
    std::set<ObjectGuid> reportedDeaths;                     // OnDamage is a pre-hook; hits on an already-dead bot re-satisfy the death check
    std::vector<std::string> recapLines;
    std::map<ObjectGuid, uint64> damageDealt;                // per bot (pets attributed to owner), vs hostile creatures
    std::map<ObjectGuid, std::string> attackerNames;
};

// Repeated runs of one scenario ("name*N" in the run list) with aggregated
// per-spec dps/ilvl statistics; runs execute in waves of BENCH_CONCURRENCY.
struct BenchState
{
    std::string scenarioName;
    TestScenario* scenario = nullptr;
    uint32 total = 0;
    uint32 started = 0;
    uint32 done = 0;
    uint32 passCount = 0;
    std::vector<double> killTimesSec;                    // passed runs only
    std::vector<uint32> deaths;
    std::map<std::string, std::vector<double>> specDps;  // per-spec dps samples across runs
    std::map<std::string, std::vector<uint32>> specIlvl;
};

// One concurrently-executing scenario run. Runs tick serially on the world
// thread; combat telemetry arrives from map threads via the participant index.
struct TestRun
{
    std::string label;            // scenario name, "#n"-suffixed when run in parallel with itself
    TestScenario* scenario = nullptr;
    TestContext ctx;
    RunCombatData combat;
    uint32 timelineAccumMs = 0;
    std::vector<std::array<double, 3>> timeline;
    size_t syncedBots = 0;        // participant-index sync watermark
    ObjectGuid syncedBoss;
    BenchState* bench = nullptr;  // owning bench when part of a "name*N" series
};

class IntegrationTestMgr
{
public:
    static IntegrationTestMgr& instance()
    {
        static IntegrationTestMgr mgr;
        return mgr;
    }

    void RegisterScenario(std::unique_ptr<TestScenario> scenario);
    void OnWorldStartup();
    void Update(uint32 diff);

    bool QueueRun(std::string const& csv, bool shutdownWhenDone);
    std::vector<std::string> ScenarioNames() const;
    bool IsRunning() const { return !_runs.empty(); }

    // Bot-picking guard: true while the guid belongs to any active run.
    bool IsParticipant(ObjectGuid guid);

    // Combat telemetry entry point (called from map-update threads).
    void RecordDamage(Unit* attacker, Unit* victim, std::string attackerName, uint32 damage);

private:
    TestScenario* FindScenario(std::string const& name) const;
    void TickRun(TestRun& run, uint32 diff);
    void FinishRun(TestRun& run, bool passed, std::string const& failedStep);
    void SyncParticipants(TestRun& run);
    void Teardown(TestRun& run);
    void WriteResults() const;
    void StartRun(TestScenario* scenario, std::string label, BenchState* bench);
    void HarvestBenchStats(TestRun& run, bool passed, uint32 fightMs);
    void FinishBench(BenchState& bench);
    void PumpBenches();
    // Starts pending runs whose ZoneKey is free; holds those whose zone has a
    // run in flight. Re-called whenever a run finishes.
    void PumpPending();

    // A requested (non-bench) run waiting for its zone to clear.
    struct PendingRun
    {
        TestScenario* scenario = nullptr;
        std::string label;
    };

    std::vector<std::unique_ptr<TestScenario>> _scenarios;
    std::vector<std::unique_ptr<TestRun>> _runs;
    std::vector<PendingRun> _pending;
    std::vector<std::unique_ptr<BenchState>> _benches;
    bool _shutdownWhenDone = false;
    bool _anyFailed = false;
    std::vector<ScenarioResult> _results;

    std::mutex _participantLock;
    std::map<ObjectGuid, TestRun*> _participants;  // bot/boss guid -> owning run
};

void AddPlayerbotsIntegrationTestScripts();

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
