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
    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr);
}

//greater blessing on party triggers
bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot(); // 获取当前角色
    Group* group = bot->GetGroup(); // 获取当前组
    if (!group || !group->isRaidGroup()) // 检查是否为团队
    {
        return false; // 不在团队中，不触发
    }

    Unit* target = GetTarget(); // 获取目标单位
    if (!target)
    {
        return false; // 目标不存在，不触发
    }

    // 定义需要检查的强化祝福名称
    const std::string greaterBlessingOfKings = "greater blessing of kings";
    const std::string greaterBlessingOfMight = "greater blessing of might";
    const std::string greaterBlessingOfWisdom = "greater blessing of wisdom";
    const std::string greaterBlessingOfSanctuary = "greater blessing of sanctuary";

    // 检查目标身上是否有任何来源的 Greater Blessing of Kings
    if (botAI->HasAura(greaterBlessingOfKings, target, false, false, 1, false))
    {
        return false; // 目标身上有 Greater Blessing of Kings（无论来源），不触发
    }

    // 检查目标身上是否有当前角色施加的其他强化祝福
    if (botAI->HasAura(greaterBlessingOfMight, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfWisdom, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfSanctuary, target, false, true, 1, false))
    {
        return false; // 目标身上有当前角色施加的其他强化祝福，不触发
    }

    // 如果目标身上没有 Greater Blessing of Kings，且没有当前角色施加的其他强化祝福，触发
    return true;
}


bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot(); // 获取当前角色
    Group* group = bot->GetGroup(); // 获取当前组
    if (!group || !group->isRaidGroup()) // 检查是否为团队
    {
        return false; // 不在团队中，不触发
    }

    Unit* target = GetTarget(); // 获取目标单位
    if (!target)
    {
        return false; // 目标不存在，不触发
    }

    // 定义需要检查的强化祝福名称
    const std::string greaterBlessingOfKings = "greater blessing of kings";
    const std::string greaterBlessingOfMight = "greater blessing of might";
    const std::string greaterBlessingOfWisdom = "greater blessing of wisdom";
    const std::string greaterBlessingOfSanctuary = "greater blessing of sanctuary";

    // 检查目标身上是否有任何来源的 Greater Blessing of Wisdom
    if (botAI->HasAura("greater" + GetActualBlessingOfWisdom(target), target, false, false, 1, false))
    {
        return false; // 目标身上有 Greater Blessing of Wisdom（无论来源），不触发
    }

    // 检查目标身上是否有当前角色施加的其他强化祝福
    if (botAI->HasAura(greaterBlessingOfKings, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfMight, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfSanctuary, target, false, true, 1, false))
    {
        return false; // 目标身上有当前角色施加的其他强化祝福，不触发
    }

    // 如果目标身上没有 Greater Blessing of Wisdom，且没有当前角色施加的其他强化祝福，触发
    return true;
}


bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot(); // 获取当前角色
    Group* group = bot->GetGroup(); // 获取当前组
    if (!group || !group->isRaidGroup()) // 检查是否为团队
    {
        return false; // 不在团队中，不触发
    }

    Unit* target = GetTarget(); // 获取目标单位
    if (!target)
    {
        return false; // 目标不存在，不触发
    }

    // 定义需要检查的强化祝福名称
    const std::string greaterBlessingOfKings = "greater blessing of kings";
    const std::string greaterBlessingOfMight = "greater blessing of might";
    const std::string greaterBlessingOfWisdom = "greater blessing of wisdom";
    const std::string greaterBlessingOfSanctuary = "greater blessing of sanctuary";

    // 检查目标身上是否有任何来源的 Greater Blessing of Might
    if (botAI->HasAura("greater" + GetActualBlessingOfMight(target), target, false, false, 1, false))
    {
        return false; // 目标身上有 Greater Blessing of Might（无论来源），不触发
    }

    // 检查目标身上是否有当前角色施加的其他强化祝福
    if (botAI->HasAura(greaterBlessingOfKings, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfWisdom, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfSanctuary, target, false, true, 1, false))
    {
        return false; // 目标身上有当前角色施加的其他强化祝福，不触发
    }

    // 如果目标身上没有 Greater Blessing of Might，且没有当前角色施加的其他强化祝福，触发
    return true;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot(); // 获取当前角色
    Group* group = bot->GetGroup(); // 获取当前组
    Unit* target = GetTarget(); // 获取目标单位
    if (!target)
    {
        return false; // 目标不存在，不触发
    }

    if (!group || !group->isRaidGroup() || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget())) // 检查是否为团队
    {
        return false; // 不在团队中，不触发
    }

    // 使用 GetPlayerRoles 函数检查当前角色的天赋专精
    BotRoles role = AiFactory::GetPlayerRoles(bot);
    if (!(role & BOT_ROLE_TANK)) // 如果天赋不是坦克专精，则不触发
    {
        return false;
    }

    // 定义需要检查的强化祝福名称
    const std::string greaterBlessingOfKings = "greater blessing of kings";
    const std::string greaterBlessingOfMight = "greater blessing of might";
    const std::string greaterBlessingOfWisdom = "greater blessing of wisdom";
    const std::string greaterBlessingOfSanctuary = "greater blessing of sanctuary";

    // 检查目标身上是否有任何来源的 Greater Blessing of Sanctuary
    if (botAI->HasAura(greaterBlessingOfSanctuary, target, false, false, 1, false))
    {
        return false; // 目标身上有 Greater Blessing of Sanctuary（无论来源），不触发
    }

    // 检查目标身上是否有当前角色施加的其他强化祝福
    if (botAI->HasAura(greaterBlessingOfKings, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfMight, target, false, true, 1, false) ||
        botAI->HasAura(greaterBlessingOfWisdom, target, false, true, 1, false))
    {
        return false; // 目标身上有当前角色施加的其他强化祝福，不触发
    }

    // 如果目标身上没有 Greater Blessing of Sanctuary，且没有当前角色施加的其他强化祝福，触发
    return true;
}





