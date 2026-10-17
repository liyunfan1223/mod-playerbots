/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestNpcsValue.h"
#include "Playerbots.h"
#include "RSActions.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace
{
    void ResolveTanks(PlayerbotAI* botAI, Player*& mainTank, Player*& assistTank)
    {
        mainTank = nullptr;
        assistTank = nullptr;

        Group* group = botAI->GetBot()->GetGroup();
        if (!group)
            return;

        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!mainTank && PlayerbotAI::IsMainTank(member))
                mainTank = member;

            if (!assistTank && PlayerbotAI::IsAssistTank(member))
                assistTank = member;

            if (mainTank && assistTank)
                break;
        }
    }

    void CollectTrashForTank(Player* bot, std::vector<Unit*>& out)
    {
        for (uint32 const entry : {NPC_CHARSCALE_INVOKER, NPC_CHARSCALE_INVOKER_H,
                                   NPC_CHARSCALE_ASSAULTER, NPC_CHARSCALE_ASSAULTER_H,
                                   NPC_CHARSCALE_ELITE, NPC_CHARSCALE_ELITE_H,
                                   NPC_CHARSCALE_COMMANDER, NPC_CHARSCALE_COMMANDER_H,
                                   NPC_ONYX_FLAMECALLER})
        {
            std::list<Creature*> found;
            bot->GetCreatureListWithEntryInGrid(found, entry, RS_TRASH_TANK_GRAB_RANGE);
            for (Creature* c : found)
                if (c && c->IsAlive() && c->IsInCombat())
                    out.push_back(c);
        }
    }

    void PartitionMobs(std::vector<Unit*> const& mobs, std::vector<Unit*>& assistSet, std::vector<Unit*>& mainSet)
    {
        Unit* assistInvoker = nullptr;
        for (Unit* mob : mobs)
        {
            if (mob && mob->IsAlive() && RsTrashIsInvoker(mob->GetEntry()))
                if (!assistInvoker || mob->GetGUID() < assistInvoker->GetGUID())
                    assistInvoker = mob;
        }

        for (Unit* mob : mobs)
        {
            if (!mob || !mob->IsAlive())
                continue;

            if (RsTrashIsCommander(mob->GetEntry()) || mob == assistInvoker)
                assistSet.push_back(mob);
            else
                mainSet.push_back(mob);
        }
    }

    bool FrontReference(PlayerbotAI* botAI, Player* bot, float& rx, float& ry)
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            float sx = 0.0f;
            float sy = 0.0f;
            int count = 0;
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive())
                    continue;

                if (member->GetMapId() != bot->GetMapId() || member->GetInstanceId() != bot->GetInstanceId())
                    continue;

                if (PlayerbotAI::IsTank(member) || !PlayerbotAI::IsRanged(member))
                    continue;

                sx += member->GetPositionX();
                sy += member->GetPositionY();
                ++count;
            }

            if (count > 0)
            {
                rx = sx / count;
                ry = sy / count;
                return true;
            }
        }

        Player* master = botAI->GetMaster();
        if (master && master->IsInWorld() && master->GetMapId() == bot->GetMapId() &&
            master->GetInstanceId() == bot->GetInstanceId() && !PlayerbotAI::IsTank(master))
        {
            rx = master->GetPositionX();
            ry = master->GetPositionY();
            return true;
        }

        return false;
    }

    bool AwayFromRanged(PlayerbotAI* botAI, Player* bot, float& awayOri)
    {
        float rx = 0.0f;
        float ry = 0.0f;
        if (!FrontReference(botAI, bot, rx, ry))
            return false;

        float const dx = bot->GetPositionX() - rx;
        float const dy = bot->GetPositionY() - ry;
        if (dx * dx + dy * dy < 0.01f)
            return false;

        awayOri = std::atan2(dy, dx);
        return true;
    }

    bool MarkableAdd(Unit* mob)
    {
        if (!mob || !mob->IsAlive())
            return false;

        if (mob->IsInCombat())
            return true;

        Unit* victim = mob->GetVictim();
        return victim && victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer());
    }
}

bool RsTrashAddsAction::Execute(Event )
{
    context->GetValue<std::string>("rti")->Set("skull");

    if (!RsIsDesignatedMarker(bot))
        return false;

    RsUpdateSkullMarker(bot, FindPriorityAdd());
    return false;
}

Unit* RsTrashAddsAction::FindPriorityAdd()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::vector<Unit*> mobs;
    RsTrashCollect(botAI, mobs);

    std::vector<Unit*> markable;
    for (Unit* mob : mobs)
        if (MarkableAdd(mob))
            markable.push_back(mob);

    if (markable.empty())
        return nullptr;

    int bestRank = std::numeric_limits<int>::max();
    for (Unit* mob : markable)
        bestRank = std::min(bestRank, RsTrashPriorityRank(mob->GetEntry()));

    Unit* current = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
    if (current && MarkableAdd(current) && RsTrashIsTrashEntry(current->GetEntry()) &&
        RsTrashPriorityRank(current->GetEntry()) == bestRank)
        return current;

    Unit* best = nullptr;
    for (Unit* mob : markable)
    {
        if (RsTrashPriorityRank(mob->GetEntry()) != bestRank)
            continue;

        if (!best || mob->GetGUID() < best->GetGUID())
            best = mob;
    }

    return best;
}

bool RsTrashTankAction::HoldAt(std::vector<Unit*> const& assigned, float spotX, float spotY, float spotZ,
                               bool moveToSpot, bool hasAway, float awayOri)
{
    Unit* nearest = nullptr;
    float best = std::numeric_limits<float>::max();
    for (Unit* mob : assigned)
    {
        if (!mob || !mob->IsAlive())
            continue;

        if (botAI->HasCheat(BotCheatMask::raid) && mob->IsInCombat())
            RsForceThreat(mob, bot);

        if (hasAway)
            mob->SetFacingTo(awayOri);

        float const d = bot->GetExactDist2d(mob);
        if (d < best)
        {
            best = d;
            nearest = mob;
        }
    }

    if (nearest)
        bot->SetTarget(nearest->GetGUID());

    if (moveToSpot && bot->GetExactDist2d(spotX, spotY) > RS_TRASH_TANK_SPACE_TOL)
        return MoveTo(bot->GetMapId(), spotX, spotY, spotZ, false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);

    if (nearest)
        bot->SetFacingToObject(nearest);

    return false;
}

bool RsTrashMainTankAction::Execute(Event )
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (RsReleaseIfFollowing(bot))
        return false;

    std::vector<Unit*> mobs;
    CollectTrashForTank(bot, mobs);
    if (mobs.empty())
        return false;

    float awayOri = 0.0f;
    bool const hasAway = AwayFromRanged(botAI, bot, awayOri);

    Player* mainTank = nullptr;
    Player* assistTank = nullptr;
    ResolveTanks(botAI, mainTank, assistTank);

    if (!assistTank || assistTank == bot)
        return HoldAt(mobs, 0.0f, 0.0f, 0.0f, false, hasAway, awayOri);

    std::vector<Unit*> assistSet;
    std::vector<Unit*> mainSet;
    PartitionMobs(mobs, assistSet, mainSet);

    bool hasCommander = false;
    for (Unit* mob : mobs)
        if (mob && mob->IsAlive() && RsTrashIsCommander(mob->GetEntry()))
        {
            hasCommander = true;
            break;
        }

    if (!hasCommander)
        return HoldAt(mainSet, 0.0f, 0.0f, 0.0f, false, hasAway, awayOri);

    float dirX = bot->GetPositionX() - assistTank->GetPositionX();
    float dirY = bot->GetPositionY() - assistTank->GetPositionY();

    float rx = 0.0f;
    float ry = 0.0f;
    if (FrontReference(botAI, bot, rx, ry))
    {
        float ux = rx - assistTank->GetPositionX();
        float uy = ry - assistTank->GetPositionY();
        float const ulen = std::sqrt(ux * ux + uy * uy);
        if (ulen > 0.01f)
        {
            ux /= ulen;
            uy /= ulen;
            float px = -uy;
            float py = ux;
            if (dirX * px + dirY * py < 0.0f)
            {
                px = -px;
                py = -py;
            }
            dirX = px;
            dirY = py;
        }
    }

    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.01f)
    {
        dirX = std::cos(bot->GetOrientation());
        dirY = std::sin(bot->GetOrientation());
        len = 1.0f;
    }

    float const spotX = assistTank->GetPositionX() + (dirX / len) * RS_TRASH_TANK_SPREAD;
    float const spotY = assistTank->GetPositionY() + (dirY / len) * RS_TRASH_TANK_SPREAD;

    return HoldAt(mainSet, spotX, spotY, bot->GetPositionZ(), true, hasAway, awayOri);
}

bool RsTrashAssistTankAction::Execute(Event )
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (RsReleaseIfFollowing(bot))
        return false;

    std::vector<Unit*> mobs;
    CollectTrashForTank(bot, mobs);
    if (mobs.empty())
        return false;

    float awayOri = 0.0f;
    bool const hasAway = AwayFromRanged(botAI, bot, awayOri);

    Player* mainTank = nullptr;
    Player* assistTank = nullptr;
    ResolveTanks(botAI, mainTank, assistTank);

    if (!mainTank || mainTank == bot)
        return HoldAt(mobs, 0.0f, 0.0f, 0.0f, false, hasAway, awayOri);

    std::vector<Unit*> assistSet;
    std::vector<Unit*> mainSet;
    PartitionMobs(mobs, assistSet, mainSet);

    return HoldAt(assistSet, 0.0f, 0.0f, 0.0f, false, hasAway, awayOri);
}

bool RsTrashRangedAction::Execute(Event )
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* nearestTank = nullptr;
    float nd = std::numeric_limits<float>::max();
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (!PlayerbotAI::IsTank(member))
            continue;

        if (member->GetMapId() != bot->GetMapId() || member->GetInstanceId() != bot->GetInstanceId())
            continue;

        float const d = bot->GetExactDist2d(member);
        if (d < nd)
        {
            nd = d;
            nearestTank = member;
        }
    }

    if (!nearestTank)
        return false;

    float ax = bot->GetPositionX() - nearestTank->GetPositionX();
    float ay = bot->GetPositionY() - nearestTank->GetPositionY();
    float alen = std::sqrt(ax * ax + ay * ay);
    if (alen < 0.01f)
    {
        ax = std::cos(bot->GetOrientation());
        ay = std::sin(bot->GetOrientation());
        alen = 1.0f;
    }

    float const desiredX = nearestTank->GetPositionX() + (ax / alen) * RS_TRASH_RANGED_DIST;
    float const desiredY = nearestTank->GetPositionY() + (ay / alen) * RS_TRASH_RANGED_DIST;

    float const dx = desiredX - bot->GetPositionX();
    float const dy = desiredY - bot->GetPositionY();
    float const dist = std::sqrt(dx * dx + dy * dy);
    if (dist <= RS_TRASH_RANGED_TOL)
        return false;

    float const stepLen = std::min(RS_TRASH_RANGED_STEP, dist);
    float const moveX = bot->GetPositionX() + (dx / dist) * stepLen;
    float const moveY = bot->GetPositionY() + (dy / dist) * stepLen;
    float const moveZ = bot->GetPositionZ();

    if (!bot->IsWithinLOS(moveX, moveY, moveZ))
        return false;

    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true);
}
