/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PartyMemberValue.h"

#include "Playerbots.h"
#include "ServerFacade.h"

Unit* PartyMemberValue::FindPartyMember(std::vector<Player*>* party, FindPlayerPredicate& predicate)
{
    for (Player* player : *party)
    {
        if (predicate.Check(player) && Check(player))
            return player;

        if (Pet* pet = player->GetPet())
            if (predicate.Check(pet) && Check(pet))
                return pet;
    }

    return nullptr;
}

Unit* PartyMemberValue::FindPartyMember(FindPlayerPredicate& predicate, bool ignoreOutOfGroup)
{
    Player* master = GetMaster();
    // GuidVector nearestPlayers;
    // if (botAI->AllowActivity(OUT_OF_PARTY_ACTIVITY))
    //     nearestPlayers = AI_VALUE(GuidVector, "nearest friendly players");

    GuidList nearestGroupPlayers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (ref->getSubGroup() != bot->GetSubGroup())
            {
                nearestGroupPlayers.push_back(ref->GetSource()->GetGUID());
            }
            else
            {
                nearestGroupPlayers.push_front(ref->GetSource()->GetGUID());
            }
        }
    }
    else
    {
        std::vector<Player*> vec;
        vec.push_back(bot);
        Unit* target = FindPartyMember(&vec, predicate);
        if (target)
            return target;
        return NULL;
    }

    // if (!ignoreOutOfGroup && !nearestPlayers.empty() && nearestPlayers.size() < 100)
    //     nearestGroupPlayers.insert(nearestGroupPlayers.end(), nearestPlayers.begin(), nearestPlayers.end());

    // nearestPlayers.insert(nearestP   layers.end(), nearestGroupPlayers.begin(), nearestGroupPlayers.end());

    std::vector<Player*> healers;
    std::vector<Player*> tanks;
    std::vector<Player*> others;
    std::vector<Player*> masters;
    if (master)
        masters.push_back(master);

    for (ObjectGuid const guid : nearestGroupPlayers)
    {
        Player* player = botAI->GetPlayer(guid);
        if (!player)
            continue;

        if (botAI->IsHeal(player))
            healers.push_back(player);
        else if (botAI->IsTank(player))
            tanks.push_back(player);
        else if (player != master)
            others.push_back(player);
    }

    std::vector<std::vector<Player*>*> lists;
    lists.push_back(&masters);
    lists.push_back(&healers);
    lists.push_back(&tanks);
    lists.push_back(&others);

    for (std::vector<std::vector<Player*>*>::iterator i = lists.begin(); i != lists.end(); ++i)
    {
        std::vector<Player*>* party = *i;

        Unit* target = FindPartyMember(party, predicate);
        if (target)
            return target;
    }

    return nullptr;
}

bool PartyMemberValue::Check(Unit* player)
{
    // return player && player != bot && player->GetMapId() == bot->GetMapId() && bot->IsWithinDistInMap(player,
    // sPlayerbotAIConfig->sightDistance, false);
    bool isGM = player->ToPlayer() && player->ToPlayer()->IsGameMaster();
    return player && player->GetMapId() == bot->GetMapId() && !isGM &&
           bot->GetDistance(player) < sPlayerbotAIConfig->spellDistance * 2 &&
           bot->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
}

bool PartyMemberValue::IsTargetOfSpellCast(Player* target, SpellEntryPredicate& predicate)
{
    // GuidVector nearestPlayers = AI_VALUE(GuidVector, "nearest friendly players");
    ObjectGuid targetGuid = target ? target->GetGUID() : bot->GetGUID();
    ObjectGuid corpseGuid = target && target->GetCorpse() ? target->GetCorpse()->GetGUID() : ObjectGuid::Empty;

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player || player == bot)
            continue;

        if (player->IsNonMeleeSpellCast(true))
        {
            for (uint8 type = CURRENT_GENERIC_SPELL; type < CURRENT_MAX_SPELL; type++)
            {
                Spell* spell = player->GetCurrentSpell((CurrentSpellTypes)type);
                if (spell && predicate.Check(spell->m_spellInfo))
                {
                    ObjectGuid unitTarget = spell->m_targets.GetUnitTargetGUID();
                    if (unitTarget && unitTarget == targetGuid)
                        return true;
                    ObjectGuid corpseTarget = spell->m_targets.GetCorpseTargetGUID();
                    if (corpseTarget && corpseTarget == corpseGuid)
                        return true;
                }
            }
        }
    }

    return false;
}

class FindMainTankPlayer : public FindPlayerPredicate
{
public:
    FindMainTankPlayer(PlayerbotAI* botAI) : botAI(botAI) {}

    virtual bool Check(Unit* unit)
    {
        Player* player = unit->ToPlayer();
        if (!player)
        {
            return false;
        }
        return botAI->IsMainTank(player);
    }

private:
    PlayerbotAI* botAI;
};

Unit* PartyMemberMainTankValue::Calculate()
{
    FindMainTankPlayer findMainTankPlayer(botAI);
    return FindPartyMember(findMainTankPlayer);
}