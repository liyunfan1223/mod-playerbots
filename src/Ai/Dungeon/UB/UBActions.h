/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBACTIONS_H
#define PLAYERBOTS_UBACTIONS_H

#include "MovementActions.h"

class UBRetreatFromFoulSporesAction : public MovementAction
{
public:
    UBRetreatFromFoulSporesAction(PlayerbotAI* botAI) : MovementAction(botAI, "ub retreat from foul spores") {}
    bool Execute(Event event) override;
};

class UBVacateSporeCloudAction : public MovementAction
{
public:
    UBVacateSporeCloudAction(PlayerbotAI* botAI) : MovementAction(botAI, "ub vacate spore cloud") {}
    bool Execute(Event event) override;
};

class UBClearUnderbatBackAction : public MovementAction
{
public:
    UBClearUnderbatBackAction(PlayerbotAI* botAI) : MovementAction(botAI, "ub clear underbat back") {}
    bool Execute(Event event) override;

private:
    uint32 _lastReposition = 0;
};

#endif
