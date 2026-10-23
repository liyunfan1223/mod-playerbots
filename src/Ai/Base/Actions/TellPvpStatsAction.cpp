/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TellPvpStatsAction.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "Event.h"
#include "Language.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include <map>

namespace
{
    inline char const* BracketName(uint8 slot)
    {
        switch (slot)
        {
            case ARENA_SLOT_2v2: return "2v2";
            case ARENA_SLOT_3v3: return "3v3";
            default:             return "5v5"; // ARENA_SLOT_5v5
        }
    }
}

bool TellPvpStatsAction::Execute(Event event)
{
    if (!bot)
        return false;

    // Prefer the actual chat sender (whisper / say / etc.) if available.
    Player* requester = nullptr;

    if (Unit* owner = event.getOwner())
        requester = owner->ToPlayer();

    // Fallback to master if event owner is not available.
    if (!requester)
        requester = GetMaster();

    // If we still do not have a valid player to answer to, bail out.
    if (!requester)
        return false;

    // PVP currencies
    std::map<std::string, std::string> currencyPlaceholders;
    currencyPlaceholders["%arena_points"] = std::to_string(bot->GetArenaPoints());
    currencyPlaceholders["%honor_points"] = std::to_string(bot->GetHonorPoints());

    std::string const currencyText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "pvp_currency",
        "[PVP] Arena points: %arena_points | Honor Points: %honor_points",
        currencyPlaceholders);

    bot->Whisper(currencyText, LANG_UNIVERSAL, requester);

    // Arena Teams by slot
    bool anyTeam = false;
    for (uint8 slot = ARENA_SLOT_2v2; slot <= ARENA_SLOT_5v5; ++slot)
    {
        uint32 const teamId = bot->GetArenaTeamId(slot);
        if (!teamId)
            continue;

        if (ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(teamId))
        {
            anyTeam = true;
            std::map<std::string, std::string> placeholders;
            placeholders["%bracket"] = BracketName(slot);
            placeholders["%team_name"] = team->GetName();
            placeholders["%team_rating"] = std::to_string(team->GetRating());

            std::string const teamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "pvp_arena_team",
                "[PVP] %bracket: <%team_name> (rating %team_rating)",
                placeholders);

            bot->Whisper(teamText, LANG_UNIVERSAL, requester);
        }
    }

    if (!anyTeam)
    {
        std::string const noTeamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pvp_no_arena_team",
            "[PVP] I have no Arena Team.",
            std::map<std::string, std::string>());

        bot->Whisper(noTeamText, LANG_UNIVERSAL, requester);
    }

    return true;
}
