/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKTRIGGERCONTEXT_H
#define PLAYERBOTS_DTKTRIGGERCONTEXT_H

#include "DTKTriggers.h"
#include "GenericTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonDTKTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonDTKTriggerContext()
        {
            creators["corpse explode"] = &WotlkDungeonDTKTriggerContext::corpse_explode;
            creators["arcane field"] = &WotlkDungeonDTKTriggerContext::arcane_field;
            // creators["crystal handler"] = &WotlkDungeonDTKTriggerContext::crystal_handler;
            creators["gift of tharon'ja"] = &WotlkDungeonDTKTriggerContext::gift_of_tharonja;
            creators["tharon'ja out of melee"] = &WotlkDungeonDTKTriggerContext::tharonja_out_of_melee;

        }
    private:
        static Trigger* corpse_explode(PlayerbotAI* botAI) { return new CorpseExplodeTrigger(botAI); }
        static Trigger* arcane_field(PlayerbotAI* botAI) { return new ArcaneFieldTrigger(botAI); }
        // static Trigger* crystal_handler(PlayerbotAI* botAI) { return new CrystalHandlerTrigger(botAI); }
        static Trigger* gift_of_tharonja(PlayerbotAI* botAI) { return new GiftOfTharonjaTrigger(botAI); }
        static Trigger* tharonja_out_of_melee(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "gift of tharon'ja", "enemy out of melee"); }
};

#endif
