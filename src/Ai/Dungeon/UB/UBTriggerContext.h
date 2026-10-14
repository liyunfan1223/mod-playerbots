/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBTRIGGERCONTEXT_H
#define PLAYERBOTS_UBTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "TriggerContext.h"
#include "UBTriggers.h"

class TbcDungeonUnderbogTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonUnderbogTriggerContext()
    {
        creators["ub foul spores"] = &TbcDungeonUnderbogTriggerContext::ub_foul_spores;
        creators["ub spore cloud danger"] = &TbcDungeonUnderbogTriggerContext::ub_spore_cloud_danger;
        creators["ub underbat lash"] = &TbcDungeonUnderbogTriggerContext::ub_underbat_lash;
    }

private:
    static Trigger* ub_foul_spores(PlayerbotAI* botAI) { return new UBFoulSporesTrigger(botAI); }

    static Trigger* ub_spore_cloud_danger(PlayerbotAI* botAI) { return new UBSporeCloudDangerTrigger(botAI); }

    static Trigger* ub_underbat_lash(PlayerbotAI* botAI) { return new UBUnderbatLashTrigger(botAI); }
};

#endif
