/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDACTIONCONTEXTACTION_H
#define _PLAYERBOT_RAIDACTIONCONTEXTACTION_H

#include "Action.h"
#include "NamedObjectContext.h"

class RaidActionContext : public NamedObjectContext<Action>
{
    public:
        RaidActionContext()
        {
        }

    private:
};

#endif
