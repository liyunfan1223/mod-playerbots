/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidHumanTankCall.h"

#include "Chat.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "RaidInstanceState.h"
#include "RaidTankDefensive.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <unordered_map>

namespace
{
RaidInstanceState<std::unordered_map<ObjectGuid, uint32>> lastCallMs;

std::string SpellNameFor(Player* human, SpellInfo const* info)
{
    char const* name = info->SpellName[human->GetSession()->GetSessionDbcLocale()];
    return name && *name ? name : info->SpellName[LOCALE_enUS];
}

void SendTo(Player* sender, Player* human, ChatMsg type, Language language, std::string const& text)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, type, language, sender->GetGUID(), human->GetGUID(), text, CHAT_TAG_NONE,
                                 sender->GetName());
    ServerFacade::instance().SendPacket(human, &data);
}
}  // namespace

bool CallHumanTankDefensive(Player* sender, Player* human, uint32 spellId, uint32 hitInMs, std::string const& why,
                            uint32 repeatMs)
{
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!sender || !human || !info || !human->GetSession() || !human->GetInstanceId() ||
        RunningTankDefensive(human, false, 0))
        return false;

    uint32 const now = getMSTime();
    uint32& last = lastCallMs.For(human->GetInstanceId())[human->GetGUID()];
    if (last && getMSTimeDiff(last, now) < repeatMs)
        return false;

    last = now;

    SendTo(sender, human, CHAT_MSG_WHISPER, LANG_ADDON,
           "PBCALL\tDEF\t" + std::to_string(spellId) + "\t" + std::to_string(hitInMs) + "\t" + why);

    // the client runs this one through format(), so a bare % breaks it
    std::string const name = SpellNameFor(human, info);
    std::string text;
    for (char const c : name + " now: " + why)
    {
        text += c;
        if (c == '%')
            text += '%';
    }

    SendTo(sender, human, CHAT_MSG_RAID_BOSS_WHISPER, LANG_UNIVERSAL, text);

    return true;
}
