/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDINSTANCESTATE_H
#define PLAYERBOTS_RAIDINSTANCESTATE_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

// Encounter state shared by the bots of one raid instance, dropped when its map is destroyed. Only the
// lookup is locked: one map updates on one thread at a time, but different maps run in parallel and
// can move between workers, so not thread_local. Don't hold a reference past the current tick.

namespace RaidInstanceStateRegistry
{
class Store
{
public:
    virtual void Drop(std::uint32_t instanceId) = 0;

protected:
    ~Store() = default;
};

struct Registry
{
    std::mutex mutex;
    std::vector<Store*> stores;
};

inline Registry& Get()
{
    static Registry registry;
    return registry;
}

inline void Add(Store* store)
{
    Registry& registry = Get();
    std::lock_guard<std::mutex> guard(registry.mutex);
    registry.stores.push_back(store);
}

inline void Remove(Store* store)
{
    Registry& registry = Get();
    std::lock_guard<std::mutex> guard(registry.mutex);
    registry.stores.erase(std::remove(registry.stores.begin(), registry.stores.end(), store),
                          registry.stores.end());
}
}  // namespace RaidInstanceStateRegistry

template <typename State>
class RaidInstanceState final : private RaidInstanceStateRegistry::Store
{
public:
    // Registered from the body, not from Store's constructor: a store built lazily can be reached
    // at once by a map being destroyed on another worker, so it has to be complete by then.
    RaidInstanceState() { RaidInstanceStateRegistry::Add(this); }
    ~RaidInstanceState() { RaidInstanceStateRegistry::Remove(this); }

    RaidInstanceState(RaidInstanceState const&) = delete;
    RaidInstanceState& operator=(RaidInstanceState const&) = delete;

    // Creates the entry on first use. A has-state check wants Find, or it brings back what a reset
    // just erased.
    State& For(std::uint32_t instanceId)
    {
        assert(instanceId != 0);
        std::lock_guard<std::mutex> guard(_mutex);
        return _states[instanceId];
    }

    State* Find(std::uint32_t instanceId)
    {
        assert(instanceId != 0);
        std::lock_guard<std::mutex> guard(_mutex);
        auto const itr = _states.find(instanceId);
        return itr == _states.end() ? nullptr : &itr->second;
    }

    void Reset(std::uint32_t instanceId)
    {
        assert(instanceId != 0);
        std::lock_guard<std::mutex> guard(_mutex);
        _states.erase(instanceId);
    }

private:
    void Drop(std::uint32_t instanceId) override
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _states.erase(instanceId);
    }

    std::mutex _mutex;
    std::unordered_map<std::uint32_t, State> _states;
};

// Erases the instance from every store.
inline void RaidInstanceStateDrop(std::uint32_t instanceId)
{
    RaidInstanceStateRegistry::Registry& registry = RaidInstanceStateRegistry::Get();
    std::lock_guard<std::mutex> guard(registry.mutex);
    for (RaidInstanceStateRegistry::Store* store : registry.stores)
        store->Drop(instanceId);
}

#endif
