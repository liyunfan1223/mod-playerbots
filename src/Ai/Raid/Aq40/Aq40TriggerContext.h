/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40TRIGGERCONTEXT_H
#define PLAYERBOTS_AQ40TRIGGERCONTEXT_H

#include "Aq40Actions.h"
#include "NamedObjectContext.h"

class RaidAq40TriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidAq40TriggerContext()
    {
        creators["aq40 encounter"] = &Encounter;
        creators["aq40 twins prepull"] = &TwinPrepull;
    }

private:
    static Trigger* Encounter(PlayerbotAI* botAI) { return new Aq40Trigger(botAI); }
    static Trigger* TwinPrepull(PlayerbotAI* botAI) { return new Aq40TwinPrepullTrigger(botAI); }
};

#endif
