/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLTRIGGERCONTEXT_H
#define PLAYERBOTS_HOLTRIGGERCONTEXT_H

#include "HoLTriggers.h"
#include "NamedObjectContext.h"

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
        static Trigger* stormforged_lieutenant(PlayerbotAI* botAI) { return new StormforgedLieutenantTrigger(botAI); }
        static Trigger* bjarngrim_whirlwind(PlayerbotAI* botAI) { return new BjarngrimWhirlwindTrigger(botAI); }
        static Trigger* volkhan(PlayerbotAI* botAI) { return new VolkhanTrigger(botAI); }
        static Trigger* static_overload(PlayerbotAI* botAI) { return new IonarStaticOverloadTrigger(botAI); }
        static Trigger* ball_lightning(PlayerbotAI* botAI) { return new IonarBallLightningTrigger(botAI); }
        static Trigger* ionar_tank_aggro(PlayerbotAI* botAI) { return new IonarTankAggroTrigger(botAI); }
        static Trigger* ionar_disperse(PlayerbotAI* botAI) { return new IonarDisperseTrigger(botAI); }
        static Trigger* loken_ranged(PlayerbotAI* botAI) { return new LokenRangedTrigger(botAI); }
        static Trigger* lightning_nova(PlayerbotAI* botAI) { return new LokenLightningNovaTrigger(botAI); }
};

#endif
