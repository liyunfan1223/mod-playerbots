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

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup(); 
    
    if (!target || !bot || !group)
       return false;
    
    if (group->isRaidGroup())
       return false;

    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr); 
}

//greater blessing on party triggers

PaladinSelectionGroup* PaladinSelectionGroupManager::GetPaladinSelectionGroup(Group* group)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (groupCache.find(group) == groupCache.end())
    {
        groupCache[group] = std::make_unique<PaladinSelectionGroup>(group);
    }
    return groupCache[group].get();
}

std::vector<Player*> PaladinSelectionGroup::GetSortedPaladins() const
{
    std::vector<Player*> selectedPaladins;
    std::set<ObjectGuid> visitedPaladins;

    for (const GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
    {
        Player* member = groupRef->GetSource();
        if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member->GetGUID()))
            continue;

        BotRoles role = AiFactory::GetPlayerRoles(member);

        if (role == BOT_ROLE_TANK)
            selectedPaladins.insert(selectedPaladins.begin(), member); 
        else
            selectedPaladins.push_back(member); 

        visitedPaladins.insert(member->GetGUID());

        if (selectedPaladins.size() >= MAX_PALADINS)
            break;
    }

    std::reverse(selectedPaladins.begin(), selectedPaladins.end());

    return selectedPaladins;
}

size_t PaladinSelectionGroup::GetPaladinOrderForBlessing(PlayerbotAI* botAI) const
{
    if (!botAI)
        return 0;

    Player* bot = botAI->GetBot();
    if (!bot || bot->getClass() != CLASS_PALADIN)
        return 0;

    std::vector<Player*> selectedPaladins = GetSortedPaladins();

    const ObjectGuid botGuid = bot->GetGUID();
    for (size_t i = 0; i < selectedPaladins.size(); ++i)
    {
        if (selectedPaladins[i]->GetGUID() == botGuid)
        {
            return i + 1; 
        }
    }

    return 0;  
}

bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot(); 
    if (!bot)
        return false;

    Group* group = bot->GetGroup(); 
    if (!group) 
        return false;

    Unit* target = GetTarget(); 
    if (!target)
        return false;

    if (!group->isRaidGroup())
        return false;

    if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false))
        return false;
    
    PaladinSelectionGroup* paladinGroup = PaladinSelectionGroupManager::GetInstance().GetPaladinSelectionGroup(group);

    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 1)
    {
        //botAI->TellMaster("准备施放强效王者祝福！");
        return true;
    }

    return false;
}


bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!group->isRaidGroup())
        return false;

    if (botAI->HasAura("greater blessing of might", target, false, true, 1, false))
        return false;

    PaladinSelectionGroup* paladinGroup = PaladinSelectionGroupManager::GetInstance().GetPaladinSelectionGroup(group);
    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 2)
    {
        //botAI->TellMaster("准备施放强效力量祝福！");
        return true;
    }

    return false;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!group->isRaidGroup())
        return false;

    if (botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
        return false;

    PaladinSelectionGroup* paladinGroup = PaladinSelectionGroupManager::GetInstance().GetPaladinSelectionGroup(group);
    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 3)
    {
        //botAI->TellMaster("准备施放强效智慧祝福！");
        return true;
    }

    return false;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!group->isRaidGroup())
        return false;

    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
        return false;

    PaladinSelectionGroup* paladinGroup = PaladinSelectionGroupManager::GetInstance().GetPaladinSelectionGroup(group);
    size_t paladinOrder = paladinGroup->GetPaladinOrderForBlessing(botAI);

    if (paladinOrder == 4)
    {
        //botAI->TellMaster("准备施放强效庇护祝福！");
        return true;
    }

    return false;
}

