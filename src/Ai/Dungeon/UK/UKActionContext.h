/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKACTIONCONTEXT_H
#define PLAYERBOTS_UKACTIONCONTEXT_H

#include "Action.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "TellLosAction.h"
#include "UKActions.h"

class WotlkDungeonUKActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonUKActionContext() {
            creators["attack frost tomb"] = &WotlkDungeonUKActionContext::attack_frost_tomb;
            creators["attack dalronn"] = &WotlkDungeonUKActionContext::attack_dalronn;
            creators["ingvar get behind"] = &WotlkDungeonUKActionContext::ingvar_get_behind;
            // creators["ingvar hide los"] = &WotlkDungeonUKActionContext::ingvar_hide_los;
            creators["ingvar dodge smash"] = &WotlkDungeonUKActionContext::ingvar_dodge_smash;
            creators["ingvar smash return"] = &WotlkDungeonUKActionContext::ingvar_smash_return;
        }
    private:
        static Action* attack_frost_tomb(PlayerbotAI* botAI) { return new AttackFrostTombAction(botAI); }
        static Action* attack_dalronn(PlayerbotAI* botAI) { return new AttackDalronnAction(botAI); }
        static Action* ingvar_get_behind(PlayerbotAI* botAI) { return new SetBehindTargetAction(botAI); }
        // static Action* ingvar_hide_los(PlayerbotAI* botAI) { return new TellLosAction(botAI); }
        static Action* ingvar_dodge_smash(PlayerbotAI* botAI) { return new IngvarDodgeSmashAction(botAI); }
        static Action* ingvar_smash_return(PlayerbotAI* botAI) { return new IngvarSmashReturnAction(botAI); }
};

#endif
