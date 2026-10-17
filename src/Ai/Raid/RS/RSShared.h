/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSSHARED_H
#define PLAYERBOTS_RSSHARED_H

#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "ThreatManager.h"
#include <vector>

template <typename Pred>
inline void RsCollectTargets(PlayerbotAI* botAI, std::vector<Unit*>& out, Pred pred)
{
    GuidVector const targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && pred(unit))
            out.push_back(unit);
    }
}

template <typename Pred>
inline Unit* RsFindTarget(PlayerbotAI* botAI, Pred pred)
{
    GuidVector const targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && pred(unit))
            return unit;
    }
    return nullptr;
}

inline void RsForceThreat(Unit* target, Player* bot)
{
    if (!target || target->GetVictim() == bot)
        return;

    ThreatManager& mgr = target->GetThreatMgr();
    mgr.AddThreat(bot, 1000000.0f, nullptr, true, true);
    mgr.FixateTarget(bot);
}

inline bool RsReleaseIfFollowing(Player* bot)
{
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        return false;

    bot->AttackStop();
    bot->CastStop();
    if (bot->GetTarget())
        bot->SetTarget(ObjectGuid::Empty);
    return true;
}

inline bool RsIsDesignatedMarker(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return true;

    ObjectGuid lowest = bot->GetGUID();
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        PlayerbotAI* memberAI = sPlayerbotsMgr.GetPlayerbotAI(member);
        if (!memberAI)
            continue;

        if (PlayerbotAI::IsTank(member) || PlayerbotAI::IsHeal(member))
            continue;

        if (member->GetMapId() != bot->GetMapId() || member->GetInstanceId() != bot->GetInstanceId())
            continue;

        if (member->GetGUID() < lowest)
            lowest = member->GetGUID();
    }

    return bot->GetGUID() == lowest;
}

inline void RsUpdateSkullMarker(Player* bot, Unit* priorityAdd)
{
    if (!priorityAdd)
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    if (group->GetTargetIcon(RtiTargetValue::skullIndex) != priorityAdd->GetGUID())
        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), priorityAdd->GetGUID());
}

#endif
