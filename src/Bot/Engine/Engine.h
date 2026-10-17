/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ENGINE_H
#define PLAYERBOTS_ENGINE_H

#include "Multiplier.h"
#include "PlayerbotAIAware.h"
#include "Queue.h"
#include "Strategy.h"
#include "Trigger.h"
#include <map>

class Action;
class ActionNode;
class AiObjectContext;
class Event;
class NextAction;
class PlayerbotAI;

enum ActionResult
{
    ACTION_RESULT_UNKNOWN,
    ACTION_RESULT_OK,
    ACTION_RESULT_IMPOSSIBLE,
    ACTION_RESULT_USELESS,
    ACTION_RESULT_FAILED
};

class ActionExecutionListener
{
public:
    virtual ~ActionExecutionListener(){};

    virtual bool Before(Action* action, Event event) = 0;
    virtual bool AllowExecution(Action* action, Event event) = 0;
    virtual void After(Action* action, bool executed, Event event) = 0;
    virtual bool OverrideResult(Action* action, bool executed, Event event) = 0;
};

class ActionExecutionListeners : public ActionExecutionListener
{
public:
    virtual ~ActionExecutionListeners();

    bool Before(Action* action, Event event) override;
    bool AllowExecution(Action* action, Event event) override;
    void After(Action* action, bool executed, Event event) override;
    bool OverrideResult(Action* action, bool executed, Event event) override;

    void Add(ActionExecutionListener* listener) { listeners.push_back(listener); }

    void Remove(ActionExecutionListener* listener) { listeners.remove(listener); }

private:
    std::list<ActionExecutionListener*> listeners;
};

class Engine : public PlayerbotAIAware
{
public:
    Engine(PlayerbotAI* botAI, AiObjectContext* factory);

    virtual void Init();
    void addStrategy(std::string const name, bool init = true);
    void addStrategies(std::string first, ...);
    void addStrategiesNoInit(std::string first, ...);
    bool removeStrategy(std::string const name, bool init = true);
    bool HasStrategy(std::string const name);
    Strategy* GetStrategy(std::string const name);
    void removeAllStrategies();
    void toggleStrategy(std::string const name);
    std::string const ListStrategies();
    std::vector<std::string> GetStrategies();
    bool ContainsStrategy(StrategyType type);
    void ChangeStrategy(std::string const names);
    std::string const GetLastAction() { return lastAction; }

    virtual bool DoNextAction(Unit*, uint32 depth = 0, bool minimal = false);
    ActionResult ExecuteAction(std::string const name, Event event = Event(), std::string const qualifier = "");

    void AddActionExecutionListener(ActionExecutionListener* listener) { actionExecutionListeners.Add(listener); }

    void removeActionExecutionListener(ActionExecutionListener* listener) { actionExecutionListeners.Remove(listener); }
    bool HasStrategyType(StrategyType type) { return strategyTypeMask & type; }
    bool HasTargetExclusions() const { return hasTargetExclusions; }
    std::vector<Multiplier*> const& GetMultipliers() const { return multipliers; }
    virtual ~Engine(void);

    bool testMode;

private:
    std::string const ResolveStrategyName(std::string const name);
    void PushDefaultActions();
    ActionNode* CreateActionNode(std::string const name);

    void LogValues();

    ActionExecutionListeners actionExecutionListeners;

protected:
    void LogAction(char const* format, ...);
    bool MultiplyAndPush(std::vector<NextAction> actions, float forceRelevance, bool skipPrerequisites, Event event,
                         char const* pushType);
    void Reset();
    void ProcessTriggers(bool minimal);
    void PushAgain(ActionNode* actionNode, float relevance, Event event);
    virtual Action* InitializeAction(ActionNode* actionNode);
    virtual bool ListenAndExecute(Action* action, Event event);

    Queue queue;
    std::vector<TriggerNode*> triggers;
    std::vector<Multiplier*> multipliers;
    AiObjectContext* aiObjectContext;
    std::map<std::string, Strategy*> strategies;
    float lastRelevance;
    std::string lastAction;
    uint32 strategyTypeMask;
    bool hasTargetExclusions = false;
    NamedObjectFactoryList<ActionNode> actionNodeFactories;
};

#endif
