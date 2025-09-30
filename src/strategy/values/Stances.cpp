/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Stances.h"

#include "Arrow.h"
#include "Event.h"
#include "Playerbots.h"

Unit* Stance::GetTarget()
{
    Unit* target = AI_VALUE(Unit*, GetTargetName());
    if (target)
        return target;

    ObjectGuid pullTarget = context->GetValue<ObjectGuid>("pull target")->Get();
    if (pullTarget)
        botAI->GetUnit(pullTarget);

    return nullptr;
}

WorldLocation Stance::GetLocation()
{
    Unit* target = GetTarget();
    if (!target)
        return Formation::NullLocation;

    return GetLocationInternal();
}

WorldLocation Stance::GetNearLocation(float angle, float distance)
{
    Unit* target = GetTarget();

    float x = target->GetPositionX() + cos(angle) * distance;
    float y = target->GetPositionY() + sin(angle) * distance;
    float z = target->GetPositionZ();

    if (bot->IsWithinLOS(x, y, z))
        return WorldLocation(bot->GetMapId(), x, y, z);

    return Formation::NullLocation;
}

WorldLocation MoveStance::GetLocationInternal()
{
    Unit* target = GetTarget();
    float distance = std::max(sPlayerbotAIConfig->meleeDistance, target->GetCombatReach());

    float angle = GetAngle();
    return GetNearLocation(angle, distance);
}

std::string const Stance::GetTargetName() { return "current target"; }

float Stance::GetMaxDistance() { return sPlayerbotAIConfig->contactDistance; }

StanceValue::~StanceValue()
{
    if (value)
    {
        delete value;
        value = nullptr;
    }
}

class NearStance : public MoveStance
{
public:
    NearStance(PlayerbotAI* botAI) : MoveStance(botAI, "near") {}

    float GetAngle() override
    {
        Unit* target = GetTarget();

        if (target->GetVictim() && target->GetVictim()->GetGUID() == bot->GetGUID())
            return target->GetOrientation();

        if (botAI->HasStrategy("behind", BOT_STATE_COMBAT))
        {
            Unit* target = GetTarget();
            Group* group = bot->GetGroup();
            uint32 index = 0;
            uint32 count = 0;
            if (group)
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member == bot)
                        index = count;

                    if (member && !botAI->IsRanged(member) && !botAI->IsTank(member))
                        count++;
                }
            }

            float angle = target->GetOrientation() + M_PI;
            if (!count)
                return angle;

            float increment = M_PI / 4 / count;
            return round((angle + index * increment - M_PI / 4) * 10.0f) / 10.0f;
        }

        float angle = GetFollowAngle() + target->GetOrientation();
        if (Player* master = GetMaster())
            angle -= master->GetOrientation();

        return angle;
    }
};

class TankStance : public MoveStance
{
public:
    TankStance(PlayerbotAI* botAI) : MoveStance(botAI, "tank") {}

    float GetAngle() override
    {
        Unit* target = GetTarget();
        return target->GetOrientation();
    }
};

class TurnBackStance : public MoveStance
{
public:
    TurnBackStance(PlayerbotAI* botAI) : MoveStance(botAI, "turnback") {}

    float GetAngle() override
    {
        Unit* target = GetTarget();

        uint32 count = 0;
        float angle = 0.0f;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (Player* member = ref->GetSource())
                    if (member != bot && botAI->IsRanged(member))
                    {
                        angle += target->GetAngle(member);
                        ++count;
                    }
            }
        }

        if (!count)
            return target->GetOrientation();

        return std::round((angle / count) * 10.0f) / 10.0f + M_PI;
    }
};

class BehindStance : public MoveStance
{
public:
    BehindStance(PlayerbotAI* botAI) : MoveStance(botAI, "behind") {}

    float GetAngle() override
    {
        Unit* target = GetTarget();

        uint32 index = 0;
        uint32 count = 0;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (Player* member = ref->GetSource())
                {
                    if (member == bot)
                        index = count;

                    if (!botAI->IsRanged(member) && !botAI->IsTank(member))
                        ++count;
                }
            }
        }

        float angle = target->GetOrientation() + M_PI;
        if (!count)
            return angle;

        float increment = M_PI / 4 / count;
        return std::round((angle + index * increment - M_PI / 4) * 10.0f) / 10.0f;
    }
};

StanceValue::StanceValue(PlayerbotAI* botAI) : ManualSetValue<Stance*>(botAI, new NearStance(botAI), "stance") {}

std::string const StanceValue::Save() { return value ? value->getName() : "?"; }

bool StanceValue::Load(std::string const name)
{
    if (name == "behind")
    {
        if (value)
            delete value;
        value = new BehindStance(botAI);
    }
    else if (name == "near" || name == "default")
    {
        if (value)
            delete value;
        value = new NearStance(botAI);
    }
    else if (name == "tank")
    {
        if (value)
            delete value;
        value = new TankStance(botAI);
    }
    else if (name == "turnback" || name == "turn")
    {
        if (value)
            delete value;
        value = new TurnBackStance(botAI);
    }
    else
        return false;

    return true;
}

bool SetStanceAction::Execute(Event event)
{
    std::string const stance = event.getParam();

    StanceValue* value = (StanceValue*)context->GetValue<Stance*>("stance");
    if (stance == "?" || stance.empty())
    {
        std::ostringstream str;
        str << "Stance: |cff00ff00" << value->Get()->getName();
        botAI->TellMaster(str);
        return true;
    }

    if (stance == "show")
    {
        WorldLocation loc = value->Get()->GetLocation();
        if (!Formation::IsNullLocation(loc))
            botAI->Ping(loc.GetPositionX(), loc.GetPositionY());

        return true;
    }

    if (!value->Load(stance))
    {
        std::ostringstream str;
        str << "Invalid stance: |cffff0000" << stance;
        botAI->TellMaster(str);
        botAI->TellMaster("Please set to any of:|cffffffff near (default), tank, turnback, behind");
        return false;
    }

    std::ostringstream str;
    str << "Stance set to: " << stance;
    botAI->TellMaster(str);
    return true;
}
