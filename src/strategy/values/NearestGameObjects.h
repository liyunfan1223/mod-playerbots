/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEARESTGAMEOBJECTS_H
#define _PLAYERBOT_NEARESTGAMEOBJECTS_H

#include "Value.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestGameObjects : public ObjectGuidListCalculatedValue
{
	public:
        NearestGameObjects(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance, bool ignoreLos = false, std::string const name = "nearest game objects") :
            ObjectGuidListCalculatedValue(botAI, name, 2 * 1000), range(range) , ignoreLos(ignoreLos) { }

    protected:
        GuidVector Calculate() override;

    private:
        float range;
        bool ignoreLos;
};

#endif
