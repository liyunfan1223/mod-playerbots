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

#ifndef PLAYERBOTS_PERFMONITOR_H
#define PLAYERBOTS_PERFMONITOR_H

#include <chrono>
#include <cstdint>
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <vector>

typedef std::vector<std::string> PerformanceStack;

struct PerformanceData
{
    uint64_t minTime;
    uint64_t maxTime;
    uint64_t totalTime;
    uint64_t count;
    uint64_t blocks;
    std::mutex lock;
};

enum PerformanceMetric
{
    PERF_MON_TRIGGER,
    PERF_MON_VALUE,
    PERF_MON_ACTION,
    PERF_MON_MULTIPLIER,
    PERF_MON_RNDBOT,
    PERF_MON_TOTAL
};

class PerfMonitorOperation
{
public:
    PerfMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack);
    void finish();

private:
    PerformanceData* data;
    std::string const name;
    PerformanceStack* stack;
    std::chrono::microseconds started;
};

class PerfMonitorScope
{
public:
    explicit PerfMonitorScope(PerformanceData* data);
    ~PerfMonitorScope();

    PerfMonitorScope(PerfMonitorScope const&) = delete;
    PerfMonitorScope& operator=(PerfMonitorScope const&) = delete;

private:
    PerformanceData* data;
    std::chrono::microseconds started{};
};

class PerfMonitor
{
public:
    static PerfMonitor& instance()
    {
        static PerfMonitor instance;

        return instance;
    }

    static bool IsEnabled();

    PerfMonitorOperation* start(PerformanceMetric metric, std::string const name,
                                       PerformanceStack* stack = nullptr);
    PerformanceData* acquire(PerformanceMetric metric, std::string const& name);
    static void CountBlock(PerformanceData* data);
    void PrintStats(bool perTick = false, bool fullStack = false);
    void DumpJson(bool perTick = false);
    void Reset();

private:
    PerfMonitor() = default;
    virtual ~PerfMonitor() = default;

    PerfMonitor(PerfMonitor const&) = delete;
    PerfMonitor& operator=(PerfMonitor const&) = delete;

    PerfMonitor(PerfMonitor&&) = delete;
    PerfMonitor& operator=(PerfMonitor&&) = delete;

    PerformanceData* GetOrCreate(PerformanceMetric metric, std::string const& name);

    std::map<PerformanceMetric, std::map<std::string, PerformanceData*> > data;
    std::mutex lock;
};

#define sPerfMonitor PerfMonitor::instance()

#endif
