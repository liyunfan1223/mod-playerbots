/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseRpgTargetAction.h"
#include "BattlegroundMgr.h"
#include "BudgetValues.h"
#include "ChatHelper.h"
#include "Event.h"
#include "Formations.h"
#include "GuildCreateActions.h"
#include "PossibleRpgTargetsValue.h"
#include "Playerbots.h"

#include <random>

bool ChooseRpgTargetAction::HasSameTarget(ObjectGuid guid, uint32 max, GuidVector const& nearGuids)
{
    if (botAI->HasRealPlayerMaster())
        return false;

    uint32 num = 0;

    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);
        if (!player)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI)
            continue;

        if (!botAI->AllowActivity(GRIND_ACTIVITY))
            continue;

        if (PAI_VALUE(GuidPosition, "rpg target") != guid)
            continue;

        num++;
        if (num >= max)
            break;
    }

    return num > 0;
}

float ChooseRpgTargetAction::getMaxRelevance(GuidPosition guidP)
{
    GuidPosition currentRpgTarget = AI_VALUE(GuidPosition, "rpg target");
    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    Strategy* rpgStrategy = botAI->GetAiObjectContext()->GetStrategy("rpg");

    std::vector<TriggerNode*> triggerNodes;
    rpgStrategy->InitTriggers(triggerNodes);

    float maxRelevance = 0.0f;

    for (auto& triggerNode : triggerNodes)
    {
        Trigger* trigger = context->GetTrigger(triggerNode->getName());
        if (trigger)
        {
            triggerNode->setTrigger(trigger);

            if (triggerNode->getFirstRelevance() < maxRelevance || triggerNode->getFirstRelevance() > 2.0f)
                continue;

            trigger = triggerNode->getTrigger();
            if (!trigger->IsActive())
                continue;

            maxRelevance = triggerNode->getFirstRelevance();
        }
    }

    SET_AI_VALUE(GuidPosition, "rpg target", currentRpgTarget);

    for (std::vector<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
    {
        TriggerNode* trigger = *i;
        delete trigger;
    }

    triggerNodes.clear();

    return (maxRelevance - 1.0) * 1000.0f;
}

bool ChooseRpgTargetAction::Execute(Event event)
{
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    uint32 num = 0;

    std::unordered_map<ObjectGuid, uint32> targets;

    GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
    GuidVector possibleObjects = AI_VALUE(GuidVector, "nearest game objects no los");
    GuidVector possiblePlayers = AI_VALUE(GuidVector, "nearest friendly players");
    GuidSet& ignoreList = AI_VALUE(GuidSet&, "ignore rpg target");

    for (auto target : possibleTargets)
        targets[target] = 0.0f;

    for (auto target : possibleObjects)
        targets[target] = 0.0f;

    for (auto target : possiblePlayers)
        targets[target] = 0.0f;

    if (targets.empty())
    {
        return false;
    }

    if (urand(0, 9))
    {
       for (auto target : ignoreList)
            targets.erase(target);
    }

    SET_AI_VALUE(std::string, "next rpg action", this->getName());

    bool hasGoodRelevance = false;

    for (auto& target : targets)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, target.first);
        if (!unit)
            continue;

        GuidPosition guidP(unit);
        if (!guidP || !guidP.getMap())
            continue;

        float priority = 1;

        if (guidP.GetWorldObject() && !isFollowValid(bot, guidP.GetWorldObject()))
            continue;

        if (guidP.IsGameObject())
        {
            GameObject* go = guidP.GetGameObject();
            if (!go || !go->isSpawned() || go->GetGoState() != GO_STATE_READY)
                continue;
        }
        else if (guidP.IsPlayer())
        {
            Player* player = guidP.GetPlayer();
            if (!player)
                continue;

            if (GET_PLAYERBOT_AI(player))
            {
                GuidPosition guidPP = PAI_VALUE(GuidPosition, "rpg target");
                if (guidPP.IsPlayer())
                {
                    continue;
                }
            }
        }

        if (possiblePlayers.size() > 200 || HasSameTarget(guidP, urand(5, 15), possiblePlayers))
            continue;

        float relevance = getMaxRelevance(guidP);

        if (!hasGoodRelevance || relevance > 1)
            target.second = relevance;

        if (target.second > 1)
            hasGoodRelevance = true;
    }

    SET_AI_VALUE(std::string, "next rpg action", "");

    for (auto it = begin(targets); it != end(targets);)
    {
        if (it->second == 0 || (hasGoodRelevance && it->second <= 1.0))
        {
            it = targets.erase(it);
        }
        else
            ++it;
    }

    if (targets.empty())
    {
        LOG_DEBUG("playerbots", "{} can't choose RPG target: all {} are not available", bot->GetName().c_str(), possibleTargets.size());
        RESET_AI_VALUE(GuidSet&, "ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    std::vector<GuidPosition> guidps;
    std::vector<int32> relevances;

    for (auto& target : targets)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, target.first);
        if (!unit)
            continue;

        GuidPosition guidP(unit);
        if (!guidP)
            continue;

        guidps.push_back(guidP);
        relevances.push_back(target.second);
    }

    std::mt19937 gen(time(0));
    sTravelMgr->weighted_shuffle(guidps.begin(), guidps.end(), relevances.begin(), relevances.end(), gen);

    GuidPosition guidP(guidps.front());
    if (!guidP)
    {
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    if (botAI->HasStrategy("debug", BOT_STATE_NON_COMBAT) && guidP.GetWorldObject())
    {
        std::ostringstream out;
        out << "found: ";
        out << chat->FormatWorldobject(guidP.GetWorldObject());
        out << " " << relevances.front();

        botAI->TellMasterNoFacing(out);
    }

    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    if (!botAI->AllowActivity(RPG_ACTIVITY))
        return false;

    if (AI_VALUE(GuidPosition, "rpg target"))
        return false;

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    if (travelTarget->isTraveling() && isFollowValid(bot, *travelTarget->getPosition()))
        return false;

    if (AI_VALUE(GuidVector, "possible rpg targets").empty())
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    return true;
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldObject* target)
{
    if (!target)
        return false;

    return isFollowValid(bot, WorldPosition(target));
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldPosition pos)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Player* master = botAI->GetGroupMaster();
    Player* realMaster = botAI->GetMaster();
    AiObjectContext* context = botAI->GetAiObjectContext();

    bool inDungeon = false;

    if (botAI->HasActivePlayerMaster())
    {
        if (realMaster->IsInWorld() && realMaster->GetMap()->IsDungeon() && bot->GetMapId() == realMaster->GetMapId())
            inDungeon = true;

        if (realMaster && realMaster->IsInWorld() && realMaster->GetMap()->IsDungeon() && (realMaster->GetMapId() != pos.getMapId()))
            return false;
    }

    if (!master || bot == master)
        return true;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return true;

    if (sqrt(bot->GetDistance(master)) > sPlayerbotAIConfig->rpgDistance * 2)
        return false;

    Formation* formation = AI_VALUE(Formation*, "formation");
    float distance = master->GetDistance2d(pos.getX(), pos.getY());

    if (!botAI->HasActivePlayerMaster() && distance < 50.0f)
    {
        Player* player = master;
        if (!master->isMoving() || PAI_VALUE(WorldPosition, "last long move").distance(pos) < sPlayerbotAIConfig->reactDistance)
            return true;
    }

    if ((inDungeon || !master->HasPlayerFlag(PLAYER_FLAGS_RESTING)) && realMaster == master && distance > 5.0f)
        return false;

    if (!master->isMoving() && distance < 25.0f)
        return true;

    if (distance < formation->GetMaxDistance())
       return true;

    return false;
}
