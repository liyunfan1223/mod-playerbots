/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

// Result side of the test runner: damage/death recording, per-run result
// assembly, bench statistics and the JSON results file. Run lifecycle lives in
// TestRunner.cpp.

#include "TestRunner.h"

#include "Item.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "StringFormat.h"
#include "Unit.h"

#include <algorithm>
#include <fstream>

namespace
{
    constexpr size_t LAST_HITS_RING_SIZE = 10;
}

void IntegrationTestMgr::RecordDamage(Unit* attacker, Unit* victim, std::string attackerName, uint32 damage)
{
    if (!victim)
        return;

    std::lock_guard<std::mutex> guard(_participantLock);
    if (_participants.empty())
        return;

    // Damage a bot (or its pet) deals to a hostile counts toward that bot's run.
    if (attacker && victim->ToCreature())
    {
        ObjectGuid dealer = attacker->GetGUID();
        if (!dealer.IsPlayer())
            dealer = attacker->GetCharmerOrOwnerGUID();

        auto dealerIt = _participants.find(dealer);
        if (dealerIt != _participants.end())
        {
            RunCombatData& combat = dealerIt->second->combat;
            combat.damageDealt[dealer] += damage;
            if (Unit* owner = attacker->GetCharmerOrOwner())
                combat.attackerNames[dealer] = owner->GetName();
            else
                combat.attackerNames[dealer] = attacker->GetName();
        }
    }

    // Victim-side tracking: ring buffer + death recap for the owning run's bots
    // and watched boss.
    auto victimIt = _participants.find(victim->GetGUID());
    if (victimIt == _participants.end())
        return;

    TestRun& run = *victimIt->second;
    bool victimIsBot = victim->IsPlayer();

    // Attacker distance at hit time: separates AoE-radius wipes from room-wide mechanics.
    if (attacker && attacker != victim && attacker->IsInWorld() && victim->IsInWorld() &&
        attacker->GetMapId() == victim->GetMapId())
        attackerName += Acore::StringFormat("@{:.0f}y", attacker->GetDistance2d(victim));

    auto& ring = run.combat.lastHits[victim->GetGUID()];
    ring.push_back(RecordedHit{
        run.ctx.elapsedMs,
        std::move(attackerName),
        damage,
        victim->GetHealth() > damage ? uint32(victim->GetHealth() - damage) : 0u });
    if (ring.size() > LAST_HITS_RING_SIZE)
        ring.pop_front();

    // Bot death: flush a recap line (once per bot; pre-hook fires again on hits landing after death).
    if (victimIsBot && damage >= victim->GetHealth() && run.combat.reportedDeaths.insert(victim->GetGUID()).second)
    {
        ++run.combat.deaths;
        std::string recap = Acore::StringFormat("[{}s] {} died. Last hits:", run.ctx.elapsedMs / 1000, victim->GetName());
        for (RecordedHit const& hit : ring)
            recap += Acore::StringFormat(" | {}s {} {} (hp->{})", hit.atMs / 1000, hit.attacker, hit.amount, hit.healthAfter);
        run.combat.recapLines.push_back(recap);
    }
}

void IntegrationTestMgr::FinishRun(TestRun& run, bool passed, std::string const& failedStep)
{
    ScenarioResult result;
    result.name = run.label;
    result.passed = passed;
    result.failedStep = failedStep;
    result.elapsedMs = run.ctx.elapsedMs;
    result.metrics = run.ctx.metrics;
    result.notes = run.ctx.notes;
    result.timeline = std::move(run.timeline);

    {
        std::lock_guard<std::mutex> guard(_participantLock);

        result.metrics["bot_deaths"] = run.combat.deaths;
        for (std::string const& line : run.combat.recapLines)
            result.notes.push_back(line);

        if (!passed)
        {
            if (!result.timeline.empty())
            {
                auto const& last = result.timeline.back();
                result.notes.push_back(Acore::StringFormat("fail state: [{:.0f}s] boss {:.0f}% alive {:.0f}",
                    last[0], last[1], last[2]));
            }

            auto bossHits = run.combat.lastHits.find(run.syncedBoss);
            if (bossHits != run.combat.lastHits.end() && !bossHits->second.empty())
            {
                std::string line = "boss last hits:";
                for (RecordedHit const& hit : bossHits->second)
                    line += Acore::StringFormat(" | {}s {} {} (hp->{})",
                        hit.atMs / 1000, hit.attacker, hit.amount, hit.healthAfter);
                result.notes.push_back(line);
            }
        }

        uint32 fightMs = run.ctx.elapsedMs;
        auto engageIt = run.ctx.metrics.find("engage_at_ms");
        if (engageIt != run.ctx.metrics.end() && run.ctx.elapsedMs > uint32(engageIt->second))
            fightMs = run.ctx.elapsedMs - uint32(engageIt->second);

        HarvestBenchStats(run, passed, fightMs);

        // Damage per raid member, highest first.
        if (!run.combat.damageDealt.empty())
        {
            std::vector<std::pair<uint64, ObjectGuid>> sorted;
            uint64 total = 0;
            for (auto const& [guid, dmg] : run.combat.damageDealt)
            {
                sorted.push_back({ dmg, guid });
                total += dmg;
            }
            std::sort(sorted.rbegin(), sorted.rend());

            std::map<ObjectGuid, std::string const*> specOf;
            for (size_t i = 0; i < run.ctx.botGuids.size() && i < run.ctx.botSpecs.size(); ++i)
                specOf[run.ctx.botGuids[i]] = &run.ctx.botSpecs[i];

            for (auto const& [dmg, guid] : sorted)
            {
                auto specIt = specOf.find(guid);
                result.notes.push_back(Acore::StringFormat("dmg: {} [{}] {} (dps {})",
                    run.combat.attackerNames[guid], specIt != specOf.end() ? *specIt->second : "?",
                    dmg, fightMs >= 1000 ? dmg / (fightMs / 1000) : dmg));
            }
            result.notes.push_back(Acore::StringFormat("dmg total: {} (raid dps {})",
                total, fightMs >= 1000 ? total / (fightMs / 1000) : total));
        }

        // Drop this run's participants from the routing index.
        for (ObjectGuid const& guid : run.ctx.botGuids)
            _participants.erase(guid);
        if (!run.syncedBoss.IsEmpty())
            _participants.erase(run.syncedBoss);
    }

    _results.push_back(std::move(result));

    if (!passed)
        _anyFailed = true;

    LOG_INFO("playerbots", "IntegrationTest: === {} {} ({} ms) ===",
        run.label, passed ? "PASSED" : "FAILED", run.ctx.elapsedMs);

    Teardown(run);
    run.scenario = nullptr;  // marks the run for removal in Update()
    WriteResults();

    // Bench slot advance: each finished run frees a slot; the summary lands
    // once the whole series is done.
    if (BenchState* bench = run.bench)
    {
        ++bench->done;
        if (bench->done == bench->total)
            FinishBench(*bench);
        PumpBenches();
    }
}

// Average equipped item level (shirt/tabard excluded).
static uint32 AverageEquippedItemLevel(Player* bot)
{
    uint32 sum = 0, count = 0;
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
            continue;
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            sum += item->GetTemplate()->ItemLevel;
            ++count;
        }
    }
    return count ? sum / count : 0;
}

void IntegrationTestMgr::HarvestBenchStats(TestRun& run, bool passed, uint32 fightMs)
{
    BenchState* bench = run.bench;
    if (!bench)
        return;

    if (passed)
        if (auto it = run.ctx.metrics.find("kill_time_ms"); it != run.ctx.metrics.end())
            bench->killTimesSec.push_back(it->second / 1000.0);
    if (passed)
        ++bench->passCount;
    bench->deaths.push_back(run.combat.deaths);

    if (fightMs < 1000)
        return;

    for (size_t i = 0; i < run.ctx.botGuids.size() && i < run.ctx.botSpecs.size(); ++i)
    {
        auto dmgIt = run.combat.damageDealt.find(run.ctx.botGuids[i]);
        if (dmgIt == run.combat.damageDealt.end())
            continue;

        std::string const& spec = run.ctx.botSpecs[i];
        bench->specDps[spec].push_back(double(dmgIt->second) / (fightMs / 1000.0));
        if (Player* bot = run.ctx.GetBot(i))
            bench->specIlvl[spec].push_back(AverageEquippedItemLevel(bot));
    }
}

void IntegrationTestMgr::FinishBench(BenchState& bench)
{
    auto stats = [](std::vector<double> const& v) -> std::array<double, 3>
    {
        if (v.empty())
            return { 0, 0, 0 };
        double sum = 0, mn = v[0], mx = v[0];
        for (double x : v) { sum += x; mn = std::min(mn, x); mx = std::max(mx, x); }
        return { sum / v.size(), mn, mx };
    };

    ScenarioResult result;
    result.name = Acore::StringFormat("bench {}", bench.scenarioName);
    result.passed = bench.passCount == bench.total;

    auto kt = stats(bench.killTimesSec);
    double deathSum = 0;
    for (uint32 d : bench.deaths)
        deathSum += d;
    result.notes.push_back(Acore::StringFormat(
        "runs {}: {} passed | kill avg {:.0f}s min {:.0f}s max {:.0f}s | deaths avg {:.1f}",
        bench.total, bench.passCount, kt[0], kt[1], kt[2],
        bench.deaths.empty() ? 0.0 : deathSum / bench.deaths.size()));

    // Per spec, strongest first.
    std::vector<std::pair<double, std::string>> order;
    for (auto const& [spec, samples] : bench.specDps)
        order.push_back({ stats(samples)[0], spec });
    std::sort(order.rbegin(), order.rend());

    for (auto const& [avg, spec] : order)
    {
        auto d = stats(bench.specDps[spec]);
        auto const& ilvls = bench.specIlvl[spec];
        uint64 ilvlSum = 0;
        for (uint32 v : ilvls)
            ilvlSum += v;
        result.notes.push_back(Acore::StringFormat(
            "spec {:<22} x{}: dps avg {:.0f} min {:.0f} max {:.0f} | ilvl {}",
            spec, bench.specDps[spec].size(), d[0], d[1], d[2],
            ilvls.empty() ? 0 : uint32(ilvlSum / ilvls.size())));
    }

    LOG_INFO("playerbots", "IntegrationTest: === bench {} finished ===", bench.scenarioName);
    for (std::string const& line : result.notes)
        LOG_INFO("playerbots", "IntegrationTest:   {}", line);

    if (!result.passed)
        _anyFailed = true;

    _results.push_back(std::move(result));
    WriteResults();
}

namespace
{
    std::string JsonEscape(std::string const& in)
    {
        std::string out;
        for (char c : in)
        {
            switch (c)
            {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    // Remaining control characters are invalid raw JSON.
                    if (uint8(c) < 0x20)
                        out += Acore::StringFormat("\\u{:04x}", uint32(uint8(c)));
                    else
                        out += c;
            }
        }
        return out;
    }
}

void IntegrationTestMgr::WriteResults() const
{
    std::ofstream file("test_results.json", std::ios::trunc);
    if (!file)
    {
        LOG_ERROR("playerbots", "IntegrationTest: cannot open test_results.json for writing");
        return;
    }

    file << "{\n  \"results\": [\n";
    for (size_t i = 0; i < _results.size(); ++i)
    {
        ScenarioResult const& r = _results[i];
        file << "    {\n";
        file << "      \"name\": \"" << JsonEscape(r.name) << "\",\n";
        file << "      \"passed\": " << (r.passed ? "true" : "false") << ",\n";
        file << "      \"failedStep\": \"" << JsonEscape(r.failedStep) << "\",\n";
        file << "      \"elapsedMs\": " << r.elapsedMs << ",\n";
        file << "      \"metrics\": {";
        size_t m = 0;
        for (auto const& [key, value] : r.metrics)
            file << (m++ ? ", " : "") << "\"" << JsonEscape(key) << "\": " << value;
        file << "},\n";
        file << "      \"notes\": [";
        for (size_t n = 0; n < r.notes.size(); ++n)
            file << (n ? ", " : "") << "\"" << JsonEscape(r.notes[n]) << "\"";
        file << "],\n";
        file << "      \"timeline\": [";
        for (size_t t = 0; t < r.timeline.size(); ++t)
            file << (t ? ", " : "") << "[" << r.timeline[t][0] << ", " << r.timeline[t][1] << ", " << r.timeline[t][2] << "]";
        file << "]\n    }" << (i + 1 < _results.size() ? "," : "") << "\n";
    }
    file << "  ]\n}\n";
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
