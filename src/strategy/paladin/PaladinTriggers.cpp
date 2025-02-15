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
                                                            "greater blessing of kings", "blessing of sanctuary", nullptr); 
}

//greater blessing on party triggers
//add function
inline size_t IsPaladinSelectedForBlessing(PlayerbotAI* botAI)
{
    // 获取当前角色
    Player* bot = botAI->GetBot();
    if (!bot)  // 确保bot不为空
        return 0;

    // 获取当前团队
    Group* group = bot->GetGroup();
    if (!group)  // 确保团队不为空
        return 0;

    // 用于存储排序后的骑士
    std::vector<Player*> selectedPaladins;
    std::set<Player*> visitedPaladins;

    // 遍历团队成员，选择骑士
    for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
    {
        Player* member = groupRef->GetSource();
        if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
            continue;
    
        BotRoles role = AiFactory::GetPlayerRoles(member);
    
        // 将防骑插入到数组的第一个位置，非防骑插入到后面
        if (role == BOT_ROLE_TANK)
            selectedPaladins.insert(selectedPaladins.begin(), member);  // 防骑放在最前面
        else
            selectedPaladins.push_back(member);  // 非防骑放在后面
    
        visitedPaladins.insert(member);
    
        if (selectedPaladins.size() >= 4)  // 限制最多选择4个骑士
            break;
    }
    
    // 将数组反转，确保最后一个骑士是防骑
    std::reverse(selectedPaladins.begin(), selectedPaladins.end());

    // 确定当前骑士的顺序
    size_t paladinOrder = 0;
    const ObjectGuid botGuid = bot->GetGUID();  // 缓存bot的GUID，避免重复调用
    for (size_t i = 0; i < selectedPaladins.size(); ++i)
    {
        if (selectedPaladins[i]->GetGUID() == botGuid)
        {
            paladinOrder = i + 1;  // 从1开始计数
            break;
        }
    }

    // 通知团队选出了多少个骑士
    if (!selectedPaladins.empty())
    {
        std::string message = "Selected " + std::to_string(selectedPaladins.size()) + " paladins for blessings.";
        botAI->Yell(message);
    }

    return paladinOrder;  // 返回骑士的顺序
}

bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();  // 获取当前角色
    if (!bot)  // 修改点1：确保bot不为空
        return false;

    Group* group = bot->GetGroup();  // 获取当前团队
    if (!group)  // 修改点2：确保团队不为空
        return false;

    Unit* target = GetTarget();  // 获取目标单位
    if (!target)  // 修改点3：确保目标不为空
        return false;

    // 确保角色和目标都在同一个团队中，并且是小队而不是团队
    if (!group->isRaidGroup())
        return false;
    // 先检查目标是否已有当前角色施放的任何强效祝福
    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
    {
        return false;
    }

    // 再检查当前骑士的顺序是否为1
    if (IsPaladinSelectedForBlessing(botAI) != 1)
    {
        return false;
    }    
    return true;
}

bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();  // 获取当前角色
    if (!bot)  // 修改点1：确保bot不为空
        return false;

    Group* group = bot->GetGroup();  // 获取当前团队
    if (!group)  // 修改点2：确保团队不为空
        return false;

    Unit* target = GetTarget();  // 获取目标单位
    if (!target)  // 修改点3：确保目标不为空
        return false;

    // 确保角色和目标都在同一个团队中，并且是小队而不是团队
    if (!group->isRaidGroup())
        return false;

    // 先检查目标是否已有当前角色施放的任何强效祝福
    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
    {
        return false;
    }

    // 再检查当前骑士的顺序是否为2
    if (IsPaladinSelectedForBlessing(botAI) != 2)
        return false;

    return true;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();  // 获取当前角色
    if (!bot)  // 修改点1：确保bot不为空
        return false;

    Group* group = bot->GetGroup();  // 获取当前团队
    if (!group)  // 修改点2：确保团队不为空
        return false;

    Unit* target = GetTarget();  // 获取目标单位
    if (!target)  // 修改点3：确保目标不为空
        return false;

    // 确保角色和目标都在同一个团队中，并且是小队而不是团队
    if (!group->isRaidGroup())
        return false;

    // 先检查目标是否已有当前角色施放的任何强效祝福
    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
    {
        return false;
    }

    // 再检查当前骑士的顺序是否为3
    if (IsPaladinSelectedForBlessing(botAI) != 3)
        return false;

    return true;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();  // 获取当前角色
    if (!bot)  // 修改点1：确保bot不为空
        return false;
    
        BotRoles role = AiFactory::GetPlayerRoles(bot);
    if (role != BOT_ROLE_TANK)  // 如果当前骑士的天赋不是防御天赋，返回false
        return false;
    
        Group* group = bot->GetGroup();  // 获取当前团队
    if (!group)  // 修改点2：确保团队不为空
        return false;

    Unit* target = GetTarget();  // 获取目标单位
    if (!target)  // 修改点3：确保目标不为空
        return false;

    // 确保角色和目标都在同一个团队中，并且是小队而不是团队
    if (!group->isRaidGroup())
        return false;

    // 先检查目标是否已有当前角色施放的任何强效祝福
    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
    {
        return false;
    }

    // 再检查当前骑士的顺序是否为4
    if (IsPaladinSelectedForBlessing(botAI) != 4)
        return false;

    return true;
}

