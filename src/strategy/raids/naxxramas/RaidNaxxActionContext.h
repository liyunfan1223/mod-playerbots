// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
//  and/or modify it under version 3 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDNAXXACTIONCONTEXT_H
#define _PLAYERBOT_RAIDNAXXACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidNaxxActions.h"

class RaidNaxxActionContext : public NamedObjectContext<Action>
{
public:
    RaidNaxxActionContext()
    {
        creators["grobbulus go behind the boss"] = &RaidNaxxActionContext::go_behind_the_boss;
        creators["rotate grobbulus"] = &RaidNaxxActionContext::rotate_grobbulus;
        creators["grobbulus move center"] = &RaidNaxxActionContext::grobbulus_move_center;

        creators["heigan dance melee"] = &RaidNaxxActionContext::heigan_dance_melee;
        creators["heigan dance ranged"] = &RaidNaxxActionContext::heigan_dance_ranged;
        creators["thaddius attack nearest pet"] = &RaidNaxxActionContext::thaddius_attack_nearest_pet;
        // creators["thaddius melee to place"] = &RaidNaxxActionContext::thaddius_tank_to_place;
        // creators["thaddius ranged to place"] = &RaidNaxxActionContext::thaddius_ranged_to_place;
        creators["thaddius move to platform"] = &RaidNaxxActionContext::thaddius_move_to_platform;
        creators["thaddius move polarity"] = &RaidNaxxActionContext::thaddius_move_polarity;

        creators["razuvious use obedience crystal"] = &RaidNaxxActionContext::razuvious_use_obedience_crystal;
        creators["razuvious target"] = &RaidNaxxActionContext::razuvious_target;

        creators["horseman attract alternatively"] = &RaidNaxxActionContext::horseman_attract_alternatively;
        creators["horseman attack in order"] = &RaidNaxxActionContext::horseman_attack_in_order;

        creators["sapphiron ground position"] = &RaidNaxxActionContext::sapphiron_ground_position;
        creators["sapphiron flight position"] = &RaidNaxxActionContext::sapphiron_flight_position;

        creators["kel'thuzad choose target"] = &RaidNaxxActionContext::kelthuzad_choose_target;
        creators["kel'thuzad position"] = &RaidNaxxActionContext::kelthuzad_position;

        creators["anub'rekhan choose target"] = &RaidNaxxActionContext::anubrekhan_choose_target;
        creators["anub'rekhan position"] = &RaidNaxxActionContext::anubrekhan_position;

        creators["gluth choose target"] = &RaidNaxxActionContext::gluth_choose_target;
        creators["gluth position"] = &RaidNaxxActionContext::gluth_position;
        creators["gluth slowdown"] = &RaidNaxxActionContext::gluth_slowdown;

        creators["loatheb position"] = &RaidNaxxActionContext::loatheb_position;
        creators["loatheb choose target"] = &RaidNaxxActionContext::loatheb_choose_target;
    }

private:
    static Action* go_behind_the_boss(PlayerbotAI* ai) { return new GrobbulusGoBehindAction(ai); }
    static Action* rotate_grobbulus(PlayerbotAI* ai) { return new GrobbulusRotateAction(ai); }
    static Action* grobbulus_move_center(PlayerbotAI* ai) { return new GrobblulusMoveCenterAction(ai); }
    static Action* heigan_dance_melee(PlayerbotAI* ai) { return new HeiganDanceMeleeAction(ai); }
    static Action* heigan_dance_ranged(PlayerbotAI* ai) { return new HeiganDanceRangedAction(ai); }
    static Action* thaddius_attack_nearest_pet(PlayerbotAI* ai) { return new ThaddiusAttackNearestPetAction(ai); }
    // static Action* thaddius_tank_to_place(PlayerbotAI* ai) { return new ThaddiusMeleeToPlaceAction(ai); }
    // static Action* thaddius_ranged_to_place(PlayerbotAI* ai) { return new ThaddiusRangedToPlaceAction(ai); }
    static Action* thaddius_move_to_platform(PlayerbotAI* ai) { return new ThaddiusMoveToPlatformAction(ai); }
    static Action* thaddius_move_polarity(PlayerbotAI* ai) { return new ThaddiusMovePolarityAction(ai); }
    static Action* razuvious_target(PlayerbotAI* ai) { return new RazuviousTargetAction(ai); }
    static Action* razuvious_use_obedience_crystal(PlayerbotAI* ai)
    {
        return new RazuviousUseObedienceCrystalAction(ai);
    }
    static Action* horseman_attract_alternatively(PlayerbotAI* ai)
    {
        return new HorsemanAttractAlternativelyAction(ai);
    }
    static Action* horseman_attack_in_order(PlayerbotAI* ai) { return new HorsemanAttactInOrderAction(ai); }
    // static Action* sapphiron_ground_main_tank_position(PlayerbotAI* ai) { return new
    // SapphironGroundMainTankPositionAction(ai); }
    static Action* sapphiron_ground_position(PlayerbotAI* ai) { return new SapphironGroundPositionAction(ai); }
    static Action* sapphiron_flight_position(PlayerbotAI* ai) { return new SapphironFlightPositionAction(ai); }
    // static Action* sapphiron_avoid_chill(PlayerbotAI* ai) { return new SapphironAvoidChillAction(ai); }
    static Action* kelthuzad_choose_target(PlayerbotAI* ai) { return new KelthuzadChooseTargetAction(ai); }
    static Action* kelthuzad_position(PlayerbotAI* ai) { return new KelthuzadPositionAction(ai); }
    static Action* anubrekhan_choose_target(PlayerbotAI* ai) { return new AnubrekhanChooseTargetAction(ai); }
    static Action* anubrekhan_position(PlayerbotAI* ai) { return new AnubrekhanPositionAction(ai); }
    static Action* gluth_choose_target(PlayerbotAI* ai) { return new GluthChooseTargetAction(ai); }
    static Action* gluth_position(PlayerbotAI* ai) { return new GluthPositionAction(ai); }
    static Action* gluth_slowdown(PlayerbotAI* ai) { return new GluthSlowdownAction(ai); }
    static Action* loatheb_position(PlayerbotAI* ai) { return new LoathebPositionAction(ai); }
    static Action* loatheb_choose_target(PlayerbotAI* ai) { return new LoathebChooseTargetAction(ai); }
};

#endif