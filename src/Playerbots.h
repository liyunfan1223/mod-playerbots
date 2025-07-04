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
#include <atomic>
#include <unordered_map>
#include <mutex>

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

// Thread-safe player update tracking
class PlayerbotsUpdateTracker
{
public:
    static bool TryAcquireUpdateFlag(uint64 playerGuid)
    {
        std::lock_guard<std::mutex> lock(s_mapMutex);
        auto& flag = s_playerUpdateFlags[playerGuid];
        bool expected = false;
        return flag.compare_exchange_strong(expected, true, std::memory_order_acquire);
    }
    
    static void ReleaseUpdateFlag(uint64 playerGuid)
    {
        s_playerUpdateFlags[playerGuid].store(false, std::memory_order_release);
    }
    
private:
    static std::unordered_map<uint64, std::atomic<bool>> s_playerUpdateFlags;
    static std::mutex s_mapMutex;
};

#endif
