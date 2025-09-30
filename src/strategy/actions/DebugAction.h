/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DEBUGACTION_H
#define _PLAYERBOT_DEBUGACTION_H

#include "Action.h"
#include "ObjectGuid.h"
#include "TravelMgr.h"

class PlayerbotAI;
class Unit;

class DebugAction : public Action
{
public:
    DebugAction(PlayerbotAI* botAI) : Action(botAI, "Debug") {}

    bool Execute(Event event) override;

    void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid::Empty,
                   GuidVector otherTargets = {}, GuidVector missTargets = {}, WorldPosition source = WorldPosition(),
                   WorldPosition dest = WorldPosition(), bool forceDest = false);
    void addAura(uint32 spellId, Unit* target);
};

#endif
