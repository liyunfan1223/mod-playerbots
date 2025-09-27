/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACTION_H
#define _PLAYERBOT_ACTION_H

#include "AiObject.h"
#include "Common.h"
#include "Event.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class NextAction
{
public:
    NextAction(std::string const name, float relevance = 0.0f)
        : relevance(relevance), name(name) {}                                  // name after relevance - whipowill
    NextAction(NextAction const& o) : relevance(o.relevance), name(o.name) {}  // name after relevance - whipowill

    std::string const getName() { return name; }
    float getRelevance() { return relevance; }

    static uint32 size(NextAction** actions);
    static NextAction** clone(NextAction** actions);
    static NextAction** merge(NextAction** what, NextAction** with);
    static NextAction** array(uint32 nil, ...);
    static void destroy(NextAction** actions);

private:
    float relevance;
    std::string const name;
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
    virtual bool isPossible() { return true; }
    virtual bool isUseful() { return true; }
    virtual NextAction** getPrerequisites() { return nullptr; }
    virtual NextAction** getAlternatives() { return nullptr; }
    virtual NextAction** getContinuers() { return nullptr; }
    virtual ActionThreatType getThreatType() { return ActionThreatType::None; }
    void Update() {}
    void Reset() {}
    virtual Unit* GetTarget();
    virtual Value<Unit*>* GetTargetValue();
    virtual std::string const GetTargetName() { return "self target"; }
    void MakeVerbose() { verbose = true; }
    void setRelevance(uint32 relevance1) { relevance = relevance1; };
    virtual float getRelevance() { return relevance; }

protected:
    bool verbose;
    float relevance = 0;
};

class ActionNode
{
public:
    ActionNode(std::string const name, NextAction** prerequisites = nullptr, NextAction** alternatives = nullptr,
               NextAction** continuers = nullptr)
        : name(name), action(nullptr), continuers(continuers), alternatives(alternatives), prerequisites(prerequisites)
    {
    }  // reorder arguments - whipowill

    virtual ~ActionNode()
    {
        NextAction::destroy(prerequisites);
        NextAction::destroy(alternatives);
        NextAction::destroy(continuers);
    }

    Action* getAction() { return action; }
    void setAction(Action* action) { this->action = action; }
    std::string const getName() { return name; }

    NextAction** getContinuers() { return NextAction::merge(NextAction::clone(continuers), action->getContinuers()); }
    NextAction** getAlternatives()
    {
        return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives());
    }
    NextAction** getPrerequisites()
    {
        return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites());
    }

private:
    std::string const name;
    Action* action;
    NextAction** continuers;
    NextAction** alternatives;
    NextAction** prerequisites;
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
    bool isExpired(uint32 msecs);

private:
    ActionNode* action;
    float relevance;
    bool skipPrerequisites;
    Event event;
    uint32 created;
};

#endif
