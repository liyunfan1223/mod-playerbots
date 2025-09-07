/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Value.h"

#include "PerformanceMonitor.h"
#include "Playerbots.h"
#include "Timer.h"

UnitCalculatedValue::UnitCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval)
    : CalculatedValue<Unit*>(botAI, name, checkInterval)
{
}

std::string const UnitCalculatedValue::Format()
{
    Unit* unit = Calculate();
    return unit ? unit->GetName() : "<none>";
}

std::string const UnitManualSetValue::Format()
{
    Unit* unit = Get();
    return unit ? unit->GetName() : "<none>";
}

std::string const Uint8CalculatedValue::Format()
{
    std::ostringstream out;
    out << Calculate();
    return out.str();
}

std::string const Uint32CalculatedValue::Format()
{
    std::ostringstream out;
    out << Calculate();
    return out.str();
}

std::string const FloatCalculatedValue::Format()
{
    std::ostringstream out;
    out << Calculate();
    return out.str();
}

CDPairCalculatedValue::CDPairCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval)
    : CalculatedValue<CreatureData const*>(botAI, name, checkInterval)
{
    // lastCheckTime = getMSTime() - checkInterval / 2;
}

std::string const CDPairCalculatedValue::Format()
{
    CreatureData const* creatureData = Calculate();
    if (creatureData)
    {
        CreatureTemplate const* bmTemplate = sObjectMgr->GetCreatureTemplate(creatureData->id1);
        return bmTemplate ? bmTemplate->Name : "<none>";
    }

    return "<none>";
}

CDPairListCalculatedValue::CDPairListCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval)
    : CalculatedValue<std::vector<CreatureData const*>>(botAI, name, checkInterval)
{
    // lastCheckTime = time(nullptr) - checkInterval / 2;
}

std::string const CDPairListCalculatedValue::Format()
{
    std::ostringstream out;
    out << "{";
    std::vector<CreatureData const*> cdPairs = Calculate();
    for (CreatureData const* cdPair : cdPairs)
    {
        out << cdPair->id1 << ",";
    }

    out << "}";
    return out.str();
}

ObjectGuidCalculatedValue::ObjectGuidCalculatedValue(PlayerbotAI* botAI, std::string const name, int32 checkInterval)
    : CalculatedValue<ObjectGuid>(botAI, name, checkInterval)
{
    // lastCheckTime = time(nullptr) - checkInterval / 2;
}

std::string const ObjectGuidCalculatedValue::Format()
{
    ObjectGuid guid = Calculate();
    return guid ? std::to_string(guid.GetRawValue()) : "<none>";
}

ObjectGuidListCalculatedValue::ObjectGuidListCalculatedValue(PlayerbotAI* botAI, std::string const name,
                                                             int32 checkInterval)
    : CalculatedValue<GuidVector>(botAI, name, checkInterval)
{
}

std::string const ObjectGuidListCalculatedValue::Format()
{
    std::ostringstream out;
    out << "{";

    GuidVector guids = Calculate();
    for (GuidVector::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        ObjectGuid guid = *i;
        out << guid.GetRawValue() << ",";
    }
    out << "}";

    return out.str();
}

Unit* UnitCalculatedValue::Get()
{
    if (checkInterval < 2)
    {
        PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(
            PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
        value = Calculate();
        if (pmo)
            pmo->finish();
    }
    else
    {
        time_t now = getMSTime();
        if (!lastCheckTime || now - lastCheckTime >= checkInterval)
        {
            lastCheckTime = now;
            PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(
                PERF_MON_VALUE, this->getName(), this->context ? &this->context->performanceStack : nullptr);
            value = Calculate();
            if (pmo)
                pmo->finish();
        }
    }
    // Prevent crashing by InWorld check
    if (value && value->IsInWorld())
        return value;
    return nullptr;
}

Unit* UnitManualSetValue::Get()
{
    // Prevent crashing by InWorld check
    if (value && value->IsInWorld())
        return value;
    return nullptr;
}