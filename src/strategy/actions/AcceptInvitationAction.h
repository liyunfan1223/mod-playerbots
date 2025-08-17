/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACCEPTINVITATIONACTION_H
#define _PLAYERBOT_ACCEPTINVITATIONACTION_H

#include "Action.h"
#include "UseMeetingStoneAction.h"

class PlayerbotAI;

class AcceptInvitationAction : public SummonAction
{
public:
    AcceptInvitationAction(PlayerbotAI* botAI) : SummonAction(botAI, "accept invitation") {}

    bool Execute(Event event) override;
};

#endif
