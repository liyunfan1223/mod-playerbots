/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIOBJECTCONTEXT_H
#define _PLAYERBOT_AIOBJECTCONTEXT_H

#include <sstream>
#include <string>

#include "Common.h"
#include "DynamicObject.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIAware.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"

class PlayerbotAI;

typedef Strategy* (*StrategyCreator)(PlayerbotAI* botAI);
typedef Action* (*ActionCreator)(PlayerbotAI* botAI);
typedef Trigger* (*TriggerCreator)(PlayerbotAI* botAI);
typedef UntypedValue* (*ValueCreator)(PlayerbotAI* botAI);

class AiObjectContext : public PlayerbotAIAware
{
public:
    static BoolCalculatedValue* custom_glyphs(PlayerbotAI* ai); // Added for cutom glyphs
    AiObjectContext(PlayerbotAI* botAI,
                    SharedNamedObjectContextList<Strategy>& sharedStrategyContext = sharedStrategyContexts,
                    SharedNamedObjectContextList<Action>& sharedActionContext = sharedActionContexts,
                    SharedNamedObjectContextList<Trigger>& sharedTriggerContext = sharedTriggerContexts,
                    SharedNamedObjectContextList<UntypedValue>& sharedValueContext = sharedValueContexts);
    virtual ~AiObjectContext() {}

    virtual Strategy* GetStrategy(std::string const name);
    virtual std::set<std::string> GetSiblingStrategy(std::string const name);
    virtual Trigger* GetTrigger(std::string const name);
    virtual Action* GetAction(std::string const name);
    virtual UntypedValue* GetUntypedValue(std::string const name);

    template <class T>
    Value<T>* GetValue(std::string const name)
    {
        return dynamic_cast<Value<T>*>(GetUntypedValue(name));
    }

    template <class T>
    Value<T>* GetValue(std::string const name, std::string const param)
    {
        return GetValue<T>((std::string(name) + "::" + param));
    }

    template <class T>
    Value<T>* GetValue(std::string const name, int32 param)
    {
        std::ostringstream out;
        out << param;
        return GetValue<T>(name, out.str());
    }

    std::set<std::string> GetValues();
    std::set<std::string> GetSupportedStrategies();
    std::set<std::string> GetSupportedActions();
    std::string const FormatValues();

    std::vector<std::string> Save();
    void Load(std::vector<std::string> data);

    std::vector<std::string> performanceStack;

    static void BuildAllSharedContexts();

    static void BuildSharedContexts();
    static void BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts);
    static void BuildSharedActionContexts(SharedNamedObjectContextList<Action>& actionContexts);
    static void BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts);
    static void BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts);

protected:
    NamedObjectContextList<Strategy> strategyContexts;
    NamedObjectContextList<Action> actionContexts;
    NamedObjectContextList<Trigger> triggerContexts;
    NamedObjectContextList<UntypedValue> valueContexts;

private:
    static SharedNamedObjectContextList<Strategy> sharedStrategyContexts;
    static SharedNamedObjectContextList<Action> sharedActionContexts;
    static SharedNamedObjectContextList<Trigger> sharedTriggerContexts;
    static SharedNamedObjectContextList<UntypedValue> sharedValueContexts;
};

#endif
