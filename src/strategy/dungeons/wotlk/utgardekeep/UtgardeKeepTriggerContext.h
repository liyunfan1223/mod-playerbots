#ifndef _PLAYERBOT_WOTLKDUNGEONUKTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONUKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "UtgardeKeepTriggers.h"

class WotlkDungeonUKTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        WotlkDungeonUKTriggerContext()
        {
            creators["keleseth frost tomb"] = &WotlkDungeonUKTriggerContext::keleseth_frost_tomb;
            creators["dalronn priority"] = &WotlkDungeonUKTriggerContext::dalronn_priority_target;
            creators["ingvar staggering roar"] = &WotlkDungeonUKTriggerContext::ingvar_staggering_roar;
            creators["ingvar dreadful roar"] = &WotlkDungeonUKTriggerContext::ingvar_dreadful_roar;
            creators["ingvar smash tank"] = &WotlkDungeonUKTriggerContext::ingvar_smash_tank;
            creators["ingvar smash tank return"] = &WotlkDungeonUKTriggerContext::ingvar_smash_tank_return;
            creators["not behind ingvar"] = &WotlkDungeonUKTriggerContext::not_behind_ingvar;
        }
    private:
        static Trigger* keleseth_frost_tomb(PlayerbotAI* ai) { return new KelesethFrostTombTrigger(ai); }
        static Trigger* dalronn_priority_target(PlayerbotAI* ai) { return new DalronnNontankTrigger(ai); }
        static Trigger* ingvar_staggering_roar(PlayerbotAI* ai) { return new IngvarStaggeringRoarTrigger(ai); }
        static Trigger* ingvar_dreadful_roar(PlayerbotAI* ai) { return new IngvarDreadfulRoarTrigger(ai); }
        static Trigger* ingvar_smash_tank(PlayerbotAI* ai) { return new IngvarSmashTankTrigger(ai); }
        static Trigger* ingvar_smash_tank_return(PlayerbotAI* ai) { return new IngvarSmashTankReturnTrigger(ai); }
        static Trigger* not_behind_ingvar(PlayerbotAI* ai) { return new NotBehindIngvarTrigger(ai); }
};

#endif
