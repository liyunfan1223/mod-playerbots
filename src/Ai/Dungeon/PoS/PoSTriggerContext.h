/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSTRIGGERCONTEXT_H
#define PLAYERBOTS_POSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "PoSTriggers.h"

class WotlkDungeonPoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonPoSTriggerContext()
    {
        creators["ick and krick"] = &WotlkDungeonPoSTriggerContext::ick_and_krick;
        creators["tyrannus"] = &WotlkDungeonPoSTriggerContext::tyrannus;
    }

private:
    static Trigger* ick_and_krick(PlayerbotAI* botAI) { return new IckAndKrickTrigger(botAI); }
    static Trigger* tyrannus(PlayerbotAI* botAI) { return new TyrannusTrigger(botAI); }
};

#endif
