/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"

#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Group.h"
#include "AiFactory.h"
#include "SharedDefines.h"

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

/*bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup(); 
    
    if (!target || !bot || !group)
       return false;
    
    if (group->isRaidGroup())
       return false;

    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                                     "blessing of kings", nullptr); 
}
 */

//greater blessing on party triggers

std::unordered_map<Group*, std::unique_ptr<PaladinSelectionGroupManager>>& PaladinSelectionGroupManager::GetInstances() 
{
    static std::unordered_map<Group*, std::unique_ptr<PaladinSelectionGroupManager>> instances;
    return instances;
}

PaladinSelectionGroupManager& PaladinSelectionGroupManager::GetInstance(Group* group) 
{
    auto& instances = GetInstances();
    auto it = instances.find(group);
    if (it == instances.end()) 
    {
        it = instances.emplace(group, std::unique_ptr<PaladinSelectionGroupManager>(new PaladinSelectionGroupManager(group))).first;
    }
    return *it->second;
}

void PaladinSelectionGroupManager::RemoveInstance(Group* group) 
{
    LOG_INFO("module.playerbots", "Removing PaladinSelectionGroupManager instance for group {}", group->GetGUID().ToString());
    GetInstances().erase(group);
}

PaladinSelectionGroup* PaladinSelectionGroupManager::GetPaladinSelectionGroup() const 
{
    return paladinSelectionGroup_.get();
}

void PaladinSelectionGroupManager::UpdatePaladinSelectionGroup(Group* group) 
{
    LOG_INFO("module.playerbots", "Updating PaladinSelectionGroup for group {}", group->GetGUID().ToString());
    paladinSelectionGroup_ = std::make_unique<PaladinSelectionGroup>(group);
}

PaladinSelectionGroupManager::PaladinSelectionGroupManager(Group* group) 
    : paladinSelectionGroup_(std::make_unique<PaladinSelectionGroup>(group)) 
{
    LOG_INFO("module.playerbots", "PaladinSelectionGroupManager created for group {}", group->GetGUID().ToString());
}

void PaladinSelectionGroup::UpdateIfNeeded(uint32 groupUpdateFlag) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if ((groupUpdateFlag & GROUP_UPDATE_FLAG_STATUS) || sortedPaladins_.empty())
    {
        sortedPaladins_.clear();
        std::set<ObjectGuid> visitedPaladins;

        for (const GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
        {
            Player* member = groupRef->GetSource();
            if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member->GetGUID()))
                continue;

            if (member->GetLevel() >= 60)
            {
                sortedPaladins_.push_back(member);
                visitedPaladins.insert(member->GetGUID());
            }
        }

        std::stable_sort(sortedPaladins_.begin(), sortedPaladins_.end(), [](Player* a, Player* b) {
            return a->GetLevel() > b->GetLevel();
        });

        Player* highestLevelTank = nullptr;
        for (Player* paladin : sortedPaladins_)
        {
            BotRoles role = AiFactory::GetPlayerRoles(paladin);
            if (role == BOT_ROLE_TANK)
            {
                highestLevelTank = paladin;
                break;
            }
        }

        std::vector<Player*> selectedPaladins;
        if (highestLevelTank)
        {
            selectedPaladins.push_back(highestLevelTank);
        }
        else if (!sortedPaladins_.empty())
        {
            selectedPaladins.push_back(sortedPaladins_[0]);
        }

        for (Player* paladin : sortedPaladins_)
        {
            if (selectedPaladins.size() >= MAX_PALADINS)
                break;

            if (std::find(selectedPaladins.begin(), selectedPaladins.end(), paladin) == selectedPaladins.end())
            {
                selectedPaladins.push_back(paladin);
            }
        }

        std::reverse(selectedPaladins.begin(), selectedPaladins.end());

        LOG_INFO("module.playerbots", "Updating PaladinSelectionGroup for group {}", group->GetGUID().ToString());
        sortedPaladins_ = selectedPaladins;
        lastGroupUpdateFlag_ = groupUpdateFlag;
    }
}

std::vector<Player*> PaladinSelectionGroup::GetSortedPaladins(uint32 groupUpdateFlag)
{
    UpdateIfNeeded(groupUpdateFlag);
    return sortedPaladins_;
}

size_t PaladinSelectionGroup::GetPaladinOrderForBlessing(PlayerbotAI* botAI) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!botAI)
        return 0;

    Player* bot = botAI->GetBot();
    if (!bot || bot->getClass() != CLASS_PALADIN)
        return 0;

    const ObjectGuid botGuid = bot->GetGUID();
    for (size_t i = 0; i < sortedPaladins_.size(); ++i)
    {
        if (sortedPaladins_[i]->GetGUID() == botGuid)
        {
            return i + 1; 
        }
    }

    return 0; 
}

bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    PaladinSelectionGroupManager& manager = PaladinSelectionGroupManager::GetInstance(group);    
    PaladinSelectionGroup* paladinGroup = manager.GetPaladinSelectionGroup();
    if (!paladinGroup)
        return false;

    uint32 currentGroupUpdateFlag = bot->GetGroupUpdateFlag();

    std::vector<Player*> sortedPaladins = paladinGroup->GetSortedPaladins(currentGroupUpdateFlag);

    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);
    
    if (paladinOrder == 1)
    {
        Unit* target = GetTarget();
        if (!target || target->GetLevel() < 50)
            return false;
       
        if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false))
            return false;
        
        return true;
    }

    return false;
}

bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;
    
    PaladinSelectionGroupManager& manager = PaladinSelectionGroupManager::GetInstance(group);
    PaladinSelectionGroup* paladinGroup = manager.GetPaladinSelectionGroup();
    
    if (!paladinGroup)
    return false;

    uint32 currentGroupUpdateFlag = bot->GetGroupUpdateFlag();

    std::vector<Player*> sortedPaladins = paladinGroup->GetSortedPaladins(currentGroupUpdateFlag);

    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 2)
    {
        Unit* target = GetTarget();
            
        if (!target || target->GetLevel() < 42)
            return false;        
          
        if (botAI->HasAura("greater blessing of might", target, false, true, 1, false))
            return false;
        return true;
    }
    return false;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    PaladinSelectionGroupManager& manager = PaladinSelectionGroupManager::GetInstance(group);
    PaladinSelectionGroup* paladinGroup = manager.GetPaladinSelectionGroup();

    if (!paladinGroup)
        return false;

    uint32 currentGroupUpdateFlag = bot->GetGroupUpdateFlag();

    std::vector<Player*> sortedPaladins = paladinGroup->GetSortedPaladins(currentGroupUpdateFlag);
    
    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 3) 
    {
        Unit* target = GetTarget();

        if (!target || target->GetLevel() < 44)
            return false;
        
            if (botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
            return false;

        return true;
    }
    return false;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup())
        return false;

    PaladinSelectionGroupManager& manager = PaladinSelectionGroupManager::GetInstance(group);
    PaladinSelectionGroup* paladinGroup = manager.GetPaladinSelectionGroup();

    if (!paladinGroup)
        return false;

    uint32 currentGroupUpdateFlag = bot->GetGroupUpdateFlag();

    std::vector<Player*> sortedPaladins = paladinGroup->GetSortedPaladins(currentGroupUpdateFlag);
    
    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 4)
    {
        Unit* target = GetTarget();
        if (!target || target->GetLevel() < 50)
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

    return BuffTrigger::IsActive() && !botAI->HasAura("greater blessing of kings", bot) && !botAI->HasAura("blessing of kings", bot);
}

bool BlessingOfWisdomTrigger::IsActive() 
{ 
    Player* bot = botAI->GetBot();
    if (!bot)
       return false;

    Group* group = bot->GetGroup();
    if (group && group->isRaidGroup() || bot->GetLevel() >= 60)
        return false;

    if (!botAI->HasAura("greater blessing of wisdom", bot))
    {
        Aura* existingAura = botAI->GetAura("blessing of wisdom", bot);

        if (existingAura)
        {
            Unit* caster = existingAura->GetCaster();
            if (caster && bot->GetLevel() > caster->GetLevel())
            {
                return true;
            }
        }
        return true;
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

    if (!botAI->HasAura("greater blessing of might", bot))
    {
        Aura* existingAura = botAI->GetAura("blessing of might", bot);

        if (existingAura)
        {
            Unit* caster = existingAura->GetCaster();
            if (caster && bot->GetLevel() > caster->GetLevel())
            {
                return true;
            }
        return true;
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
        
    return !botAI->HasAura("greater blessing of sanctuary", bot) && !botAI->HasAura("blessing of sanctuary", bot); 
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
    !botAI->HasAura("blessing of kings", target);
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
    
    if (!botAI->HasAura("greater blessing of wisdom", target))
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
    
    if (!botAI->HasAura("greater blessing of might", target))
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
    !botAI->HasAura("greater blessing of sanctuary", target) && 
    !botAI->HasAura("blessing of sanctuary", target) && !botAI->HasAura("blessing of kings", target, false, true, 1, false);}
