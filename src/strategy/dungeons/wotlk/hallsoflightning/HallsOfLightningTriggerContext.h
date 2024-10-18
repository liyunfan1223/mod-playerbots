#ifndef _PLAYERBOT_WOTLKDUNGEONHOLTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONHOLTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "HallsOfLightningTriggers.h"

class WotlkDungeonHoLTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonHoLTriggerContext()
        {
            creators["stormforged lieutenant"] = &WotlkDungeonHoLTriggerContext::stormforged_lieutenant;
            creators["whirlwind"] = &WotlkDungeonHoLTriggerContext::bjarngrim_whirlwind;
            creators["volkhan"] = &WotlkDungeonHoLTriggerContext::volkhan;
            creators["static overload"] = &WotlkDungeonHoLTriggerContext::static_overload;
            creators["ball lightning"] = &WotlkDungeonHoLTriggerContext::ball_lightning;
            creators["ionar tank aggro"] = &WotlkDungeonHoLTriggerContext::ionar_tank_aggro;
            creators["ionar disperse"] = &WotlkDungeonHoLTriggerContext::ionar_disperse;
            creators["loken ranged"] = &WotlkDungeonHoLTriggerContext::loken_ranged;
            creators["lightning nova"] = &WotlkDungeonHoLTriggerContext::lightning_nova;
        }
    private:
        static Trigger* stormforged_lieutenant(PlayerbotAI* ai) { return new StormforgedLieutenantTrigger(ai); }
        static Trigger* bjarngrim_whirlwind(PlayerbotAI* ai) { return new BjarngrimWhirlwindTrigger(ai); }
        static Trigger* volkhan(PlayerbotAI* ai) { return new VolkhanTrigger(ai); }
        static Trigger* static_overload(PlayerbotAI* ai) { return new IonarStaticOverloadTrigger(ai); }
        static Trigger* ball_lightning(PlayerbotAI* ai) { return new IonarBallLightningTrigger(ai); }
        static Trigger* ionar_tank_aggro(PlayerbotAI* ai) { return new IonarTankAggroTrigger(ai); }
        static Trigger* ionar_disperse(PlayerbotAI* ai) { return new IonarDisperseTrigger(ai); }
        static Trigger* loken_ranged(PlayerbotAI* ai) { return new LokenRangedTrigger(ai); }
        static Trigger* lightning_nova(PlayerbotAI* ai) { return new LokenLightningNovaTrigger(ai); }
};

#endif
