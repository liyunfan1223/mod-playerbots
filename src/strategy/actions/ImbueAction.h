/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_IMBUEACTION_H
#define _PLAYERBOT_IMBUEACTION_H

#include "Action.h"

class PlayerbotAI;

class ImbueWithPoisonAction : public Action
{
public:
    ImbueWithPoisonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class ImbueWithStoneAction : public Action
{
public:
    ImbueWithStoneAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class ImbueWithOilAction : public Action
{
public:
    ImbueWithOilAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class TryEmergencyAction : public Action
{
public:
    TryEmergencyAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
