/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACTION_H
#define PLAYERBOTS_ACTION_H

#include "AiObject.h"
#include "Event.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class NextAction
{
public:
    NextAction(std::string const name, float relevance = 0.0f)
        : relevance(relevance), name(name) {}  // name after relevance - whipowill

    std::string const getName() { return name; }
    float getRelevance() { return relevance; }

    static std::vector<NextAction> merge(std::vector<NextAction> const& what, std::vector<NextAction> const& with)
    {
        std::vector<NextAction> result = {};

        for (NextAction const& action : what)
        {
            result.push_back(action);
        }

        for (NextAction const& action : with)
        {
            result.push_back(action);
        }

        return result;
    };

private:
    float relevance;
    std::string name;
};

class Action : public AiNamedObject
{
public:
    enum class ActionThreatType
    {
        None = 0,
        Single = 1,
        Aoe = 2
    };

    Action(PlayerbotAI* botAI, std::string const name = "action")
        : AiNamedObject(botAI, name), verbose(false) {}  // verbose after ainamedobject - whipowill
    virtual ~Action(void) {}

    virtual bool Execute([[maybe_unused]] Event event) { return true; }

    /**
     * @brief First validation check - determines if this action is contextually useful
     *
     * Performs lightweight checks to evaluate whether the action makes sense
     * in the current situation. Called before isPossible() during action selection.
     *
     * @return true if the action is useful, false otherwise
     */
    virtual bool isUseful() { return true; }

    /**
     * @brief Second validation check - determines if this action can be executed
     *
     * Performs hard pre-execution validation against the event and game state.
     * Called after isUseful() passes, before Execute().
     *
     * @return true if the action is possible, false otherwise
     */
    virtual bool isPossible() { return true; }

    virtual std::vector<NextAction> getPrerequisites() { return {}; }
    virtual std::vector<NextAction> getAlternatives() { return {}; }
    virtual std::vector<NextAction> getContinuers() { return {}; }
    virtual ActionThreatType getThreatType() { return ActionThreatType::None; }
    virtual bool isUsefulWithoutControl() { return false; }
    void Update() {}
    void Reset() {}
    virtual Unit* GetTarget();
    virtual Value<Unit*>* GetTargetValue();
    virtual std::string const GetTargetName() { return "self target"; }
    void MakeVerbose() { verbose = true; }
    void setRelevance(uint32 relevance1) { relevance = relevance1; };
    virtual float getRelevance() { return relevance; }

    // Reaction system
    bool IsReaction() const { return reaction; }
    void SetReaction(bool inReaction) { reaction = inReaction; }
    uint32 GetDuration() const { return duration; }
    void ResetDuration() { duration = 0; }
    virtual bool ShouldReactionInterruptCast() const { return false; }
    virtual bool ShouldReactionInterruptMovement() const { return false; }

protected:
    void SetDuration(uint32 inDuration) { duration = inDuration; }
    bool verbose;
    float relevance = 0;
    bool reaction = false;
    uint32 duration = 0;
};

class ActionNode
{
public:
    ActionNode(
        std::string name,
        std::vector<NextAction> prerequisites = {},
        std::vector<NextAction> alternatives = {},
        std::vector<NextAction> continuers = {}
    ) :
    name(std::move(name)),
    action(nullptr),
    continuers(continuers),
    alternatives(alternatives),
    prerequisites(prerequisites)
    {}

    virtual ~ActionNode() = default;

    Action* getAction() { return action; }
    void setAction(Action* action) { this->action = action; }
    const std::string getName() { return name; }

    std::vector<NextAction> getContinuers()
    {
        return NextAction::merge(this->continuers, action->getContinuers());
    }
    std::vector<NextAction> getAlternatives()
    {
        return NextAction::merge(this->alternatives, action->getAlternatives());
    }
    std::vector<NextAction> getPrerequisites()
    {
        return NextAction::merge(this->prerequisites, action->getPrerequisites());
    }

private:
    const std::string name;
    Action* action;
    std::vector<NextAction> continuers;
    std::vector<NextAction> alternatives;
    std::vector<NextAction> prerequisites;
};

class ActionBasket
{
public:
    ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event);

    virtual ~ActionBasket(void) {}

    float getRelevance() { return relevance; }
    ActionNode* getAction() { return action; }
    Event getEvent() { return event; }
    bool isSkipPrerequisites() { return skipPrerequisites; }
    void AmendRelevance(float k) { relevance *= k; }
    void setRelevance(float relevance) { this->relevance = relevance; }
    bool isExpired(uint32_t msecs);

private:
    ActionNode* action;
    float relevance;
    bool skipPrerequisites;
    Event event;
    uint32_t created;
};

#endif
