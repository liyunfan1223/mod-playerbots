/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    if (!target->ToPlayer())
    {
        return "blessing of might";
    }
    int tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return "blessing of wisdom";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ELEMENTAL || tab == SHAMAN_TAB_RESTORATION)
            {
                return "blessing of wisdom";
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION || tab == DRUID_TAB_BALANCE)
            {
                return "blessing of wisdom";
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
            {
                return "blessing of wisdom";
            }
            break;
    }

    return "blessing of might";
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    if (!target->ToPlayer())
    {
        return "blessing of might";
    }
    int tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        case CLASS_HUNTER:
            return "blessing of might";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
            {
                return "blessing of might";
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
            {
                return "blessing of might";
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION || tab == PALADIN_TAB_RETRIBUTION)
            {
                return "blessing of might";
            }
            break;
    }

    return "blessing of wisdom";
}

Value<Unit*>* CastBlessingOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", name);
}

bool CastBlessingOfMightAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return botAI->CastSpell(GetActualBlessingOfMight(target), target);
}

Value<Unit*>* CastBlessingOfMightOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of might,blessing of wisdom");
}

bool CastBlessingOfMightOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return botAI->CastSpell(GetActualBlessingOfMight(target), target);
}

bool CastBlessingOfWisdomAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return botAI->CastSpell(GetActualBlessingOfWisdom(target), target);
}

Value<Unit*>* CastBlessingOfWisdomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of might,blessing of wisdom");
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return botAI->CastSpell(GetActualBlessingOfWisdom(target), target);
}

bool CastSealSpellAction::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

Value<Unit*>* CastTurnUndeadAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
    {
        return NULL;
    }
    return current_target->GetVictim();
}


bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event event)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}

bool CastGreaterBlessingOfKingsOnPartyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    Unit* target = GetTarget();
    if (!bot || !group)
        return false;

    // 定义骑士的天赋顺序
    enum PaladinSpecOrder {
        SPEC_PALADIN_HOLY,    // 奶骑
        SPEC_PALADIN_RETRIBUTION, // 惩戒骑
        SPEC_PALADIN_PROTECTION   // 防骑
    };

    std::vector<Player*> selectedPaladins; // 用于存储选中的骑士
    std::set<Player*> visitedPaladins; // 防止重复选择同一个骑士

    // 最多选择4个骑士
    while (selectedPaladins.size() < 4)
    {
        bool found = false;

        // 按照天赋顺序选择骑士
        for (int specOrder : {SPEC_PALADIN_HOLY, SPEC_PALADIN_RETRIBUTION, SPEC_PALADIN_PROTECTION})
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                Player* member = groupRef->GetSource();
                if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
                    continue;

                // 检查天赋是否匹配
                BotRoles role = AiFactory::GetPlayerRoles(member);
                bool isHoly = (specOrder == SPEC_PALADIN_HOLY && role == BOT_ROLE_HEALER);
                bool isRetribution = (specOrder == SPEC_PALADIN_RETRIBUTION && role == BOT_ROLE_DPS);
                bool isProtection = (specOrder == SPEC_PALADIN_PROTECTION && role == BOT_ROLE_TANK);

                if (isHoly || isRetribution || isProtection)
                {
                    selectedPaladins.push_back(member);
                    visitedPaladins.insert(member);
                    found = true;
                    break; // 找到一个骑士后跳出内层循环
                }
            }

            if (found)
                break; // 找到一个骑士后跳出外层循环
        }

        if (!found)
            break; // 如果没有找到骑士，结束循环
    }

    // 如果没有选中任何骑士，返回 false
    if (selectedPaladins.empty())
        return false;

    // 如果当前目标是第一个骑士，施放 Greater Blessing of Kings
    if (selectedPaladins.front()->GetGUID() == bot->GetGUID())
    {
        botAI->CastSpell("greater blessing of kings", target);
        return true;
    }

    return false;
}

bool CastGreaterBlessingOfMightOnPartyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    Unit* target = GetTarget();

    if (!bot || !group)
        return false;

    // 定义骑士的天赋顺序
    enum PaladinSpecOrder {
        SPEC_PALADIN_HOLY,    // 奶骑
        SPEC_PALADIN_RETRIBUTION, // 惩戒骑
        SPEC_PALADIN_PROTECTION   // 防骑
    };

    std::vector<Player*> selectedPaladins; // 用于存储选中的骑士
    std::set<Player*> visitedPaladins; // 防止重复选择同一个骑士

    // 最多选择4个骑士
    while (selectedPaladins.size() < 4)
    {
        bool found = false;

        // 按照天赋顺序选择骑士
        for (int specOrder : {SPEC_PALADIN_HOLY, SPEC_PALADIN_RETRIBUTION, SPEC_PALADIN_PROTECTION})
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                Player* member = groupRef->GetSource();
                if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
                    continue;

                // 检查天赋是否匹配
                BotRoles role = AiFactory::GetPlayerRoles(member);
                bool isHoly = (specOrder == SPEC_PALADIN_HOLY && role == BOT_ROLE_HEALER);
                bool isRetribution = (specOrder == SPEC_PALADIN_RETRIBUTION && role == BOT_ROLE_DPS);
                bool isProtection = (specOrder == SPEC_PALADIN_PROTECTION && role == BOT_ROLE_TANK);

                if (isHoly || isRetribution || isProtection)
                {
                    selectedPaladins.push_back(member);
                    visitedPaladins.insert(member);
                    found = true;
                    break; // 找到一个骑士后跳出内层循环
                }
            }

            if (found)
                break; // 找到一个骑士后跳出外层循环
        }

        if (!found)
            break; // 如果没有找到骑士，结束循环
    }

    // 如果没有选中任何骑士，或者没有第二个骑士，返回 false
    if (selectedPaladins.size() < 2)
        return false;

    // 如果当前角色是第二个选中的骑士，施放 Greater Blessing of Might
    if (selectedPaladins[1]->GetGUID() == bot->GetGUID())
    {
        botAI->CastSpell("greater" +GetActualBlessingOfMight(target), target);
        return true;
    }

    return false;
}

bool CastGreaterBlessingOfWisdomOnPartyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    Unit* target = GetTarget();

    if (!bot || !group)
        return false;

    // 定义骑士的天赋顺序
    enum PaladinSpecOrder {
        SPEC_PALADIN_HOLY,    // 奶骑
        SPEC_PALADIN_RETRIBUTION, // 惩戒骑
        SPEC_PALADIN_PROTECTION   // 防骑
    };

    std::vector<Player*> selectedPaladins; // 用于存储选中的骑士
    std::set<Player*> visitedPaladins; // 防止重复选择同一个骑士

    // 最多选择4个骑士
    while (selectedPaladins.size() < 4)
    {
        bool found = false;

        // 按照天赋顺序选择骑士
        for (int specOrder : {SPEC_PALADIN_HOLY, SPEC_PALADIN_RETRIBUTION, SPEC_PALADIN_PROTECTION})
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                Player* member = groupRef->GetSource();
                if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
                    continue;

                // 检查天赋是否匹配
                BotRoles role = AiFactory::GetPlayerRoles(member);
                bool isHoly = (specOrder == SPEC_PALADIN_HOLY && role == BOT_ROLE_HEALER);
                bool isRetribution = (specOrder == SPEC_PALADIN_RETRIBUTION && role == BOT_ROLE_DPS);
                bool isProtection = (specOrder == SPEC_PALADIN_PROTECTION && role == BOT_ROLE_TANK);

                if (isHoly || isRetribution || isProtection)
                {
                    selectedPaladins.push_back(member);
                    visitedPaladins.insert(member);
                    found = true;
                    break; // 找到一个骑士后跳出内层循环
                }
            }

            if (found)
                break; // 找到一个骑士后跳出外层循环
        }

        if (!found)
            break; // 如果没有找到骑士，结束循环
    }

    // 如果没有选中任何骑士，或者没有第三个骑士，返回 false
    if (selectedPaladins.size() < 3)
        return false;

    // 如果当前角色是第三个选中的骑士，施放 Greater Blessing of Wisdom
    if (selectedPaladins[2]->GetGUID() == bot->GetGUID())
    {
        botAI->CastSpell("greater" +GetActualBlessingOfWisdom(target), target);
        return true;
    }

    return false;
}

bool CastGreaterBlessingOfSanctuaryOnPartyAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    Unit* target = GetTarget();

    if (!bot || !group)
        return false;

    // 定义骑士的天赋顺序
    enum PaladinSpecOrder {
        SPEC_PALADIN_HOLY,    // 奶骑
        SPEC_PALADIN_RETRIBUTION, // 惩戒骑
        SPEC_PALADIN_PROTECTION   // 防骑
    };

    std::vector<Player*> selectedPaladins; // 用于存储选中的骑士
    std::set<Player*> visitedPaladins; // 防止重复选择同一个骑士

    // 最多选择4个骑士
    while (selectedPaladins.size() < 4)
    {
        bool found = false;

        // 按照天赋顺序选择骑士
        for (int specOrder : {SPEC_PALADIN_HOLY, SPEC_PALADIN_RETRIBUTION, SPEC_PALADIN_PROTECTION})
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                Player* member = groupRef->GetSource();
                if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
                    continue;

                // 检查天赋是否匹配
                BotRoles role = AiFactory::GetPlayerRoles(member);
                bool isHoly = (specOrder == SPEC_PALADIN_HOLY && role == BOT_ROLE_HEALER);
                bool isRetribution = (specOrder == SPEC_PALADIN_RETRIBUTION && role == BOT_ROLE_DPS);
                bool isProtection = (specOrder == SPEC_PALADIN_PROTECTION && role == BOT_ROLE_TANK);

                if (isHoly || isRetribution || isProtection)
                {
                    selectedPaladins.push_back(member);
                    visitedPaladins.insert(member);
                    found = true;
                    break; // 找到一个骑士后跳出内层循环
                }
            }

            if (found)
                break; // 找到一个骑士后跳出外层循环
        }

        if (!found)
            break; // 如果没有找到骑士，结束循环
    }

    // 如果没有选中任何骑士，或者没有第四个骑士，返回 false
    if (selectedPaladins.size() < 4)
        return false;

    // 如果当前角色是第四个选中的骑士，施放 Greater Blessing of Sanctuary
    if (selectedPaladins[3]->GetGUID() == bot->GetGUID())
    {
        botAI->CastSpell("greater blessing of sanctuary", target);
        return true;
    }

    return false;
}
