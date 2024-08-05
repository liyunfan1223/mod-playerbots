/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
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
    Strategy* rpgStrategy = botAI->GetAiObjectContext()->GetStrategy("rpg");

    std::vector<Action*> actions;
    std::vector<uint32> relevances;
    std::vector<TriggerNode*> triggerNodes;
    rpgStrategy->InitTriggers(triggerNodes);

    for (auto& triggerNode : triggerNodes)
    {
        Trigger* trigger = context->GetTrigger(triggerNode->getName());
        if (trigger)
        {
            triggerNode->setTrigger(trigger);

            NextAction** nextActions = triggerNode->getHandlers();

            trigger = triggerNode->getTrigger();

            bool isChecked = false;
            for (int32 i = 0; i < NextAction::size(nextActions); i++)
            {
                NextAction* nextAction = nextActions[i];

                if (nextAction->getRelevance() > 2.0f)
                    continue;

                if (!isChecked && !trigger->IsActive())
                    break;

                isChecked = true;

                Action* action = botAI->GetAiObjectContext()->GetAction(nextAction->getName());

                if (!action->isPossible() || !action->isUseful())
                    continue;

                actions.push_back(action);
                relevances.push_back((nextAction->getRelevance() - 1) * 1000);
            }

            NextAction::destroy(nextActions);
        }
    }

    if (actions.empty())
        return false;

    std::mt19937 gen(time(0));
    sTravelMgr->weighted_shuffle(actions.begin(), actions.end(), relevances.begin(), relevances.end(), gen);

    Action* action = actions.front();

    for (std::vector<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
    {
        TriggerNode* trigger = *i;
        delete trigger;
    }

    triggerNodes.clear();

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
