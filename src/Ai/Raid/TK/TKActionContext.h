/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKACTIONCONTEXT_H
#define PLAYERBOTS_TKACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "TKActions.h"

class RaidTempestKeepActionContext : public NamedObjectContext<Action>
{
public:
    RaidTempestKeepActionContext()
    {
        // General
        creators["tempest keep reset encounter states"] =
            &RaidTempestKeepActionContext::tempest_keep_reset_encounter_states;

        creators["tempest keep clear stale falling flag"] =
            &RaidTempestKeepActionContext::tempest_keep_clear_stale_falling_flag;

        // Trash
        creators["crimson hand centurion cast polymorph"] =
            &RaidTempestKeepActionContext::crimson_hand_centurion_cast_polymorph;

        // Al'ar <Phoenix God>
        creators["al'ar misdirect boss to main tank"] =
            &RaidTempestKeepActionContext::alar_misdirect_boss_to_main_tank;

        creators["al'ar boss tanks move between platforms"] =
            &RaidTempestKeepActionContext::alar_boss_tanks_move_between_platforms;

        creators["al'ar melee dps move between platforms"] =
            &RaidTempestKeepActionContext::alar_melee_dps_move_between_platforms;

        creators["al'ar ranged and ember tank move under platforms"] =
            &RaidTempestKeepActionContext::alar_ranged_and_ember_tank_move_under_platforms;

        creators["al'ar assist tanks pick up embers"] =
            &RaidTempestKeepActionContext::alar_assist_tanks_pick_up_embers;

        creators["al'ar assign non-tank target"] =
            &RaidTempestKeepActionContext::alar_assign_non_tank_target;

        creators["al'ar jump from platform"] =
            &RaidTempestKeepActionContext::alar_jump_from_platform;

        creators["al'ar move away from rebirth"] =
            &RaidTempestKeepActionContext::alar_move_away_from_rebirth;

        creators["al'ar swap tanks on boss"] =
            &RaidTempestKeepActionContext::alar_swap_tanks_on_boss;

        creators["al'ar avoid flame patches and dive bombs"] =
            &RaidTempestKeepActionContext::alar_avoid_flame_patches_and_dive_bombs;

        creators["al'ar manage phase tracker"] =
            &RaidTempestKeepActionContext::alar_manage_phase_tracker;

        // Void Reaver
        creators["void reaver tanks position boss"] =
            &RaidTempestKeepActionContext::void_reaver_tanks_position_boss;

        creators["void reaver use aggro dump ability"] =
            &RaidTempestKeepActionContext::void_reaver_use_aggro_dump_ability;

        creators["void reaver ranged back off and spread"] =
            &RaidTempestKeepActionContext::void_reaver_ranged_back_off_and_spread;

        creators["void reaver avoid arcane orb"] =
            &RaidTempestKeepActionContext::void_reaver_avoid_arcane_orb;

        // High Astromancer Solarian
        creators["high astromancer solarian main tank pick up boss"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_main_tank_pick_up_boss;

        creators["high astromancer solarian move away from group"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_move_away_from_group;

        creators["high astromancer solarian target solarium priests"] =
            &RaidTempestKeepActionContext::high_astromancer_solarian_target_solarium_priests;

        // Kael'thas Sunstrider <Lord of the Blood Elves>
        creators["kael'thas sunstrider kite thaladred"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_kite_thaladred;

        creators["kael'thas sunstrider misdirect advisors to tanks"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_misdirect_advisors_to_tanks;

        creators["kael'thas sunstrider melee tanks position advisors"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_melee_tanks_position_advisors;

        creators["kael'thas sunstrider warlock tank position capernian"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_warlock_tank_position_capernian;

        creators["kael'thas sunstrider spread and move away from capernian"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_spread_and_move_away_from_capernian;

        creators["kael'thas sunstrider handle advisor roles in phase 3"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_handle_advisor_roles_in_phase_3;

        creators["kael'thas sunstrider assign advisor dps priority"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_advisor_dps_priority;

        creators["kael'thas sunstrider manage advisor dps timer"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_manage_advisor_dps_timer;

        creators["kael'thas sunstrider assign legendary weapon dps priority"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_legendary_weapon_dps_priority;

        creators["kael'thas sunstrider move devastation away"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_move_devastation_away;

        creators["kael'thas sunstrider loot legendary weapons"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_loot_legendary_weapons;

        creators["kael'thas sunstrider use legendary weapons"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_use_legendary_weapons;

        creators["kael'thas sunstrider reequip gear"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_reequip_gear;

        creators["kael'thas sunstrider tanks position boss"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_tanks_position_boss;

        creators["kael'thas sunstrider avoid flame strike"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_avoid_flame_strike;

        creators["kael'thas sunstrider assign final phase target"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_assign_final_phase_target;

        creators["kael'thas sunstrider break mind control"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_break_mind_control;

        creators["kael'thas sunstrider spread out in midair"] =
            &RaidTempestKeepActionContext::kaelthas_sunstrider_spread_out_in_midair;
    }

private:
    // General
    static Action* tempest_keep_reset_encounter_states(PlayerbotAI* botAI) {
        return new TempestKeepResetEncounterStatesAction(botAI);
    }
    static Action* tempest_keep_clear_stale_falling_flag(PlayerbotAI* botAI) {
        return new TempestKeepClearStaleFallingFlagAction(botAI);
    }

    // Trash
    static Action* crimson_hand_centurion_cast_polymorph(PlayerbotAI* botAI) {
        return new CrimsonHandCenturionCastPolymorphAction(botAI);
    }

    // Al'ar <Phoenix God>
    static Action* alar_misdirect_boss_to_main_tank(PlayerbotAI* botAI) {
        return new AlarMisdirectBossToMainTankAction(botAI);
    }
    static Action* alar_boss_tanks_move_between_platforms(PlayerbotAI* botAI) {
        return new AlarBossTanksMoveBetweenPlatformsAction(botAI);
    }
    static Action* alar_melee_dps_move_between_platforms(PlayerbotAI* botAI) {
        return new AlarMeleeDpsMoveBetweenPlatformsAction(botAI);
    }
    static Action* alar_ranged_and_ember_tank_move_under_platforms(PlayerbotAI* botAI) {
        return new AlarRangedAndEmberTankMoveUnderPlatformsAction(botAI);
    }
    static Action* alar_assist_tanks_pick_up_embers(PlayerbotAI* botAI) {
        return new AlarAssistTanksPickUpEmbersAction(botAI);
    }
    static Action* alar_assign_non_tank_target(PlayerbotAI* botAI) {
        return new AlarAssignNonTankTargetAction(botAI);
    }
    static Action* alar_jump_from_platform(PlayerbotAI* botAI) {
        return new AlarJumpFromPlatformAction(botAI);
    }
    static Action* alar_move_away_from_rebirth(PlayerbotAI* botAI) {
        return new AlarMoveAwayFromRebirthAction(botAI);
    }
    static Action* alar_swap_tanks_on_boss(PlayerbotAI* botAI) {
        return new AlarSwapTanksOnBossAction(botAI);
    }
    static Action* alar_avoid_flame_patches_and_dive_bombs(PlayerbotAI* botAI) {
        return new AlarAvoidFlamePatchesAndDiveBombsAction(botAI);
    }
    static Action* alar_manage_phase_tracker(PlayerbotAI* botAI) {
        return new AlarManagePhaseTrackerAction(botAI);
    }

    // Void Reaver
    static Action* void_reaver_tanks_position_boss(PlayerbotAI* botAI) {
        return new VoidReaverTanksPositionBossAction(botAI);
    }
    static Action* void_reaver_use_aggro_dump_ability(PlayerbotAI* botAI) {
        return new VoidReaverUseAggroDumpAbilityAction(botAI);
    }
    static Action* void_reaver_ranged_back_off_and_spread(PlayerbotAI* botAI) {
        return new VoidReaverRangedBackOffAndSpreadAction(botAI);
    }
    static Action* void_reaver_avoid_arcane_orb(PlayerbotAI* botAI) {
        return new VoidReaverAvoidArcaneOrbAction(botAI);
    }

    // High Astromancer Solarian
    static Action* high_astromancer_solarian_main_tank_pick_up_boss(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianMainTankPickUpBossAction(botAI);
    }
    static Action* high_astromancer_solarian_move_away_from_group(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianMoveAwayFromGroupAction(botAI);
    }
    static Action* high_astromancer_solarian_target_solarium_priests(PlayerbotAI* botAI) {
        return new HighAstromancerSolarianTargetSolariumPriestsAction(botAI);
    }

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    static Action* kaelthas_sunstrider_kite_thaladred(PlayerbotAI* botAI) {
        return new KaelthasSunstriderKiteThaladredAction(botAI);
    }
    static Action* kaelthas_sunstrider_misdirect_advisors_to_tanks(PlayerbotAI* botAI) {
        return new KaelthasSunstriderMisdirectAdvisorsToTanksAction(botAI);
    }
    static Action* kaelthas_sunstrider_melee_tanks_position_advisors(PlayerbotAI* botAI) {
        return new KaelthasSunstriderMeleeTanksPositionAdvisorsAction(botAI);
    }
    static Action* kaelthas_sunstrider_warlock_tank_position_capernian(PlayerbotAI* botAI) {
        return new KaelthasSunstriderWarlockTankPositionCapernianAction(botAI);
    }
    static Action* kaelthas_sunstrider_spread_and_move_away_from_capernian(PlayerbotAI* botAI) {
        return new KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction(botAI);
    }
    static Action* kaelthas_sunstrider_handle_advisor_roles_in_phase_3(PlayerbotAI* botAI) {
        return new KaelthasSunstriderHandleAdvisorRolesInPhase3Action(botAI);
    }
    static Action* kaelthas_sunstrider_assign_advisor_dps_priority(PlayerbotAI* botAI) {
        return new KaelthasSunstriderAssignAdvisorDpsPriorityAction(botAI);
    }
    static Action* kaelthas_sunstrider_manage_advisor_dps_timer(PlayerbotAI* botAI) {
        return new KaelthasSunstriderManageAdvisorDpsTimerAction(botAI);
    }
    static Action* kaelthas_sunstrider_assign_legendary_weapon_dps_priority(PlayerbotAI* botAI) {
        return new KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction(botAI);
    }
    static Action* kaelthas_sunstrider_move_devastation_away(PlayerbotAI* botAI) {
        return new KaelthasSunstriderMoveDevastationAwayAction(botAI);
    }
    static Action* kaelthas_sunstrider_loot_legendary_weapons(PlayerbotAI* botAI) {
        return new KaelthasSunstriderLootLegendaryWeaponsAction(botAI);
    }
    static Action* kaelthas_sunstrider_use_legendary_weapons(PlayerbotAI* botAI) {
        return new KaelthasSunstriderUseLegendaryWeaponsAction(botAI);
    }
    static Action* kaelthas_sunstrider_reequip_gear(PlayerbotAI* botAI) {
        return new KaelthasSunstriderReequipGearAction(botAI);
    }
    static Action* kaelthas_sunstrider_tanks_position_boss(PlayerbotAI* botAI) {
        return new KaelthasSunstriderTanksPositionBossAction(botAI);
    }
    static Action* kaelthas_sunstrider_avoid_flame_strike(PlayerbotAI* botAI) {
        return new KaelthasSunstriderAvoidFlameStrikeAction(botAI);
    }
    static Action* kaelthas_sunstrider_assign_final_phase_target(PlayerbotAI* botAI) {
        return new KaelthasSunstriderAssignFinalPhaseTargetAction(botAI);
    }
    static Action* kaelthas_sunstrider_break_mind_control(PlayerbotAI* botAI) {
        return new KaelthasSunstriderBreakMindControlAction(botAI);
    }
    static Action* kaelthas_sunstrider_spread_out_in_midair(PlayerbotAI* botAI) {
        return new KaelthasSunstriderSpreadOutInMidairAction(botAI);
    }
};

#endif
