/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RpgAction.h"

#include <random>

#include "BattlegroundMgr.h"
#include "ChatHelper.h"
#include "EmoteAction.h"
#include "Event.h"
#include "Formations.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "RpgSubActions.h"

bool RpgAction::Execute(Event event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    if (!guidP && botAI->GetMaster())
    {
        if (WorldObject* target = ObjectAccessor::GetWorldObject(*bot, botAI->GetMaster()->GetTarget()))
        {
            guidP = GuidPosition(target);
            if (guidP)
            {
                RemIgnore(guidP);
                SET_AI_VALUE(GuidPosition, "rpg target", guidP);
            }
        }
    }

    if (bot->GetShapeshiftForm() > 0)
        bot->SetShapeshiftForm(FORM_NONE);

    if (!SetNextRpgAction())
        RESET_AI_VALUE(GuidPosition, "rpg target");

    return true;
}

bool RpgAction::isUseful() { return AI_VALUE(GuidPosition, "rpg target"); }

bool RpgAction::SetNextRpgAction()
{
    Strategy* rpgStrategy;
    std::vector<Action*> actions;
    std::vector<uint32> relevances;
    std::vector<TriggerNode*> triggerNodes;


    for (auto& strategy : botAI->GetAiObjectContext()->GetSupportedStrategies())
    {
        if (strategy.find("rpg") == std::string::npos)
            continue;

        rpgStrategy = botAI->GetAiObjectContext()->GetStrategy(strategy);

        rpgStrategy->InitTriggers(triggerNodes);

        for (auto& triggerNode : triggerNodes)
        {
            Trigger* trigger = context->GetTrigger(triggerNode->getName());

            if (trigger)
            {

                triggerNode->setTrigger(trigger);

                NextAction** nextActions = triggerNode->getHandlers();

                Trigger* trigger = triggerNode->getTrigger();

                bool isChecked = false;

                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                {
                    NextAction* nextAction = nextActions[i];

                    if (nextAction->getRelevance() > 5.0f)
                        continue;

                    if (!isChecked && !trigger->IsActive())
                        break;

                    isChecked = true;

                    Action* action = botAI->GetAiObjectContext()->GetAction(nextAction->getName());
                    if (!dynamic_cast<RpgEnabled*>(action) || !action->isPossible() || !action->isUseful())
                        continue;

                    actions.push_back(action);
                    relevances.push_back((nextAction->getRelevance() - 1) * 500);
                }
                NextAction::destroy(nextActions);
            }
        }

        for (const auto i : triggerNodes)
        {
            delete i;
        }
        triggerNodes.clear();
    }

    if (actions.empty())
        return false;

    if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
    {
        std::vector<std::pair<Action*, uint32>> sortedActions;

        for (int i = 0; i < actions.size(); i++)
            sortedActions.push_back(std::make_pair(actions[i], relevances[i]));

        std::sort(sortedActions.begin(), sortedActions.end(), [](std::pair<Action*, uint32>i, std::pair<Action*, uint32> j) {return i.second > j.second; });

        std::stringstream ss;
        ss << "------" << chat->FormatWorldobject(AI_VALUE(GuidPosition, "rpg target").GetWorldObject()) << "------";
        bot->Say(ss.str(), LANG_UNIVERSAL);
        botAI->TellMasterNoFacing(ss.str());

        for (auto action : sortedActions)
        {
            std::ostringstream out;

            out << " " << action.first->getName() << " " << action.second;

            botAI->TellMasterNoFacing(out);
        }
    }

    std::mt19937 gen(time(0));

    sTravelMgr->weighted_shuffle(actions.begin(), actions.end(), relevances.begin(), relevances.end(), gen);

    Action* action = actions.front();

    if ((botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT) || botAI->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT)))
    {
        std::ostringstream out;
        out << "do: ";
        out << chat->FormatWorldobject(AI_VALUE(GuidPosition, "rpg target").GetWorldObject());

        out << " " << action->getName();

        botAI->TellMasterNoFacing(out);
    }

    SET_AI_VALUE(std::string, "next rpg action", action->getName());

    return true;
}

bool RpgAction::AddIgnore(ObjectGuid guid)
{
    if (HasIgnore(guid))
    {
        return false;
    }

    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    ignoreList.insert(guid);

    if (ignoreList.size() > 50)
        ignoreList.erase(ignoreList.begin());

    context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);

    return true;
}
bool RpgAction::RemIgnore(ObjectGuid guid)
{
    if (!HasIgnore(guid))
        return false;

    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    ignoreList.erase(ignoreList.find(guid));

    context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);

    return true;
}

bool RpgAction::HasIgnore(ObjectGuid guid)
{
    GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();
    if (ignoreList.empty())
        return false;

    if (ignoreList.find(guid) == ignoreList.end())
        return false;

    return true;
}

bool CRpgAction::isUseful()
{
    RESET_AI_VALUE(GuidPosition, "rpg target");
    return true;
};
