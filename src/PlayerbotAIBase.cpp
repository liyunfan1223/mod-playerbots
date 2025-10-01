/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIBase.h"

#include "Playerbots.h"

PlayerbotAIBase::PlayerbotAIBase(bool isBotAI) : nextAICheckDelay(0), _isBotAI(isBotAI) {}

void PlayerbotAIBase::UpdateAI(uint32 elapsed, bool minimal)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerformanceMonitor->start(PERF_MON_TOTAL, "PlayerbotAIBase::FullTick");

    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    if (!CanUpdateAI())
        return;

    UpdateAIInternal(elapsed, minimal);
    YieldThread();
}

void PlayerbotAIBase::SetNextCheckDelay(uint32 const delay)
{
    // if (nextAICheckDelay < delay)
    // LOG_DEBUG("playerbots", "Setting lesser delay {} -> {}", nextAICheckDelay, delay);

    nextAICheckDelay = delay;

    // if (nextAICheckDelay > sPlayerbotAIConfig->globalCoolDown)
    // LOG_DEBUG("playerbots",  "std::set next check delay: {}", nextAICheckDelay);
}

void PlayerbotAIBase::IncreaseNextCheckDelay(uint32 delay)
{
    nextAICheckDelay += delay;

    // if (nextAICheckDelay > sPlayerbotAIConfig->globalCoolDown)
    //     LOG_DEBUG("playerbots",  "increase next check delay: {}", nextAICheckDelay);
}

bool PlayerbotAIBase::CanUpdateAI() { return nextAICheckDelay == 0; }

void PlayerbotAIBase::YieldThread(uint32 delay)
{
    if (nextAICheckDelay < delay)
        nextAICheckDelay = delay;
}

bool PlayerbotAIBase::IsActive() { return nextAICheckDelay < sPlayerbotAIConfig->maxWaitForMove; }

bool PlayerbotAIBase::IsBotAI() const { return _isBotAI; }
