/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GDTRIGGERCONTEXT_H
#define PLAYERBOTS_GDTRIGGERCONTEXT_H

#include "GDTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonGDTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonGDTriggerContext()
        {
            creators["poison nova"] = &WotlkDungeonGDTriggerContext::poison_nova;
            creators["snake wrap"] = &WotlkDungeonGDTriggerContext::snake_wrap;
            creators["slad'ran stack on tank"] = &WotlkDungeonGDTriggerContext::sladran_stack_on_tank;
            creators["slad'ran tank hold"] = &WotlkDungeonGDTriggerContext::sladran_tank_hold;
            creators["whirling slash"] = &WotlkDungeonGDTriggerContext::whirling_slash;
        }
    private:
        static Trigger* poison_nova(PlayerbotAI* botAI) { return new SladranPoisonNovaTrigger(botAI); }
        static Trigger* snake_wrap(PlayerbotAI* botAI) { return new SladranSnakeWrapTrigger(botAI); }
        static Trigger* sladran_stack_on_tank(PlayerbotAI* botAI) { return new SladranStackOnTankTrigger(botAI); }
        static Trigger* sladran_tank_hold(PlayerbotAI* botAI) { return new SladranTankHoldTrigger(botAI); }
        static Trigger* whirling_slash(PlayerbotAI* botAI) { return new GaldarahWhirlingSlashTrigger(botAI); }
};

#endif
