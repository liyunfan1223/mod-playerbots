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
    Group* group = botAI->GetBot()->GetGroup();
    if ((!group || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget())) && !group->isRaidGroup()) 
    {
        return false; // // 同一小队，否则不触发
    }
    return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr);
}

//greater blessing on party triggers
//add function
inline bool IsPaladinSelectedForBlessing(ObjectGuid botGUID, const std::string& blessingName)
{
    if (blessingName.empty())
        return false;

    Player* bot = ObjectAccessor::FindPlayer(botGUID);
    if (!bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> selectedPaladins;
    std::set<Player*> visitedPaladins;

    for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
    {
        Player* member = groupRef->GetSource();
        if (!member || member->getClass() != CLASS_PALADIN || visitedPaladins.count(member))
            continue;

        BotRoles role = AiFactory::GetPlayerRoles(member);
        if (role == BOT_ROLE_TANK)
            selectedPaladins.push_back(member);
        else
            selectedPaladins.insert(selectedPaladins.begin(), member);

        visitedPaladins.insert(member);

        if (selectedPaladins.size() >= 4)
            break;
    }

    size_t paladinOrder = 0;
    for (size_t i = 0; i < selectedPaladins.size(); ++i)
    {
        if (selectedPaladins[i]->GetGUID() == botGUID)
        {
            paladinOrder = i + 1;
            break;
        }
    }

    if (paladinOrder == 0)
        return false;

    static const std::string BLESSING_KINGS = "greater blessing of kings";
    static const std::string BLESSING_MIGHT = "greater blessing of might";
    static const std::string BLESSING_WISDOM = "greater blessing of wisdom";
    static const std::string BLESSING_SANCTUARY = "greater blessing of sanctuary";

    if (blessingName == BLESSING_KINGS && paladinOrder == 1)
        return true;
    else if (blessingName == BLESSING_MIGHT && paladinOrder == 2)
        return true;
    else if (blessingName == BLESSING_WISDOM && paladinOrder == 3)
        return true;
    else if (blessingName == BLESSING_SANCTUARY && paladinOrder == 4)
        return true;

    return false;
}

bool GreaterBlessingOfKingsOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup() || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget()))
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!IsPaladinSelectedForBlessing(bot->GetGUID(), "greater blessing of kings"))
        return false;

    if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false))
        return false;

    if (botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
        return false;

    return true;
}

bool GreaterBlessingOfWisdomOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup() || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget()))
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!IsPaladinSelectedForBlessing(bot->GetGUID(), "greater blessing of wisdom"))
        return false;

    if (botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
        return false;

    if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
        return false;

    return true;
}

bool GreaterBlessingOfMightOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup() || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget()))
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    // 使用 std::string 对象作为参数
    if (!IsPaladinSelectedForBlessing(bot->GetGUID(), std::string("greater blessing of might")))
        return false;

    if (botAI->HasAura("greater blessing of might", target, false, true, 1, false))
        return false;

    if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
        return false;

    return true;
}

bool GreaterBlessingOfSanctuaryOnPartyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !group->isRaidGroup() || !botAI->GetBot()->IsInSameGroupWith((Player*)GetTarget()))
        return false;

    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!IsPaladinSelectedForBlessing(bot->GetGUID(), "greater blessing of sanctuary"))
        return false;

    if (botAI->HasAura("greater blessing of sanctuary", target, false, true, 1, false))
        return false;

    if (botAI->HasAura("greater blessing of kings", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of might", target, false, true, 1, false) ||
        botAI->HasAura("greater blessing of wisdom", target, false, true, 1, false))
        return false;

    return true;
}





