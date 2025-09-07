/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MOVETORPGTARGETACTION_H
#define _PLAYERBOT_MOVETORPGTARGETACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class MoveToRpgTargetAction : public MovementAction
{
public:
    MoveToRpgTargetAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to rpg target") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
