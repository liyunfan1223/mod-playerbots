/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PartyMemberToHeal.h"
#include "Playerbots.h"
#include "ServerFacade.h"

class IsTargetOfHealingSpell : public SpellEntryPredicate
{
    public:
        bool Check(SpellInfo const* spellInfo) override
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL || spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                    spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MECHANICAL)
                    return true;
            }

            return false;
        }
};

inline bool compareByHealth(Unit const* u1, Unit const* u2)
{
    return u1->GetHealthPct() < u2->GetHealthPct();
}

Unit* PartyMemberToHeal::Calculate()
{
    IsTargetOfHealingSpell predicate;
    std::vector<Unit*> needHeals;
    std::vector<Unit*> tankTargets;

    if (bot->GetTarget())
    {
        Unit* target = botAI->GetUnit(bot->GetTarget());
        if (target && target->IsFriendlyTo(bot) && target->HealthBelowPct(100))
            needHeals.push_back(target);
    }

    Group* group = bot->GetGroup();
    if (!group && needHeals.empty())
        return nullptr;

    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (!Check(player) || !player->IsAlive())
                continue;

            // do not heal dueling members
            if (player->duel && player->duel->Opponent)
                continue;

            uint8 health = player->GetHealthPct();
            if (health < sPlayerbotAIConfig->almostFullHealth || !IsTargetOfSpellCast(player, predicate))
                needHeals.push_back(player);

            if (botAI->IsTank(player) && bot->IsInSameRaidWith(player))
                tankTargets.push_back(player);
        }
    }

    if (needHeals.empty() && tankTargets.empty())
        return nullptr;

    if (needHeals.empty() && !tankTargets.empty())
        needHeals = tankTargets;

    std::sort(needHeals.begin(), needHeals.end(), compareByHealth);

    uint32 healerIndex = 0;
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (!player) continue;
            if (player == bot) break;
            if (botAI->IsHeal(player))
            {
                float percent = (float)player->GetPower(POWER_MANA) / (float)player->GetMaxPower(POWER_MANA) * 100.0;
                if (percent > sPlayerbotAIConfig->lowMana)
                    healerIndex++;
            }
        }
    }
    else
        healerIndex = 1;

    healerIndex = healerIndex % needHeals.size();
    return needHeals[healerIndex];
}

bool PartyMemberToHeal::Check(Unit* player)
{
    return player && player != bot && player->GetMapId() == bot->GetMapId() && player->IsInWorld() &&
        sServerFacade->GetDistance2d(bot, player) < (player->IsPlayer() && botAI->IsTank((Player*)player) ? 50.0f : 40.0f);
}

Unit* PartyMemberToProtect::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::vector<Unit*> needProtect;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (GuidVector::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        Unit* pVictim = unit->GetVictim();
        if (!pVictim || !pVictim->IsPlayer())
            continue;

        if (pVictim == bot)
            continue;

        float attackDistance = 30.0f;
        if (sServerFacade->GetDistance2d(pVictim, unit) > attackDistance)
            continue;

        if (botAI->IsTank((Player*)pVictim) && pVictim->GetHealthPct() > 10)
            continue;
        else if (pVictim->GetHealthPct() > 30)
            continue;

        if (find(needProtect.begin(), needProtect.end(), pVictim) == needProtect.end())
            needProtect.push_back(pVictim);
    }

   if (needProtect.empty())
        return nullptr;

    sort(needProtect.begin(), needProtect.end(), compareByHealth);

    return needProtect[0];
}

