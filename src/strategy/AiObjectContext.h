/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIOBJECTCONTEXT_H
#define _PLAYERBOT_AIOBJECTCONTEXT_H

#include <sstream>
#include <string>

#include "Common.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIAware.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"

class PlayerbotAI;

class AiObjectContext : public PlayerbotAIAware
{
public:
    AiObjectContext(PlayerbotAI* botAI);
    virtual ~AiObjectContext() {}

    virtual Strategy* GetStrategy(std::string_view name);
    virtual std::set<std::string> GetSiblingStrategy(std::string_view name);
    virtual Trigger* GetTrigger(std::string_view name);
    virtual Action* GetAction(std::string_view name);
    virtual UntypedValue* GetUntypedValue(std::string_view name);

    template <class T>
    Value<T>* GetValue(std::string_view name)
    {
        return dynamic_cast<Value<T>*>(GetUntypedValue(name));
    }

    template <class T>
    Value<T>* GetValue(std::string_view name, std::string_view param)
    {
        std::stringstream ss;
        ss << name << "::" << param;
        return GetValue<T>(ss.str());
    }

    template <class T>
    Value<T>* GetValue(std::string_view name, int32 param)
    {
        std::ostringstream out;
        out << param;
        return GetValue<T>(name, out.str());
    }

    std::set<std::string> GetValues();
    std::set<std::string> GetSupportedStrategies();
    std::set<std::string> GetSupportedActions();
    std::string const FormatValues();

    virtual void Update();
    virtual void Reset();
    virtual void AddShared(NamedObjectContext<UntypedValue>* sharedValues);

    std::vector<std::string> Save();
    void Load(std::vector<std::string> data);

    std::vector<std::string> performanceStack;

protected:
    NamedObjectContextList<Strategy> strategyContexts;
    NamedObjectContextList<Action> actionContexts;
    NamedObjectContextList<Trigger> triggerContexts;
    NamedObjectContextList<UntypedValue> valueContexts;
};

#endif
