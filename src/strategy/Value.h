/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VALUE_H
#define _PLAYERBOT_VALUE_H

#include <time.h>

#include "AiObject.h"
#include "ObjectGuid.h"
#include "PerformanceMonitor.h"
#include "Timer.h"
#include "Unit.h"

class PlayerbotAI;
class Unit;

class FleeInfo
{
public:
    Position fromPos;
    float radius;
    float angle;
    uint32 timestamp;
    int GetAngleRangeIndex() { return (angle + 2 * M_PI) / (M_PI / 2); }  // [0, 7)
};

struct CreatureData;

class UntypedValue : public AiNamedObject
{
public:
    UntypedValue(PlayerbotAI* botAI, std::string const name) : AiNamedObject(botAI, name) {}
    virtual ~UntypedValue() {}
    virtual void Update() {}
    virtual void Reset() {}
    virtual std::string const Format() { return "?"; }
    virtual std::string const Save() { return "?"; }
    virtual bool Load([[maybe_unused]] std::string const value) { return false; }
};

template <class T>
class Value
{
public:
    virtual ~Value() {}
    virtual T Get() = 0;
    virtual T LazyGet() = 0;
    virtual T& RefGet() = 0;
    virtual void Reset() {}
    virtual void Set(T value) = 0;
    operator T() { return Get(); }
};

template <class T>
class CalculatedValue : public UntypedValue, public Value<T>
{
public:
    CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", uint32 checkInterval = 1)
        : UntypedValue(botAI, name),
          checkInterval(
              checkInterval == 1 ? 1 : (checkInterval < 100 ? checkInterval * 1000 : checkInterval)) /*turn s -> ms?*/,
          lastCheckTime(0)
    {
    }

    virtual ~CalculatedValue() {}

    T Get() override
    {
        if (checkInterval < 2)
        {
            // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(),
            // this->context ? &this->context->performanceStack : nullptr);
            value = Calculate();
            // if (pmo)
            //     pmo->finish();
        }
        else
        {
            time_t now = getMSTime();
            if (!lastCheckTime || now - lastCheckTime >= checkInterval)
            {
                lastCheckTime = now;
                // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(),
                // this->context ? &this->context->performanceStack : nullptr);
                value = Calculate();
                // if (pmo)
                //     pmo->finish();
            }
        }
        return value;
    }

    T LazyGet() override
    {
        if (!lastCheckTime)
            return Get();

        return value;
    }
    T& RefGet() override
    {
        if (checkInterval < 2)
        {
            // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(),
            // this->context ? &this->context->performanceStack : nullptr);
            value = Calculate();
            // if (pmo)
            //     pmo->finish();
        }
        else
        {
            time_t now = getMSTime();
            if (!lastCheckTime || now - lastCheckTime >= checkInterval)
            {
                lastCheckTime = now;
                // PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, this->getName(),
                // this->context ? &this->context->performanceStack : nullptr);
                value = Calculate();
                // if (pmo)
                //     pmo->finish();
            }
        }
        return value;
    }
    void Set(T val) override { value = val; }
    void Update() override {}
    void Reset() override { lastCheckTime = 0; }

protected:
    virtual T Calculate() = 0;

    uint32 checkInterval;
    uint32 lastCheckTime;
    T value;
};

template <class T>
class SingleCalculatedValue : public CalculatedValue<T>
{
public:
    SingleCalculatedValue(PlayerbotAI* botAI, std::string const name = "value") : CalculatedValue<T>(botAI, name)
    {
        this->Reset();
    }

    T Get() override
    {
        time_t now = time(0);
        if (!this->lastCheckTime)
        {
            this->lastCheckTime = now;

            PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(
                PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
            this->value = this->Calculate();
            if (pmo)
                pmo->finish();
        }

        return this->value;
    }
};

template <class T>
class MemoryCalculatedValue : public CalculatedValue<T>
{
public:
    MemoryCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1)
        : CalculatedValue<T>(botAI, name, checkInterval)
    {
        lastChangeTime = time(0);
    }

    virtual bool EqualToLast(T value) = 0;
    virtual bool CanCheckChange() { return time(0) - lastChangeTime < minChangeInterval || EqualToLast(this->value); }

    virtual bool UpdateChange()
    {
        if (CanCheckChange())
            return false;

        lastChangeTime = time(0);
        lastValue = this->value;
        return true;
    }

    void Set([[maybe_unused]] T value) override
    {
        CalculatedValue<T>::Set(this->value);
        UpdateChange();
    }

    T Get() override
    {
        this->value = CalculatedValue<T>::Get();
        UpdateChange();
        return this->value;
    }

    T LazyGet() override { return this->value; }

    time_t LastChangeOn()
    {
        Get();
        UpdateChange();
        return lastChangeTime;
    }

    uint32 LastChangeDelay() { return time(0) - LastChangeOn(); }

    void Reset() override
    {
        CalculatedValue<T>::Reset();
        lastChangeTime = time(0);
    }

protected:
    T lastValue;
    uint32 minChangeInterval = 0;
    time_t lastChangeTime;
};

template <class T>
class LogCalculatedValue : public MemoryCalculatedValue<T>
{
public:
    LogCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1)
        : MemoryCalculatedValue<T>(botAI, name, checkInterval)
    {
    }

    bool UpdateChange() override
    {
        if (MemoryCalculatedValue<T>::UpdateChange())
            return false;

        valueLog.push_back(std::make_pair(this->value, time(0)));

        if (valueLog.size() > logLength)
            valueLog.pop_front();
        return true;
    }

    std::list<std::pair<T, time_t>> ValueLog() { return valueLog; }

    void Reset() override
    {
        MemoryCalculatedValue<T>::Reset();
        valueLog.clear();
    }

protected:
    std::list<std::pair<T, time_t>> valueLog;
    uint8 logLength = 10;
};

class Uint8CalculatedValue : public CalculatedValue<uint8>
{
public:
    Uint8CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", uint32 checkInterval = 1)
        : CalculatedValue<uint8>(botAI, name, checkInterval)
    {
    }

    std::string const Format() override;
};

class Uint32CalculatedValue : public CalculatedValue<uint32>
{
public:
    Uint32CalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1)
        : CalculatedValue<uint32>(botAI, name, checkInterval)
    {
    }

    std::string const Format() override;
};

class FloatCalculatedValue : public CalculatedValue<float>
{
public:
    FloatCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1)
        : CalculatedValue<float>(botAI, name, checkInterval)
    {
    }

    std::string const Format() override;
};

class BoolCalculatedValue : public CalculatedValue<bool>
{
public:
    BoolCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int checkInterval = 1)
        : CalculatedValue<bool>(botAI, name, checkInterval)
    {
    }

    std::string const Format() override { return Calculate() ? "true" : "false"; }
};

class UnitCalculatedValue : public CalculatedValue<Unit*>
{
public:
    UnitCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

    std::string const Format() override;
    Unit* Get() override;
};

class CDPairCalculatedValue : public CalculatedValue<CreatureData const*>
{
public:
    CDPairCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

    std::string const Format() override;
};

class CDPairListCalculatedValue : public CalculatedValue<std::vector<CreatureData const*>>
{
public:
    CDPairListCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

    std::string const Format() override;
};

class ObjectGuidCalculatedValue : public CalculatedValue<ObjectGuid>
{
public:
    ObjectGuidCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

    std::string const Format() override;
};

class ObjectGuidListCalculatedValue : public CalculatedValue<GuidVector>
{
public:
    ObjectGuidListCalculatedValue(PlayerbotAI* botAI, std::string const name = "value", int32 checkInterval = 1);

    std::string const Format() override;
};

template <class T>
class ManualSetValue : public UntypedValue, public Value<T>
{
public:
    ManualSetValue(PlayerbotAI* botAI, T defaultValue, std::string const name = "value")
        : UntypedValue(botAI, name), value(defaultValue), defaultValue(defaultValue)
    {
    }

    virtual ~ManualSetValue() {}

    T Get() override { return value; }
    T LazyGet() override { return value; }
    T& RefGet() override { return value; }
    void Set(T val) override { value = val; }
    void Update() override {}
    void Reset() override { value = defaultValue; }

protected:
    T value;
    T defaultValue;
};

class UnitManualSetValue : public ManualSetValue<Unit*>
{
public:
    UnitManualSetValue(PlayerbotAI* botAI, Unit* defaultValue, std::string const name = "value")
        : ManualSetValue<Unit*>(botAI, defaultValue, name)
    {
    }

    std::string const Format() override;
    Unit* Get() override;
};

class DisperseDistanceValue : public ManualSetValue<float>
{
public:
    DisperseDistanceValue(PlayerbotAI* botAI, float defaultValue = -1.0f, std::string const name = "disperse distance")
        : ManualSetValue<float>(botAI, defaultValue, name)
    {
    }
};

class LastFleeAngleValue : public ManualSetValue<float>
{
public:
    LastFleeAngleValue(PlayerbotAI* botAI, float defaultValue = 0.0f, std::string const name = "last flee angle")
        : ManualSetValue<float>(botAI, defaultValue, name)
    {
    }
};

class LastFleeTimestampValue : public ManualSetValue<uint32>
{
public:
    LastFleeTimestampValue(PlayerbotAI* botAI, uint32 defaultValue = 0, std::string const name = "last flee timestamp")
        : ManualSetValue<uint32>(botAI, defaultValue, name)
    {
    }
};

class RecentlyFleeInfo : public ManualSetValue<std::list<FleeInfo>&>
{
public:
    RecentlyFleeInfo(PlayerbotAI* botAI, std::string const name = "recently flee info")
        : ManualSetValue<std::list<FleeInfo>&>(botAI, data, name)
    {
    }
private:
    std::list<FleeInfo> data = {};
};

#endif
