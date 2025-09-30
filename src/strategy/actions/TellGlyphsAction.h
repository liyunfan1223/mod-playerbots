/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELLGLYPHSACTION_H
#define _PLAYERBOT_TELLGLYPHSACTION_H

#include "Action.h"

class TellGlyphsAction : public Action
{
public:
    TellGlyphsAction(PlayerbotAI* ai, std::string const name = "glyphs")
        : Action(ai, name) {}

    bool Execute(Event event) override;
};

#endif

