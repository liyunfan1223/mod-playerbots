/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TargetValue.h"

#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ScriptedCreature.h"
#include "ThreatMgr.h"

Unit* FindTargetStrategy::GetResult() { return result; }

Unit* TargetValue::FindTarget(FindTargetStrategy* strategy)
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        ThreatMgr& ThreatMgr = unit->GetThreatMgr();
        strategy->CheckAttacker(unit, &ThreatMgr);
    }

    return strategy->GetResult();
}

bool FindNonCcTargetStrategy::IsCcTarget(Unit* attacker)
{
    if (Group* group = botAI->GetBot()->GetGroup())
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* member = ObjectAccessor::FindPlayer(itr->guid);
            if (!member || !member->IsAlive())
                continue;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
            {
                if (botAI->GetAiObjectContext()->GetValue<Unit*>("rti cc target")->Get() == attacker)
                    return true;

                std::string const rti = botAI->GetAiObjectContext()->GetValue<std::string>("rti cc")->Get();
                int32 index = RtiTargetValue::GetRtiIndex(rti);
                if (index != -1)
                {
                    if (ObjectGuid guid = group->GetTargetIcon(index))
                        if (attacker->GetGUID() == guid)
                            return true;
                }
            }
        }

        if (ObjectGuid guid = group->GetTargetIcon(4))
            if (attacker->GetGUID() == guid)
                return true;
    }

    return false;
}

void FindTargetStrategy::GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount)
{
    Player* bot = botAI->GetBot();
    if (tankCountCache.find(creature) != tankCountCache.end())
    {
        *tankCount = tankCountCache[creature];
        *dpsCount = dpsCountCache[creature];
        return;
    }

    *tankCount = 0;
    *dpsCount = 0;

    Unit::AttackerSet attackers(creature->getAttackers());
    for (Unit* attacker : attackers)
    {
        if (!attacker || !attacker->IsAlive() || attacker == bot)
            continue;

        Player* player = attacker->ToPlayer();
        if (!player)
            continue;

        if (botAI->IsTank(player))
            ++(*tankCount);
        else
            ++(*dpsCount);
    }

    tankCountCache[creature] = *tankCount;
    dpsCountCache[creature] = *dpsCount;
}

bool FindTargetStrategy::IsHighPriority(Unit* attacker)
{
    if (Group* group = botAI->GetBot()->GetGroup())
    {
        ObjectGuid guid = group->GetTargetIcon(7);
        if (guid && attacker->GetGUID() == guid)
        {
            return true;
        }
    }
    GuidVector prioritizedTargets = botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get();
    for (ObjectGuid targetGuid : prioritizedTargets)
    {
        if (targetGuid && attacker->GetGUID() == targetGuid)
        {
            return true;
        }
    }
    return false;
}

WorldPosition LastLongMoveValue::Calculate()
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");
    if (lastMove.lastPath.empty())
        return WorldPosition();

    return lastMove.lastPath.getBack();
}

WorldPosition HomeBindValue::Calculate()
{
    return WorldPosition(bot->m_homebindMapId, bot->m_homebindX, bot->m_homebindY, bot->m_homebindZ, 0.f);
}

Unit* FindTargetValue::Calculate()
{
    if (qualifier == "")
    {
        return nullptr;
    }
    Group* group = bot->GetGroup();
    if (!group)
    {
        return nullptr;
    }
    HostileReference* ref = bot->getHostileRefMgr().getFirst();
    while (ref)
    {
        ThreatMgr* threatManager = ref->GetSource();
        Unit* unit = threatManager->GetOwner();
        std::wstring wnamepart;
        Utf8toWStr(unit->GetName(), wnamepart);
        wstrToLower(wnamepart);
        if (!qualifier.empty() && qualifier.length() == wnamepart.length() && Utf8FitTo(qualifier, wnamepart))
        {
            return unit;
        }
        ref = ref->next();
    }
    return nullptr;
}

void FindBossTargetStrategy::CheckAttacker(Unit* attacker, ThreatMgr* threatManager)
{
    UnitAI* unitAI = attacker->GetAI();
    BossAI* bossAI = dynamic_cast<BossAI*>(unitAI);
    if (bossAI)
    {
        result = attacker;
    }
}

Unit* BossTargetValue::Calculate()
{
    FindBossTargetStrategy strategy(botAI);
    return FindTarget(&strategy);
}