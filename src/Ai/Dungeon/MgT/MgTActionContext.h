/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTACTIONCONTEXT_H
#define PLAYERBOTS_MGTACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MgTActions.h"

class TbcDungeonMagistersTerraceActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonMagistersTerraceActionContext()
    {
        creators["mgt kill crystal"] = &TbcDungeonMagistersTerraceActionContext::mgt_kill_crystal;
        creators["mgt return to room"] = &TbcDungeonMagistersTerraceActionContext::mgt_return_to_room;
        creators["mgt leave dampening field"] = &TbcDungeonMagistersTerraceActionContext::mgt_leave_dampening_field;
        creators["mgt close on mage guard"] = &TbcDungeonMagistersTerraceActionContext::mgt_close_on_mage_guard;
        creators["mgt clear arcane nova"] = &TbcDungeonMagistersTerraceActionContext::mgt_clear_arcane_nova;
        creators["mgt priority interrupt"] = &TbcDungeonMagistersTerraceActionContext::mgt_priority_interrupt;
        creators["mgt focus target"] = &TbcDungeonMagistersTerraceActionContext::mgt_focus_target;
        creators["mgt taunt enraged wretched"] = &TbcDungeonMagistersTerraceActionContext::mgt_taunt_enraged_wretched;
        creators["mgt delrissa interrupt"] = &TbcDungeonMagistersTerraceActionContext::mgt_delrissa_interrupt;
        creators["mgt delrissa focus target"] = &TbcDungeonMagistersTerraceActionContext::mgt_delrissa_focus_target;
        creators["mgt delrissa tremor totem"] = &TbcDungeonMagistersTerraceActionContext::mgt_delrissa_tremor_totem;
        creators["mgt leave flame strike"] = &TbcDungeonMagistersTerraceActionContext::mgt_leave_flame_strike;
        creators["mgt leave phoenix burn"] = &TbcDungeonMagistersTerraceActionContext::mgt_leave_phoenix_burn;
        creators["mgt kael focus target"] = &TbcDungeonMagistersTerraceActionContext::mgt_kael_focus_target;
        creators["mgt kael interrupt"] = &TbcDungeonMagistersTerraceActionContext::mgt_kael_interrupt;
        creators["mgt take lapse spot"] = &TbcDungeonMagistersTerraceActionContext::mgt_take_lapse_spot;
    }

private:
    static Action* mgt_kill_crystal(PlayerbotAI* botAI) { return new MgTKillCrystalAction(botAI); }
    static Action* mgt_return_to_room(PlayerbotAI* botAI) { return new MgTReturnToRoomAction(botAI); }
    static Action* mgt_leave_dampening_field(PlayerbotAI* botAI) { return new MgTLeaveDampeningFieldAction(botAI); }
    static Action* mgt_close_on_mage_guard(PlayerbotAI* botAI) { return new MgTCloseOnMageGuardAction(botAI); }
    static Action* mgt_clear_arcane_nova(PlayerbotAI* botAI) { return new MgTClearArcaneNovaAction(botAI); }
    static Action* mgt_priority_interrupt(PlayerbotAI* botAI) { return new MgTPriorityInterruptAction(botAI); }
    static Action* mgt_focus_target(PlayerbotAI* botAI) { return new MgTFocusTargetAction(botAI); }
    static Action* mgt_taunt_enraged_wretched(PlayerbotAI* botAI)
    {
        return new MgTTauntEnragedWretchedAction(botAI);
    }
    static Action* mgt_delrissa_interrupt(PlayerbotAI* botAI) { return new MgTDelrissaInterruptAction(botAI); }
    static Action* mgt_delrissa_focus_target(PlayerbotAI* botAI)
    {
        return new MgTDelrissaFocusTargetAction(botAI);
    }
    static Action* mgt_delrissa_tremor_totem(PlayerbotAI* botAI)
    {
        return new MgTDelrissaTremorTotemAction(botAI);
    }
    static Action* mgt_leave_flame_strike(PlayerbotAI* botAI) { return new MgTLeaveFlameStrikeAction(botAI); }
    static Action* mgt_leave_phoenix_burn(PlayerbotAI* botAI) { return new MgTLeavePhoenixBurnAction(botAI); }
    static Action* mgt_kael_focus_target(PlayerbotAI* botAI) { return new MgTKaelFocusTargetAction(botAI); }
    static Action* mgt_kael_interrupt(PlayerbotAI* botAI) { return new MgTKaelInterruptAction(botAI); }
    static Action* mgt_take_lapse_spot(PlayerbotAI* botAI) { return new MgTTakeLapseSpotAction(botAI); }
};

#endif
