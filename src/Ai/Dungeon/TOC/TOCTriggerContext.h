/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCTRIGGERCONTEXT_H
#define PLAYERBOTS_TOCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "TOCTriggers.h"

class WotlkDungeonToCTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonToCTriggerContext()
        {
            creators["toc lance"] = &WotlkDungeonToCTriggerContext::toc_lance;
            creators["toc ue lance"] = &WotlkDungeonToCTriggerContext::toc_ue_lance;
            creators["toc mount near"] = &WotlkDungeonToCTriggerContext::toc_mount_near;
            creators["toc mounted"] = &WotlkDungeonToCTriggerContext::toc_mounted;
            creators["toc eadric"] = &WotlkDungeonToCTriggerContext::toc_eadric;
        }
    private:
        static Trigger* toc_lance(PlayerbotAI* botAI) { return new ToCLanceTrigger(botAI); }
        static Trigger* toc_ue_lance(PlayerbotAI* botAI) { return new ToCUELanceTrigger(botAI); }
        static Trigger* toc_mount_near(PlayerbotAI* botAI) { return new ToCMountNearTrigger(botAI); }
        static Trigger* toc_mounted(PlayerbotAI* botAI) { return new ToCMountedTrigger(botAI); }
        static Trigger* toc_eadric(PlayerbotAI* botAI) { return new ToCEadricTrigger(botAI); }
};

#endif
