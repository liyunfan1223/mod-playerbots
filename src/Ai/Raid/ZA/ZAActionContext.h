/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAACTIONCONTEXT_H
#define PLAYERBOTS_ZAACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "ZAActions.h"
#include "ZAHelpers.h"

class RaidZulAmanActionContext : public NamedObjectContext<Action>
{
public:
    RaidZulAmanActionContext()
    {
        // General
        creators["zul'aman reset encounter states"] =
            &RaidZulAmanActionContext::zulaman_reset_encounter_states;

        // Trash
        creators["amani'shi medicine man mark ward"] =
            &RaidZulAmanActionContext::amanishi_medicine_man_mark_ward;

        // Akil'zon <Eagle Avatar>
        creators["akil'zon misdirect boss to main tank"] =
            &RaidZulAmanActionContext::akilzon_misdirect_boss_to_main_tank;

        creators["akil'zon tanks position boss"] =
            &RaidZulAmanActionContext::akilzon_tanks_position_boss;

        creators["akil'zon spread ranged"] = &RaidZulAmanActionContext::akilzon_spread_ranged;

        creators["akil'zon move to eye of the storm"] =
            &RaidZulAmanActionContext::akilzon_move_to_eye_of_the_storm;

        creators["akil'zon start electrical storm timer"] =
            &RaidZulAmanActionContext::akilzon_start_electrical_storm_timer;

        // Nalorakk <Bear Avatar>
        creators["nalorakk misdirect boss to main tank"] =
            &RaidZulAmanActionContext::nalorakk_misdirect_boss_to_main_tank;

        creators["nalorakk tanks position boss"] =
            &RaidZulAmanActionContext::nalorakk_tanks_position_boss;

        creators["nalorakk spread ranged"] = &RaidZulAmanActionContext::nalorakk_spread_ranged;

        // Jan'alai <Dragonhawk Avatar>
        creators["jan'alai misdirect boss to main tank"] =
            &RaidZulAmanActionContext::janalai_misdirect_boss_to_main_tank;

        creators["jan'alai tanks position boss"] =
            &RaidZulAmanActionContext::janalai_tanks_position_boss;

        creators["jan'alai spread ranged in circle"] =
            &RaidZulAmanActionContext::janalai_spread_ranged_in_circle;

        creators["jan'alai avoid fire bombs"] = &RaidZulAmanActionContext::janalai_avoid_fire_bombs;

        creators["jan'alai mark amani'shi hatchers"] =
            &RaidZulAmanActionContext::janalai_mark_amanishi_hatchers;

        // Halazzi <Lynx Avatar>
        creators["halazzi misdirect boss to main tank"] =
            &RaidZulAmanActionContext::halazzi_misdirect_boss_to_main_tank;

        creators["halazzi main tank position boss"] =
            &RaidZulAmanActionContext::halazzi_main_tank_position_boss;

        creators["halazzi first assist tank attack spirit lynx"] =
            &RaidZulAmanActionContext::halazzi_first_assist_tank_attack_spirit_lynx;

        creators["halazzi dps attack totem and boss"] =
            &RaidZulAmanActionContext::halazzi_dps_attack_totem_and_boss;

        // Hex Lord Malacrass
        creators["hex lord malacrass misdirect boss to main tank"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_misdirect_boss_to_main_tank;

        creators["hex lord malacrass assign dps priority"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_assign_dps_priority;

        creators["hex lord malacrass run away from whirlwind"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_run_away_from_whirlwind;

        creators["hex lord malacrass move away from freezing trap"] =
            &RaidZulAmanActionContext::hex_lord_malacrass_move_away_from_freezing_trap;

        // Zul'jin
        creators["zul'jin misdirect boss to main tank"] =
            &RaidZulAmanActionContext::zuljin_misdirect_boss_to_main_tank;

        creators["zul'jin tanks position boss"] =
            &RaidZulAmanActionContext::zuljin_tanks_position_boss;

        creators["zul'jin run away from whirlwind"] =
            &RaidZulAmanActionContext::zuljin_run_away_from_whirlwind;

        creators["zul'jin mass dispel creeping paralysis"] =
            &RaidZulAmanActionContext::zuljin_mass_dispel_creeping_paralysis;

        creators["zul'jin position ranged for cyclones"] =
            &RaidZulAmanActionContext::zuljin_position_ranged_for_cyclones;

        creators["zul'jin spread ranged"] = &RaidZulAmanActionContext::zuljin_spread_ranged;
    }

private:
    // General
    static Action* zulaman_reset_encounter_states(PlayerbotAI* botAI) {
        return new ZulAmanResetEncounterStatesAction(botAI);
    }

    // Trash
    static Action* amanishi_medicine_man_mark_ward(PlayerbotAI* botAI) {
        return new AmanishiMedicineManMarkWardAction(botAI);
    }

    // Akil'zon <Eagle Avatar>
    static Action* akilzon_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "akil'zon misdirect boss to main tank", "akil'zon");
    }
    static Action* akilzon_tanks_position_boss(PlayerbotAI* botAI) {
        return new ZulAmanTanksPositionBossAction(
            botAI, "akil'zon tanks position boss", "akil'zon", ZaHelpers::AKILZON_TANK_POSITION);
    }
    static Action* akilzon_spread_ranged(PlayerbotAI* botAI) {
        return new ZulAmanSpreadRangedAction(botAI, "akil'zon spread ranged", 13.0f);
    }
    static Action* akilzon_move_to_eye_of_the_storm(PlayerbotAI* botAI) {
        return new AkilzonMoveToEyeOfTheStormAction(botAI);
    }
    static Action* akilzon_start_electrical_storm_timer(PlayerbotAI* botAI) {
        return new AkilzonStartElectricalStormTimerAction(botAI);
    }

    // Nalorakk <Bear Avatar>
    static Action* nalorakk_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "nalorakk misdirect boss to main tank", "nalorakk");
    }
    static Action* nalorakk_tanks_position_boss(PlayerbotAI* botAI) {
        return new NalorakkTanksPositionBossAction(botAI);
    }
    static Action* nalorakk_spread_ranged(PlayerbotAI* botAI) {
        return new ZulAmanSpreadRangedAction(botAI, "nalorakk spread ranged", 11.0f);
    }

    // Jan'alai <Dragonhawk Avatar>
    static Action* janalai_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "jan'alai misdirect boss to main tank", "jan'alai");
    }
    static Action* janalai_tanks_position_boss(PlayerbotAI* botAI) {
        return new ZulAmanTanksPositionBossAction(
            botAI, "jan'alai tanks position boss", "jan'alai",
            ZaHelpers::JANALAI_TANK_POSITION);
    }
    static Action* janalai_spread_ranged_in_circle(PlayerbotAI* botAI) {
        return new JanalaiSpreadRangedInCircleAction(botAI);
    }
    static Action* janalai_avoid_fire_bombs(PlayerbotAI* botAI) {
        return new JanalaiAvoidFireBombsAction(botAI);
    }
    static Action* janalai_mark_amanishi_hatchers(PlayerbotAI* botAI) {
        return new JanalaiMarkAmanishiHatchersAction(botAI);
    }

    // Halazzi <Lynx Avatar>
    static Action* halazzi_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "halazzi misdirect boss to main tank", "halazzi");
    }
    static Action* halazzi_main_tank_position_boss(PlayerbotAI* botAI) {
        return new ZulAmanTanksPositionBossAction(
            botAI, "halazzi main tank position boss", "halazzi", ZaHelpers::HALAZZI_TANK_POSITION);
    }
    static Action* halazzi_first_assist_tank_attack_spirit_lynx(PlayerbotAI* botAI) {
        return new HalazziFirstAssistTankAttackSpiritLynxAction(botAI);
    }
    static Action* halazzi_dps_attack_totem_and_boss(PlayerbotAI* botAI) {
        return new HalazziDpsAttackTotemAndBossAction(botAI);
    }

    // Hex Lord Malacrass
    static Action* hex_lord_malacrass_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "hex lord malacrass misdirect boss to main tank", "hex lord malacrass");
    }
    static Action* hex_lord_malacrass_assign_dps_priority(PlayerbotAI* botAI) {
        return new HexLordMalacrassAssignDpsPriorityAction(botAI);
    }
    static Action* hex_lord_malacrass_run_away_from_whirlwind(PlayerbotAI* botAI) {
        return new ZulAmanRunAwayFromWhirlwindAction(
            botAI, "hex lord malacrass run away from whirlwind", "hex lord malacrass");
    }
    static Action* hex_lord_malacrass_move_away_from_freezing_trap(PlayerbotAI* botAI) {
        return new HexLordMalacrassMoveAwayFromFreezingTrapAction(botAI);
    }

    // Zul'jin
    static Action* zuljin_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new ZulAmanMisdirectBossToMainTankAction(
            botAI, "zul'jin misdirect boss to main tank", "zul'jin");
    }
    static Action* zuljin_tanks_position_boss(PlayerbotAI* botAI) {
        return new ZulAmanTanksPositionBossAction(
            botAI, "zul'jin tanks position boss", "zul'jin", ZaHelpers::ZULJIN_TANK_POSITION);
    }
    static Action* zuljin_spread_ranged(PlayerbotAI* botAI) {
        return new ZulAmanSpreadRangedAction(botAI, "zul'jin spread ranged", 6.0f);
    }
    static Action* zuljin_run_away_from_whirlwind(PlayerbotAI* botAI) {
        return new ZulAmanRunAwayFromWhirlwindAction(
            botAI, "zul'jin run away from whirlwind", "zul'jin");
    }
    static Action* zuljin_mass_dispel_creeping_paralysis(PlayerbotAI* botAI) {
        return new ZuljinMassDispelCreepingParalysisAction(botAI);
    }
    static Action* zuljin_position_ranged_for_cyclones(PlayerbotAI* botAI) {
        return new ZuljinPositionRangedForCyclonesAction(botAI);
    }
};

#endif
