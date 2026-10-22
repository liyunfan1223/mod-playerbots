/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDRESURRECTTRIGGER_H
#define PLAYERBOTS_WORLDRESURRECTTRIGGER_H

#include "Trigger.h"

class WorldResurrectTrigger : public Trigger
{
public:
    WorldResurrectTrigger(PlayerbotAI* botAI) : Trigger(botAI, "world resurrect available") {}

    bool IsActive() override;
};

#endif
