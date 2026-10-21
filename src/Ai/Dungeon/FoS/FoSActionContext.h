/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSACTIONCONTEXT_H
#define PLAYERBOTS_FOSACTIONCONTEXT_H

#include "Action.h"
#include "FoSActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonFoSActionContext : public NamedObjectContext<Action>
{
    public:
        WotlkDungeonFoSActionContext()
        {
            creators["move from bronjahm"] = &WotlkDungeonFoSActionContext::move_from_bronjahm;
            creators["attack corrupted soul fragment"] = &WotlkDungeonFoSActionContext::attack_corrupted_soul_fragment;
            creators["bronjahm group position"] = &WotlkDungeonFoSActionContext::bronjahm_group_position;
            creators["devourer wailing souls"] = &WotlkDungeonFoSActionContext::devourer_wailing_souls;
            creators["devourer well of souls"] = &WotlkDungeonFoSActionContext::devourer_well_of_souls;
            creators["devourer stop damage"] = &WotlkDungeonFoSActionContext::devourer_stop_damage;
            creators["devourer restore pets"] = &WotlkDungeonFoSActionContext::devourer_restore_pets;
            creators["devourer interrupt phantom blast"] = &WotlkDungeonFoSActionContext::devourer_interrupt_phantom_blast;
            creators["devourer stop cast"] = &WotlkDungeonFoSActionContext::devourer_stop_cast;
        }
    private:
        static Action* move_from_bronjahm(PlayerbotAI* botAI) { return new MoveFromBronjahmAction(botAI); }
        static Action* attack_corrupted_soul_fragment(PlayerbotAI* botAI) { return new AttackCorruptedSoulFragmentAction(botAI); }
        static Action* bronjahm_group_position(PlayerbotAI* botAI) { return new BronjahmGroupPositionAction(botAI); }
        static Action* devourer_wailing_souls(PlayerbotAI* botAI) { return new DevourerWailingSoulsAction(botAI); }
        static Action* devourer_well_of_souls(PlayerbotAI* botAI) { return new DevourerWellOfSoulsAction(botAI); }
        static Action* devourer_stop_damage(PlayerbotAI* botAI) { return new DevourerStopDamageAction(botAI); }
        static Action* devourer_restore_pets(PlayerbotAI* botAI) { return new DevourerRestorePetsAction(botAI); }
        static Action* devourer_interrupt_phantom_blast(PlayerbotAI* botAI) { return new DevourerInterruptPhantomBlastAction(botAI); }
        static Action* devourer_stop_cast(PlayerbotAI* botAI) { return new DevourerStopCastAction(botAI); }
};

#endif
