/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SetFocusHealTargetsAction.h"
#include "ObjectAccessor.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include <algorithm>
#include <cctype>

static std::string LowercaseString(std::string const& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

static Player* FindGroupPlayerByName(Player* player, std::string const& playerName)
{
    if (!player)
        return nullptr;

    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member)
        {
            std::string memberName = member->GetName();
            if (LowercaseString(memberName) == playerName)
                return member;
        }
    }

    return nullptr;
}

bool SetFocusHealTargetsAction::Execute(Event event)
{
    if (!PlayerbotAI::IsHeal(bot) && !botAI->HasStrategy("offheal", BOT_STATE_COMBAT))
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "focus_heal_not_healer",
            "I'm not a healer or offhealer (please change my strats to heal or offheal)",
            {});
        botAI->TellMasterNoFacing(text);
        return false;
    }

    std::string const param = LowercaseString(event.getParam());
    if (param.empty())
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "focus_heal_provide_names",
            "Please provide one or more player names",
            {});
        botAI->TellMasterNoFacing(text);
        return false;
    }

    std::list<ObjectGuid> focusHealTargets =
        AI_VALUE(std::list<ObjectGuid>, "focus heal targets");

    // Query current focus targets
    if (param.find('?') != std::string::npos)
    {
        if (focusHealTargets.empty())
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_no_targets",
                "I don't have any focus heal targets",
                {});
            botAI->TellMasterNoFacing(text);
        }
        else
        {
            std::stringstream targetNames;
            for (auto it = focusHealTargets.begin(); it != focusHealTargets.end(); ++it)
            {
                Unit* target = botAI->GetUnit(*it);
                if (target)
                {
                    if (it != focusHealTargets.begin())
                        targetNames << ", ";
                    targetNames << target->GetName();
                }
            }

            std::map<std::string, std::string> placeholders;
            placeholders["%targets"] = targetNames.str();
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_current_targets",
                "My focus heal targets are %targets",
                placeholders);
            botAI->TellMasterNoFacing(text);
        }

        return true;
    }

    // Clear all targets
    if (param == "none" || param == "unset" || param == "clear")
    {
        focusHealTargets.clear();
        SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "focus_heal_cleared",
            "Removed focus heal targets",
            {});
        botAI->TellMasterNoFacing(text);
        return true;
    }

    // Parse multiple targets separated by commas
    std::vector<std::string> targetNames;
    if (param.find(',') != std::string::npos)
    {
        std::string targetName;
        std::stringstream ss(param);
        while (std::getline(ss, targetName, ','))
            targetNames.push_back(targetName);
    }
    else
        targetNames.push_back(param);

    if (targetNames.empty())
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "focus_heal_provide_names",
            "Please provide one or more player names",
            {});
        botAI->TellMasterNoFacing(text);
        return false;
    }

    if (!bot->GetGroup())
    {
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "focus_heal_not_in_group",
            "I'm not in a group",
            {});
        botAI->TellMasterNoFacing(text);
        return false;
    }

    for (std::string const& targetName : targetNames)
    {
        bool const add = targetName.find("+") != std::string::npos;
        bool const remove = targetName.find("-") != std::string::npos;
        if (!add && !remove)
        {
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_add_remove_syntax",
                "Please specify a + for add or - to remove a target",
                {});
            botAI->TellMasterNoFacing(text);
            continue;
        }

        std::string const playerName = targetName.substr(1);
        Player* target = FindGroupPlayerByName(bot, playerName);
        if (!target)
        {
            std::map<std::string, std::string> placeholders;
            placeholders["%player_name"] = playerName;
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_not_in_group_with",
                "I'm not in a group with %player_name",
                placeholders);
            botAI->TellMasterNoFacing(text);
            continue;
        }

        ObjectGuid const& targetGuid = target->GetGUID();
        if (add)
        {
            if (std::find(focusHealTargets.begin(), focusHealTargets.end(), targetGuid) ==
                focusHealTargets.end())
                focusHealTargets.push_back(targetGuid);

            std::map<std::string, std::string> placeholders;
            placeholders["%player_name"] = playerName;
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_added",
                "Added %player_name to focus heal targets",
                placeholders);
            botAI->TellMasterNoFacing(text);
        }
        else
        {
            focusHealTargets.remove(targetGuid);
            std::map<std::string, std::string> placeholders;
            placeholders["%player_name"] = playerName;
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "focus_heal_removed",
                "Removed %player_name from focus heal targets",
                placeholders);
            botAI->TellMasterNoFacing(text);
        }
    }

    SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);

    if (focusHealTargets.empty())
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
    else
        botAI->ChangeStrategy("+focus heal targets", BOT_STATE_COMBAT);

    return true;
}
