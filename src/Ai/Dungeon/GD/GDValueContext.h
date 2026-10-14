/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDVALUECONTEXT_H
#define PLAYERBOTS_GDVALUECONTEXT_H

#include "GDTriggers.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"

class SladranAssignedSnakeWrapValue : public CalculatedValue<ObjectGuid>
{
public:
    SladranAssignedSnakeWrapValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(botAI, "slad'ran snake wrap target", GundrakSladran::SNAKE_WRAP_SCAN_INTERVAL)
    {
    }

protected:
    ObjectGuid Calculate() override { return GundrakSladran::CalculateAssignedSnakeWrap(botAI); }
};

class WotlkDungeonGDValueContext : public NamedObjectContext<UntypedValue>
{
public:
    WotlkDungeonGDValueContext()
    {
        creators["slad'ran snake wrap target"] = &WotlkDungeonGDValueContext::sladran_snake_wrap_target;
    }

private:
    static UntypedValue* sladran_snake_wrap_target(PlayerbotAI* botAI)
    {
        return new SladranAssignedSnakeWrapValue(botAI);
    }
};

#endif
