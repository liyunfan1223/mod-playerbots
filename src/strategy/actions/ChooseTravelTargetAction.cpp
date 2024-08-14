/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseTravelTargetAction.h"

#include "ChatHelper.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

bool ChooseTravelTargetAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    //Get the current travel target. This target is no longer active.
    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    //Select a new target to travel to. 
    TravelTarget newTarget = TravelTarget(botAI);

    if (!oldTarget) return false;

    if (!oldTarget->isForced() || oldTarget->getStatus() == TravelStatus::TRAVEL_STATUS_EXPIRED)
        getNewTarget(&newTarget, oldTarget);
    else
        newTarget.copyTarget(oldTarget);

    //If the new target is not active we failed.
    if (!newTarget.isActive() && !newTarget.isForced())
        return false;

    setNewTarget(&newTarget, oldTarget);
    return true;
}

// Select a new travel target.
// Currently this selectes mostly based on priority (current quest > new quest).
// This works fine because destinations can be full (max 15 bots per quest giver, max 1 bot per quest mob).
//
// Eventually we want to rewrite this to be more intelligent.
void ChooseTravelTargetAction::getNewTarget(TravelTarget* newTarget, TravelTarget* oldTarget)
{
    // Join groups members
    bool foundTarget = foundTarget = SetGroupTarget(newTarget);

    //Empty bags/repair
    if (!foundTarget && urand(1, 100) > 10 && bot->GetLevel() > 5)           //90% chance
    {
        if  (AI_VALUE2(bool, "group or", "should sell,can sell,following party,near leader") ||
             AI_VALUE2(bool, "group or", "should repair,can repair,following party,near leader")
            )
        {
            foundTarget = SetRpgTarget(newTarget);                           //Go to town to sell items or repair
        }
    }

    //Rpg in city
    if (!foundTarget && urand(1, 100) > 90 && bot->GetLevel() > 5)           //10% chance
    {
        foundTarget = SetNpcFlagTarget(newTarget, { UNIT_NPC_FLAG_BANKER,UNIT_NPC_FLAG_BATTLEMASTER,UNIT_NPC_FLAG_AUCTIONEER });
    }

    // PvP activities
    bool pvpActivate = false;
    if (pvpActivate && !foundTarget && urand(0, 4) && bot->GetLevel() > 50)
    {
        WorldPosition pos = WorldPosition(bot);
        WorldPosition* botPos = &pos;
        TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

        TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, "Tarren Mill");
        if (dest)
        {
            std::vector<WorldPosition*> points = dest->nextPoint(botPos, true);
            if (!points.empty())
            {
                target->setTarget(dest, points.front());
                target->setForced(true);

                std::ostringstream out; out << "Traveling to " << dest->getTitle();
                botAI->TellMasterNoFacing(out.str());
                foundTarget = true;
            }
        }
    }

    //Grind for money
    if (!foundTarget && AI_VALUE(bool, "should get money"))
    {
        //Empty mail for money
        //if (AI_VALUE(bool, "can get mail"))
        //{
        //    foundTarget = SetGOTypeTarget(requester, newTarget, GAMEOBJECT_TYPE_MAILBOX, "", false);  //Find a mailbox
        //}

        if (!foundTarget)
        {
            // 50% Focus on active quests for money.
            if (urand(1, 100) > 50)
            {
                // 50% Focus on active quests for money.
                if (urand(1, 100) > 50)
                {
                    // Turn in quests for money.
                    foundTarget = SetQuestTarget(newTarget, true, false, true, true);
                }

                if (!foundTarget)
                {
                    // Find new (low) level quests
                    foundTarget = SetQuestTarget(newTarget, false, true, false, false);
                }
            }
            else
            {
                // Go grind mobs for money
                foundTarget = SetGrindTarget(newTarget);
            }
        }
    }


    //Continue current target. 90% chance
    if (!foundTarget && urand(1, 100) > 10)
    {
        // Extend current target.
        foundTarget = SetCurrentTarget(newTarget, oldTarget);
    }

    //Get mail 30% chance
    //if (!foundTarget && urand(1, 100) > 70)
    //{
    //    if (AI_VALUE(bool, "can get mail"))
    //    {
    //        foundTarget = SetGOTypeTarget(requester, newTarget, GAMEOBJECT_TYPE_MAILBOX, "", false);  //Find a mailbox
    //    }
    //}

    //Dungeon in group. 50% chance
    if (!foundTarget && urand(1, 100) > 50)
    {
        if (AI_VALUE(bool, "can fight boss"))
        {
            // Go fight a (dungeon boss)
            foundTarget = SetBossTarget(newTarget);
        }
    }
    
    //Do quests (start, do, end) 95% chance
    if (!foundTarget && urand(1, 100) > 5)
    {
        // Do any nearby
        foundTarget = SetQuestTarget(newTarget, false, true, true, true);
    }

    //Explore a nearby unexplored area.
    if (!foundTarget && botAI->HasStrategy("explore", BotState::BOT_STATE_NON_COMBAT) && urand(1, 100) > 90)  //10% chance Explore a unexplored sub-zone.
    {
        foundTarget = SetExploreTarget(newTarget);
    }

    //Just hang with an npc 50% chance
    if (!foundTarget && urand(1, 100) > 50)
    {
        foundTarget = SetRpgTarget(newTarget);
        if (foundTarget)
            newTarget->setForced(true);
    }

    if (!foundTarget)
    {
        foundTarget = SetGrindTarget(newTarget);
    }

    // Idle a bit.
    if (!foundTarget)
        SetNullTarget(newTarget);
}

void ChooseTravelTargetAction::setNewTarget(TravelTarget* newTarget, TravelTarget* oldTarget)
{
    // Tell the master where we are going.
    if (!bot->GetGroup() || (botAI->GetGroupMaster() == bot))
        ReportTravelTarget(newTarget, oldTarget);

    // If we are heading to a creature/npc clear it from the ignore list.
    if (oldTarget && oldTarget == newTarget && newTarget->getEntry())
    {
        GuidSet& ignoreList = context->GetValue<GuidSet&>("ignore rpg target")->Get();

        for (ObjectGuid const guid : ignoreList)
        {
            if (guid.GetEntry() == newTarget->getEntry())
            {
                ignoreList.erase(guid);
            }
        }

        context->GetValue<GuidSet&>("ignore rpg target")->Set(ignoreList);
    }

    // Actually apply the new target to the travel target used by the bot.
    oldTarget->copyTarget(newTarget);

    // If we are idling but have a master. Idle only 10 seconds.
    if (botAI->GetMaster() && oldTarget->isActive() &&
        oldTarget->getDestination()->getName() == "NullTravelDestination")
        oldTarget->setExpireIn(10 * IN_MILLISECONDS);
    else if (oldTarget->isForced())  // Make sure travel goes into cooldown after getting to the destination.
        oldTarget->setExpireIn(HOUR * IN_MILLISECONDS);

    // Clear rpg and pull/grind target. We want to travel, not hang around some more.
    RESET_AI_VALUE(GuidPosition, "rpg target");
    RESET_AI_VALUE(ObjectGuid, "pull target");
}

// Tell the master what travel target we are moving towards.
// This should at some point be rewritten to be denser or perhaps logic moved to ->getTitle()
void ChooseTravelTargetAction::ReportTravelTarget(TravelTarget* newTarget, TravelTarget* oldTarget)
{
    TravelDestination* destination = newTarget->getDestination();
    TravelDestination* oldDestination = oldTarget->getDestination();

    std::ostringstream out;

    if (newTarget->isForced())
        out << "(Forced) ";

    if (destination->getName() == "QuestRelationTravelDestination" ||
        destination->getName() == "QuestObjectiveTravelDestination")
    {
        QuestTravelDestination* QuestDestination = (QuestTravelDestination*)destination;
        Quest const* quest = QuestDestination->GetQuestTemplate();
        WorldPosition botLocation(bot);

        CreatureTemplate const* cInfo = nullptr;
        GameObjectTemplate const* gInfo = nullptr;

        if (destination->getEntry() > 0)
            cInfo = sObjectMgr->GetCreatureTemplate(destination->getEntry());
        else
            gInfo = sObjectMgr->GetGameObjectTemplate(destination->getEntry() * -1);

        std::string Sub;

        if (newTarget->isGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->getDestination()->distanceTo(&botLocation)) << "y";

        out << " for " << chat->FormatQuest(quest);

        out << " to " << QuestDestination->getTitle();

        botAI->TellMaster(out);
    }
    else if (destination->getName() == "RpgTravelDestination")
    {
        RpgTravelDestination* RpgDestination = (RpgTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->isGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->getDestination()->distanceTo(&botLocation)) << "y";

        out << " for ";

        if (AI_VALUE2(bool, "group or", "should sell,can sell"))
            out << "selling items";
        else if (AI_VALUE2(bool, "group or", "should repair,can repair"))
            out << "repairing";
        else
            out << "rpg";

        out << " to " << RpgDestination->getTitle();

        botAI->TellMaster(out);
    }
    else if (destination->getName() == "ExploreTravelDestination")
    {
        ExploreTravelDestination* ExploreDestination = (ExploreTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->isGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->getDestination()->distanceTo(&botLocation)) << "y";

        out << " for exploration";

        out << " to " << ExploreDestination->getTitle();

        botAI->TellMaster(out);
    }
    else if (destination->getName() == "GrindTravelDestination")
    {
        GrindTravelDestination* GrindDestination = (GrindTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->isGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->getDestination()->distanceTo(&botLocation)) << "y";

        out << " for grinding money";

        out << " to " << GrindDestination->getTitle();

        botAI->TellMaster(out);
    }
    else if (destination->getName() == "BossTravelDestination")
    {
        BossTravelDestination* BossDestination = (BossTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->isGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->getDestination()->distanceTo(&botLocation)) << "y";

        out << " for good loot";

        out << " to " << BossDestination->getTitle();

        botAI->TellMaster(out);
    }
    else if (destination->getName() == "NullTravelDestination")
    {
        if (!oldTarget->getDestination() || oldTarget->getDestination()->getName() != "NullTravelDestination")
        {
            botAI->TellMaster("No where to travel. Idling a bit.");
        }
    }
}

bool ChooseTravelTargetAction::getBestDestination(std::vector<TravelDestination*>* activeDestinations,
                                                  std::vector<WorldPosition*>* activePoints)
{
    if (activeDestinations->empty() || activePoints->empty())  // No targets or no points.
        return false;

    WorldPosition botLocation(bot);
    std::vector<WorldPosition*> availablePoints =
        sTravelMgr->getNextPoint(&botLocation, *activePoints);  // Pick a good point.

    if (availablePoints.empty())  // No points available.
        return false;

    TravelDestination* targetDestination;

    for (auto activeTarget : *activeDestinations)  // Pick the destination that has this point.
        if (activeTarget->distanceTo(availablePoints.front()) == 0)
            targetDestination = activeTarget;

    if (!targetDestination)
        return false;

    activeDestinations->clear();
    activePoints->clear();

    activeDestinations->push_back(targetDestination);
    activePoints->push_back(availablePoints.front());

    return true;
}

bool ChooseTravelTargetAction::SetGroupTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    GuidList groupPlayers;

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (ref->GetSource() != bot)
            {
                if (ref->getSubGroup() != bot->GetSubGroup())
                {
                    groupPlayers.push_back(ref->GetSource()->GetGUID());
                }
                else
                {
                    groupPlayers.push_front(ref->GetSource()->GetGUID());
                }
            }
        }
    }

    // Find targets of the group.
    for (auto& member : groupPlayers)
    {
        Player* player = ObjectAccessor::FindPlayer(member);
        if (!player)
            continue;

        PlayerbotAI* playerBotAI = GET_PLAYERBOT_AI(player);
        if (!playerBotAI)
            continue;

        if (!playerBotAI->GetAiObjectContext())
            continue;

        TravelTarget* groupTarget = playerBotAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

        if (groupTarget->isGroupCopy())
            continue;

        if (!groupTarget->isActive())
            continue;

        if (!groupTarget->getDestination()->isActive(bot) ||
            groupTarget->getDestination()->getName() == "RpgTravelDestination")
            continue;

        activeDestinations.push_back(groupTarget->getDestination());
        activePoints.push_back(groupTarget->getPosition());
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front(), true);

    return target->isActive();
}

bool ChooseTravelTargetAction::SetCurrentTarget(TravelTarget* target, TravelTarget* oldTarget)
{
    TravelDestination* oldDestination = oldTarget->getDestination();

    if (oldTarget->isMaxRetry(false))
        return false;

    if (!oldDestination)  // Does this target have a destination?
        return false;

    if (!oldDestination->isActive(bot))  // Is the destination still valid?
        return false;

    WorldPosition botLocation(bot);
    std::vector<WorldPosition*> availablePoints = oldDestination->nextPoint(&botLocation);
    if (availablePoints.empty())
        return false;

    target->setTarget(oldTarget->getDestination(), availablePoints.front());
    target->setStatus(TRAVEL_STATUS_TRAVEL);
    target->setRetry(false, oldTarget->getRetryCount(false) + 1);

    return target->isActive();
}

bool ChooseTravelTargetAction::SetQuestTarget(TravelTarget* target, bool onlyCompleted, bool newQuests, bool activeQuests, bool completedQuests)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;
    WorldPosition botLocation(bot);
    bool onlyClassQuest = !urand(0, 10);

    if (newQuests)
    {
        // Prefer new quests near the player at lower levels.
        activeDestinations = sTravelMgr->getQuestTravelDestinations(bot, -1, true, false, 400 + bot->GetLevel() * 10);
    }
    if (activeQuests || completedQuests)
    {
        QuestStatusMap& questMap = bot->getQuestStatusMap();
        //Find destinations related to the active quests.
        for (auto& quest : questMap)
        {
            if (bot->IsQuestRewarded(quest.first))
                continue;

            uint32 questId = quest.first;
            QuestStatusData* questStatus = &quest.second;
            const auto questTemplate = sObjectMgr->GetQuestTemplate(questId);

            if (!activeQuests && !bot->CanRewardQuest(questTemplate, false))
                continue;

            if (!completedQuests && bot->CanRewardQuest(questTemplate, false))
                continue;

            //Find quest takers or objectives
            std::vector<TravelDestination*> questDestinations = sTravelMgr->getQuestTravelDestinations(bot, questId, true, false, 0);

            if (onlyClassQuest && activeDestinations.size() && questDestinations.size()) //Only do class quests if we have any.
            {
                if (activeDestinations.front()->GetQuestTemplate()->GetRequiredClasses() && !questTemplate->GetRequiredClasses())
                    continue;

                if (!activeDestinations.front()->GetQuestTemplate()->GetRequiredClasses() && questTemplate->GetRequiredClasses())
                    activeDestinations.clear();
            }

            activeDestinations.insert(activeDestinations.end(), questDestinations.begin(), questDestinations.end());
        }
    }
    if (newQuests && activeDestinations.empty())
        activeDestinations = sTravelMgr->getQuestTravelDestinations(bot, -1, true, false); //If we really don't find any new quests look futher away.

    if (botAI->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        botAI->TellMasterNoFacing(std::to_string(activeDestinations.size()) + " quest destinations found.");

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

bool ChooseTravelTargetAction::SetNewQuestTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    WorldPosition botLocation(bot);

    // Find quest givers.
    std::vector<TravelDestination*> TravelDestinations =
        sTravelMgr->getQuestTravelDestinations(bot, -1, botAI->HasRealPlayerMaster());

    activeDestinations.insert(activeDestinations.end(), TravelDestinations.begin(), TravelDestinations.end());

    // Pick one good point per destination.
    for (auto& activeTarget : activeDestinations)
    {
        std::vector<WorldPosition*> points = activeTarget->nextPoint(&botLocation);
        if (!points.empty())
            activePoints.push_back(points.front());
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

bool ChooseTravelTargetAction::SetRpgTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    WorldPosition botLocation(bot);

    // Find rpg npcs
    std::vector<TravelDestination*> TravelDestinations =
        sTravelMgr->getRpgTravelDestinations(bot, botAI->HasRealPlayerMaster());

    activeDestinations.insert(activeDestinations.end(), TravelDestinations.begin(), TravelDestinations.end());

    // Pick one good point per destination.
    for (auto& activeTarget : activeDestinations)
    {
        std::vector<WorldPosition*> points = activeTarget->nextPoint(&botLocation);
        if (!points.empty())
            activePoints.push_back(points.front());
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

bool ChooseTravelTargetAction::SetGrindTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    WorldPosition botLocation(bot);

    // Find grind mobs.
    std::vector<TravelDestination*> TravelDestinations =
        sTravelMgr->getGrindTravelDestinations(bot, botAI->HasRealPlayerMaster());

    activeDestinations.insert(activeDestinations.end(), TravelDestinations.begin(), TravelDestinations.end());

    // Pick one good point per destination.
    for (auto& activeTarget : activeDestinations)
    {
        std::vector<WorldPosition*> points = activeTarget->nextPoint(&botLocation);
        if (!points.empty())
            activePoints.push_back(points.front());
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

bool ChooseTravelTargetAction::SetBossTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    WorldPosition botLocation(bot);

    // Find boss mobs.
    std::vector<TravelDestination*> TravelDestinations =
        sTravelMgr->getBossTravelDestinations(bot, botAI->HasRealPlayerMaster());

    activeDestinations.insert(activeDestinations.end(), TravelDestinations.begin(), TravelDestinations.end());

    // Pick one good point per destination.
    for (auto& activeTarget : activeDestinations)
    {
        std::vector<WorldPosition*> points = activeTarget->nextPoint(&botLocation);
        if (!points.empty())
            activePoints.push_back(points.front());
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

bool ChooseTravelTargetAction::SetExploreTarget(TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    WorldPosition botLocation(bot);

    // Find quest givers.
    std::vector<TravelDestination*> TravelDestinations = sTravelMgr->getExploreTravelDestinations(bot, true, true);

    activeDestinations.insert(activeDestinations.end(), TravelDestinations.begin(), TravelDestinations.end());
    /*
    //Pick one good point per destination.
    for (auto& activeTarget : activeDestinations)
    {
        //271 south shore
        //35 booty bay
        //380 The Barrens The Crossroads
        if(((ExploreTravelDestination * )activeTarget)->getAreaId() == 380)
        {
            activePoints.push_back(activeTarget->getPoints(true)[0]);
        }
    }
    */

    if (activePoints.empty())
    {
        TravelDestinations = sTravelMgr->getExploreTravelDestinations(bot, botAI->HasRealPlayerMaster());

        for (auto& activeTarget : activeDestinations)
        {
            std::vector<WorldPosition*> points = activeTarget->nextPoint(&botLocation);
            if (!points.empty())
            {
                activePoints.push_back(points.front());
            }
        }
    }

    if (!getBestDestination(&activeDestinations, &activePoints))
        return false;

    target->setTarget(activeDestinations.front(), activePoints.front());

    return target->isActive();
}

char* strstri(char const* haystack, char const* needle);

bool ChooseTravelTargetAction::SetNpcFlagTarget(TravelTarget* target, std::vector<NPCFlags> flags,
                                                std::string const name, std::vector<uint32> items)
{
    WorldPosition botPos(bot);

    std::vector<TravelDestination*> dests;

    for (auto& d : sTravelMgr->getRpgTravelDestinations(bot, true, true))
    {
        if (!d->getEntry())
            continue;

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(d->getEntry());
        if (!cInfo)
            continue;

        bool foundFlag = false;
        for (auto flag : flags)
            if (cInfo->npcflag & flag)
            {
                foundFlag = true;
                break;
            }

        if (!foundFlag)
            continue;

        if (!name.empty() && !strstri(cInfo->Name.c_str(), name.c_str()) &&
            !strstri(cInfo->SubName.c_str(), name.c_str()))
            continue;

        if (!items.empty())
        {
            bool foundItem = false;
            VendorItemData const* vItems = sObjectMgr->GetNpcVendorItemList(d->getEntry());
            if (vItems)
            {
                for (auto item : items)
                {
                    for (auto vitem : vItems->m_items)
                    {
                        if (vitem->item == item)
                        {
                            foundItem = true;
                            break;
                        }
                    }
                }
            }

            if (!foundItem)
                continue;
        }

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
        ReputationRank reaction = Unit::GetFactionReactionTo(botAI->GetBot()->GetFactionTemplateEntry(), factionEntry);

        if (reaction < REP_NEUTRAL)
            continue;

        dests.push_back(d);
    }

    if (!dests.empty())
    {
        TravelDestination* dest = *std::min_element(dests.begin(), dests.end(),
                                                    [botPos](TravelDestination* i, TravelDestination* j) {
                                                        return i->distanceTo(const_cast<WorldPosition*>(&botPos)) <
                                                               j->distanceTo(const_cast<WorldPosition*>(&botPos));
                                                    });

        std::vector<WorldPosition*> points = dest->nextPoint(const_cast<WorldPosition*>(&botPos), true);
        if (points.empty())
            return false;

        target->setTarget(dest, points.front());
        target->setForced(true);

        return true;
    }

    return false;
}

std::vector<TravelDestination*> TravelMgr::getBossTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                     float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : bossMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

bool ChooseTravelTargetAction::SetNullTarget(TravelTarget* target)
{
    target->setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);

    return true;
}

std::vector<std::string> split(std::string const s, char delim);
char* strstri(char const* haystack, char const* needle);

TravelDestination* ChooseTravelTargetAction::FindDestination(Player* bot, std::string const name, bool zones, bool npcs, bool quests, bool mobs, bool bosses)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    AiObjectContext* context = botAI->GetAiObjectContext();

    std::vector<TravelDestination*> dests;

    //Quests
    if (quests)
    {
        for (auto& d : sTravelMgr->getQuestTravelDestinations(bot, 0, true, true))
        {
            if (strstri(d->getTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Zones
    if (zones)
    {
        for (auto& d : sTravelMgr->getExploreTravelDestinations(bot, true, true))
        {
            if (strstri(d->getTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Npcs
    if (npcs)
    {
        for (auto& d : sTravelMgr->getRpgTravelDestinations(bot, true, true))
        {
            if (strstri(d->getTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Mobs
    if (mobs)
    {
        for (auto& d : sTravelMgr->getGrindTravelDestinations(bot, true, true, 5000.0f))
        {
            if (strstri(d->getTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Bosses
    if (bosses)
    {
        for (auto& d : sTravelMgr->getBossTravelDestinations(bot, true, true))
        {
            if (strstri(d->getTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    WorldPosition botPos(bot);

    if (dests.empty())
        return nullptr;

    return *std::min_element(dests.begin(), dests.end(),
    [botPos](TravelDestination* i, TravelDestination* j)
    {
        return i->distanceTo(const_cast<WorldPosition*>(&botPos)) < j->distanceTo(const_cast<WorldPosition*>(&botPos));
    });
};

bool ChooseTravelTargetAction::isUseful()
{
    if (!botAI->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    return !context->GetValue<TravelTarget*>("travel target")->Get()->isActive() &&
           !context->GetValue<LootObject>("loot target")->Get().IsLootPossible(bot) && !bot->IsInCombat();
}

bool ChooseTravelTargetAction::needForQuest(Unit* target)
{
    bool justCheck = (bot->GetGUID() == target->GetGUID());

    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (auto& quest : questMap)
    {
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(quest.first);
        if (!questTemplate)
            continue;

        uint32 questId = questTemplate->GetQuestId();

        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
        {
            if (!justCheck && !target->hasInvolvedQuest(questId))
                continue;

            return true;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            QuestStatusData questStatus = quest.second;

            if (questTemplate->GetQuestLevel() > bot->GetLevel())
                continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                int32 entry = questTemplate->RequiredNpcOrGo[j];

                if (entry && entry > 0)
                {
                    int required = questTemplate->RequiredNpcOrGoCount[j];
                    int available = questStatus.CreatureOrGOCount[j];

                    if (required && available < required && (target->GetEntry() == entry || justCheck))
                        return true;
                }

                if (justCheck)
                {
                    int32 itemId = questTemplate->RequiredItemId[j];

                    if (itemId && itemId > 0)
                    {
                        int required = questTemplate->RequiredItemCount[j];
                        int available = questStatus.ItemCount[j];

                        if (required && available < required)
                            return true;
                    }
                }
            }

            if (!justCheck)
            {
                if (CreatureTemplate const* data = sObjectMgr->GetCreatureTemplate(target->GetEntry()))
                {
                    if (uint32 lootId = data->lootid)
                    {
                        if (LootTemplates_Creature.HaveQuestLootForPlayer(lootId, bot))
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ChooseTravelTargetAction::needItemForQuest(uint32 itemId, const Quest* questTemplate,
                                                const QuestStatusData* questStatus)
{
    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (questTemplate->RequiredItemId[i] != itemId)
            continue;

        uint32 required = questTemplate->RequiredItemCount[i];
        uint32 available = questStatus->ItemCount[i];

        if (!required)
            continue;

        return available < required;
    }

    return false;
}
