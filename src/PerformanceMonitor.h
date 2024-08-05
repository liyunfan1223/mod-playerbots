/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PERFORMANCEMONITOR_H
#define _PLAYERBOT_PERFORMANCEMONITOR_H

#include <chrono>
#include <ctime>
#include <map>
#include <mutex>
#include <vector>

#include "Common.h"

typedef std::vector<std::string> PerformanceStack;

struct PerformanceData
{
    uint64 minTime;
    uint64 maxTime;
    uint64 totalTime;
    uint32 count;
    std::mutex lock;
};

enum PerformanceMetric
{
    PERF_MON_TRIGGER,
    PERF_MON_VALUE,
    PERF_MON_ACTION,
    PERF_MON_RNDBOT,
    PERF_MON_TOTAL
};

class PerformanceMonitorOperation
{
public:
    PerformanceMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack);
    void finish();

private:
    PerformanceData* data;
    std::string const name;
    PerformanceStack* stack;
    std::chrono::microseconds started;
};

class PerformanceMonitor
{
public:
    PerformanceMonitor(){};
    virtual ~PerformanceMonitor(){};
    static PerformanceMonitor* instance()
    {
        static PerformanceMonitor instance;
        return &instance;
    }

public:
    PerformanceMonitorOperation* start(PerformanceMetric metric, std::string const name,
                                       PerformanceStack* stack = nullptr);
    void PrintStats(bool perTick = false, bool fullStack = false);
    void Reset();

private:
    std::map<PerformanceMetric, std::map<std::string, PerformanceData*> > data;
    std::mutex lock;
};

#define sPerformanceMonitor PerformanceMonitor::instance()

#endif
