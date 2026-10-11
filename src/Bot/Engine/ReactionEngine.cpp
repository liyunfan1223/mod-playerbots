/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported from cmangos/playerbots
 */

#include "ReactionEngine.h"

#include "Action.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Queue.h"
#include "Strategy.h"

void Reaction::SetAction(Action* inAction)
{
    if (inAction)
    {
        SetDuration(inAction->GetDuration());
        action = inAction;
    }
}

bool Reaction::Update(uint32 elapsed)
{
    duration = duration > elapsed ? duration - elapsed : 0;
    return !IsActive();
}

ReactionEngine::ReactionEngine(PlayerbotAI* botAI, AiObjectContext* factory)
    : Engine(botAI, factory), aiReactionUpdateDelay(0U)
{
}

void ReactionEngine::Init()
{
    Reset();

    hasTargetExclusions = false;
    for (auto& pair : strategies)
    {
        Strategy* strategy = pair.second;
        strategyTypeMask |= strategy->GetType();
        hasTargetExclusions |= strategy->HasTargetExclusions();
        strategy->InitReactionMultipliers(multipliers);
        strategy->InitReactionTriggers(triggers);
        for (auto& iter : strategy->actionNodeFactories.creators)
        {
            actionNodeFactories.creators[iter.first] = iter.second;
        }
    }
}

bool ReactionEngine::FindReaction(bool minimal, bool canControlSelf)
{
    if (!IsReacting())
    {
        // Skip on a taxi: handling commands here would consume them, and most reactions can't run anyway.
        if (canControlSelf)
            botAI->HandleCommands();

        ProcessTriggers(minimal);

        ActionBasket* reactionItem = nullptr;

        int iterations = 0;
        int iterationsPerTick = queue.Size() * sPlayerbotAIConfig.IterationsPerTick;
        do
        {
            reactionItem = queue.Peek();
            if (reactionItem)
            {
                bool const skipReactionPrerequisites = reactionItem->isSkipPrerequisites();
                float reactionRelevance = reactionItem->getRelevance();
                Event const reactionEvent = reactionItem->getEvent();

                // Pop() removes the basket Peek() returned and deletes it; the node is ours.
                ActionNode* reactionNode = queue.Pop();
                if (reactionNode)
                {
                    Action* reaction = InitializeAction(reactionNode);
                    if (reaction)
                    {
                        reaction->setRelevance(reactionRelevance);

                        if (reaction->isUseful() && (canControlSelf || reaction->isUsefulWithoutControl()))
                        {
                            for (Multiplier* multiplier : multipliers)
                            {
                                reactionRelevance *= multiplier->GetValue(reaction);
                                reaction->setRelevance(reactionRelevance);
                                if (reactionRelevance <= 0.0f)
                                    break;
                            }

                            if (reactionRelevance > 0.0f)
                                reactionRelevance = ApplyMainEngineMultipliers(reaction, reactionRelevance);

                            if ((reactionRelevance > 0.0f) && reaction->isPossible())
                            {
                                if (!skipReactionPrerequisites &&
                                    MultiplyAndPush(reactionNode->getPrerequisites(), reactionRelevance + 0.02f,
                                                    false, reactionEvent, "prereq"))
                                {
                                    PushAgain(reactionNode, reactionRelevance + 0.01f, reactionEvent);
                                    continue;
                                }

                                incomingReaction.SetAction(reaction);
                                incomingReaction.SetEvent(reactionEvent);
                                delete reactionNode;
                                break;
                            }
                            else
                            {
                                MultiplyAndPush(reactionNode->getAlternatives(), reactionRelevance + 0.03f,
                                                false, reactionEvent, "alt");
                            }
                        }
                    }

                    delete reactionNode;
                }
            }
        } while (reactionItem && ++iterations <= iterationsPerTick);

        queue.RemoveExpired();

        return incomingReaction.IsValid();
    }

    return false;
}

float ReactionEngine::ApplyMainEngineMultipliers(Action* reaction, float relevance)
{
    Engine const* mainEngine = botAI->GetCurrentEngine();
    if (!mainEngine || mainEngine == this)
        return relevance;

    for (Multiplier* multiplier : mainEngine->GetMultipliers())
    {
        relevance *= multiplier->GetValue(reaction);
        reaction->setRelevance(relevance);
        if (relevance <= 0.0f)
        {
            LogAction("Multiplier %s made reaction %s useless", multiplier->getName().c_str(),
                      reaction->getName().c_str());
            break;
        }
    }

    return relevance;
}

bool ReactionEngine::StartReaction()
{
    bool reactionExecuted = false;
    if (incomingReaction.IsValid())
    {
        // Engine::ListenAndExecute runs the listeners and the debug output, then hands the
        // action's duration to PlayerbotAI::SetActionDuration, which routes reactions back
        // to SetReactionDuration below.
        reactionExecuted = ListenAndExecute(incomingReaction.GetAction(), incomingReaction.GetEvent());
        if (reactionExecuted)
            ongoingReaction = incomingReaction;

        incomingReaction.Reset();
    }

    return reactionExecuted;
}

void ReactionEngine::StopReaction()
{
    ongoingReaction.Reset();
    aiReactionUpdateDelay = 0U;
}

bool ReactionEngine::Update(uint32 elapsed, bool minimal, bool canControlSelf, bool& reactionFound)
{
    aiReactionUpdateDelay = aiReactionUpdateDelay > elapsed ? aiReactionUpdateDelay - elapsed : 0U;

    reactionFound = false;
    bool reactionFinished = false;

    if (CanUpdateAIReaction())
    {
        if (!IsReacting())
            reactionFinished = true;
        else if (ongoingReaction.Update(elapsed))
        {
            StopReaction();
            reactionFinished = true;
        }

        if (reactionFinished)
        {
            if (HasIncomingReaction())
            {
                StartReaction();
            }
            else
            {
                if (FindReaction(minimal, canControlSelf))
                    reactionFound = true;
            }
        }

        if (!HasIncomingReaction() && !IsReacting() && aiReactionUpdateDelay < sPlayerbotAIConfig.ReactDelay)
            aiReactionUpdateDelay = minimal ? sPlayerbotAIConfig.ReactDelay * 10 : sPlayerbotAIConfig.ReactDelay;
    }

    return HasIncomingReaction() || IsReacting();
}

Action* ReactionEngine::InitializeAction(ActionNode* actionNode)
{
    Action* action = actionNode->getAction();
    if (!action)
    {
        action = aiObjectContext->GetAction(actionNode->getName());
        actionNode->setAction(action);
    }

    if (action)
    {
        action->SetReaction(true);
        // Clear any duration left over from a previous execution; the real duration is
        // applied after Execute via SetActionDuration -> SetReactionDuration.
        action->ResetDuration();
    }

    return action;
}

void ReactionEngine::SetReactionDuration(Action const* action)
{
    if (action && (IsReacting() || HasIncomingReaction()))
    {
        if (ongoingReaction.GetAction() == action)
            ongoingReaction.SetDuration(action->GetDuration());
        else if (incomingReaction.GetAction() == action)
            incomingReaction.SetDuration(action->GetDuration());
    }
}

void ReactionEngine::ResetReactions()
{
    ongoingReaction.Reset();
    incomingReaction.Reset();
    aiReactionUpdateDelay = 0U;
}

bool ReactionEngine::CanUpdateAIReaction() const
{
    // The delay is reset to reactDelay after every empty search; only search again once it
    // has fully elapsed, otherwise the trigger pass runs on every world tick.
    Player* bot = botAI->GetBot();
    return (aiReactionUpdateDelay == 0U) &&
           bot->IsInWorld() &&
           !bot->IsBeingTeleported();
}

Reaction const* ReactionEngine::GetReaction() const
{
    if (ongoingReaction.IsValid())
        return &ongoingReaction;

    if (incomingReaction.IsValid())
        return &incomingReaction;

    return nullptr;
}
