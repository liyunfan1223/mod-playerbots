/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"

#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

#include "GroupMgr.h"
#include "AiFactory.h"
#include <thread>


bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("seal of justice", target) && !botAI->HasAura("seal of command", target) &&
           !botAI->HasAura("seal of vengeance", target) && !botAI->HasAura("seal of corruption", target) &&
           !botAI->HasAura("seal of righteousness", target) && !botAI->HasAura("seal of light", target) &&
           (!botAI->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

/*
bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr);
}
*/

//greater blessing on party triggers
std::unordered_map<uint32, std::shared_ptr<TeamPaladinManager>> TeamPaladinManagerFactory::teamManagerMap;
std::mutex TeamPaladinManagerFactory::teamManagerMutex;

std::shared_ptr<TeamPaladinManager> TeamPaladinManagerFactory::GetTeamPaladinManager(ObjectGuid groupGuid, int lifetimeSeconds)
{
    std::lock_guard<std::mutex> lock(teamManagerMutex);
    uint32 groupId = groupGuid.GetCounter(); 

    auto it = teamManagerMap.find(groupId);
    if (it != teamManagerMap.end())
    {
        return it->second;
    }

    auto manager = std::make_shared<TeamPaladinManager>(groupGuid);
    teamManagerMap[groupId] = manager;

    ScheduleDestruction(groupId, lifetimeSeconds);

    return manager;
}

void TeamPaladinManagerFactory::ClearTeamPaladinManager(uint32 groupId)
{
    std::lock_guard<std::mutex> lock(teamManagerMutex);
    teamManagerMap.erase(groupId);
}

void TeamPaladinManagerFactory::ScheduleDestruction(uint32 groupId, int lifetimeSeconds)
{
    std::thread([groupId, lifetimeSeconds]() {
        std::this_thread::sleep_for(std::chrono::seconds(lifetimeSeconds));
        ClearTeamPaladinManager(groupId);
    }).detach();
}

TeamPaladinManager::TeamPaladinManager(ObjectGuid groupGuid) : groupGuid(groupGuid)
{
    UpdatePaladinList(); 
}

TeamPaladinManager::~TeamPaladinManager()
{

}

void TeamPaladinManager::UpdatePaladinList()
{
    Group* group = sGroupMgr->GetGroupByGUID(groupGuid.GetCounter()); 
    if (!group)
        return;

    std::vector<ObjectGuid> allPaladins;
    std::set<ObjectGuid> visitedPaladins;

    for (const GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
    {
        Player* member = groupRef->GetSource();
        if (!member || member->getClass() != CLASS_PALADIN || member->GetLevel() < 60 || !member->IsInWorld() || visitedPaladins.count(member->GetGUID()))
            continue;

        allPaladins.push_back(member->GetGUID());
        visitedPaladins.insert(member->GetGUID());
    }

    std::stable_sort(allPaladins.begin(), allPaladins.end(), [](ObjectGuid a, ObjectGuid b) {
        Player* playerA = ObjectAccessor::FindPlayer(a);
        Player* playerB = ObjectAccessor::FindPlayer(b);
        return playerA && playerB && playerA->GetLevel() > playerB->GetLevel();
    });

    sortedPaladins.clear();

    ObjectGuid tankGuid; 
    
    for (ObjectGuid guid : allPaladins)
    {
        Player* paladin = ObjectAccessor::FindPlayer(guid);
        if (!paladin)
            continue;

        BotRoles role = AiFactory::GetPlayerRoles(paladin);
        if (role == BOT_ROLE_TANK)
        {
            tankGuid = guid; 
            break;
        }
    }

    for (ObjectGuid guid : allPaladins)
    {
        if (sortedPaladins.size() >= 4)
            break;

        if (guid != tankGuid)
        {
            sortedPaladins.push_back(guid);
        }
    }

    if (!tankGuid.IsEmpty())
    {
        sortedPaladins.insert(sortedPaladins.begin(), tankGuid);
    }

    if (sortedPaladins.size() > 4)
    {
        sortedPaladins.resize(4);
    }

    std::reverse(sortedPaladins.begin(), sortedPaladins.end());
}


int TeamPaladinManager::GetPaladinOrder(ObjectGuid guid) const
{
    for (size_t i = 0; i < sortedPaladins.size(); ++i)
    {
        if (sortedPaladins[i] == guid)
        {
            return static_cast<int>(i) + 1; 
        }
    }
    return 0; 
}

bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
            return false;
    
    if (bot->isDead())
            return false;
    
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    ObjectGuid groupId = group->GetGUID();
    auto manager = TeamPaladinManagerFactory::GetTeamPaladinManager(groupId);

    if (!manager)
        return false;

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();

    int paladinOrder = manager->GetPaladinOrder(botGuid);

    if (paladinOrder == 1)
    {
        Unit* target = GetTarget();
        if (!target || target->GetLevel() < 50 || target->isDead())
            return false;

        if (botAI->HasAura("greater blessing of kings", target, false, true))
            return false;

        return true;
    }

    return false;
}


bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
        return false;
    
    if (bot->isDead())
        return false;
        
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;
    
    ObjectGuid groupId = group->GetGUID(); 
    auto manager = TeamPaladinManagerFactory::GetTeamPaladinManager(groupId);
    
    if (!manager)
        return false;
    
    ObjectGuid botGuid = botAI->GetBot()->GetGUID();
    
    int paladinOrder = manager->GetPaladinOrder(botGuid);

    if (paladinOrder == 2)
    {
        Unit* target = GetTarget();
            
        if (!target || !target->IsPlayer() || target->GetLevel() < 42)
            return false;

        if (botAI->HasAura("greater blessing of might", target, false, true))
            return false;

        return true;
    }
    return false;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
        return false;

    if (bot->isDead())
        return false;

    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    ObjectGuid groupId = group->GetGUID(); 
    auto manager = TeamPaladinManagerFactory::GetTeamPaladinManager(groupId);

    if (!manager)
        return false;

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();

    int paladinOrder = manager->GetPaladinOrder(botGuid);

    if (paladinOrder == 3) 
    {
        Unit* target = GetTarget();

        if (!target || target->isDead() || target->GetLevel() < 44 || !target->IsPlayer())
            return false;

        if (botAI->HasAura("greater blessing of wisdom", target, false, true))
            return false;
    
        return true;
    }

    return false;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
        return false;

    if (bot->isDead())
        return false;

    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    ObjectGuid groupId = group->GetGUID(); 

    auto manager = TeamPaladinManagerFactory::GetTeamPaladinManager(groupId);

    if (!manager)
        return false;

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();

    int paladinOrder = manager->GetPaladinOrder(botGuid);

    if (paladinOrder == 4)
    {
        Unit* target = GetTarget();
        if (!target || target->GetLevel() < 50 ||target->isDead())
            return false;

        if (botAI->HasAura("greater blessing of sanctuary", target, false, true))
            return false;

        return true;
    }
    return false;
}

//blessing triggers
bool BlessingOfKingsTrigger::IsActive() 
{ 
    return BuffTrigger::IsActive() && !botAI->HasAnyAuraOf(botAI->GetBot(), "greater blessing of kings", "blessing of kings", nullptr) 
                                    && !botAI->HasAnyMyAuraOf(botAI->GetBot(), "greater blessing of might", "blessing of might", 
                                                                               "greater blessing of wisdom", "blessing of wisdom", 
                                                                               "greater blessing of sanctuary", "blessing of sanctuary", nullptr) ;
}

bool BlessingOfWisdomTrigger::IsActive() 
{ 
    
    return BuffTrigger::IsActive() && !botAI->HasAnyAuraOf(botAI->GetBot(), "greater blessing of wisdom", "blessing of wisdom", nullptr)  
                                    && !botAI->HasAnyMyAuraOf(botAI->GetBot(), "greater blessing of kings", "blessing of kings", 
                                                                               "greater blessing of might", "blessing of might", 
                                                                               "greater blessing of sanctuary", "blessing of sanctuary", nullptr) ;
}

bool BlessingOfMightTrigger::IsActive() 
{
    return BuffTrigger::IsActive() && !botAI->HasAnyAuraOf(botAI->GetBot(), "greater blessing of might", "blessing of might", nullptr)  
                                    && !botAI->HasAnyMyAuraOf(botAI->GetBot(), "greater blessing of kings", "blessing of kings", 
                                                                               "greater blessing of wisdom", "blessing of wisdom", 
                                                                               "greater blessing of sanctuary", "blessing of sanctuary", nullptr) ;
}
  

bool BlessingOfSanctuaryTrigger::IsActive() 
{ 
    int tab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (tab == !PALADIN_TAB_PROTECTION)
        return false;
        
    return BuffTrigger::IsActive() && !botAI->HasAnyAuraOf(botAI->GetBot(), "greater blessing of sanctuary", "blessing of sanctuary", nullptr)  
                                    && !botAI->HasAnyMyAuraOf(botAI->GetBot(), "greater blessing of wisdom", "blessing of wisdom", 
                                                                               "greater blessing of might", "blessing of might",
                                                                               "greater blessing of kings", "blessing of kings", nullptr); 
}

//blessing on party triggers
bool BlessingOfKingsOnPartyTrigger::IsActive() 
{ 
    Unit* target = GetTarget();
    
    if (!target || !target->IsPlayer() || target->GetLevel() < 20 || (botAI->GetBot()->GetLevel() > 60 && target->GetLevel() > 40)) 
        return false;
    
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "greater blessing of kings", "blessing of kings", nullptr) 
                                    && !botAI->HasAnyMyAuraOf(target, "blessing of wisdom", "blessing of might", "blessing of sanctuary", nullptr) ;
    
}

bool BlessingOfWisdomOnPartyTrigger::IsActive() 
{ 
    Unit* target = GetTarget();
    
    if (!target || !target->IsPlayer() || target->GetLevel() < 20 || (botAI->GetBot()->GetLevel() > 60 && target->GetLevel() > 40))
        return false;

//    if (target->getClass() == CLASS_WARRIOR || target->getClass() == CLASS_ROGUE || target->getClass() == CLASS_DEATH_KNIGHT)
//        return false;

    return BuffOnPartyTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "greater blessing of wisdom", "blessing of wisdom", nullptr) 
                                    && !botAI->HasAnyMyAuraOf(target, "blessing of kings", "blessing of might", "blessing of sanctuary", nullptr) ;

}

bool BlessingOfMightOnPartyTrigger::IsActive() 
{ 
    Unit* target = GetTarget();
    
    if (!target || !target->IsPlayer() || target->GetLevel() < 20 || (botAI->GetBot()->GetLevel() > 60 && target->GetLevel() > 40))
        return false;

//    if (target->getClass() == CLASS_MAGE || target->getClass() == CLASS_PRIEST || target->getClass() == CLASS_WARLOCK)
//        return false;

    return BuffOnPartyTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "greater blessing of might", "blessing of might", nullptr) 
                                    && !botAI->HasAnyMyAuraOf(target, "blessing of kings", "blessing of wisdom", "blessing of sanctuary", nullptr) ;

}

bool BlessingOfSanctuaryOnPartyTrigger::IsActive() 
{ 
    int tab = AiFactory::GetPlayerSpecTab(botAI->GetBot());
    if (tab == !PALADIN_TAB_PROTECTION)
        return false;

    Unit* target = GetTarget();
    
    if (!target || !target->IsPlayer() || target->GetLevel() < 20 || (botAI->GetBot()->GetLevel() > 60 && target->GetLevel() > 40))
        return false;    

    return BuffOnPartyTrigger::IsActive() && !botAI->HasAnyAuraOf(target,"blessing of sanctuary", "greater blessing of sanctuary", nullptr )
                                    && !botAI->HasAnyMyAuraOf(target, "greater blessing of kings", "blessing of kings", 
                                                                      "greater blessing of wisdom", "blessing of wisdom",
                                                                      "greater blessing of might", "blessing of might", nullptr);
}
