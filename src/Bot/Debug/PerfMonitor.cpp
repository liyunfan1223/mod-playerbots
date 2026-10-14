/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   ike3 <ike@email.org> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 */

#include "PerfMonitor.h"
#include "Config.h"
#include "Playerbots.h"
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace
{
std::string MetricName(PerformanceMetric metric)
{
    switch (metric)
    {
        case PERF_MON_TRIGGER:
            return "Trigger";
        case PERF_MON_VALUE:
            return "Value";
        case PERF_MON_ACTION:
            return "Action";
        case PERF_MON_MULTIPLIER:
            return "Mult";
        case PERF_MON_RNDBOT:
            return "RndBot";
        case PERF_MON_TOTAL:
            return "Total";
        default:
            return "?";
    }
}

std::string JsonEscape(std::string const& value)
{
    std::ostringstream out;
    for (char c : value)
    {
        switch (c)
        {
            case '"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\b':
                out << "\\b";
                break;
            case '\f':
                out << "\\f";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<uint32>(c)
                        << std::dec;
                else
                    out << c;
                break;
        }
    }

    return out.str();
}

std::string JsonNumber(double value)
{
    if (!std::isfinite(value))
        value = 0.0;

    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << value;

    return out.str();
}

std::string JsonRatio(double value, double total)
{
    return JsonNumber(total > 0.0 ? value / total : 0.0);
}

std::string JsonPath(bool perTick)
{
    std::string dir = sConfigMgr->GetOption<std::string>("LogsDir", "", false);
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
        dir.push_back('/');

    return dir + (perTick ? "pmon_tick.json" : "pmon_total.json");
}
}  // namespace

namespace
{
std::chrono::microseconds Now()
{
    return (std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()))
        .time_since_epoch();
}

void RecordSample(PerformanceData* data, uint64 elapsed)
{
    std::lock_guard<std::mutex> guard(data->lock);
    if (elapsed > 0)
    {
        if (!data->minTime || data->minTime > elapsed)
            data->minTime = elapsed;

        if (!data->maxTime || data->maxTime < elapsed)
            data->maxTime = elapsed;

        data->totalTime += elapsed;
    }

    ++data->count;
}
}  // namespace

bool PerfMonitor::IsEnabled() { return sPlayerbotAIConfig.PerfMonEnabled; }

PerformanceData* PerfMonitor::GetOrCreate(PerformanceMetric metric, std::string const& name)
{
    std::lock_guard<std::mutex> guard(lock);
    PerformanceData*& pd = data[metric][name];
    if (!pd)
    {
        pd = new PerformanceData();
        pd->minTime = 0;
        pd->maxTime = 0;
        pd->totalTime = 0;
        pd->count = 0;
        pd->blocks = 0;
    }

    return pd;
}

PerformanceData* PerfMonitor::acquire(PerformanceMetric metric, std::string const& name)
{
    if (!IsEnabled())
        return nullptr;

    return GetOrCreate(metric, name);
}

void PerfMonitor::CountBlock(PerformanceData* data)
{
    if (!data)
        return;

    std::lock_guard<std::mutex> guard(data->lock);
    ++data->blocks;
}

PerfMonitorOperation* PerfMonitor::start(PerformanceMetric metric, std::string const name,
                                                       PerformanceStack* stack)
{
    if (!IsEnabled())
        return nullptr;

    std::string stackName = name;

    if (stack)
    {
        if (!stack->empty())
        {
            std::ostringstream out;
            out << stackName << " [";

            for (std::vector<std::string>::reverse_iterator i = stack->rbegin(); i != stack->rend(); ++i)
                out << *i << (std::next(i) == stack->rend() ? "" : "|");

            out << "]";

            stackName = out.str().c_str();
        }

        stack->push_back(name);
    }

    return new PerfMonitorOperation(GetOrCreate(metric, stackName), name, stack);
}

void PerfMonitor::PrintStats(bool perTick, bool fullStack)
{
    if (data.empty())
        return;

    if (!perTick)
    {
        float updateAITotalTime = 0;
        for (auto& map : data[PERF_MON_TOTAL])
            if (map.first.find("PlayerbotAI::UpdateAIInternal") != std::string::npos)
                updateAITotalTime += map.second->totalTime;

        LOG_INFO(
            "playerbots",
            "--------------------------------------[TOTAL BOT]------------------------------------------------------");
        LOG_INFO("playerbots",
                 "percentage     time  |     min ..     max (      avg  of      count) - type      : name");
        LOG_INFO(
            "playerbots",
            "-------------------------------------------------------------------------------------------------------");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin();
             i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string const key = MetricName(i->first);

            std::vector<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                if (key == "Total" && j->first.find("PlayerbotAI::UpdateAIInternal") == std::string::npos)
                    continue;

                names.push_back(j->first);
            }

            std::sort(names.begin(), names.end(),
                      [pdMap](std::string const i, std::string const j)
                      { return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime; });

            uint64 typeTotalTime = 0;
            uint64 typeMinTime = 0xffffffffu;
            uint64 typeMaxTime = 0;
            uint64 typeCount = 0;
            uint64 typeBlocks = 0;
            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                typeTotalTime += pd->totalTime;
                typeCount += pd->count;
                typeBlocks += pd->blocks;
                if (typeMinTime > pd->minTime)
                    typeMinTime = pd->minTime;
                if (typeMaxTime < pd->maxTime)
                    typeMaxTime = pd->maxTime;
                float perc = (float)pd->totalTime / updateAITotalTime * 100.0f;
                float time = (float)pd->totalTime / 1000000.0f;
                float minTime = (float)pd->minTime / 1000.0f;
                float maxTime = (float)pd->maxTime / 1000.0f;
                float avg = (float)pd->totalTime / (float)pd->count / 1000.0f;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";

                bool show = perc >= 0.1f || avg >= 0.25f || pd->maxTime > 1000;
                if (i->first == PERF_MON_MULTIPLIER)
                {
                    if (pd->blocks)
                        disName += " [blocked " + std::to_string(pd->blocks) + "]";

                    show = show || pd->blocks > 0 || perc >= 0.01f;
                }

                if (show)
                {
                    LOG_INFO("playerbots",
                             "{:7.3f}% {:10.3f}s | {:7.1f} .. {:7.1f} ({:10.3f} of {:10d}) - {:6}    : {}", perc, time,
                             minTime, maxTime, avg, pd->count, key.c_str(), disName.c_str());
                }
            }
            float tPerc = (float)typeTotalTime / (float)updateAITotalTime * 100.0f;
            float tTime = (float)typeTotalTime / 1000000.0f;
            float tMinTime = (float)typeMinTime / 1000.0f;
            float tMaxTime = (float)typeMaxTime / 1000.0f;
            float tAvg = (float)typeTotalTime / (float)typeCount / 1000.0f;
            std::string totalName = "Total";
            if (i->first == PERF_MON_MULTIPLIER && typeBlocks)
                totalName += " [blocked " + std::to_string(typeBlocks) + "]";

            LOG_INFO("playerbots", "{:7.3f}% {:10.3f}s | {:7.1f} .. {:7.1f} ({:10.3f} of {:10d}) - {:6}    : {}", tPerc,
                     tTime, tMinTime, tMaxTime, tAvg, typeCount, key.c_str(), totalName.c_str());
            LOG_INFO("playerbots", " ");
        }
    }
    else
    {
        float fullTickCount = data[PERF_MON_TOTAL]["PlayerbotAIBase::FullTick"]->count;
        float fullTickTotalTime = data[PERF_MON_TOTAL]["PlayerbotAIBase::FullTick"]->totalTime;

        LOG_INFO(
            "playerbots",
            "---------------------------------------[PER TICK]------------------------------------------------------");
        LOG_INFO("playerbots",
                 "percentage     time  |     min ..     max (      avg  of      count) - type      : name");
        LOG_INFO(
            "playerbots",
            "-------------------------------------------------------------------------------------------------------");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin();
             i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string const key = MetricName(i->first);

            std::vector<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                names.push_back(j->first);
            }

            std::sort(names.begin(), names.end(),
                      [pdMap](std::string const i, std::string const j)
                      { return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime; });

            uint64 typeTotalTime = 0;
            uint64 typeMinTime = 0xffffffffu;
            uint64 typeMaxTime = 0;
            uint64 typeCount = 0;
            uint64 typeBlocks = 0;
            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                typeTotalTime += pd->totalTime;
                typeCount += pd->count;
                typeBlocks += pd->blocks;
                if (typeMinTime > pd->minTime)
                    typeMinTime = pd->minTime;
                if (typeMaxTime < pd->maxTime)
                    typeMaxTime = pd->maxTime;
                float perc = (float)pd->totalTime / fullTickTotalTime * 100.0f;
                float time = (float)pd->totalTime / fullTickCount / 1000.0f;
                float minTime = (float)pd->minTime / 1000.0f;
                float maxTime = (float)pd->maxTime / 1000.0f;
                float avg = (float)pd->totalTime / (float)pd->count / 1000.0f;
                float amount = (float)pd->count / fullTickCount;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";
                bool show = perc >= 0.1f || avg >= 0.25f || pd->maxTime > 1000;
                if (i->first == PERF_MON_MULTIPLIER)
                {
                    if (pd->blocks)
                        disName += " [blocked " + std::to_string(pd->blocks) + "]";

                    show = show || pd->blocks > 0 || perc >= 0.01f;
                }

                if (show)
                {
                    LOG_INFO("playerbots",
                             "{:7.3f}% {:9.3f}ms | {:7.1f} .. {:7.1f} ({:10.3f} of {:10.2f}) - {:6}    : {}", perc,
                             time, minTime, maxTime, avg, amount, key.c_str(), disName.c_str());
                }
            }
            if (i->first != PERF_MON_TOTAL)
            {
                float tPerc = (float)typeTotalTime / (float)fullTickTotalTime * 100.0f;
                float tTime = (float)typeTotalTime / fullTickCount / 1000.0f;
                float tMinTime = (float)typeMinTime / 1000.0f;
                float tMaxTime = (float)typeMaxTime / 1000.0f;
                float tAvg = (float)typeTotalTime / (float)typeCount / 1000.0f;
                float tAmount = (float)typeCount / fullTickCount;
                std::string totalName = "Total";
                if (i->first == PERF_MON_MULTIPLIER && typeBlocks)
                    totalName += " [blocked " + std::to_string(typeBlocks) + "]";

                LOG_INFO("playerbots", "{:7.3f}% {:9.3f}ms | {:7.1f} .. {:7.1f} ({:10.3f} of {:10.2f}) - {:6}    : {}",
                         tPerc, tTime, tMinTime, tMaxTime, tAvg, tAmount, key.c_str(), totalName.c_str());
            }
            LOG_INFO("playerbots", " ");
        }
    }
}

void PerfMonitor::DumpJson(bool perTick)
{
    std::map<PerformanceMetric, std::map<std::string, PerformanceData*>> snapshot;
    {
        std::lock_guard<std::mutex> guard(lock);
        snapshot = data;
    }

    if (snapshot.empty())
        return;

    struct Sample
    {
        std::string name;
        uint64 totalTime;
        uint64 minTime;
        uint64 maxTime;
        uint64 count;
        uint64 blocks;
    };

    std::map<PerformanceMetric, std::vector<Sample>> samples;
    for (auto const& metric : snapshot)
    {
        std::vector<Sample>& rows = samples[metric.first];
        for (auto const& entry : metric.second)
        {
            PerformanceData* pd = entry.second;
            if (!pd)
                continue;

            std::lock_guard<std::mutex> guard(pd->lock);
            rows.push_back({entry.first, pd->totalTime, pd->minTime, pd->maxTime, pd->count, pd->blocks});
        }

        std::sort(rows.begin(), rows.end(),
                  [](Sample const& i, Sample const& j) { return i.totalTime > j.totalTime; });
    }

    double updateAiTotalTime = 0.0;
    double fullTickTotalTime = 0.0;
    double fullTickCount = 0.0;
    auto const totals = samples.find(PERF_MON_TOTAL);
    if (totals != samples.end())
    {
        for (Sample const& row : totals->second)
        {
            if (row.name.find("PlayerbotAI::UpdateAIInternal") != std::string::npos)
                updateAiTotalTime += static_cast<double>(row.totalTime);

            if (row.name == "PlayerbotAIBase::FullTick")
            {
                fullTickTotalTime = static_cast<double>(row.totalTime);
                fullTickCount = static_cast<double>(row.count);
            }
        }
    }

    double const denominator = perTick ? fullTickTotalTime : updateAiTotalTime;

    char stamp[32] = "";
    std::time_t const now = std::time(nullptr);
    if (std::tm const* utc = std::gmtime(&now))
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%SZ", utc);

    std::ostringstream out;
    out << "{\n";
    out << "  \"generatedAt\": " << static_cast<uint64>(now) << ",\n";
    out << "  \"generatedAtUtc\": \"" << stamp << "\",\n";
    out << "  \"mode\": \"" << (perTick ? "tick" : "total") << "\",\n";
    out << "  \"enabled\": " << (sPlayerbotAIConfig.PerfMonEnabled ? "true" : "false") << ",\n";
    out << "  \"timeUnit\": \"microseconds\",\n";
    out << "  \"fullTickCount\": " << static_cast<uint64>(fullTickCount) << ",\n";
    out << "  \"fullTickTotalTime\": " << static_cast<uint64>(fullTickTotalTime) << ",\n";
    out << "  \"updateAiTotalTime\": " << static_cast<uint64>(updateAiTotalTime) << ",\n";
    out << "  \"metrics\": [\n";

    bool firstMetric = true;
    for (auto const& metric : samples)
    {
        if (!firstMetric)
            out << ",\n";
        firstMetric = false;

        out << "    {\n";
        out << "      \"type\": \"" << MetricName(metric.first) << "\",\n";

        if (metric.first != PERF_MON_TOTAL)
        {
            uint64 typeTotalTime = 0;
            uint64 typeMinTime = 0;
            uint64 typeMaxTime = 0;
            uint64 typeCount = 0;
            uint64 typeBlocks = 0;
            bool firstSample = true;
            for (Sample const& row : metric.second)
            {
                typeTotalTime += row.totalTime;
                typeCount += row.count;
                typeBlocks += row.blocks;
                if (firstSample || typeMinTime > row.minTime)
                    typeMinTime = row.minTime;
                if (typeMaxTime < row.maxTime)
                    typeMaxTime = row.maxTime;
                firstSample = false;
            }

            out << "      \"totalTime\": " << typeTotalTime << ",\n";
            out << "      \"count\": " << typeCount << ",\n";
            out << "      \"blocks\": " << typeBlocks << ",\n";
            out << "      \"minTime\": " << typeMinTime << ",\n";
            out << "      \"maxTime\": " << typeMaxTime << ",\n";
            out << "      \"avgTime\": "
                << JsonRatio(static_cast<double>(typeTotalTime), static_cast<double>(typeCount)) << ",\n";
            out << "      \"percent\": "
                << JsonNumber(denominator > 0.0 ? static_cast<double>(typeTotalTime) / denominator * 100.0 : 0.0)
                << ",\n";
            out << "      \"timePerTick\": " << JsonRatio(static_cast<double>(typeTotalTime), fullTickCount) << ",\n";
            out << "      \"callsPerTick\": " << JsonRatio(static_cast<double>(typeCount), fullTickCount) << ",\n";
        }

        out << "      \"rows\": [\n";

        bool firstRow = true;
        for (Sample const& row : metric.second)
        {
            if (!firstRow)
                out << ",\n";
            firstRow = false;

            out << "        {\"name\": \"" << JsonEscape(row.name) << "\"";
            out << ", \"totalTime\": " << row.totalTime;
            out << ", \"count\": " << row.count;
            out << ", \"blocks\": " << row.blocks;
            out << ", \"minTime\": " << row.minTime;
            out << ", \"maxTime\": " << row.maxTime;
            out << ", \"avgTime\": " << JsonRatio(static_cast<double>(row.totalTime), static_cast<double>(row.count));
            out << ", \"percent\": "
                << JsonNumber(denominator > 0.0 ? static_cast<double>(row.totalTime) / denominator * 100.0 : 0.0);
            out << ", \"timePerTick\": " << JsonRatio(static_cast<double>(row.totalTime), fullTickCount);
            out << ", \"callsPerTick\": " << JsonRatio(static_cast<double>(row.count), fullTickCount);
            out << "}";
        }

        out << (firstRow ? "" : "\n") << "      ]\n";
        out << "    }";
    }

    out << (firstMetric ? "" : "\n") << "  ]\n";
    out << "}\n";

    std::string const path = JsonPath(perTick);
    std::string const tempPath = path + ".tmp";

    std::ofstream file(tempPath.c_str(), std::ios::out | std::ios::trunc);
    if (!file)
    {
        LOG_ERROR("playerbots", "Performance monitor could not write {}", tempPath);
        return;
    }

    file << out.str();
    file.close();

    if (!file)
    {
        LOG_ERROR("playerbots", "Performance monitor failed to write {}", tempPath);

        std::error_code removeError;
        std::filesystem::remove(tempPath, removeError);
        return;
    }

    std::error_code renameError;
    std::filesystem::rename(tempPath, path, renameError);
    if (renameError)
    {
        LOG_ERROR("playerbots", "Performance monitor could not replace {}: {}", path, renameError.message());

        std::error_code removeError;
        std::filesystem::remove(tempPath, removeError);
        return;
    }

    LOG_INFO("playerbots", "Performance monitor dump written to {}", path);
}

void PerfMonitor::Reset()
{
    for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin();
         i != data.end(); ++i)
    {
        std::map<std::string, PerformanceData*> pdMap = i->second;
        for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
        {
            PerformanceData* pd = j->second;
            std::lock_guard<std::mutex> guard(pd->lock);
            pd->minTime = 0;
            pd->maxTime = 0;
            pd->totalTime = 0;
            pd->count = 0;
            pd->blocks = 0;
        }
    }
}

PerfMonitorOperation::PerfMonitorOperation(PerformanceData* data, std::string const name,
                                                         PerformanceStack* stack)
    : data(data), name(name), stack(stack), started(Now())
{
}

void PerfMonitorOperation::finish()
{
    RecordSample(data, (Now() - started).count());

    if (stack)
    {
        stack->erase(std::remove(stack->begin(), stack->end(), name), stack->end());
    }

    delete this;
}

PerfMonitorScope::PerfMonitorScope(PerformanceData* data) : data(data)
{
    if (!data)
        return;

    started = Now();
}

PerfMonitorScope::~PerfMonitorScope()
{
    if (!data)
        return;

    RecordSample(data, (Now() - started).count());
}
