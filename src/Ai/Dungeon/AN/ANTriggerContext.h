/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ANTRIGGERCONTEXT_H
#define PLAYERBOTS_ANTRIGGERCONTEXT_H

#include "ANTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonANTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonANTriggerContext()
        {
            creators["krik'thir web wrap"] = &WotlkDungeonANTriggerContext::krikthir_web_wrap;
            creators["krik'thir watchers"] = &WotlkDungeonANTriggerContext::krikthir_watchers;
            // creators["anub'arak impale"] = &WotlkDungeonANTriggerContext::anubarak_impale;
            creators["anub'arak pound"] = &WotlkDungeonANTriggerContext::anubarak_pound;
        }
    private:
        static Trigger* krikthir_web_wrap(PlayerbotAI* botAI) { return new KrikthirWebWrapTrigger(botAI); }
        static Trigger* krikthir_watchers(PlayerbotAI* botAI) { return new KrikthirWatchersTrigger(botAI); }
        // static Trigger* anubarak_impale(PlayerbotAI* botAI) { return new AnubarakImpaleTrigger(botAI); }
        static Trigger* anubarak_pound(PlayerbotAI* botAI) { return new AnubarakPoundTrigger(botAI); }
};

#endif
