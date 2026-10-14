/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBVALUECONTEXT_H
#define PLAYERBOTS_UBVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Timer.h"
#include "UBShared.h"
#include "Unit.h"
#include "Value.h"

class UnderbogMushroomsValue : public CalculatedValue<GuidVector>
{
public:
    UnderbogMushroomsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "ub mushrooms", UnderbogHungarfen::MUSHROOM_SCAN_INTERVAL)
    {
    }

protected:
    GuidVector Calculate() override
    {
        if (!_scanning && !bot->IsInCombat())
            return {};

        Unit* boss = UnderbogHungarfen::HungarfenTarget(context);

        if (!_scanning)
        {
            if (!boss)
                return {};

            _scanning = true;
            _skipped = 0;
            _sawMushrooms = false;
        }

        if (boss)
        {
            _hungarfen = boss->GetGUID();
            _deadSince = 0;
        }

        if (!boss && !_sawMushrooms && _skipped++ < UnderbogHungarfen::MUSHROOM_IDLE_SCAN_SKIPS)
            return {};

        _skipped = 0;
        GuidVector mushrooms = UnderbogHungarfen::FindMushroomGuids(bot);
        _sawMushrooms = !mushrooms.empty();

        if (!boss)
            CheckHungarfenGone();

        return mushrooms;
    }

private:
    void CheckHungarfenGone()
    {
        if (_hungarfen.IsEmpty())
            _hungarfen = UnderbogHungarfen::FindHungarfenGuid(bot);

        if (_hungarfen.IsEmpty() || !UnderbogHungarfen::HungarfenGone(bot, _hungarfen))
        {
            _deadSince = 0;
            return;
        }

        if (!_deadSince)
            _deadSince = getMSTime();
        else if (GetMSTimeDiffToNow(_deadSince) >= UnderbogHungarfen::MUSHROOM_LINGER_TIME)
            _scanning = false;
    }

    ObjectGuid _hungarfen;
    uint32 _deadSince = 0;
    uint32 _skipped = 0;
    bool _sawMushrooms = false;
    bool _scanning = true;
};

class TbcDungeonUnderbogValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonUnderbogValueContext() { creators["ub mushrooms"] = &TbcDungeonUnderbogValueContext::ub_mushrooms; }

private:
    static UntypedValue* ub_mushrooms(PlayerbotAI* botAI) { return new UnderbogMushroomsValue(botAI); }
};

#endif
