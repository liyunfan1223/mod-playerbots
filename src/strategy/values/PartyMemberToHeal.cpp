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

    Group* group = bot->GetGroup();
    if (!group)
        return bot;

    bool isRaid = bot->GetGroup()->isRaidGroup();
    MinValueCalculator calc(100);
    
    for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (player && Check(player) && player->IsAlive()) {
            uint8 health = player->GetHealthPct();
            if (isRaid || health < sPlayerbotAIConfig->mediumHealth || !IsTargetOfSpellCast(player, predicate)) {
                if (player->GetDistance2d(bot) > sPlayerbotAIConfig->healDistance) {
                    calc.probe(health + 30, player);
                } else {
                    calc.probe(health + player->GetDistance2d(bot) / 10, player);
                }
            }
        }

        Pet* pet = player->GetPet();
        if (pet && Check(pet) && pet->IsAlive()) {
            uint8 health = ((Unit*)pet)->GetHealthPct();
            if (isRaid || health < sPlayerbotAIConfig->mediumHealth)
                calc.probe(health + 30, pet);
        }

        Unit* charm = player->GetCharm();
        if (charm && Check(charm) && charm->IsAlive()) {
            uint8 health = charm->GetHealthPct();
            if (isRaid || health < sPlayerbotAIConfig->mediumHealth)
                calc.probe(health, charm);
        }
    }
    return (Unit*)calc.param;
}

bool PartyMemberToHeal::Check(Unit* player)
{
    // return player && player != bot && player->GetMapId() == bot->GetMapId() && player->IsInWorld() &&
    //     sServerFacade->GetDistance2d(bot, player) < (player->IsPlayer() && botAI->IsTank((Player*)player) ? 50.0f : 40.0f);
    return player->GetMapId() == bot->GetMapId() &&
        bot->GetDistance2d(player) < sPlayerbotAIConfig->healDistance * 2 &&
        bot->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
}

Unit* PartyMemberToProtect::Calculate()
{
    return nullptr;
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

