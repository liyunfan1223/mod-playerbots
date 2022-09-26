/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackersValue.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "ServerFacade.h"

GuidVector AttackersValue::Calculate()
{
    std::set<Unit*> targets;

    GuidVector result;
    if (!botAI->AllowActivity(ALL_ACTIVITY))
        return std::move(result);

    AddAttackersOf(bot, targets);

    if (Group* group = bot->GetGroup())
        AddAttackersOf(group, targets);

    RemoveNonThreating(targets);

	for (Unit* unit : targets)
		result.push_back(unit->GetGUID());

    if (bot->duel && bot->duel->Opponent)
        result.push_back(bot->duel->Opponent->GetGUID());

	return std::move(result);
}

void AttackersValue::AddAttackersOf(Group* group, std::set<Unit*>& targets)
{
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player *member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive() || member == bot || member->GetMapId() != bot->GetMapId() || sServerFacade->GetDistance2d(bot, member) > sPlayerbotAIConfig->sightDistance)
            continue;

        AddAttackersOf(member, targets);
    }
}

struct AddGuardiansHelper
{
    explicit AddGuardiansHelper(std::vector<Unit*> &units) : units(units) { }

    void operator()(Unit* target) const
    {
        units.push_back(target);
    }

    std::vector<Unit*> &units;
};

void AttackersValue::AddAttackersOf(Player* player, std::set<Unit*>& targets)
{
    if (!player || !player->IsInWorld() || player->IsBeingTeleported())
        return;

	std::list<Unit*> units;
	Acore::AnyUnfriendlyUnitInObjectRangeCheck u_check(player, player, sPlayerbotAIConfig->sightDistance);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(player, units, u_check);
    Cell::VisitAllObjects(player, searcher, sPlayerbotAIConfig->sightDistance);

	for (Unit* unit : units)
    {
        if (!player->GetGroup())
        {
            if (!unit->GetThreatMgr().GetThreat(player) && (!unit->GetThreatMgr().getCurrentVictim() || unit->GetThreatMgr().getCurrentVictim()->getTarget() != player))
                continue;
        }

        targets.insert(unit);
    }
}

void AttackersValue::RemoveNonThreating(std::set<Unit*>& targets)
{
    for (std::set<Unit*>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        Unit* unit = *tIter;
        if (!IsValidTarget(unit, bot))
        {
            std::set<Unit*>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }
}

bool AttackersValue::IsPossibleTarget(Unit* attacker, Player* bot, float range)
{
    Creature* c = attacker->ToCreature();
    bool rti = false;
    if (attacker && bot->GetGroup())
        rti = bot->GetGroup()->GetTargetIcon(7) == attacker->GetGUID();

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    bool leaderHasThreat = false;
    if (attacker && bot->GetGroup() && botAI->GetMaster())
        leaderHasThreat = attacker->GetThreatMgr().GetThreat(botAI->GetMaster());

    bool isMemberBotGroup = false;
    if (bot->GetGroup() && botAI->GetMaster())
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(botAI->GetMaster());
        if (masterBotAI && !masterBotAI->IsRealPlayer())
            isMemberBotGroup = true;
    }

    bool inCannon = botAI->IsInVehicle(false, true);

    bool enemy = botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target")->Get();

    return attacker && attacker->IsInWorld() && attacker->GetMapId() == bot->GetMapId() && !attacker->isDead() && !attacker->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2) &&
        (inCannon || !attacker->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)) && attacker->CanSeeOrDetect(bot) &&
        !(attacker->HasUnitState(UNIT_STATE_STUNNED) && botAI->HasAura("shackle undead", attacker)) && !((attacker->IsPolymorphed() || botAI->HasAura("sap", attacker) || /*attacker->IsCharmed() ||*/ attacker->isFeared()) && !rti) &&
        /*!sServerFacade->IsInRoots(attacker) &&*/!attacker->IsFriendlyTo(bot) && bot->IsWithinDistInMap(attacker, range) &&
        !attacker->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) &&
        !(attacker->GetGUID().IsPet() && enemy) &&
        !(attacker->GetCreatureType() == CREATURE_TYPE_CRITTER && !attacker->IsInCombat()) && !(sPlayerbotAIConfig->IsInPvpProhibitedZone(attacker->GetAreaId()) &&
        (attacker->GetGUID().IsPlayer() || attacker->GetGUID().IsPet())) && (!c || (!c->IsInEvadeMode() && ((!isMemberBotGroup && botAI->HasStrategy("attack tagged", BOT_STATE_NON_COMBAT)) ||
        leaderHasThreat || (!c->hasLootRecipient() && (!c->GetVictim() || c->GetVictim() && ((!c->GetVictim()->IsPlayer() || bot->IsInSameGroupWith(c->GetVictim()->ToPlayer())) ||
        (botAI->GetMaster() && c->GetVictim() == botAI->GetMaster())))) || c->isTappedBy(bot))));
}

bool AttackersValue::IsValidTarget(Unit *attacker, Player *bot)
{
    return  IsPossibleTarget(attacker, bot) && (attacker->GetThreatMgr().getCurrentVictim() || attacker->GetGuidValue(UNIT_FIELD_TARGET) ||
        attacker->GetGUID().IsPlayer() || attacker->GetGUID() == GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<ObjectGuid>("pull target")->Get());
}
bool PossibleAddsValue::Calculate()
{
    GuidVector possible = botAI->GetAiObjectContext()->GetValue<GuidVector >("possible targets no los")->Get();
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector >("attackers")->Get();

    for (ObjectGuid const guid : possible)
    {
        if (find(attackers.begin(), attackers.end(), guid) != attackers.end())
            continue;

        if (Unit* add = botAI->GetUnit(guid))
        {
            if (!add->GetTarget() && !add->GetThreatMgr().getCurrentVictim() && add->IsHostileTo(bot))
            {
                for (ObjectGuid const attackerGUID : attackers)
                {
                    Unit* attacker = botAI->GetUnit(attackerGUID);
                    if (!attacker)
                        continue;

                    float dist = sServerFacade->GetDistance2d(attacker, add);
                    if (sServerFacade->IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig->aoeRadius * 1.5f))
                        continue;

                    if (sServerFacade->IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig->aggroDistance))
                        return true;
                }
            }
        }
    }

    return false;
}
