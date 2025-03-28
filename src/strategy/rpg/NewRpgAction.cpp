#include "NewRpgAction.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>

#include "ChatHelper.h"
#include "G3D/Vector2.h"
#include "GossipDef.h"
#include "IVMapMgr.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Position.h"
#include "QuestDef.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "StatsWeightCalculator.h"
#include "Timer.h"
#include "TravelMgr.h"
#include "BroadcastHelper.h"
#include "World.h"

bool TellRpgStatusAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;
    std::string out = botAI->rpgInfo.ToString();
    bot->Whisper(out.c_str(), LANG_UNIVERSAL, owner);
    return true;
}

bool StartRpgDoQuestAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(owner);
    uint32 questId = ch.extractQuestId(text);
    const Quest* quest = sObjectMgr->GetQuestTemplate(questId);
    if (quest) 
    {
        botAI->rpgInfo.ChangeToDoQuest(questId, quest);
        bot->Whisper("Start to do quest " + std::to_string(questId), LANG_UNIVERSAL, owner);
        return true;
    }
    bot->Whisper("Invalid quest " + text, LANG_UNIVERSAL, owner);
    return false;
}

bool NewRpgStatusUpdateAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;
    /// @TODO: Refactor by transition probability
    switch (info.status)
    {
        case RPG_IDLE:
        {
            uint32 roll = urand(1, 100);
            // IDLE -> NEAR_NPC
            if (roll <= 30)
            {
                GuidVector possibleTargets = AI_VALUE(GuidVector, "possible new rpg targets");
                if (possibleTargets.size() >= 3)
                {
                    info.ChangeToNearNpc();
                    return true;
                }
            }
            // IDLE -> GO_INNKEEPER
            else if (roll <= 45)
            {
                WorldPosition pos = SelectRandomInnKeeperPos(bot);
                if (pos != WorldPosition() && bot->GetExactDist(pos) > 50.0f)
                {
                    info.ChangeToGoInnkeeper(pos);
                    return true;
                }
            }
            // IDLE -> GO_GRIND
            else if (roll <= 100)
            {
                if (roll >= 60)
                {
                    std::vector<uint32> availableQuests;
                    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
                    {
                        uint32 questId = bot->GetQuestSlotQuestId(slot);
                        if (botAI->lowPriorityQuest.find(questId) != botAI->lowPriorityQuest.end())
                            continue;

                        std::vector<POIInfo> poiInfo;
                        if (GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
                        {
                            availableQuests.push_back(questId);
                        }
                    }
                    if (availableQuests.size())
                    {
                        uint32 questId = availableQuests[urand(0, availableQuests.size() - 1)];
                        const Quest* quest = sObjectMgr->GetQuestTemplate(questId);
                        if (quest)
                        {
                            // IDLE -> DO_QUEST
                            info.ChangeToDoQuest(questId, quest);
                            return true;
                        }
                    }
                }
                WorldPosition pos = SelectRandomGrindPos(bot);
                if (pos != WorldPosition())
                {
                    info.ChangeToGoGrind(pos);
                    return true;
                }
            }
            // IDLE -> REST
            info.ChangeToRest();
            bot->SetStandState(UNIT_STAND_STATE_SIT);
            return true;
        }
        case RPG_GO_GRIND:
        {
            WorldPosition& originalPos = info.go_grind.pos;
            assert(info.go_grind.pos != WorldPosition());
            // GO_GRIND -> NEAR_RANDOM
            if (bot->GetExactDist(originalPos) < 10.0f)
            {
                info.ChangeToNearRandom();
                return true;
            }
            break;
        }
        case RPG_GO_INNKEEPER:
        {
            WorldPosition& originalPos = info.go_innkeeper.pos;
            assert(info.go_innkeeper.pos != WorldPosition());
            // GO_INNKEEPER -> NEAR_NPC
            if (bot->GetExactDist(originalPos) < 10.0f)
            {
                info.ChangeToNearNpc();
                return true;
            }
            break;
        }
        case RPG_NEAR_RANDOM:
        {
            // NEAR_RANDOM -> IDLE
            if (info.HasStatusPersisted(statusNearRandomDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_DO_QUEST:
        {
            // DO_QUEST -> IDLE
            if (info.HasStatusPersisted(statusDoQuestDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_NEAR_NPC:
        {
            if (info.HasStatusPersisted(statusNearNpcDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_REST:
        {
            // REST -> IDLE
            if (info.HasStatusPersisted(statusRestDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

bool NewRpgGoGrindAction::Execute(Event event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    return MoveFarTo(botAI->rpgInfo.go_grind.pos);
}

bool NewRpgGoInnKeeperAction::Execute(Event event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    return MoveFarTo(botAI->rpgInfo.go_innkeeper.pos);
}

bool NewRpgMoveRandomAction::Execute(Event event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;
    
    return MoveRandomNear();
}

bool NewRpgMoveNpcAction::Execute(Event event)
{
    NewRpgInfo& info = botAI->rpgInfo;
    if (!info.near_npc.npcOrGo)
    {
        // No npc can be found, switch to IDLE
        ObjectGuid npcOrGo = ChooseNpcOrGameObjectToInteract();
        if (npcOrGo.IsEmpty())
        {
            info.ChangeToIdle();
            return true;
        }
        info.near_npc.npcOrGo = npcOrGo;
        info.near_npc.lastReach = 0;
        return true;
    }

    WorldObject* object = ObjectAccessor::GetWorldObject(*bot, info.near_npc.npcOrGo);
    if (object && IsWithinInteractionDist(object))
    {
        if (!info.near_npc.lastReach)
        {
            info.near_npc.lastReach = getMSTime();
            if (bot->CanInteractWithQuestGiver(object))
                InteractWithNpcOrGameObjectForQuest(info.near_npc.npcOrGo);
            return true;
        }

        if (info.near_npc.lastReach && GetMSTimeDiffToNow(info.near_npc.lastReach) < npcStayTime)
            return false;

        // has reached the npc for more than `npcStayTime`, select the next target
        info.near_npc.npcOrGo = ObjectGuid();
        info.near_npc.lastReach = 0;
    }
    else
    {
        return MoveWorldObjectTo(info.near_npc.npcOrGo);
    }
    return true;
}

bool NewRpgDoQuestAction::Execute(Event event)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    NewRpgInfo& info = botAI->rpgInfo;
    uint32 questId = RPG_INFO(quest, questId);
    const Quest* quest = RPG_INFO(quest, quest);
    uint8 questStatus = bot->GetQuestStatus(questId);
    switch (questStatus)
    {
        case QUEST_STATUS_INCOMPLETE:
            return DoIncompleteQuest();
        case QUEST_STATUS_COMPLETE:
            return DoCompletedQuest();
        default:
            break;
    }
    botAI->rpgInfo.ChangeToIdle();
    return true;
}

bool NewRpgDoQuestAction::DoIncompleteQuest()
{
    uint32 questId = RPG_INFO(do_quest, questId);
    if (botAI->rpgInfo.do_quest.pos != WorldPosition())
    {
        /// @TODO: extract to a new function
        int32 currentObjective = botAI->rpgInfo.do_quest.objectiveIdx;
        // check if the objective has completed
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        const QuestStatusData& q_status = bot->getQuestStatusMap().at(questId);
        bool completed = true;
        if (currentObjective < QUEST_OBJECTIVES_COUNT)
        {
            if (q_status.CreatureOrGOCount[currentObjective] < quest->RequiredNpcOrGoCount[currentObjective])
                completed = false;
        }
        else if (currentObjective < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        {
            if (q_status.ItemCount[currentObjective - QUEST_OBJECTIVES_COUNT] <
                quest->RequiredItemCount[currentObjective - QUEST_OBJECTIVES_COUNT])
                completed = false;
        }
        // the current objective is completed, clear and find a new objective later
        if (completed)
        {
            botAI->rpgInfo.do_quest.lastReachPOI = 0;
            botAI->rpgInfo.do_quest.pos = WorldPosition();
            botAI->rpgInfo.do_quest.objectiveIdx = 0;
        }
    }
    if (botAI->rpgInfo.do_quest.pos == WorldPosition())
    {
        std::vector<POIInfo> poiInfo;
        if (!GetQuestPOIPosAndObjectiveIdx(questId, poiInfo))
        {
            // can't find a poi pos to go, stop doing quest for now
            botAI->rpgInfo.ChangeToIdle();
            return true;
        }
        uint32 rndIdx = urand(0, poiInfo.size() - 1);
        G3D::Vector2 nearestPoi = poiInfo[rndIdx].pos;
        int32 objectiveIdx = poiInfo[rndIdx].objectiveIdx;

        float dx = nearestPoi.x, dy = nearestPoi.y;

        // z = MAX_HEIGHT as we do not know accurate z
        float dz = std::max(bot->GetMap()->GetHeight(dx, dy, MAX_HEIGHT), bot->GetMap()->GetWaterLevel(dx, dy));

        // double check for GetQuestPOIPosAndObjectiveIdx
        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
            return false;

        WorldPosition pos(bot->GetMapId(), dx, dy, dz);
        botAI->rpgInfo.do_quest.lastReachPOI = 0;
        botAI->rpgInfo.do_quest.pos = pos;
        botAI->rpgInfo.do_quest.objectiveIdx = objectiveIdx;
    }

    if (bot->GetDistance(botAI->rpgInfo.do_quest.pos) > 10.0f && !botAI->rpgInfo.do_quest.lastReachPOI)
    {
        return MoveFarTo(botAI->rpgInfo.do_quest.pos);
    }
    // Now we are near the quest objective
    // kill mobs and looting quest should be done automatically by grind strategy

    if (!botAI->rpgInfo.do_quest.lastReachPOI)
    {
        botAI->rpgInfo.do_quest.lastReachPOI = getMSTime();
        return true;
    }
    // stayed at this POI for more than 5 minutes
    if (GetMSTimeDiffToNow(botAI->rpgInfo.do_quest.lastReachPOI) >= poiStayTime)
    {
        bool hasProgression = false;
        int32 currentObjective = botAI->rpgInfo.do_quest.objectiveIdx;
        // check if the objective has progression
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        const QuestStatusData& q_status = bot->getQuestStatusMap().at(questId);
        if (currentObjective < QUEST_OBJECTIVES_COUNT)
        {
            if (q_status.CreatureOrGOCount[currentObjective] != 0 && quest->RequiredNpcOrGoCount[currentObjective])
                hasProgression = true;
        }
        else if (currentObjective < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        {
            if (q_status.ItemCount[currentObjective - QUEST_OBJECTIVES_COUNT] != 0 &&
                quest->RequiredItemCount[currentObjective - QUEST_OBJECTIVES_COUNT])
                hasProgression = true;
        }
        if (!hasProgression)
        {
            // we has reach the poi for more than 5 mins but no progession
            // may not be able to complete this quest, marked as abandoned
            /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
            botAI->lowPriorityQuest.insert(questId);
            botAI->rpgStatistic.questAbandoned++;
            LOG_DEBUG("playerbots", "[New rpg] {} marked as abandoned quest {}", bot->GetName(), questId);
            botAI->rpgInfo.ChangeToIdle();
            return true;
        }
        // clear and select another poi later
        botAI->rpgInfo.do_quest.lastReachPOI = 0;
        botAI->rpgInfo.do_quest.pos = WorldPosition();
        botAI->rpgInfo.do_quest.objectiveIdx = 0;
        return true;
    }

    return MoveRandomNear(20.0f);
}

bool NewRpgDoQuestAction::DoCompletedQuest()
{
    uint32 questId = RPG_INFO(quest, questId);
    const Quest* quest = RPG_INFO(quest, quest);
    
    if (RPG_INFO(quest, objectiveIdx) != -1)
    {
        // if quest is completed, back to poi with -1 idx to reward
        BroadcastHelper::BroadcastQuestUpdateComplete(botAI, bot, quest);
        botAI->rpgStatistic.questCompleted++;
        std::vector<POIInfo> poiInfo;
        if (!GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
        {
            // can't find a poi pos to reward, stop doing quest for now
            botAI->rpgInfo.ChangeToIdle();
            return false;
        }
        assert(poiInfo.size() > 0);
        // now we get the place to get rewarded
        float dx = poiInfo[0].pos.x, dy = poiInfo[0].pos.y;
        // z = MAX_HEIGHT as we do not know accurate z
        float dz = std::max(bot->GetMap()->GetHeight(dx, dy, MAX_HEIGHT), bot->GetMap()->GetWaterLevel(dx, dy));

        // double check for GetQuestPOIPosAndObjectiveIdx
        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
            return false;
        
        WorldPosition pos(bot->GetMapId(), dx, dy, dz);
        botAI->rpgInfo.do_quest.lastReachPOI = 0;
        botAI->rpgInfo.do_quest.pos = pos;
        botAI->rpgInfo.do_quest.objectiveIdx = -1;
    }

    if (botAI->rpgInfo.do_quest.pos == WorldPosition())
        return false;

    if (bot->GetDistance(botAI->rpgInfo.do_quest.pos) > 10.0f && !botAI->rpgInfo.do_quest.lastReachPOI)
        return MoveFarTo(botAI->rpgInfo.do_quest.pos);

    // Now we are near the qoi of reward
    // the quest should be rewarded by SearchQuestGiverAndAcceptOrReward
    if (!botAI->rpgInfo.do_quest.lastReachPOI)
    {
        botAI->rpgInfo.do_quest.lastReachPOI = getMSTime();
        return true;
    }
    // stayed at this POI for more than 5 minutes
    if (GetMSTimeDiffToNow(botAI->rpgInfo.do_quest.lastReachPOI) >= poiStayTime)
    {
        // e.g. Can not reward quest to gameobjects
        /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
        botAI->lowPriorityQuest.insert(questId);
        botAI->rpgStatistic.questAbandoned++;
        LOG_DEBUG("playerbots", "[New rpg] {} marked as abandoned quest {}", bot->GetName(), questId);
        botAI->rpgInfo.ChangeToIdle();
        return true;
    }
    return false;
}
