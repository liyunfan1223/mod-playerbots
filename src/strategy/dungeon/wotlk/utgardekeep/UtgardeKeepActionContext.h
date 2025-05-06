#ifndef _PLAYERBOT_WOTLKDUNGEONUKACTIONCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONUKACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "UtgardeKeepActions.h"

class WotlkDungeonUKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonUKActionContext() {
            creators["attack frost tomb"] = &WotlkDungeonUKActionContext::attack_frost_tomb;
            creators["attack dalronn"] = &WotlkDungeonUKActionContext::attack_dalronn;
            creators["ingvar stop casting"] = &WotlkDungeonUKActionContext::ingvar_stop_casting;
            creators["ingvar get behind"] = &WotlkDungeonUKActionContext::ingvar_get_behind;
            // creators["ingvar hide los"] = &WotlkDungeonUKActionContext::ingvar_hide_los;
            creators["ingvar dodge smash"] = &WotlkDungeonUKActionContext::ingvar_dodge_smash;
            creators["ingvar smash return"] = &WotlkDungeonUKActionContext::ingvar_smash_return;
        }
    private:
        static Action* attack_frost_tomb(PlayerbotAI* ai) { return new AttackFrostTombAction(ai); }
        static Action* attack_dalronn(PlayerbotAI* ai) { return new AttackDalronnAction(ai); }
        static Action* ingvar_stop_casting(PlayerbotAI* ai) { return new IngvarStopCastingAction(ai); }
        static Action* ingvar_get_behind(PlayerbotAI* ai) { return new SetBehindTargetAction(ai); }
        // static Action* ingvar_hide_los(PlayerbotAI* ai) { return new TellLosAction(ai); }
        static Action* ingvar_dodge_smash(PlayerbotAI* ai) { return new IngvarDodgeSmashAction(ai); }
        static Action* ingvar_smash_return(PlayerbotAI* ai) { return new IngvarSmashReturnAction(ai); }
};

#endif
