/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXXACTIONCONTEXT_H
#define PLAYERBOTS_NAXXACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "NaxxActions.h"

class RaidNaxxActionContext : public NamedObjectContext<Action>
{
public:
    RaidNaxxActionContext()
    {
        creators["grobbulus go behind the boss"] = &RaidNaxxActionContext::go_behind_the_boss;
        creators["rotate grobbulus"] = &RaidNaxxActionContext::rotate_grobbulus;
        creators["grobbulus move center"] = &RaidNaxxActionContext::grobbulus_move_center;
        creators["grobbulus move away"] = &RaidNaxxActionContext::grobbulus_move_away;

        creators["heigan dance melee"] = &RaidNaxxActionContext::heigan_dance_melee;
        creators["heigan dance ranged"] = &RaidNaxxActionContext::heigan_dance_ranged;
        creators["thaddius attack nearest pet"] = &RaidNaxxActionContext::thaddius_attack_nearest_pet;
        // creators["thaddius melee to place"] = &RaidNaxxActionContext::thaddius_tank_to_place;
        // creators["thaddius ranged to place"] = &RaidNaxxActionContext::thaddius_ranged_to_place;
        creators["thaddius move to platform"] = &RaidNaxxActionContext::thaddius_move_to_platform;
        creators["thaddius move polarity"] = &RaidNaxxActionContext::thaddius_move_polarity;

        creators["razuvious use obedience crystal"] = &RaidNaxxActionContext::razuvious_use_obedience_crystal;
        creators["razuvious target"] = &RaidNaxxActionContext::razuvious_target;

        creators["four horsemen attract alternatively"] = &RaidNaxxActionContext::four_horsemen_attract_alternatively;
        creators["four horsemen attack in order"] = &RaidNaxxActionContext::four_horsemen_attack_in_order;

        creators["sapphiron ground position"] = &RaidNaxxActionContext::sapphiron_ground_position;
        creators["sapphiron flight position"] = &RaidNaxxActionContext::sapphiron_flight_position;

        creators["kel'thuzad choose target"] = &RaidNaxxActionContext::kelthuzad_choose_target;
        creators["kel'thuzad position"] = &RaidNaxxActionContext::kelthuzad_position;

        creators["anub'rekhan choose target"] = &RaidNaxxActionContext::anubrekhan_choose_target;
        creators["anub'rekhan position"] = &RaidNaxxActionContext::anubrekhan_position;

        creators["gluth choose target"] = &RaidNaxxActionContext::gluth_choose_target;
        creators["gluth position"] = &RaidNaxxActionContext::gluth_position;
        creators["gluth slowdown"] = &RaidNaxxActionContext::gluth_slowdown;

        //creators["patchwerk ranged position"] = &RaidNaxxActionContext::patchwerk_ranged_position;

        creators["loatheb position"] = &RaidNaxxActionContext::loatheb_position;
        creators["loatheb choose target"] = &RaidNaxxActionContext::loatheb_choose_target;
    }

private:
    static Action* go_behind_the_boss(PlayerbotAI* botAI) { return new GrobbulusGoBehindAction(botAI); }
    static Action* rotate_grobbulus(PlayerbotAI* botAI) { return new GrobbulusRotateAction(botAI); }
    static Action* grobbulus_move_center(PlayerbotAI* botAI) { return new GrobbulusMoveCenterAction(botAI); }
    static Action* grobbulus_move_away(PlayerbotAI* botAI) { return new GrobbulusMoveAwayAction(botAI); }
    static Action* heigan_dance_melee(PlayerbotAI* botAI) { return new HeiganDanceAction(botAI, false); }
    static Action* heigan_dance_ranged(PlayerbotAI* botAI) { return new HeiganDanceAction(botAI, true); }
    static Action* thaddius_attack_nearest_pet(PlayerbotAI* botAI) { return new ThaddiusAttackNearestPetAction(botAI); }
    // static Action* thaddius_tank_to_place(PlayerbotAI* botAI) { return new ThaddiusMeleeToPlaceAction(botAI); }
    // static Action* thaddius_ranged_to_place(PlayerbotAI* botAI) { return new ThaddiusRangedToPlaceAction(botAI); }
    static Action* thaddius_move_to_platform(PlayerbotAI* botAI) { return new ThaddiusMoveToPlatformAction(botAI); }
    static Action* thaddius_move_polarity(PlayerbotAI* botAI) { return new ThaddiusMovePolarityAction(botAI); }
    static Action* razuvious_target(PlayerbotAI* botAI) { return new RazuviousTargetAction(botAI); }
    static Action* razuvious_use_obedience_crystal(PlayerbotAI* botAI)
    {
        return new RazuviousUseObedienceCrystalAction(botAI);
    }
    static Action* four_horsemen_attract_alternatively(PlayerbotAI* botAI) { return new FourHorsemenAttractAlternativelyAction(botAI); }
    static Action* four_horsemen_attack_in_order(PlayerbotAI* botAI) { return new FourHorsemenAttackInOrderAction(botAI); }
    // static Action* sapphiron_ground_main_tank_position(PlayerbotAI* botAI) { return new
    // SapphironGroundMainTankPositionAction(botAI); }
    static Action* sapphiron_ground_position(PlayerbotAI* botAI) { return new SapphironGroundPositionAction(botAI); }
    static Action* sapphiron_flight_position(PlayerbotAI* botAI) { return new SapphironFlightPositionAction(botAI); }
    // static Action* sapphiron_avoid_chill(PlayerbotAI* botAI) { return new SapphironAvoidChillAction(botAI); }
    static Action* kelthuzad_choose_target(PlayerbotAI* botAI) { return new KelthuzadChooseTargetAction(botAI); }
    static Action* kelthuzad_position(PlayerbotAI* botAI) { return new KelthuzadPositionAction(botAI); }
    static Action* anubrekhan_choose_target(PlayerbotAI* botAI) { return new AnubrekhanChooseTargetAction(botAI); }
    static Action* anubrekhan_position(PlayerbotAI* botAI) { return new AnubrekhanPositionAction(botAI); }
    static Action* gluth_choose_target(PlayerbotAI* botAI) { return new GluthChooseTargetAction(botAI); }
    static Action* gluth_position(PlayerbotAI* botAI) { return new GluthPositionAction(botAI); }
    static Action* gluth_slowdown(PlayerbotAI* botAI) { return new GluthSlowdownAction(botAI); }
    //static Action* patchwerk_ranged_position(PlayerbotAI* botAI) { return new PatchwerkRangedPositionAction(botAI); }
    static Action* loatheb_position(PlayerbotAI* botAI) { return new LoathebPositionAction(botAI); }
    static Action* loatheb_choose_target(PlayerbotAI* botAI) { return new LoathebChooseTargetAction(botAI); }
};

#endif
