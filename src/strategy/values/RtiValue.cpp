/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RtiValue.h"

#include "Playerbots.h"

RtiValue::RtiValue(PlayerbotAI* botAI) : ManualSetValue<std::string>(botAI, "skull", "rti") {}

std::string const RtiValue::Save() { return value; }

bool RtiValue::Load(std::string const text)
{
    value = text;
    return true;
}

RtiCcValue::RtiCcValue(PlayerbotAI* botAI) : ManualSetValue<std::string>(botAI, "moon", "rti cc") {}

std::string const RtiCcValue::Save() { return value; }

bool RtiCcValue::Load(std::string const text)
{
    value = text;
    return true;
}
