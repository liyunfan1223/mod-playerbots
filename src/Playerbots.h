/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include "AiObjectContext.h"
#include "Group.h"
#include "Pet.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "SharedValueContext.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "TravelNode.h"

std::vector<std::string> split(std::string const s, char delim);
void split(std::vector<std::string>& dest, std::string const str, char const* delim);
#ifndef WIN32
int strcmpi(char const* s1, char const* s2);
#endif

#define CAST_ANGLE_IN_FRONT (2.f * static_cast<float>(M_PI) / 3.f)
#define EMOTE_ANGLE_IN_FRONT (2.f * static_cast<float>(M_PI) / 6.f)

#define GET_PLAYERBOT_AI(object) sPlayerbotsMgr->GetPlayerbotAI(object)
#define GET_PLAYERBOT_MGR(object) sPlayerbotsMgr->GetPlayerbotMgr(object)

#define AI_VALUE(type, name) context->GetValue<type>(name)->Get()
#define AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Get()

#define AI_VALUE_LAZY(type, name) context->GetValue<type>(name)->LazyGet()
#define AI_VALUE2_LAZY(type, name, param) context->GetValue<type>(name, param)->LazyGet()

#define AI_VALUE_REF(type, name) context->GetValue<type>(name)->RefGet()

#define SET_AI_VALUE(type, name, value) context->GetValue<type>(name)->Set(value)
#define SET_AI_VALUE2(type, name, param, value) context->GetValue<type>(name, param)->Set(value)
#define RESET_AI_VALUE(type, name) context->GetValue<type>(name)->Reset()
#define RESET_AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Reset()

#define PAI_VALUE(type, name) sPlayerbotsMgr->GetPlayerbotAI(player)->GetAiObjectContext()->GetValue<type>(name)->Get()
#define PAI_VALUE2(type, name, param) \
    sPlayerbotsMgr->GetPlayerbotAI(player)->GetAiObjectContext()->GetValue<type>(name, param)->Get()
#define GAI_VALUE(type, name) sSharedValueContext->getGlobalValue<type>(name)->Get()
#define GAI_VALUE2(type, name, param) sSharedValueContext->getGlobalValue<type>(name, param)->Get()

// ---- Safe teleport wrappers (module-only) ----
#include "Map.h"
#include <cmath>
#include "TravelMgr.h"

inline bool TeleportToSafe(Player* p, uint32 mapId, float x, float y, float z, float o)
{
    if (!p) return false;

    // If the height is invalid (-200000) or not finite, attempt ONE correction on the same map.
    if (z <= -199000.0f || !std::isfinite(z))
    {
        if (p->GetMapId() == mapId && p->GetMap())
        {
            float hz = p->GetMap()->GetHeight(p->GetPhaseMask(), x, y, p->GetPositionZ(), true);
            if (hz > -199000.0f && std::isfinite(hz))
                z = hz;
            else
                return false; // still invalid -> cancel the TP
        }
        else
        {
            return false; // different map: do not "guess" the height here
        }
    }
    return p->TeleportTo(mapId, x, y, z, o);
}

inline bool TeleportToSafe(Player* p, Position const& pos)
{
    // Position doesn't have mapId: we keep actual bot map
    return TeleportToSafe(p, p->GetMapId(),
                          pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                          pos.GetOrientation());
}

inline bool TeleportToSafe(Player* p, WorldPosition pos)
{
    return TeleportToSafe(p, pos.getMapId(), pos.getX(), pos.getY(), pos.getZ(), pos.getO());
}
// ---- /Safe teleport wrappers ----

#endif
