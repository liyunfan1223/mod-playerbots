/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

// Run lifecycle of the test runner: scenario registry, run queueing (including
// "name*N" benches), per-tick stepping and teardown. Damage recording, bench
// statistics and the results file live in TestReporting.cpp.

#include "TestRunner.h"

#include "Chat.h"
#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "World.h"

#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "StringFormat.h"

#include <set>
#include <sstream>

namespace
{
    // Global cap of concurrently running bench runs across ALL "name*N" series —
    // benches drain in order, each finished run frees a slot for the next.
    constexpr uint32 BENCH_CONCURRENCY = 10;
    constexpr uint32 TIMELINE_SAMPLE_MS = 5000;
}

void IntegrationTestMgr::RegisterScenario(std::unique_ptr<TestScenario> scenario)
{
    _scenarios.push_back(std::move(scenario));
}

std::vector<std::string> IntegrationTestMgr::ScenarioNames() const
{
    std::vector<std::string> names;
    for (auto const& scenario : _scenarios)
        names.push_back(scenario->GetName());

    return names;
}

bool IntegrationTestMgr::QueueRun(std::string const& csv, bool shutdownWhenDone)
{
    std::vector<std::string> requested;
    std::vector<std::pair<std::string, uint32>> benches;   // "name*N" tokens
    std::stringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (token.empty())
            continue;

        if (token == "all")
        {
            requested = ScenarioNames();
            break;
        }

        // "name*N": a bench — N runs of the scenario with aggregated dps stats.
        uint32 benchCount = 0;
        if (size_t star = token.find('*'); star != std::string::npos)
        {
            benchCount = uint32(atoi(token.substr(star + 1).c_str()));
            token = token.substr(0, star);
        }

        // Exact scenario name, or a prefix expanding to every scenario that
        // starts with it ("mc" = all MC bosses, "quest" = the quest ladder,
        // "mc*10" = every MC boss benched 10x).
        std::vector<std::string> expanded;
        for (auto const& scenario : _scenarios)
        {
            if (scenario->GetName() == token)
            {
                expanded = { token };
                break;
            }
            if (scenario->GetName().compare(0, token.size(), token) == 0)
                expanded.push_back(scenario->GetName());
        }

        if (expanded.empty())
        {
            LOG_ERROR("playerbots", "IntegrationTest: unknown scenario '{}'", token);
            return false;
        }
        if (expanded.size() > 1)
            LOG_INFO("playerbots", "IntegrationTest: '{}' expands to {} scenarios", token, expanded.size());

        for (std::string& name : expanded)
            if (benchCount)
                benches.push_back({ std::move(name), benchCount });
            else
                requested.push_back(std::move(name));
    }

    if (requested.empty() && benches.empty())
        return false;

    if (_runs.empty())
        _anyFailed = false;
    if (shutdownWhenDone)
        _shutdownWhenDone = true;

    // Requested runs start concurrently. A continent scenario can't repeat concurrently
    // (copies share the world and fight over spawns); instanced ones each get an instance.
    std::map<std::string, uint32> nameCounts;
    for (std::string const& name : requested)
        ++nameCounts[name];

    for (auto const& [name, count] : nameCounts)
    {
        TestScenario const* scenario = FindScenario(name);
        if (count > 1 && scenario && scenario->SharesWorldWithCopies())
        {
            LOG_ERROR("playerbots",
                      "IntegrationTest: '{}' requested {}x, but it shares its world with its own "
                      "copies - they would contend for the same spawns. Boot once per repetition.",
                      name, count);
            return false;
        }
    }

    std::map<std::string, uint32> nameSeen;
    for (std::string const& name : requested)
        _pending.push_back({ FindScenario(name),
            nameCounts[name] > 1 ? Acore::StringFormat("{}#{}", name, ++nameSeen[name]) : name });
    PumpPending();

    for (auto const& [name, count] : benches)
    {
        auto bench = std::make_unique<BenchState>();
        bench->scenarioName = name;
        bench->scenario = FindScenario(name);
        bench->total = count;
        _benches.push_back(std::move(bench));

        LOG_INFO("playerbots", "IntegrationTest: bench {}*{} queued ({} slots)", name, count, BENCH_CONCURRENCY);
    }
    PumpBenches();

    LOG_INFO("playerbots", "IntegrationTest: {} run(s) in flight", _runs.size());
    return true;
}

void IntegrationTestMgr::PumpPending()
{
    // Zone keys currently occupied by a run in flight. A finished run (scenario
    // cleared, not yet erased from _runs) no longer holds its zone.
    std::set<std::string> busyZones;
    for (auto const& run : _runs)
        if (run->scenario)
            if (std::string zone = run->scenario->ZoneKey(); !zone.empty())
                busyZones.insert(std::move(zone));

    for (size_t i = 0; i < _pending.size();)
    {
        std::string const zone = _pending[i].scenario ? _pending[i].scenario->ZoneKey() : "";
        if (!zone.empty() && busyZones.count(zone))
        {
            ++i;  // zone busy — hold this run
            continue;
        }

        if (!zone.empty())
            busyZones.insert(zone);  // claim it so the next same-zone pending waits
        StartRun(_pending[i].scenario, std::move(_pending[i].label), nullptr);
        _pending.erase(_pending.begin() + i);
        // no ++i: erase shifted the next pending into slot i
    }
}

void IntegrationTestMgr::PumpBenches()
{
    uint32 active = 0;
    std::set<BenchState const*> busy;
    for (auto const& run : _runs)
        if (run->bench)
        {
            ++active;
            busy.insert(run->bench);
        }

    for (auto& bench : _benches)
    {
        // Continent benches run serially (shared world); instanced ones fill the wave.
        bool const serial = bench->scenario && bench->scenario->SharesWorldWithCopies();
        if (serial && busy.count(bench.get()))
            continue;

        while (active < BENCH_CONCURRENCY && bench->started < bench->total)
        {
            ++bench->started;
            ++active;
            StartRun(bench->scenario, Acore::StringFormat("{}#{}", bench->scenarioName, bench->started),
                     bench.get());
            if (serial)
                break;
        }
    }
}

TestScenario* IntegrationTestMgr::FindScenario(std::string const& name) const
{
    for (auto const& scenario : _scenarios)
        if (scenario->GetName() == name)
            return scenario.get();

    return nullptr;
}

void IntegrationTestMgr::StartRun(TestScenario* scenario, std::string label, BenchState* bench)
{
    auto run = std::make_unique<TestRun>();
    run->scenario = scenario;
    run->label = std::move(label);
    run->bench = bench;

    scenario->Setup(run->ctx);
    LOG_INFO("playerbots", "IntegrationTest: === {} started ({} steps) ===", run->label, run->ctx.steps.size());
    _runs.push_back(std::move(run));
}

bool IntegrationTestMgr::IsParticipant(ObjectGuid guid)
{
    std::lock_guard<std::mutex> guard(_participantLock);
    return _participants.count(guid) != 0;
}

void IntegrationTestMgr::SyncParticipants(TestRun& run)
{
    // Bot lists only grow and the boss guid is set once per fight; sync the flat
    // index (read by map threads in RecordDamage) when either changes.
    if (run.ctx.botGuids.size() == run.syncedBots && run.ctx.bossGuid == run.syncedBoss)
        return;

    std::lock_guard<std::mutex> guard(_participantLock);
    for (size_t i = run.syncedBots; i < run.ctx.botGuids.size(); ++i)
        _participants[run.ctx.botGuids[i]] = &run;
    run.syncedBots = run.ctx.botGuids.size();

    if (run.ctx.bossGuid != run.syncedBoss)
    {
        if (!run.syncedBoss.IsEmpty())
            _participants.erase(run.syncedBoss);
        if (!run.ctx.bossGuid.IsEmpty())
            _participants[run.ctx.bossGuid] = &run;
        run.syncedBoss = run.ctx.bossGuid;
    }
}

void IntegrationTestMgr::Update(uint32 diff)
{
    bool removed = false;
    for (size_t i = 0; i < _runs.size();)
    {
        TestRun& run = *_runs[i];
        TickRun(run, diff);

        // TickRun may have finished the run (it removes itself from _participants
        // and pushes its result); drop it here.
        if (run.scenario == nullptr)
        {
            _runs.erase(_runs.begin() + i);
            removed = true;
        }
        else
            ++i;
    }

    // A finished run frees its zone — start any pending run that was held on it.
    if (removed && !_pending.empty())
        PumpPending();

    if (_runs.empty() && _pending.empty() && _shutdownWhenDone && !_results.empty())
    {
        LOG_INFO("playerbots", "IntegrationTest: all scenarios finished, {}",
            _anyFailed ? "FAILURES present" : "all passed");
        World::StopNow(_anyFailed ? ERROR_EXIT_CODE : SHUTDOWN_EXIT_CODE);
    }
}

void IntegrationTestMgr::TickRun(TestRun& run, uint32 diff)
{
    run.ctx.elapsedMs += diff;
    SyncParticipants(run);

    run.timelineAccumMs += diff;
    if (!run.ctx.bossGuid.IsEmpty() && run.timelineAccumMs >= TIMELINE_SAMPLE_MS)
    {
        run.timelineAccumMs = 0;
        if (Player* ref = run.ctx.FirstAliveBot())
            if (Creature* boss = ObjectAccessor::GetCreature(*ref, run.ctx.bossGuid))
                run.timeline.push_back({ double(run.ctx.elapsedMs) / 1000.0, double(boss->GetHealthPct()), double(run.ctx.AliveBotCount()) });
    }

    if (run.ctx.steps.empty())
    {
        FinishRun(run, true, "");
        return;
    }

    TestStep& step = run.ctx.steps.front();
    step.elapsedMs += diff;

    StepStatus status = step.tick(run.ctx);

    // Publish picks before the next run ticks, so concurrent SpawnBots don't reuse them.
    SyncParticipants(run);

    if (status == StepStatus::InProgress && step.timeoutMs && step.elapsedMs > step.timeoutMs)
    {
        run.ctx.AddNote(Acore::StringFormat("step '{}' timed out after {} ms", step.name, step.elapsedMs));
        status = StepStatus::Failed;
    }

    if (status == StepStatus::Done)
    {
        LOG_INFO("playerbots", "IntegrationTest: [{}] step '{}' done ({} ms)", run.label, step.name, step.elapsedMs);
        run.ctx.steps.pop_front();
    }
    else if (status == StepStatus::Failed)
    {
        LOG_ERROR("playerbots", "IntegrationTest: [{}] step '{}' FAILED", run.label, step.name);
        FinishRun(run, false, step.name);
    }
}

void IntegrationTestMgr::Teardown(TestRun& run)
{
    // Log out spawned bots and disband the group; scenario-spawned creatures
    // despawn naturally via LogoutPlayerBot / world cleanup.
    Player* groupMember = run.ctx.groupLeader ? ObjectAccessor::FindConnectedPlayer(run.ctx.groupLeader) : nullptr;
    if (!groupMember)
        for (ObjectGuid const& guid : run.ctx.botGuids)
            if ((groupMember = ObjectAccessor::FindConnectedPlayer(guid)) != nullptr)
                break;

    if (groupMember)
        if (Group* group = groupMember->GetGroup())
            group->Disband();

    for (ObjectGuid const& guid : run.ctx.botGuids)
        sRandomPlayerbotMgr.LogoutPlayerBot(guid);
}

void IntegrationTestMgr::OnWorldStartup()
{
    std::string run = sConfigMgr->GetOption<std::string>("Playerbots.IntegrationTest.Run", "");
    if (run.empty())
        return;

    // Results must not depend on local config: enforce every setting here.
    sPlayerbotAIConfig.MinRandomBots = 0;             // no random-bot horde around the runs
    sPlayerbotAIConfig.MaxRandomBots = 0;
    sPlayerbotAIConfig.BotActiveAlone = 100;          // no activity throttling without real players
    sPlayerbotAIConfig.BotActiveAloneSmartScale = false;
    sPlayerbotAIConfig.DynamicReactDelay = false;     // master-equivalent combat cadence
    sPlayerbotAIConfig.ReactDelay = 40;
    sPlayerbotAIConfig.AutoGearBisCommand = 1;        // GearDef bis gearing
    sPlayerbotAIConfig.AutoGearQualityLimit = 4;
    sPlayerbotAIConfig.AutoGearScoreLimit = 999;
    sPlayerbotAIConfig.LogInGroupOnly = false;        // test bots are masterless; log them anyway

    // Quest scenarios pin RPG_DO_QUEST; every status re-roll must land back on
    // it instead of wandering off to grind/camp/rest.
    for (auto& [status, weight] : sPlayerbotAIConfig.RpgStatusProbWeight)
        weight = 0;
    sPlayerbotAIConfig.RpgStatusProbWeight[RPG_DO_QUEST] = 100;

    LOG_INFO("playerbots", "IntegrationTest: test config enforced (random bots off, full activity, "
        "react delay 40, autogear bis, rpg status pinned to DO_QUEST)");

    if (!QueueRun(run, true))
    {
        LOG_ERROR("playerbots", "IntegrationTest: could not queue configured run '{}'", run);
        World::StopNow(ERROR_EXIT_CODE);
    }
}

class IntegrationTestWorldScript : public WorldScript
{
public:
    IntegrationTestWorldScript()
        : WorldScript("IntegrationTestWorldScript", { WORLDHOOK_ON_STARTUP, WORLDHOOK_ON_UPDATE }) { }

    void OnStartup() override { IntegrationTestMgr::instance().OnWorldStartup(); }
    void OnUpdate(uint32 diff) override { IntegrationTestMgr::instance().Update(diff); }
};

using namespace Acore::ChatCommands;

class IntegrationTestCommandScript : public CommandScript
{
public:
    IntegrationTestCommandScript() : CommandScript("IntegrationTestCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable pbtestTable =
        {
            { "run",  HandleRunCommand,  SEC_GAMEMASTER, Console::Yes },
            { "list", HandleListCommand, SEC_GAMEMASTER, Console::Yes },
        };
        static ChatCommandTable commandTable =
        {
            { "pbtest", pbtestTable },
        };
        return commandTable;
    }

    static bool HandleRunCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->SendSysMessage("Usage: .pbtest run <scenario[,scenario...]>");
            return true;
        }
        if (!IntegrationTestMgr::instance().QueueRun(args, false))
            handler->SendSysMessage("pbtest: unknown scenario.");
        return true;
    }

    static bool HandleListCommand(ChatHandler* handler, char const* /*args*/)
    {
        for (std::string const& name : IntegrationTestMgr::instance().ScenarioNames())
            handler->SendSysMessage(name.c_str());
        handler->SendSysMessage("pbtest: end of scenario list.");
        return true;
    }
};

// One registration function per Scenarios/*Registrations.cpp file.
void RegisterGenericScenarios(IntegrationTestMgr& mgr);
void AddPlayerbotsTestCombatRecorder();

void AddPlayerbotsIntegrationTestScripts()
{
    new IntegrationTestWorldScript();
    new IntegrationTestCommandScript();
    AddPlayerbotsTestCombatRecorder();
    RegisterGenericScenarios(IntegrationTestMgr::instance());
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
