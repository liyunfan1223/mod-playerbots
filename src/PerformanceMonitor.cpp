/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PerformanceMonitor.h"
#include "Playerbots.h"

PerformanceMonitorOperation* PerformanceMonitor::start(PerformanceMetric metric, std::string const name, PerformanceStack* stack)
{
    if (!sPlayerbotAIConfig->perfMonEnabled)
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

    std::lock_guard<std::mutex> guard(lock);
    PerformanceData* pd = data[metric][stackName];
    if (!pd)
    {
        pd = new PerformanceData();
        pd->minTime = 0;
        pd->maxTime = 0;
        pd->totalTime = 0;
        pd->count = 0;
        data[metric][stackName] = pd;
    }

    return new PerformanceMonitorOperation(pd, name, stack);
}

void PerformanceMonitor::PrintStats(bool perTick, bool fullStack)
{
    if (data.empty())
        return;

    uint32 total = 0;

    if (!perTick)
    {
        for (auto& map : data[PERF_MON_TOTAL])
            if (map.first.find("PlayerbotAI::UpdateAIInternal") != std::string::npos)
                total += map.second->totalTime;

        LOG_INFO("playerbots", "--------------------------------------[TOTAL BOT]------------------------------------------------------");
        LOG_INFO("playerbots", "percentage   time    |   min  ..    max (     avg  of     count ) - type : name                        ");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string key;
            switch (i->first)
            {
                case PERF_MON_TRIGGER:
                    key = "T";
                    break;
                case PERF_MON_VALUE:
                    key = "V";
                    break;
                case PERF_MON_ACTION:
                    key = "A";
                    break;
                case PERF_MON_RNDBOT:
                    key = "RndBot";
                    break;
                case PERF_MON_TOTAL:
                    key = "Total";
                    break;
                default:
                    key = "?";
                    break;
            }

            std::vector<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                if (key == "Total" && j->first.find("PlayerbotAI::UpdateAIInternal") == std::string::npos)
                    continue;

                names.push_back(j->first);
            }

            std::sort(names.begin(), names.end(), [pdMap](std::string const i, std::string const j)
            {
                return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime;
            });

           for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;
                float secs = (float)pd->totalTime / 1000.0f;
                float avg = (float)pd->totalTime / (float)pd->count;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";

                if (avg >= 0.5f || pd->maxTime > 10)
                {
                    LOG_INFO("playerbots", "%7.3f%% %10.3fs | %6u .. %6u (%9.4f of %10u) - {}    : {}"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , pd->count
                        , key.c_str()
                        , disName.c_str());
                }
            }
            LOG_INFO("playerbots", " ");
        }
    }
    else
    {
        float totalCount = data[PERF_MON_TOTAL]["RandomPlayerbotMgr::FullTick"]->count;
        total = data[PERF_MON_TOTAL]["RandomPlayerbotMgr::FullTick"]->totalTime;

        LOG_INFO("playerbots", " ");
        LOG_INFO("playerbots", " ");
        LOG_INFO("playerbots", "---------------------------------------[PER TICK]------------------------------------------------------");
        LOG_INFO("playerbots", "percentage   time    |   min  ..    max (     avg  of     count ) - type : name                        ");

        for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*>>::iterator i = data.begin(); i != data.end(); ++i)
        {
            std::map<std::string, PerformanceData*> pdMap = i->second;

            std::string key;
            switch (i->first)
            {
                case PERF_MON_TRIGGER:
                    key = "T";
                    break;
                case PERF_MON_VALUE:
                    key = "V";
                    break;
                case PERF_MON_ACTION:
                    key = "A";
                    break;
                case PERF_MON_RNDBOT:
                    key = "RndBot";
                    break;
                case PERF_MON_TOTAL:
                    key = "Total";
                    break;
                default:
                    key = "?";
            }

            std::vector<std::string> names;

            for (std::map<std::string, PerformanceData*>::iterator j = pdMap.begin(); j != pdMap.end(); ++j)
            {
                names.push_back(j->first);
            }

            std::sort(names.begin(), names.end(), [pdMap](std::string const i, std::string const j)
            {
                return pdMap.at(i)->totalTime < pdMap.at(j)->totalTime;
            });

            for (auto& name : names)
            {
                PerformanceData* pd = pdMap[name];
                float perc = (float)pd->totalTime / (float)total * 100.0f;
                uint32 secs = pd->totalTime / totalCount;
                float avg = (float)pd->totalTime / (float)pd->count;
                float amount = (float)pd->count / (float)totalCount;
                std::string disName = name;
                if (!fullStack && disName.find("|") != std::string::npos)
                    disName = disName.substr(0, disName.find("|")) + "]";

                if (avg >= 0.5f || pd->maxTime > 10)
                {
                    LOG_INFO("playerbots", "%7.3f%% %9ums | %6u .. %6u (%9.4f of %10.3f) - {}    : {}"
                        , perc
                        , secs
                        , pd->minTime
                        , pd->maxTime
                        , avg
                        , amount
                        , key.c_str()
                        , disName.c_str());
                }
            }
            LOG_INFO("playerbots", " ");
        }
    }
}

void PerformanceMonitor::Reset()
{
    for (std::map<PerformanceMetric, std::map<std::string, PerformanceData*> >::iterator i = data.begin(); i != data.end(); ++i)
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
        }
    }
}

PerformanceMonitorOperation::PerformanceMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack) : data(data), name(name), stack(stack)
{
    started = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
}

void PerformanceMonitorOperation::finish()
{
    std::chrono::milliseconds finished = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
    uint32 elapsed = (finished - started).count();

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

    if (stack)
    {
        stack->erase(std::remove(stack->begin(), stack->end(), name), stack->end());
    }

    delete this;
}

