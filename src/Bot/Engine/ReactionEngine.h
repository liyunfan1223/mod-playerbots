/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported from cmangos/playerbots
 */

#ifndef _PLAYERBOT_REACTIONENGINE_H
#define _PLAYERBOT_REACTIONENGINE_H

#include "Engine.h"

struct Reaction
{
    bool IsValid() const { return action != nullptr; }
    bool IsActive() const { return IsValid() ? duration > 100U : false; }
    void Reset() { action = nullptr; duration = 0U; }

    bool Update(uint32 elapsed);

    void SetAction(Action* inAction);
    Action* GetAction() const { return action; }

    void SetEvent(Event const& inEvent) { event = Event(inEvent); }
    Event& GetEvent() { return event; }

    void SetDuration(uint32 inDuration) { duration = inDuration; }
    bool ShouldInterruptCast() const { return action ? action->ShouldReactionInterruptCast() : false; }
    bool ShouldInterruptMovement() const { return action ? action->ShouldReactionInterruptMovement() : false; }

private:
    Event event;
    Action* action = nullptr;
    uint32 duration = 0U;
};

class ReactionEngine : public Engine
{
public:
    ReactionEngine(PlayerbotAI* botAI, AiObjectContext* factory);

    void Init() override;
    void ResetReactions();
    bool Update(uint32 elapsed, bool minimal, bool canControlSelf, bool& reactionFound);
    bool IsReacting() const { return ongoingReaction.IsValid(); }
    bool HasIncomingReaction() const { return incomingReaction.IsValid(); }
    void SetReactionDuration(Action const* action);
    Reaction const* GetReaction() const;

private:
    bool FindReaction(bool minimal, bool canControlSelf);
    float ApplyMainEngineMultipliers(Action* reaction, float relevance);
    bool StartReaction();
    void StopReaction();

    bool CanUpdateAIReaction() const;

    Action* InitializeAction(ActionNode* actionNode) override;

protected:
    Reaction incomingReaction;
    Reaction ongoingReaction;

private:
    uint32 aiReactionUpdateDelay;
};

#endif
