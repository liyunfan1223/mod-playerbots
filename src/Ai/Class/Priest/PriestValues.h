/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PRIESTVALUES_H
#define PLAYERBOTS_PRIESTVALUES_H

#include "Value.h"

class MassDispelTargetValue : public ObjectGuidCalculatedValue
{
public:
    MassDispelTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mass dispel target", IN_MILLISECONDS)
    {
    }

    ObjectGuid Calculate() override;
    static bool IsValidTarget(PlayerbotAI* botAI, Unit* target);
};

#endif
