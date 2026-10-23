/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLGLYPHSACTION_H
#define PLAYERBOTS_TELLGLYPHSACTION_H

#include "Action.h"

class TellGlyphsAction : public Action
{
public:
    TellGlyphsAction(PlayerbotAI* botAI, std::string const name = "glyphs")
        : Action(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
