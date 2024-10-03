#include "EstimatedLifetimeValue.h"

#include "AiFactory.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"

float EstimatedLifetimeValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target || !target->IsAlive())
    {
        return 0.0f;
    }
    float dps = AI_VALUE(float, "estimated group dps");
    bool aoePenalty = AI_VALUE(uint8, "attacker count") >= 3;
    if (aoePenalty)
        dps *= 0.75;
    float res = target->GetHealth() / dps;
    // bot->Say(target->GetName() + " lifetime: " + std::to_string(res), LANG_UNIVERSAL);
    return res;
}

float EstimatedGroupDpsValue::Calculate()
{
    float totalDps = 0;

    std::vector<Player*> groupPlayer = {bot};
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member == bot)  // calculated
                continue;

            if (!member || !member->IsInWorld())
                continue;

            if (member->GetMapId() != bot->GetMapId())
                continue;

            if (member->GetExactDist(bot) > sPlayerbotAIConfig->sightDistance)
                continue;

            groupPlayer.push_back(member);
        }
    }
    for (Player* player : groupPlayer)
    {
        float roleMultiplier;
        if (botAI->IsTank(player))
            roleMultiplier = 0.3f;
        else if (botAI->IsHeal(player))
            roleMultiplier = 0.1f;
        else
            roleMultiplier = 1.0f;
        float basicDps = GetBasicDps(player->GetLevel());
        float basicGs = GetBasicGs(player->GetLevel());
        uint32 mixedGearScore = PlayerbotAI::GetMixedGearScore(player, true, false, 12);
        float gs_modifier = (float)mixedGearScore / basicGs;
        // bonus for wotlk epic gear
        if (mixedGearScore >= 300)
        {
            gs_modifier *= 1 + (mixedGearScore - 300) * 0.01;
        }
        if (gs_modifier < 0.75)
            gs_modifier = 0.75;
        if (gs_modifier > 4)
            gs_modifier = 4;
        totalDps += basicDps * roleMultiplier * gs_modifier;
    }
    // Group buff bonus
    if (groupPlayer.size() >= 25)
        totalDps *= 1.2;
    else if (groupPlayer.size() >= 10)
        totalDps *= 1.1;
    else if (groupPlayer.size() >= 5)
        totalDps *= 1.05;
    return totalDps;
}

float EstimatedGroupDpsValue::GetBasicDps(uint32 level)
{
    float basic_dps;

    if (level <= 15)
    {
        basic_dps = 5 + level * 1;
    }
    else if (level <= 25)
    {
        basic_dps = 20 + (level - 15) * 2;
    }
    else if (level <= 45)
    {
        basic_dps = 40 + (level - 25) * 3;
    }
    else if (level <= 55)
    {
        basic_dps = 100 + (level - 45) * 20;
    }
    else if (level <= 60)
    {
        basic_dps = 300 + (level - 55) * 50;
    }
    else if (level <= 70)
    {
        basic_dps = 550 + (level - 60) * 65;
    }
    else
    {
        basic_dps = 1200 + (level - 70) * 200;
    }
    return basic_dps;
}

float EstimatedGroupDpsValue::GetBasicGs(uint32 level)
{
    float basic_gs;

    if (level <= 8)
    {
        basic_gs = PlayerbotFactory::CalcMixedGearScore(level + 5, ITEM_QUALITY_NORMAL);
    }
    else if (level <= 15)
    {
        basic_gs = PlayerbotFactory::CalcMixedGearScore(level + 5, ITEM_QUALITY_UNCOMMON);
    }
    else if (level <= 60)
    {
        basic_gs = PlayerbotFactory::CalcMixedGearScore(level + 5, ITEM_QUALITY_RARE);
    }
    else if (level <= 70)
    {
        basic_gs = PlayerbotFactory::CalcMixedGearScore(85 + (level - 60) * 3, ITEM_QUALITY_RARE);
    }
    else
    {
        basic_gs = PlayerbotFactory::CalcMixedGearScore(155 + (level - 70) * 4, ITEM_QUALITY_RARE);
    }
    return basic_gs;
}