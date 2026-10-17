/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSTRIGGERCONTEXT_H
#define PLAYERBOTS_HOSTRIGGERCONTEXT_H

#include "HoSTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoSTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonHoSTriggerContext()
        {
            creators["ground slam"] = &WotlkDungeonHoSTriggerContext::ground_slam;
            creators["lightning ring"] = &WotlkDungeonHoSTriggerContext::lightning_ring;
        }
    private:
        static Trigger* ground_slam(PlayerbotAI* botAI) { return new KrystallusGroundSlamTrigger(botAI); }
        static Trigger* lightning_ring(PlayerbotAI* botAI) { return new SjonnirLightningRingTrigger(botAI); }
};

#endif
