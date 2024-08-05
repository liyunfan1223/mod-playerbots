/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FlagAction.h"

#include "Event.h"
#include "Playerbots.h"

bool FlagAction::TellUsage()
{
    botAI->TellError("Usage: flag cloak/helm/pvp on/set/off/clear/toggle/?");
    return false;
}

bool FlagAction::Execute(Event event)
{
    std::string const cmd = event.getParam();
    std::vector<std::string> ss = split(cmd, ' ');
    if (ss.size() != 2)
        return TellUsage();

    bool setFlag = (ss[1] == "set" || ss[1] == "on");
    bool clearFlag = (ss[1] == "clear" || ss[1] == "off");
    bool toggleFlag = (ss[1] == "toggle");
    if (ss[0] == "pvp")
    {
        if (setFlag)
            bot->SetPvP(true);
        else if (clearFlag)
            bot->SetPvP(false);
        else if (toggleFlag)
            bot->SetPvP(!bot->IsPvP());

        std::ostringstream out;
        out << ss[0] << " flag is " << chat->FormatBoolean(bot->IsPvP());
        botAI->TellMaster(out.str());
        return true;
    }

    uint32 playerFlags;
    if (ss[0] == "cloak")
        playerFlags = PLAYER_FLAGS_HIDE_CLOAK;

    if (ss[0] == "helm")
        playerFlags = PLAYER_FLAGS_HIDE_HELM;

    if (clearFlag)
        bot->SetFlag(PLAYER_FLAGS, playerFlags);
    else if (setFlag)
        bot->RemoveFlag(PLAYER_FLAGS, playerFlags);
    else if (toggleFlag && bot->HasFlag(PLAYER_FLAGS, playerFlags))
        bot->RemoveFlag(PLAYER_FLAGS, playerFlags);
    else if (toggleFlag && !bot->HasFlag(PLAYER_FLAGS, playerFlags))
        bot->SetFlag(PLAYER_FLAGS, playerFlags);

    std::ostringstream out;
    out << ss[0] << " flag is " << chat->FormatBoolean(!bot->HasFlag(PLAYER_FLAGS, playerFlags));
    botAI->TellMaster(out.str());
    return true;
}
