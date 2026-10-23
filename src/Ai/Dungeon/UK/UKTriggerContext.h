/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKTRIGGERCONTEXT_H
#define PLAYERBOTS_UKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "UKTriggers.h"

class WotlkDungeonUKTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonUKTriggerContext()
        {
            creators["keleseth frost tomb"] = &WotlkDungeonUKTriggerContext::keleseth_frost_tomb;
            creators["dalronn priority"] = &WotlkDungeonUKTriggerContext::dalronn_priority_target;
            creators["ingvar dreadful roar"] = &WotlkDungeonUKTriggerContext::ingvar_dreadful_roar;
            creators["ingvar smash tank"] = &WotlkDungeonUKTriggerContext::ingvar_smash_tank;
            creators["ingvar smash tank return"] = &WotlkDungeonUKTriggerContext::ingvar_smash_tank_return;
            creators["not behind ingvar"] = &WotlkDungeonUKTriggerContext::not_behind_ingvar;
        }
    private:
        static Trigger* keleseth_frost_tomb(PlayerbotAI* botAI) { return new KelesethFrostTombTrigger(botAI); }
        static Trigger* dalronn_priority_target(PlayerbotAI* botAI) { return new DalronnDpsTrigger(botAI); }
        static Trigger* ingvar_dreadful_roar(PlayerbotAI* botAI) { return new IngvarDreadfulRoarTrigger(botAI); }
        static Trigger* ingvar_smash_tank(PlayerbotAI* botAI) { return new IngvarSmashTankTrigger(botAI); }
        static Trigger* ingvar_smash_tank_return(PlayerbotAI* botAI) { return new IngvarSmashTankReturnTrigger(botAI); }
        static Trigger* not_behind_ingvar(PlayerbotAI* botAI) { return new NotBehindIngvarTrigger(botAI); }
};

#endif
