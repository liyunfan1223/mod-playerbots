/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ESTIMATEDLIFETIMEVALUE_H
#define PLAYERBOTS_ESTIMATEDLIFETIMEVALUE_H

#include "NamedObjectContext.h"
#include "PossibleTargetsValue.h"
#include "TargetValue.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

// [target health] / [expected group single target dps] = [expected lifetime]
class EstimatedLifetimeValue : public FloatCalculatedValue, public Qualified
{
public:
    EstimatedLifetimeValue(PlayerbotAI* botAI) : FloatCalculatedValue(botAI, "estimated lifetime") {}

public:
    float Calculate() override;
};

class EstimatedGroupDpsValue : public FloatCalculatedValue
{
public:
    EstimatedGroupDpsValue(PlayerbotAI* botAI)
        : FloatCalculatedValue(botAI, "estimated group dps", 20 * IN_MILLISECONDS) {}

public:
    float Calculate() override;

protected:
    float GetBasicDps(uint32 level);
    float GetBasicGs(uint32 level);
};

#endif
