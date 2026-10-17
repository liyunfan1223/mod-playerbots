/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Formations.h"
#include "Arrow.h"
#include "Event.h"
#include "Map.h"
#include "Playerbots.h"
#include "ServerFacade.h"

WorldLocation Formation::NullLocation = WorldLocation();

bool IsSameLocation(WorldLocation const& a, WorldLocation const& b)
{
    return a.GetPositionX() == b.GetPositionX() && a.GetPositionY() == b.GetPositionY() &&
           a.GetPositionZ() == b.GetPositionZ() && a.GetMapId() == b.GetMapId();
}

bool Formation::IsNullLocation(WorldLocation const& loc) { return IsSameLocation(loc, Formation::NullLocation); }

bool ValidateTargetContext(Unit* a, Unit* b, Map*& outMap)
{
    if (!a || !b || a == b)
        return false;

    if (!a->IsInWorld() || !b->IsInWorld())
        return false;

    if (a->IsDuringRemoveFromWorld() || b->IsDuringRemoveFromWorld())
        return false;

    Map* map = a->GetMap();
    if (!map || map != b->GetMap())
        return false;

    outMap = map;

    return true;
}

bool ValidateTargetContext(Unit* a, Unit* b)
{
    Map* unused = nullptr;
    return ValidateTargetContext(a, b, unused);
}

WorldLocation MoveAheadFormation::GetLocation()
{
    Player* master = GetMaster();
    if (!ValidateTargetContext(master, bot))
        return Formation::NullLocation;

    WorldLocation loc = GetLocationInternal();
    if (Formation::IsNullLocation(loc))
        return loc;

    float x = loc.GetPositionX();
    float y = loc.GetPositionY();
    float z = loc.GetPositionZ();

    // if (master->isMoving())
    // {
    //     float ori = master->GetOrientation();
    //     float x1 = x + sPlayerbotAIConfig.TooCloseDistance * cos(ori);
    //     float y1 = y + sPlayerbotAIConfig.TooCloseDistance * sin(ori);
    //     float ground = map->GetHeight(x1, y1, z);
    //     if (ground > INVALID_HEIGHT)
    //     {
    //         x = x1;
    //         y = y1;
    //     }
    // }

    // float ground = map->GetHeight(x, y, z);
    // if (ground <= INVALID_HEIGHT)
    //     return Formation::NullLocation;

    // z += CONTACT_DISTANCE;
    // bot->UpdateAllowedPositionZ(x, y, z);
    return WorldLocation(master->GetMapId(), x, y, z);
}

class MeleeFormation : public FollowFormation
{
public:
    MeleeFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "melee") {}

    std::string const GetTargetName() override { return "group leader"; }
};

class QueueFormation : public FollowFormation
{
public:
    QueueFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "queue") {}

    std::string const GetTargetName() override { return "line target"; }
};

class NearFormation : public MoveAheadFormation
{
public:
    NearFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "near") {}

    WorldLocation GetLocationInternal() override
    {
        Player* master = GetMaster();
        Map* map = nullptr;
        if (!ValidateTargetContext(master, bot, map))
            return Formation::NullLocation;

        float range = sPlayerbotAIConfig.FollowDistance;
        float angle = GetFollowAngle();
        float x = master->GetPositionX() + cos(angle) * range;
        float y = master->GetPositionY() + sin(angle) * range;
        float z = master->GetPositionZ() + master->GetHoverHeight();
        if (!map->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                  master->GetPositionZ(), x, y, z))
        {
            x = master->GetPositionX() + cos(angle) * range;
            y = master->GetPositionY() + sin(angle) * range;
            z = master->GetPositionZ() + master->GetHoverHeight();
            master->UpdateAllowedPositionZ(x, y, z);
        }
        return WorldLocation(master->GetMapId(), x, y, z);
    }

    float GetMaxDistance() override { return sPlayerbotAIConfig.FollowDistance; }
};

class ChaosFormation : public MoveAheadFormation
{
public:
    ChaosFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "chaos"), lastChangeTime(0) {}

    WorldLocation GetLocationInternal() override
    {
        Player* master = GetMaster();
        Map* map = nullptr;
        if (!ValidateTargetContext(master, bot, map))
            return Formation::NullLocation;

        float range = sPlayerbotAIConfig.FollowDistance;
        float angle = GetFollowAngle();

        time_t now = time(nullptr);
        if (!lastChangeTime || now - lastChangeTime >= 3)
        {
            lastChangeTime = now;

            dx = (urand(0, 10) / 10.0f - 0.5f) * sPlayerbotAIConfig.TooCloseDistance;
            dy = (urand(0, 10) / 10.0f - 0.5f) * sPlayerbotAIConfig.TooCloseDistance;
            dr = std::sqrt(dx * dx + dy * dy);
        }

        float x = master->GetPositionX() + std::cos(angle) * range + dx;
        float y = master->GetPositionY() + std::sin(angle) * range + dy;
        float z = master->GetPositionZ() + master->GetHoverHeight();

        if (!map->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                  master->GetPositionZ(), x, y, z))
        {
            // Recompute a clean fallback and clamp Z
            x = master->GetPositionX() + std::cos(angle) * range + dx;
            y = master->GetPositionY() + std::sin(angle) * range + dy;
            z = master->GetPositionZ() + master->GetHoverHeight();

            master->UpdateAllowedPositionZ(x, y, z);
        }

        return WorldLocation(master->GetMapId(), x, y, z);
    }

    float GetMaxDistance() override { return sPlayerbotAIConfig.FollowDistance + dr; }

private:
    time_t lastChangeTime;
    float dx = 0.f;
    float dy = 0.f;
    float dr = 0.f;
};

class CircleFormation : public MoveFormation
{
public:
    CircleFormation(PlayerbotAI* botAI) : MoveFormation(botAI, "circle") {}

    WorldLocation GetLocation() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        Player* master = GetMaster();

        // Fix: if no target OR target is the bot, fall back to master
        if (!target || target == bot)
            target = master;

        Map* map = nullptr;
        if (!ValidateTargetContext(master, bot, map))
            return Formation::NullLocation;

        float range = 2.0f;
        switch (bot->getClass())
        {
            case CLASS_HUNTER:
            case CLASS_MAGE:
            case CLASS_PRIEST:
            case CLASS_WARLOCK:
                range = botAI->GetRange("flee");
                break;
            case CLASS_DRUID:
                if (!PlayerbotAI::IsTank(bot))
                    range = botAI->GetRange("flee");
                break;
            case CLASS_SHAMAN:
                if (PlayerbotAI::IsHeal(bot))
                    range = botAI->GetRange("flee");
                break;
        }

        float angle = GetFollowAngle();
        float x = target->GetPositionX() + cos(angle) * range;
        float y = target->GetPositionY() + sin(angle) * range;
        float z = target->GetPositionZ();

        if (!map->CheckCollisionAndGetValidCoords(target, target->GetPositionX(), target->GetPositionY(),
                                                  target->GetPositionZ(), x, y, z))
        {
            x = target->GetPositionX() + cos(angle) * range;
            y = target->GetPositionY() + sin(angle) * range;
            z = target->GetPositionZ();
            target->UpdateAllowedPositionZ(x, y, z);
        }

        return WorldLocation(bot->GetMapId(), x, y, z);
    }
};

class LineFormation : public MoveAheadFormation
{
public:
    LineFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "line") {}

    WorldLocation GetLocationInternal() override
    {
        Group* group = bot->GetGroup();
        if (!group)
            return Formation::NullLocation;

        float range = 2.0f;

        Player* master = GetMaster();
        if (!master)
            return Formation::NullLocation;

        float x = master->GetPositionX();
        float y = master->GetPositionY();
        float z = master->GetPositionZ();
        float orientation = master->GetOrientation();

        std::vector<Player*> players;
        players.reserve(group->GetMembersCount());

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || member == master)
                continue;

            players.push_back(member);
        }

        players.insert(players.begin() + players.size() / 2, master);
        return MoveLine(players, 0.0f, x, y, z, orientation, range);
    }
};

class ShieldFormation : public MoveFormation
{
public:
    ShieldFormation(PlayerbotAI* botAI) : MoveFormation(botAI, "shield") {}

    WorldLocation GetLocation() override
    {
        Group* group = bot->GetGroup();
        if (!group)
            return Formation::NullLocation;

        float range = sPlayerbotAIConfig.FollowDistance;

        Player* master = GetMaster();
        if (!master)
            return Formation::NullLocation;

        float x = master->GetPositionX();
        float y = master->GetPositionY();
        float z = master->GetPositionZ();
        float orientation = master->GetOrientation();

        std::vector<Player*> tanks;
        std::vector<Player*> dps;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || member == master)
                continue;

            if (PlayerbotAI::IsTank(member))
                tanks.push_back(member);
            else
                dps.push_back(member);
        }

        if (PlayerbotAI::IsTank(master))
            tanks.insert(tanks.begin() + (tanks.size() + 1) / 2, master);
        else
            dps.insert(dps.begin() + (dps.size() + 1) / 2, master);

        if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsTank(master))
            return MoveLine(tanks, 0.0f, x, y, z, orientation, range);

        if (!PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsTank(master))
            return MoveLine(dps, 0.0f, x, y, z, orientation, range);

        if (PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsTank(master))
        {
            float diff = (tanks.size() % 2 == 0) ? -sPlayerbotAIConfig.TooCloseDistance / 2.0f : 0.0f;
            return MoveLine(tanks, diff, x + cos(orientation) * range, y + sin(orientation) * range, z, orientation,
                            range);
        }

        if (!PlayerbotAI::IsTank(bot) && PlayerbotAI::IsTank(master))
        {
            float diff = (dps.size() % 2 == 0) ? -sPlayerbotAIConfig.TooCloseDistance / 2.0f : 0.0f;
            return MoveLine(dps, diff, x - cos(orientation) * range, y - sin(orientation) * range, z, orientation,
                            range);
        }

        return Formation::NullLocation;
    }
};

class FarFormation : public FollowFormation
{
public:
    FarFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "far") {}

    WorldLocation GetLocation() override
    {
        Player* master = GetMaster();
        Map* map = nullptr;
        if (!ValidateTargetContext(master, bot, map))
            return Formation::NullLocation;

        float range = sPlayerbotAIConfig.FarDistance;
        float followRange = sPlayerbotAIConfig.FollowDistance;

        if (ServerFacade::instance().GetDistance2d(bot, master) <= range)
            return Formation::NullLocation;

        float angleToBot = master->GetAngle(bot);
        float followAngle = GetFollowAngle();

        float x = master->GetPositionX() + cos(angleToBot) * range + cos(followAngle) * followRange;
        float y = master->GetPositionY() + sin(angleToBot) * range + sin(followAngle) * followRange;
        float z = master->GetPositionZ();

        float ground = master->GetMapHeight(x, y, z + 30.0f);
        if (ground <= INVALID_HEIGHT)
        {
            float minDist = 0.f;
            float minX = 0.f, minY = 0.f;

            for (float a = 0.0f; a <= 2 * M_PI; a += M_PI / 16.0f)
            {
                float tx = master->GetPositionX() + cos(a) * range + cos(followAngle) * followRange;
                float ty = master->GetPositionY() + sin(a) * range + sin(followAngle) * followRange;

                float dist = ServerFacade::instance().GetDistance2d(bot, tx, ty);
                float tg = master->GetMapHeight(tx, ty, z + 30.0f);

                if (tg > INVALID_HEIGHT && (!minDist || dist < minDist))
                {
                    minDist = dist;
                    minX = tx;
                    minY = ty;
                }
            }

            if (!minDist)
                return Formation::NullLocation;

            float lz = z;
            if (!map->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                      master->GetPositionZ(), minX, minY, lz))
            {
                lz = z + master->GetHoverHeight();
                master->UpdateAllowedPositionZ(minX, minY, lz);
            }

            return WorldLocation(bot->GetMapId(), minX, minY, lz);
        }

        if (!map->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                  master->GetPositionZ(), x, y, z))
        {
            x = master->GetPositionX() + cos(angleToBot) * range + cos(followAngle) * followRange;
            y = master->GetPositionY() + sin(angleToBot) * range + sin(followAngle) * followRange;
            z = master->GetPositionZ() + master->GetHoverHeight();
            master->UpdateAllowedPositionZ(x, y, z);
        }

        return WorldLocation(bot->GetMapId(), x, y, z);
    }
};

float Formation::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = bot->GetGroup();

    // If there's no master and no group
    if (!master && !group)
        return 0.0f;

    uint32 index = 1;
    uint32 total = 1;

    std::vector<Player*> roster;

    if (group)
    {
        bool left = true;  // Used for alternating tanks' positions

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();

            // Skip invalid, dead, or out-of-map members
            if (!member || !member->IsAlive() || bot->GetMapId() != member->GetMapId())
                continue;

            // Skip the master
            if (member == master)
                continue;

            // Put DPS in the middle
            if (!PlayerbotAI::IsTank(member) && !PlayerbotAI::IsHeal(member))
            {
                roster.insert(roster.begin() + roster.size() / 2, member);
            }

            // Put Healers in the middle
            else if (PlayerbotAI::IsHeal(member))
            {
                roster.insert(roster.begin() + roster.size() / 2, member);
            }

            // Handle tanks (alternate between front and back)
            else if (PlayerbotAI::IsTank(member))
            {
                if (left)
                    roster.push_back(member);  // Place tank at the back
                else
                    roster.insert(roster.begin(), member);  // Place tank at the front

                left = !left;  // Alternate for the next tank
            }

            total++;
        }
    }
    else if (master)
    {
        // If the bot is following a master, look up the bot's position in the master's list
        PlayerbotMgr* masterBotMgr = GET_PLAYERBOT_MGR(master);
        if (masterBotMgr && !GET_PLAYERBOT_AI(master))
        {
            for (auto it = masterBotMgr->GetPlayerBotsBegin(); it != masterBotMgr->GetPlayerBotsEnd(); ++it)
            {
                if (it->second == bot)
                {
                    index = total;  // Found bot in master's list, set the index
                    break;
                }
                ++total;
            }
        }
    }

    // Find the bot's position in the roster
    auto it = std::find(roster.begin(), roster.end(), bot);
    if (it != roster.end())
    {
        index = std::distance(roster.begin(), it) + 1;  // Find bot's index in the roster
    }

    // Return
    float start = (master ? master->GetOrientation() : 0.0f);
    return start + (0.125f + 1.75f * index / total + (total == 2 ? 0.125f : 0.0f)) * M_PI;
}

FormationValue::FormationValue(PlayerbotAI* botAI)
    : ManualSetValue<Formation*>(botAI, new ChaosFormation(botAI), "formation")
{
}

FormationValue::~FormationValue()
{
    if (value)
    {
        delete value;
        value = nullptr;
    }
}

std::string const FormationValue::Save() { return value ? value->getName() : "?"; }

bool FormationValue::Load(std::string const formation)
{
    if (formation == "melee")
    {
        if (value)
            delete value;

        value = new MeleeFormation(botAI);
    }
    else if (formation == "queue")
    {
        if (value)
            delete value;

        value = new QueueFormation(botAI);
    }
    else if (formation == "chaos" || formation == "default")
    {
        if (value)
            delete value;

        value = new ChaosFormation(botAI);
    }
    else if (formation == "circle")
    {
        if (value)
            delete value;

        value = new CircleFormation(botAI);
    }
    else if (formation == "line")
    {
        if (value)
            delete value;

        value = new LineFormation(botAI);
    }
    else if (formation == "shield")
    {
        if (value)
            delete value;

        value = new ShieldFormation(botAI);
    }
    else if (formation == "arrow")
    {
        if (value)
            delete value;

        value = new ArrowFormation(botAI);
    }
    else if (formation == "near")
    {
        if (value)
            delete value;

        value = new NearFormation(botAI);
    }
    else if (formation == "far")
    {
        if (value)
            delete value;

        value = new FarFormation(botAI);
    }
    else
        return false;

    return true;
}

bool SetFormationAction::Execute(Event event)
{
    std::string const formation = event.getParam();

    FormationValue* value = (FormationValue*)context->GetValue<Formation*>("formation");
    if (formation == "?" || formation.empty())
    {
        std::ostringstream str;
        str << "Formation: |cff00ff00" << value->Get()->getName();
        botAI->TellMaster(str);
        return true;
    }

    if (formation == "show")
    {
        WorldLocation loc = value->Get()->GetLocation();
        if (!Formation::IsNullLocation(loc))
            botAI->Ping(loc.GetPositionX(), loc.GetPositionY());

        return true;
    }

    if (!value->Load(formation))
    {
        std::ostringstream str;
        str << "Invalid formation: |cffff0000" << formation;
        botAI->TellMaster(str);
        botAI->TellMaster(
            "Please set to any of:|cffffffff chaos (default), near, queue, circle, line, shield, arrow, melee, far");
        return false;
    }

    std::ostringstream str;
    str << "Formation set to: " << formation;
    botAI->TellMaster(str);
    return true;
}

WorldLocation MoveFormation::MoveLine(std::vector<Player*> line, float diff, float cx, float cy, float cz,
                                      float orientation, float range)
{
    if (line.size() < 5)
    {
        return MoveSingleLine(line, diff, cx, cy, cz, orientation, range);
    }

    uint32 lines = ceil((double)line.size() / 5.0);
    for (uint32 i = 0; i < lines; i++)
    {
        float radius = range * i;
        float x = cx + cos(orientation) * radius;
        float y = cy + sin(orientation) * radius;

        std::vector<Player*> singleLine;
        for (uint32 j = 0; j < 5 && !line.empty(); j++)
        {
            singleLine.push_back(line[line.size() - 1]);
            line.pop_back();
        }

        WorldLocation loc = MoveSingleLine(singleLine, diff, x, y, cz, orientation, range);
        if (!Formation::IsNullLocation(loc))
            return loc;
    }

    return Formation::NullLocation;
}

WorldLocation MoveFormation::MoveSingleLine(std::vector<Player*> line, float diff, float cx, float cy, float cz,
                                            float orientation, float range)
{
    float count = line.size();
    float angleLeft = orientation - M_PI / 2.0f;
    float x0 = cx + std::cos(angleLeft) * (range * std::floor(count / 2.0f) + diff);
    float y0 = cy + std::sin(angleLeft) * (range * std::floor(count / 2.0f) + diff);

    uint32 index = 0;
    for (Player* member : line)
    {
        if (member == bot)
        {
            float angleRight = orientation + M_PI / 2.0f;
            float radius = range * index;

            float lx = x0 + std::cos(angleRight) * radius;
            float ly = y0 + std::sin(angleRight) * radius;
            float lz = cz;

            Player* master = botAI->GetMaster();
            Map* map = master ? master->GetMap() : nullptr;

            // if not fully in world ignore collision corrections.
            if (!master || !map || !bot || map != bot->GetMap() || !master->IsInWorld() ||
                master->IsDuringRemoveFromWorld() || !bot->IsInWorld() || bot->IsDuringRemoveFromWorld())
            {
                return WorldLocation(bot->GetMapId(), lx, ly, lz);
            }

            // if fully loaded check collision and applies coordinate corrections if needed
            map->CheckCollisionAndGetValidCoords(master, master->GetPositionX(), master->GetPositionY(),
                                                 master->GetPositionZ(), lx, ly, lz);

            return WorldLocation(bot->GetMapId(), lx, ly, lz);
        }

        ++index;
    }

    return Formation::NullLocation;
}
