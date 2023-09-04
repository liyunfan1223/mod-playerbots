/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Formations.h"
#include "Arrow.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"

WorldLocation Formation::NullLocation = WorldLocation();

bool IsSameLocation(WorldLocation const &a, WorldLocation const &b)
{
	return a.GetPositionX() == b.GetPositionX() && a.GetPositionY() == b.GetPositionY() && a.GetPositionZ() == b.GetPositionZ() && a.GetMapId() == b.GetMapId();
}

bool Formation::IsNullLocation(WorldLocation const& loc)
{
	return IsSameLocation(loc, Formation::NullLocation);
}

WorldLocation MoveAheadFormation::GetLocation()
{
    Player* master = botAI->GetGroupMaster();
    if (!master || master == bot)
        return WorldLocation();

    WorldLocation loc = GetLocationInternal();
    if (Formation::IsNullLocation(loc))
        return loc;

    float x = loc.GetPositionX();
    float y = loc.GetPositionY();
    float z = loc.GetPositionZ();

    // if (master->isMoving())
    // {
    //     float ori = master->GetOrientation();
    //     float x1 = x + sPlayerbotAIConfig->tooCloseDistance * cos(ori);
    //     float y1 = y + sPlayerbotAIConfig->tooCloseDistance * sin(ori);
    //     float ground = master->GetMap()->GetHeight(x1, y1, z);
    //     if (ground > INVALID_HEIGHT)
    //     {
    //         x = x1;
    //         y = y1;
    //     }
    // }

    // float ground = master->GetMap()->GetHeight(x, y, z);
    // if (ground <= INVALID_HEIGHT)
    //     return Formation::NullLocation;

    //z += CONTACT_DISTANCE;
    //bot->UpdateAllowedPositionZ(x, y, z);
    return WorldLocation(master->GetMapId(), x, y, z);
}

class MeleeFormation : public FollowFormation
{
    public:
        MeleeFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "melee") { }

        std::string const GetTargetName() override { return "master target"; }
};

class QueueFormation : public FollowFormation
{
    public:
        QueueFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "queue") { }

        std::string const GetTargetName() override { return "line target"; }
};

class NearFormation : public MoveAheadFormation
{
    public:
        NearFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "near") { }

        WorldLocation GetLocationInternal() override
        {
            Player* master = GetMaster();
            if (!master)
                return WorldLocation();

            float range = sPlayerbotAIConfig->followDistance;
            float angle = GetFollowAngle();
            float x = master->GetPositionX() + cos(angle) * range;
            float y = master->GetPositionY() + sin(angle) * range;
            float z = master->GetPositionZ();
            float ground = master->GetMap()->GetHeight(x, y, z + 30.0f);
            if (ground <= INVALID_HEIGHT)
                return Formation::NullLocation;

            //z += CONTACT_DISTANCE;
            //bot->UpdateAllowedPositionZ(x, y, z);
            return WorldLocation(master->GetMapId(), x, y, z);
        }

        float GetMaxDistance() override { return sPlayerbotAIConfig->followDistance; }
};

class ChaosFormation : public MoveAheadFormation
{
    public:
        ChaosFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "chaos"), lastChangeTime(0) { }

        WorldLocation GetLocationInternal() override
        {
            Player* master = botAI->GetGroupMaster();
            if (!master)
                return WorldLocation();

            float range = sPlayerbotAIConfig->followDistance;
			float angle = GetFollowAngle();

            time_t now = time(nullptr);
            if (!lastChangeTime || now - lastChangeTime >= 3)
            {
                lastChangeTime = now;
                dx = (urand(0, 10) / 10.0 - 0.5) * sPlayerbotAIConfig->tooCloseDistance;
                dy = (urand(0, 10) / 10.0 - 0.5) * sPlayerbotAIConfig->tooCloseDistance;
                dr = sqrt(dx*dx + dy*dy);
            }

            float x = master->GetPositionX() + cos(angle) * range + dx;
            float y = master->GetPositionY() + sin(angle) * range + dy;
            float z = master->GetPositionZ();
            float ground = master->GetMap()->GetHeight(x, y, z + 30.0f);
            if (ground <= INVALID_HEIGHT)
                return Formation::NullLocation;

            z += CONTACT_DISTANCE;
            bot->UpdateAllowedPositionZ(x, y, z);
            return WorldLocation(master->GetMapId(), x, y, z);
        }

        float GetMaxDistance() override { return sPlayerbotAIConfig->followDistance + dr; }

    private:
        time_t lastChangeTime;
        float dx = 0.f;
        float dy = 0.f;
        float dr = 0.f;
};

class CircleFormation : public MoveFormation
{
    public:
        CircleFormation(PlayerbotAI* botAI) : MoveFormation(botAI, "circle") { }

        WorldLocation GetLocation() override
        {
            float range = 2.0f;

            Unit* target = AI_VALUE(Unit*, "current target");
            Player* master = botAI->GetGroupMaster();
            if (!target && target != bot)
                target = master;

            if (!target)
				return Formation::NullLocation;

            switch (bot->getClass())
            {
                case CLASS_HUNTER:
                case CLASS_MAGE:
                case CLASS_PRIEST:
                case CLASS_WARLOCK:
                    range = botAI->GetRange("flee");
                    break;
                case CLASS_DRUID:
                    if (!botAI->IsTank(bot))
                        range = botAI->GetRange("flee");
                    break;
                case CLASS_SHAMAN:
                    if (botAI->IsHeal(bot))
                        range = botAI->GetRange("flee");
                    break;
            }

            float angle = GetFollowAngle();
            float x = target->GetPositionX() + cos(angle) * range;
            float y = target->GetPositionY() + sin(angle) * range;
            float z = target->GetPositionZ();
            float ground = target->GetMap()->GetHeight(x, y, z + 30.0f);
            if (ground <= INVALID_HEIGHT)
                return Formation::NullLocation;

            z += CONTACT_DISTANCE;
            bot->UpdateAllowedPositionZ(x, y, z);

            return WorldLocation(bot->GetMapId(), x, y, z);
        }
};

class LineFormation : public MoveAheadFormation
{
    public:
        LineFormation(PlayerbotAI* botAI) : MoveAheadFormation(botAI, "line") { }

        WorldLocation GetLocationInternal() override
        {
            Group* group = bot->GetGroup();
            if (!group)
                return Formation::NullLocation;

            float range = 2.0f;

            Player* master = botAI->GetGroupMaster();
            if (!master)
                return Formation::NullLocation;

            float x = master->GetPositionX();
            float y = master->GetPositionY();
            float z = master->GetPositionZ();
            float orientation = master->GetOrientation();

            std::vector<Player*> players;
            GroupReference* gref = group->GetFirstMember();
            while (gref)
            {
                Player* member = gref->GetSource();
                if (member != master)
                    players.push_back(member);

                gref = gref->next();
            }

            players.insert(players.begin() + group->GetMembersCount() / 2, master);

            return MoveLine(players, 0.0f, x, y, z, orientation, range);
        }
};

class ShieldFormation : public MoveFormation
{
    public:
        ShieldFormation(PlayerbotAI* botAI) : MoveFormation(botAI, "shield") { }

        WorldLocation GetLocation() override
        {
            Group* group = bot->GetGroup();
            if (!group)
                return Formation::NullLocation;

            float range = sPlayerbotAIConfig->followDistance;

            Player* master = GetMaster();
            if (!master)
                return Formation::NullLocation;

            float x = master->GetPositionX();
            float y = master->GetPositionY();
            float z = master->GetPositionZ();
            float orientation = master->GetOrientation();

            std::vector<Player*> tanks;
            std::vector<Player*> dps;
            GroupReference* gref = group->GetFirstMember();
            while (gref)
            {
                Player* member = gref->GetSource();
                if (member != master)
                {
                    if (botAI->IsTank(member))
                        tanks.push_back(member);
                    else
                        dps.push_back(member);
                }

                gref = gref->next();
            }

            if (botAI->IsTank(master))
                tanks.insert(tanks.begin() + (tanks.size() + 1) / 2, master);
            else
                dps.insert(dps.begin() + (dps.size() + 1) / 2, master);

            if (botAI->IsTank(bot) && botAI->IsTank(master))
            {
                return MoveLine(tanks, 0.0f, x, y, z, orientation, range);
            }

            if (!botAI->IsTank(bot) && !botAI->IsTank(master))
            {
                return MoveLine(dps, 0.0f, x, y, z, orientation, range);
            }

            if (botAI->IsTank(bot) && !botAI->IsTank(master))
            {
                float diff = tanks.size() % 2 == 0 ? -sPlayerbotAIConfig->tooCloseDistance / 2.0f : 0.0f;
                return MoveLine(tanks, diff, x + cos(orientation) * range, y + sin(orientation) * range, z, orientation, range);
            }

            if (!botAI->IsTank(bot) && botAI->IsTank(master))
            {
                float diff = dps.size() % 2 == 0 ? -sPlayerbotAIConfig->tooCloseDistance / 2.0f : 0.0f;
                return MoveLine(dps, diff, x - cos(orientation) * range, y - sin(orientation) * range, z, orientation, range);
            }

            return Formation::NullLocation;
        }
};

class FarFormation : public FollowFormation
{
    public:
        FarFormation(PlayerbotAI* botAI) : FollowFormation(botAI, "far") { }

        WorldLocation GetLocation() override
        {
            float range = sPlayerbotAIConfig->farDistance;
            float followRange = sPlayerbotAIConfig->followDistance;

            Player* master = GetMaster();
            if (!master)
                return Formation::NullLocation;

            if (sServerFacade->GetDistance2d(bot, master) <= range)
                return Formation::NullLocation;

            float angle = master->GetAngle(bot);
            float followAngle = GetFollowAngle();

            float x = master->GetPositionX() + cos(angle) * range + cos(followAngle) * followRange;
            float y = master->GetPositionY() + sin(angle) * range + sin(followAngle) * followRange;
            float z = master->GetPositionZ();

            float ground = master->GetMap()->GetHeight(x, y, z + 30.0f);
            if (ground <= INVALID_HEIGHT)
            {
                float minDist = 0, minX = 0, minY = 0;
                for (float angle = 0.0f; angle <= 2 * M_PI; angle += M_PI / 16.0f)
                {
                    x = master->GetPositionX() + cos(angle) * range + cos(followAngle) * followRange;
                    y = master->GetPositionY() + sin(angle) * range + sin(followAngle) * followRange;
                    float dist = sServerFacade->GetDistance2d(bot, x, y);
                    float ground = master->GetMap()->GetHeight(x, y, z + 30.0f);
                    if (ground > INVALID_HEIGHT && (!minDist || minDist > dist))
                    {
                        minDist = dist;
                        minX = x;
                        minY = y;
                    }
                }

                if (minDist)
                {
                    z += CONTACT_DISTANCE;
                    bot->UpdateAllowedPositionZ(minX, minY, z);
                    return WorldLocation(bot->GetMapId(), minX, minY, z);
                }

                return Formation::NullLocation;
            }

            z += CONTACT_DISTANCE;
            bot->UpdateAllowedPositionZ(x, y, z);
            return WorldLocation(bot->GetMapId(), x, y, z);
        }
};

float Formation::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = bot->GetGroup();
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    uint32 index = 1;
    uint32 total = 1;

    PlayerbotMgr* masterBotMgr = nullptr;
    if (master)
        masterBotMgr = GET_PLAYERBOT_MGR(master);
    if (!group && master && !GET_PLAYERBOT_AI(master) && masterBotMgr)
    {
        for (PlayerBotMap::const_iterator i = masterBotMgr->GetPlayerBotsBegin(); i != masterBotMgr->GetPlayerBotsEnd(); ++i)
        {
            if (i->second == bot)
                index = total;

            ++total;
        }
    }
    else if (group)
    {
        std::vector<Player*> roster;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = ref->GetSource())
            {
                if (member != master && !botAI->IsTank(member) && !botAI->IsHeal(member))
                {
                    roster.insert(roster.begin() + roster.size() / 2, member);
                }
            }
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = ref->GetSource())
            {
                if (member != master && botAI->IsHeal(member))
                {
                    roster.insert(roster.begin() + roster.size() / 2, member);
                }
            }
        }

        bool left = true;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = ref->GetSource())
            {
                if (member != master && botAI->IsTank(member))
                {
                    if (left)
                        roster.push_back(member);
                    else
                        roster.insert(roster.begin(), member);

                    left = !left;
                }
            }
        }

        for (Player* playerRoster : roster)
        {
            if (playerRoster == bot)
                break;

            ++index;
        }

        total = roster.size() + 1;
    }

    float start = (master ? master->GetOrientation() : 0.0f);
    return start + (0.125f + 1.75f * index / total + (total == 2 ? 0.125f : 0.0f)) * M_PI;
}

FormationValue::FormationValue(PlayerbotAI* botAI) : ManualSetValue<Formation*>(botAI, new NearFormation(botAI), "formation")
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

std::string const FormationValue::Save()
{
    return value ? value->getName() : "?";
}

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
    else if (formation == "chaos")
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
    else if (formation == "near" || formation == "default")
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
        botAI->TellMaster("Please set to any of:|cffffffff near (default), queue, chaos, circle, line, shield, arrow, melee, far");
        return false;
    }

    std::ostringstream str;
    str << "Formation set to: " << formation;
    botAI->TellMaster(str);
    return true;
}

WorldLocation MoveFormation::MoveLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation, float range)
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

        WorldLocation loc = MoveSingleLine(singleLine, diff, x, y,cz, orientation, range);
        if (!Formation::IsNullLocation(loc))
            return loc;
    }

    return Formation::NullLocation;
}

WorldLocation MoveFormation::MoveSingleLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation, float range)
{
    float count = line.size();
    float angle = orientation - M_PI / 2.0f;
    float x = cx + cos(angle) * (range * floor(count / 2.0f) + diff);
    float y = cy + sin(angle) * (range * floor(count / 2.0f) + diff);

    uint32 index = 0;
    for (Player* member : line)
    {
        if (member == bot)
        {
            float angle = orientation + M_PI / 2.0f;
            float radius = range * index;

            float lx = x + cos(angle) * radius;
            float ly = y + sin(angle) * radius;
            float lz = cz;
            float ground = bot->GetMap()->GetHeight(lx, ly, lz + 30.0f);
            if (ground <= INVALID_HEIGHT)
                return Formation::NullLocation;

            lz += CONTACT_DISTANCE;
            bot->UpdateAllowedPositionZ(lx, ly, lz);
            return WorldLocation(bot->GetMapId(), lx, ly, lz);
        }

        ++index;
    }

    return Formation::NullLocation;
}
