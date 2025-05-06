/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ACCEPTBATTLEGROUNDINVITATIONACTION_H
#define _PLAYERBOT_ACCEPTBATTLEGROUNDINVITATIONACTION_H

#include "Action.h"

class PlayerbotAI;

class AcceptBgInvitationAction : public Action
{
public:
    AcceptBgInvitationAction(PlayerbotAI* botAI) : Action(botAI, "accept bg invitatio") {}

    bool Execute(Event event) override;
};

#endif
