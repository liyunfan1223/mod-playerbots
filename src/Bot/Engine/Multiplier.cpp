/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Multiplier.h"
#include "PerfMonitor.h"

PerformanceData* Multiplier::GetPerfData()
{
    if (!PerfMonitor::IsEnabled())
        return nullptr;

    if (!perfData)
        perfData = sPerfMonitor.acquire(PERF_MON_MULTIPLIER, getName());

    return perfData;
}

void Multiplier::NoteBlock() { PerfMonitor::CountBlock(GetPerfData()); }
