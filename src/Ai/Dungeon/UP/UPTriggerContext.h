/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UPTRIGGERCONTEXT_H
#define PLAYERBOTS_UPTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "UPTriggers.h"

class WotlkDungeonUPTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonUPTriggerContext()
        {
            creators["freezing cloud"] = &WotlkDungeonUPTriggerContext::freezing_cloud;
            creators["skadi whirlwind"] = &WotlkDungeonUPTriggerContext::whirlwind;
            creators["ymiron bane"] = &WotlkDungeonUPTriggerContext::bane;
        }
    private:
        static Trigger* freezing_cloud(PlayerbotAI* botAI) { return new SkadiFreezingCloudTrigger(botAI); }
        static Trigger* whirlwind(PlayerbotAI* botAI) { return new SkadiWhirlwindTrigger(botAI); }
        static Trigger* bane(PlayerbotAI* botAI) { return new YmironBaneTrigger(botAI); }
};

#endif
