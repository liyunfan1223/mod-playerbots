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

    // Schedule destruction after the specified lifetime
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
    {
        return false;
    }

    if (bot->isDead())
    {
        return false;
    }

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

        if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false))
            return false;

        return true;
    }

    return false;
}


bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
    {
        return false;
    }

    if (bot->isDead())
    {
        return false;
    }
    
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
            
        if (!target || target->GetLevel() < 42 || target->isDead())
            return false;        
          
        if (botAI->HasAura("greater blessing of might", target, false, true, 1, false))
            return false;
        return true;
    }
    return false;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
    {
        return false;
    }

    if (bot->isDead())
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
    {
        return false;
    }

    ObjectGuid groupId = group->GetGUID(); 
    auto manager = TeamPaladinManagerFactory::GetTeamPaladinManager(groupId);

    if (!manager)
        return false;

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();

    int paladinOrder = manager->GetPaladinOrder(botGuid);

    if (paladinOrder == 3) 
    {
        Unit* target = GetTarget();

        if (!target || target->isDead() || target->GetLevel() < 44)
        {
            return false;
        }
        
        if (botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    if (!BuffOnPartyTrigger::IsActive())
    {
        return false;
    }

    if (bot->isDead())
    {
        return false;
    }

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

        if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
            return false;

        return true;
    }
    return false;
}

//blessing triggers
bool BlessingOfKingsTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    if (!bot) 
        return false;

    Group* group = bot->GetGroup();

    if (group && group->isRaidGroup() || bot->GetLevel() >= 60)
        return false;

    return BuffTrigger::IsActive() && !botAI->HasAura("greater blessing of kings", bot) && 
    !botAI->HasAura("blessing of might", bot, false, true) && 
    !botAI->HasAura("blessing of wisdom", bot, false, true) && 
    !botAI->HasAura("blessing of sanctuary", bot, false, true);
}

bool BlessingOfWisdomTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    if (!bot)
       return false;

    Group* group = bot->GetGroup();
    if (group && group->isRaidGroup() || bot->GetLevel() >= 60)
        return false;

    if (!botAI->HasAura("greater blessing of wisdom", bot) && !botAI->HasAura("blessing of kings", bot, false, true))
    {
        Aura* existingAura = botAI->GetAura("blessing of wisdom", bot);

        if (existingAura)
        {
            Unit* caster = existingAura->GetCaster();
            if (caster && bot->GetLevel() > caster->GetLevel())
            {
                return BuffTrigger::IsActive();
            }
        }
        return BuffTrigger::IsActive();
    }
    return false;
}

bool BlessingOfMightTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();

    if (group && group->isRaidGroup() || bot->GetLevel() >= 60)
        return false;

    if (!botAI->HasAura("greater blessing of might", bot) && !botAI->HasAura("blessing of kings", bot, false, true))
    {
        Aura* existingAura = botAI->GetAura("blessing of might", bot);

        if (existingAura)
        {
            Unit* caster = existingAura->GetCaster();
            if (caster && bot->GetLevel() > caster->GetLevel())
            {
                return BuffTrigger::IsActive();
            }
        return BuffTrigger::IsActive();
        }
    }
    return false;
}

bool BlessingOfSanctuaryTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    if (!bot) 
        return false;

    Group* group = bot->GetGroup();

    if (group && group->isRaidGroup() || bot->GetLevel() >= 60)
        return false;

    int tab = AiFactory::GetPlayerSpecTab(bot);
    if (tab == !PALADIN_TAB_PROTECTION)
        return false;
        
    return BuffTrigger::IsActive() && !botAI->HasAura("greater blessing of sanctuary", bot) && !botAI->HasAura("blessing of kings", bot, false, true); 
}

//blessing on party triggers
bool BlessingOfKingsOnPartyTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    Group* group = bot->GetGroup();

    if (!bot || !target || !group) 
    {
        return false;
    }
    
    if (bot->GetLevel() >= 60 && group->isRaidGroup())
    {
        return false;
    }
    
    return BuffOnPartyTrigger::IsActive() && 
    !botAI->HasAura("greater blessing of kings", target) && 
    !botAI->HasAura("blessing of might", target, false, true) && 
    !botAI->HasAura("blessing of wisdom", target, false, true) && 
    !botAI->HasAura("blessing of sanctuary", target, false, true);
}


bool BlessingOfWisdomOnPartyTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    Group* group = bot->GetGroup();

    if (!bot || !target || !group)
    {
        return false;
    }
    if (bot->GetLevel() >= 60 && group->isRaidGroup())
    {
        return false;
    }    
    
    if (!botAI->HasAura("greater blessing of wisdom", target) && !botAI->HasAura("blessing of kings", target, false, true))
    {
        Aura* existingAura = botAI->GetAura("blessing of wisdom", target);
        if (existingAura)
        {
            uint8 existingCasterLevel = existingAura->GetCasterLevel();
            if (bot->GetLevel() > existingCasterLevel)
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }
    return false; 
}

bool BlessingOfMightOnPartyTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    Group* group = bot->GetGroup();

    if (!bot || !target || !group)
    {
        return false;
    }
    if (bot->GetLevel() >= 60 && group->isRaidGroup())
    {
        return false;
    }    
    
    if (!botAI->HasAura("greater blessing of might", target) && !botAI->HasAura("blessing of kings", target, false, true))
    {
        Aura* existingAura = botAI->GetAura("blessing of might", target);
        if (existingAura)
        {
            uint8 existingCasterLevel = existingAura->GetCasterLevel();
            if (bot->GetLevel() > existingCasterLevel)
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }
    return false; 
}

bool BlessingOfSanctuaryOnPartyTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    Unit* target = GetTarget();
    Group* group = bot->GetGroup();

    if (!bot || !target || !group) 
    {
        return false;
    }
    
    if (bot->GetLevel() >= 60 && group->isRaidGroup())
    {
        return false;
    }
    
    int tab = AiFactory::GetPlayerSpecTab(bot);
    if (tab == !PALADIN_TAB_PROTECTION)
    {
        return false;
    }   

    return BuffOnPartyTrigger::IsActive() && 
    !botAI->HasAura("greater blessing of sanctuary", target) && !botAI->HasAura("blessing of kings", target, false, true);}
