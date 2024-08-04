/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHECKMOUNTSTATEACTION_H
#define _PLAYERBOT_CHECKMOUNTSTATEACTION_H

#include "UseItemAction.h"

class PlayerbotAI;

class CheckMountStateAction : public UseItemAction
{
public:
    CheckMountStateAction(PlayerbotAI* botAI) : UseItemAction(botAI, "check mount state", true) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Mount();
};

#endif
