#ifndef _PLAYERBOT_WOTLKDUNGEONDTKTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONDTKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "DrakTharonKeepTriggers.h"

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
        static Trigger* corpse_explode(PlayerbotAI* ai) { return new CorpseExplodeTrigger(ai); }
        static Trigger* arcane_field(PlayerbotAI* ai) { return new ArcaneFieldTrigger(ai); }
        // static Trigger* crystal_handler(PlayerbotAI* ai) { return new CrystalHandlerTrigger(ai); }
        static Trigger* gift_of_tharonja(PlayerbotAI* ai) { return new GiftOfTharonjaTrigger(ai); }
        static Trigger* tharonja_out_of_melee(PlayerbotAI* ai) { return new TwoTriggers(ai, "gift of tharon'ja", "enemy out of melee"); }
};

#endif
