/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTOPERATION_H
#define PLAYERBOTS_PLAYERBOTOPERATION_H

#include "Common.h"
#include "ObjectGuid.h"
#include <memory>

/**
 * @brief Base class for thread-unsafe operations that must be executed in the world thread
 *
 * PlayerbotOperation represents an operation that needs to be deferred from a map thread
 * to the world thread for safe execution. Examples include group modifications, LFG operations,
 * guild operations, etc.
 *
 * Thread Safety:
 * - The constructor and data members must be thread-safe (use copies, not pointers)
 * - Execute() is called in the world thread and can safely perform thread-unsafe operations
 * - Subclasses must not store raw pointers to (core/world thread) game object (use ObjectGuid instead)
 */
class PlayerbotOperation
{
public:
    virtual ~PlayerbotOperation() = default;

    /**
     * @brief Execute this operation in the world thread
     *
     * This method is called by PlayerbotWorldThreadProcessor::Update() which runs in the world thread.
     * It's safe to perform any thread-unsafe operation here (Group, LFG, Guild, etc.)
     *
     * @return true if operation succeeded, false if it failed
     */
    virtual bool Execute() = 0;

    /**
     * @brief Get the bot GUID this operation is for (optional)
     *
     * Used for logging and debugging purposes.
     *
     * @return ObjectGuid of the bot, or ObjectGuid::Empty if not applicable
     */
    virtual ObjectGuid GetBotGuid() const { return ObjectGuid::Empty; }

    /**
     * @brief Get the operation priority (higher = more urgent)
     *
     * Priority levels:
     * - 100: Critical (crash prevention, cleanup operations)
     * - 50: High (player-facing operations like group invites)
     * - 10: Normal (background operations)
     * - 0: Low (statistics, logging)
     *
     * @return Priority value (0-100)
     */
    virtual uint32 GetPriority() const { return 10; }

    /**
     * @brief Get a human-readable name for this operation
     *
     * Used for logging and debugging.
     *
     * @return Operation name
     */
    virtual std::string GetName() const { return "Unknown Operation"; }

    /**
     * @brief Check if this operation is still valid
     *
     * Called before Execute() to check if the operation should still be executed.
     * For example, if a bot logged out, group invite operations for that bot can be skipped.
     *
     * @return true if operation should be executed, false to skip
     */
    virtual bool IsValid() const { return true; }
};

/**
 * @brief Comparison operator for priority queue (higher priority first)
 */
struct PlayerbotOperationComparator
{
    bool operator()(std::unique_ptr<PlayerbotOperation> const& a, std::unique_ptr<PlayerbotOperation> const& b) const
    {
        return a->GetPriority() < b->GetPriority();  // Lower priority goes to back of queue
    }
};

#endif
