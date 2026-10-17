/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotAIBase.h"
#include "PerfMonitor.h"
#include "Playerbots.h"

PlayerbotAIBase::PlayerbotAIBase(bool isBotAI) : nextAICheckDelay(0), _isBotAI(isBotAI) {}

void PlayerbotAIBase::UpdateAI(uint32 elapsed, bool minimal)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerfMonitor.start(PERF_MON_TOTAL, "PlayerbotAIBase::FullTick");

    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    if (!CanUpdateAI())
        return;

    UpdateAIInternal(elapsed, minimal);
    YieldThread(nullptr);
}

void PlayerbotAIBase::SetNextCheckDelay(uint32 const delay)
{
    // if (nextAICheckDelay < delay)
    // LOG_DEBUG("playerbots", "Setting lesser delay {} -> {}", nextAICheckDelay, delay);

    nextAICheckDelay = delay;

    // if (nextAICheckDelay > sPlayerbotAIConfig.GlobalCoolDown)
    // LOG_DEBUG("playerbots",  "std::set next check delay: {}", nextAICheckDelay);
}

void PlayerbotAIBase::IncreaseNextCheckDelay(uint32 delay)
{
    nextAICheckDelay += delay;

    // if (nextAICheckDelay > sPlayerbotAIConfig.GlobalCoolDown)
    //     LOG_DEBUG("playerbots",  "increase next check delay: {}", nextAICheckDelay);
}

bool PlayerbotAIBase::CanUpdateAI() { return nextAICheckDelay == 0; }

void PlayerbotAIBase::YieldThread(Player* bot, uint32 delay)
{
    if (nextAICheckDelay < delay)
    {
        // Adding a deterministic per-bot slight offset (0–200 ms) to stagger updates and prevent cpu spikes.
        uint32 offset = bot ? (bot->GetGUID().GetCounter() % 201) : 0;
        nextAICheckDelay = delay + offset;
    }
}

bool PlayerbotAIBase::IsActive() { return nextAICheckDelay < sPlayerbotAIConfig.MaxWaitForMove; }

bool PlayerbotAIBase::IsBotAI() const { return _isBotAI; }

void PlayerbotAIBase::SetActionDuration(uint32 duration)
{
    nextAICheckDelay = duration;
}

void PlayerbotAIBase::ResetActionDuration()
{
    nextAICheckDelay = 0;
}
