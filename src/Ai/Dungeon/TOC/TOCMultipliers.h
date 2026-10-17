/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCMULTIPLIERS_H
#define PLAYERBOTS_TOCMULTIPLIERS_H

#include "Multiplier.h"

class ToCJoustMultiplier : public Multiplier
{
public:
    ToCJoustMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "toc joust") {}
    float GetValue(Action* action) override;

private:
    bool SpareMountNearby();

    uint32 mountCheckAt = 0;
    bool spareMount = false;
};

#endif
