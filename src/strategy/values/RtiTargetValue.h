/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTITARGETVALUE_H
#define _PLAYERBOT_RTITARGETVALUE_H

#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class RtiTargetValue : public TargetValue
{
    public:
        RtiTargetValue(PlayerbotAI* botAI, std::string const type = "rti", std::string const name = "rti target") : TargetValue(botAI, name), type(type) { }

        static int32 GetRtiIndex(std::string const rti);
        Unit* Calculate() override;

    private:
        std::string const type;
};

class RtiCcTargetValue : public RtiTargetValue
{
    public:
        RtiCcTargetValue(PlayerbotAI* botAI, std::string const name = "rti cc target") : RtiTargetValue(botAI, "rti cc", name) { }
};

#endif
