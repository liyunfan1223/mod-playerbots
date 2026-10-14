/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MECHVALUECONTEXT_H
#define PLAYERBOTS_MECHVALUECONTEXT_H

#include "MechShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"

class SepethreaFixatingFlameValue : public CalculatedValue<ObjectGuid>
{
public:
    SepethreaFixatingFlameValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(botAI, "sepethrea fixating flame", 200) {}

protected:
    ObjectGuid Calculate() override { return MechanarFlames::FindFixatingFlameGuid(bot); }
};

class SepethreaFlameNearValue : public BoolCalculatedValue
{
public:
    SepethreaFlameNearValue(PlayerbotAI* botAI)
        : BoolCalculatedValue(botAI, "sepethrea flame near", 200) {}

protected:
    bool Calculate() override
    {
        return MechanarFlames::GetNearestFlame(bot, MechanarFlames::INFERNO_AVOID_RANGE) != nullptr;
    }
};

class SepethreaTrailDangerValue : public BoolCalculatedValue
{
public:
    SepethreaTrailDangerValue(PlayerbotAI* botAI)
        : BoolCalculatedValue(botAI, "sepethrea trail danger", 200) {}

protected:
    bool Calculate() override { return MechanarFlames::InTrailDanger(bot); }
};

class TbcDungeonMechValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonMechValueContext()
    {
        creators["sepethrea fixating flame"] =
            &TbcDungeonMechValueContext::sepethrea_fixating_flame;
        creators["sepethrea flame near"] =
            &TbcDungeonMechValueContext::sepethrea_flame_near;
        creators["sepethrea trail danger"] =
            &TbcDungeonMechValueContext::sepethrea_trail_danger;
    }

private:
    static UntypedValue* sepethrea_fixating_flame(PlayerbotAI* botAI)
    {
        return new SepethreaFixatingFlameValue(botAI);
    }

    static UntypedValue* sepethrea_flame_near(PlayerbotAI* botAI)
    {
        return new SepethreaFlameNearValue(botAI);
    }

    static UntypedValue* sepethrea_trail_danger(PlayerbotAI* botAI)
    {
        return new SepethreaTrailDangerValue(botAI);
    }
};

#endif
