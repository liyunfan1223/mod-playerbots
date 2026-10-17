/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXTRIGGERCONTEXT_H
#define PLAYERBOTS_NEXTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "NexTriggers.h"

class WotlkDungeonNexTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonNexTriggerContext()
        {
            creators["faction commander whirlwind"] = &WotlkDungeonNexTriggerContext::faction_commander_whirlwind;
            creators["telestra firebomb"] = &WotlkDungeonNexTriggerContext::telestra_firebomb;
            creators["telestra split phase"] = &WotlkDungeonNexTriggerContext::telestra_split_phase;
            creators["chaotic rift"] = &WotlkDungeonNexTriggerContext::chaotic_rift;
            creators["ormorok spikes"] = &WotlkDungeonNexTriggerContext::ormorok_spikes;
            creators["ormorok stack"] = &WotlkDungeonNexTriggerContext::ormorok_stack;
            creators["intense cold"] = &WotlkDungeonNexTriggerContext::intense_cold;
            creators["keristrasza positioning"] = &WotlkDungeonNexTriggerContext::keristrasza_positioning;
        }
    private:
        static Trigger* faction_commander_whirlwind(PlayerbotAI* botAI) { return new FactionCommanderWhirlwindTrigger(botAI); }
        static Trigger* telestra_firebomb(PlayerbotAI* botAI) { return new TelestraFirebombTrigger(botAI); }
        static Trigger* telestra_split_phase(PlayerbotAI* botAI) { return new TelestraSplitPhaseTrigger(botAI); }
        static Trigger* chaotic_rift(PlayerbotAI* botAI) { return new ChaoticRiftTrigger(botAI); }
        static Trigger* ormorok_spikes(PlayerbotAI* botAI) { return new OrmorokSpikesTrigger(botAI); }
        static Trigger* ormorok_stack(PlayerbotAI* botAI) { return new OrmorokStackTrigger(botAI); }
        static Trigger* intense_cold(PlayerbotAI* botAI) { return new IntenseColdTrigger(botAI); }
        static Trigger* keristrasza_positioning(PlayerbotAI* botAI) { return new KeristraszaPositioningTrigger(botAI); }
};

#endif
