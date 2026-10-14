/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKVALUECONTEXT_H
#define PLAYERBOTS_TKVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "TKHelpers.h"
#include "Value.h"

class TKFlamePatchesValue : public CalculatedValue<GuidVector>
{
public:
    TKFlamePatchesValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "tk flame patches", 200) {}

protected:
    GuidVector Calculate() override { return TkHelpers::FindFlamePatchGuids(bot); }
};

class TKLegendaryWeaponsValue : public CalculatedValue<GuidVector>
{
public:
    TKLegendaryWeaponsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "tk legendary weapons", 200) {}

protected:
    GuidVector Calculate() override { return TkHelpers::FindLegendaryWeaponGuids(bot); }
};

class RaidTempestKeepValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidTempestKeepValueContext()
    {
        creators["tk flame patches"] = &RaidTempestKeepValueContext::tk_flame_patches;
        creators["tk legendary weapons"] = &RaidTempestKeepValueContext::tk_legendary_weapons;
    }

private:
    static UntypedValue* tk_flame_patches(PlayerbotAI* botAI) {
        return new TKFlamePatchesValue(botAI);
    }
    static UntypedValue* tk_legendary_weapons(PlayerbotAI* botAI) {
        return new TKLegendaryWeaponsValue(botAI);
    }
};

#endif
