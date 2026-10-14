/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NaxxBossHelper.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "Timer.h"
#include <mutex>
#include <unordered_map>

namespace
{
    // A fight not observed for this long is over (wipe / evade); the next pull starts from scratch.
    constexpr uint32 HeiganStateStaleMs = 30000;

    // Fight clock shared by every bot fighting the same Heigan (keyed by the boss guid).
    struct HeiganFightRegistry
    {
        std::mutex lock;
        std::unordered_map<ObjectGuid, HeiganBossHelper::FightState> states;
    };

    HeiganFightRegistry& GetHeiganFightRegistry()
    {
        static HeiganFightRegistry registry;
        return registry;
    }
}

bool HeiganBossHelper::UpdateBossAI()
{
    if (_unit && (!_unit->IsInWorld() || !_unit->IsAlive() || !_unit->IsInCombat()))
        Reset();

    if (!_unit)
    {
        if (!bot->IsInCombat())
        {
            Reset();
            return false;
        }

        _unit = AI_VALUE2(Unit*, "find target", "heigan the unclean");
        if (!_unit)
            return false;
    }

    uint32 now = getMSTime();

    // Fast dance: Heigan teleports onto his platform, stops attacking (REACT_PASSIVE) and, one second later,
    // channels Plague Cloud for the rest of the phase. The aura is authoritative; "on the platform without a
    // victim" only bridges that first second (and a possibly interrupted channel).
    Aura* plagueCloud = _unit->GetAura(NaxxSpellIds::PlagueCloud);
    bool idleOnPlatform = !_unit->GetVictim() && _unit->IsWithinDist2d(PlatformX, PlatformY, PlatformRadius);

    HeiganFightRegistry& registry = GetHeiganFightRegistry();
    std::scoped_lock lock(registry.lock);

    // Drop stale entries (previous attempts / other instances that are long over).
    for (auto itr = registry.states.begin(); itr != registry.states.end();)
    {
        if (now - itr->second.lastSeenMs > HeiganStateStaleMs)
            itr = registry.states.erase(itr);
        else
            ++itr;
    }

    FightState& state = registry.states[_unit->GetGUID()];
    bool firstObservation = state.lastSeenMs == 0;
    uint32 elapsed = firstObservation ? 0 : now - state.phaseStartMs;

    bool fast = DetectFastPhase(state, plagueCloud != nullptr, idleOnPlatform, firstObservation, elapsed);
    UpdateFightState(state, fast, plagueCloud, firstObservation, now);

    _fastPhase = state.fastPhase;
    _phaseElapsedMs = now - state.phaseStartMs;
    return true;
}

bool HeiganBossHelper::DetectFastPhase(FightState const& state, bool plagueCloudUp, bool idleOnPlatform,
                                       bool firstObservation, uint32 elapsed) const
{
    if (plagueCloudUp)
        return true;

    // Still on the platform without a victim: the fast dance goes on (bridges the second before the aura).
    if (state.fastPhase)
        return idleOnPlatform;

    // A slow dance always lasts SlowPhaseDurationMs, so ignore an idle boss on the platform before that
    // (e.g. victim just died while he was still standing there at the pull).
    return idleOnPlatform && !firstObservation && elapsed + 5000 >= SlowPhaseDurationMs;
}

void HeiganBossHelper::UpdateFightState(FightState& state, bool fast, Aura const* plagueCloud,
                                        bool firstObservation, uint32 now)
{
    if (fast)
    {
        if (plagueCloud)
        {
            // Exact: the aura is applied PlagueCloudDelayMs into the phase and lasts FastPhaseDurationMs.
            int32 auraElapsed = plagueCloud->GetMaxDuration() - plagueCloud->GetDuration();
            if (auraElapsed < 0)
                auraElapsed = 0;
            state.phaseStartMs = now - PlagueCloudDelayMs - uint32(auraElapsed);
            state.exact = true;
        }
        else if (!state.fastPhase)
        {
            // Just teleported, Plague Cloud not up yet.
            state.phaseStartMs = now;
            state.exact = false;
        }
        state.fastPhase = true;
    }
    else
    {
        if (state.fastPhase)
        {
            // Fast -> slow: the fast dance lasts exactly FastPhaseDurationMs.
            state.phaseStartMs = state.exact ? state.phaseStartMs + FastPhaseDurationMs : now;
        }
        else if (firstObservation)
        {
            // Pull (or nobody has been watching this fight for a while).
            state.phaseStartMs = now;
            state.exact = false;
        }
        state.fastPhase = false;
    }
    state.lastSeenMs = now;
}

uint32 HeiganBossHelper::GetEruptionIndex() const
{
    uint32 first = _fastPhase ? FastFirstEruptionMs : SlowFirstEruptionMs;
    uint32 interval = _fastPhase ? FastEruptionIntervalMs : SlowEruptionIntervalMs;
    if (_phaseElapsedMs < first + EruptionSafetyMarginMs)
        return 0;
    return (_phaseElapsedMs - EruptionSafetyMarginMs - first) / interval + 1;
}

uint32 HeiganBossHelper::GetSafeWaypoint() const
{
    uint32 index = GetEruptionIndex();
    // After the last eruption of a phase the next spot is the first one of the following phase.
    if (index >= (_fastPhase ? FastEruptionCount : SlowEruptionCount))
        return 0;
    uint32 r = index % (2 * (WaypointCount - 1));
    return r < WaypointCount ? r : 2 * (WaypointCount - 1) - r;
}

uint32 HeiganBossHelper::GetMsUntilNextEruption() const
{
    uint32 first = _fastPhase ? FastFirstEruptionMs : SlowFirstEruptionMs;
    uint32 interval = _fastPhase ? FastEruptionIntervalMs : SlowEruptionIntervalMs;
    uint32 count = _fastPhase ? FastEruptionCount : SlowEruptionCount;
    uint32 happened = _phaseElapsedMs < first ? 0 : (_phaseElapsedMs - first) / interval + 1;
    uint32 next;
    if (happened >= count)
    {
        // No more eruptions this phase; the next one is the first of the following phase.
        uint32 phaseLength = _fastPhase ? FastPhaseDurationMs : SlowPhaseDurationMs;
        uint32 nextFirst = _fastPhase ? SlowFirstEruptionMs : FastFirstEruptionMs;
        next = phaseLength + nextFirst;
    }
    else
        next = first + happened * interval;

    return next > _phaseElapsedMs ? next - _phaseElapsedMs : 0;
}

bool HeiganBossHelper::IsAtWaypoint(uint32 index, float tolerance) const
{
    if (index >= WaypointCount)
        return false;
    return bot->GetDistance2d(WaypointX[index], WaypointY[index]) <= tolerance;
}

bool HeiganBossHelper::ShouldDance()
{
    if (_fastPhase)
        return true;
    if (PlayerbotAI::IsRanged(bot))
        return false;
    if (PlayerbotAI::IsMainTank(bot) && !AI_VALUE2(bool, "has aggro", "boss target"))
        return false;
    return true;
}

bool HeiganBossHelper::CanStandStillFor(uint32 ms)
{
    if (!ShouldDance())
        return true;
    return IsAtWaypoint(GetSafeWaypoint()) && GetMsUntilNextEruption() > ms;
}
