/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAACTIONCONTEXT_H
#define PLAYERBOTS_KARAACTIONCONTEXT_H

#include "KaraActions.h"
#include "NamedObjectContext.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        // General
        creators["karazhan reset encounter states"] =
            &RaidKarazhanActionContext::karazhan_reset_encounter_states;

        creators["karazhan set tremor totem"] =
            &RaidKarazhanActionContext::karazhan_set_tremor_totem;

        // Trash
        creators["mana warp stun creature before warp breach"] =
            &RaidKarazhanActionContext::mana_warp_stun_creature_before_warp_breach;

        // Attumen the Huntsman
        creators["attumen the huntsman handle phase one"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_handle_phase_one;

        creators["attumen the huntsman handle phase two"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_handle_phase_two;

        creators["attumen the huntsman set dps timer"] =
            &RaidKarazhanActionContext::attumen_the_huntsman_manage_dps_timer;

        // Moroes
        creators["moroes mark target"] =
            &RaidKarazhanActionContext::moroes_mark_target;

        // Maiden of Virtue
        creators["maiden of virtue tank position boss"] =
            &RaidKarazhanActionContext::maiden_of_virtue_tank_position_boss;

        creators["maiden of virtue position ranged between pillars"] =
            &RaidKarazhanActionContext::maiden_of_virtue_position_ranged_between_pillars;

        creators["maiden of virtue set grounding totem"] =
            &RaidKarazhanActionContext::maiden_of_virtue_set_grounding_totem;

        // The Big Bad Wolf
        creators["big bad wolf position boss"] =
            &RaidKarazhanActionContext::big_bad_wolf_position_boss;

        creators["big bad wolf little red riding hood run away"] =
            &RaidKarazhanActionContext::big_bad_wolf_little_red_riding_hood_run_away;

        // Romulo and Julianne
        creators["romulo and julianne mark target"] =
            &RaidKarazhanActionContext::romulo_and_julianne_mark_target;

        // The Wizard of Oz
        creators["wizard of oz mark target"] =
            &RaidKarazhanActionContext::wizard_of_oz_mark_target;

        creators["wizard of oz scorch strawman"] =
            &RaidKarazhanActionContext::wizard_of_oz_scorch_strawman;

        // The Curator
        creators["the curator mark astral flare"] =
            &RaidKarazhanActionContext::the_curator_mark_astral_flare;

        creators["the curator position boss"] =
            &RaidKarazhanActionContext::the_curator_position_boss;

        creators["the curator spread ranged"] =
            &RaidKarazhanActionContext::the_curator_spread_ranged;

        // Terestian Illhoof
        creators["terestian illhoof mark target"] =
            &RaidKarazhanActionContext::terestian_illhoof_mark_target;

        // Shade of Aran
        creators["shade of aran run away from arcane explosion"] =
            &RaidKarazhanActionContext::shade_of_aran_run_away_from_arcane_explosion;

        creators["shade of aran stop moving during flame wreath"] =
            &RaidKarazhanActionContext::shade_of_aran_stop_moving_during_flame_wreath;

        creators["shade of aran mark conjured elemental"] =
            &RaidKarazhanActionContext::shade_of_aran_mark_conjured_elemental;

        creators["shade of aran ranged maintain distance"] =
            &RaidKarazhanActionContext::shade_of_aran_ranged_maintain_distance;

        // Netherspite
        creators["netherspite block red beam"] =
            &RaidKarazhanActionContext::netherspite_block_red_beam;

        creators["netherspite block blue beam"] =
            &RaidKarazhanActionContext::netherspite_block_blue_beam;

        creators["netherspite block green beam"] =
            &RaidKarazhanActionContext::netherspite_block_green_beam;

        creators["netherspite avoid beam and void zone"] =
            &RaidKarazhanActionContext::netherspite_avoid_beam_and_void_zone;

        creators["netherspite banish phase avoid void zone"] =
            &RaidKarazhanActionContext::netherspite_banish_phase_avoid_void_zone;

        creators["netherspite manage timers and trackers"] =
            &RaidKarazhanActionContext::netherspite_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar enfeebled bot avoid hazard"] =
            &RaidKarazhanActionContext::prince_malchezaar_enfeebled_bot_avoid_hazard;

        creators["prince malchezaar non-tank avoid infernal"] =
            &RaidKarazhanActionContext::prince_malchezaar_non_tank_avoid_infernal;

        creators["prince malchezaar tanks position boss"] =
            &RaidKarazhanActionContext::prince_malchezaar_tanks_position_boss;

        // Nightbane
        creators["nightbane ground phase tanks position boss"] =
            &RaidKarazhanActionContext::nightbane_ground_phase_position_boss;

        creators["nightbane ground phase coordinate ranged movement"] =
            &RaidKarazhanActionContext::nightbane_ground_phase_rotate_ranged_positions;

        creators["nightbane control pet aggression"] =
            &RaidKarazhanActionContext::nightbane_control_pet_aggression;

        creators["nightbane flight phase stack and move"] =
            &RaidKarazhanActionContext::nightbane_flight_phase_stack_and_move;

        creators["nightbane teleport back to terrace"] =
            &RaidKarazhanActionContext::nightbane_teleport_back_to_terrace;

        creators["nightbane manage timers and trackers"] =
            &RaidKarazhanActionContext::nightbane_manage_timers_and_trackers;
    }

private:
    // General
    static Action* karazhan_reset_encounter_states(PlayerbotAI* botAI) {
        return new KarazhanResetEncounterStatesAction(botAI);
    }
    static Action* karazhan_set_tremor_totem(PlayerbotAI* botAI) {
        return new KarazhanSetTremorTotemAction(botAI);
    }

    // Trash
    static Action* mana_warp_stun_creature_before_warp_breach(PlayerbotAI* botAI) {
        return new ManaWarpStunCreatureBeforeWarpBreachAction(botAI);
    }

    // Attumen the Huntsman
    static Action* attumen_the_huntsman_handle_phase_one(PlayerbotAI* botAI) {
        return new AttumenTheHuntsmanHandlePhaseOneAction(botAI);
    }
    static Action* attumen_the_huntsman_handle_phase_two(PlayerbotAI* botAI) {
        return new AttumenTheHuntsmanHandlePhaseTwoAction(botAI);
    }
    static Action* attumen_the_huntsman_manage_dps_timer(PlayerbotAI* botAI) {
        return new AttumenTheHuntsmanSetDpsTimerAction(botAI);
    }

    // Moroes
    static Action* moroes_mark_target(PlayerbotAI* botAI) {
        return new MoroesMarkTargetAction(botAI);
    }

    // Maiden of Virtue
    static Action* maiden_of_virtue_tank_position_boss(PlayerbotAI* botAI) {
        return new MaidenOfVirtueTankPositionBossAction(botAI);
    }
    static Action* maiden_of_virtue_position_ranged_between_pillars(PlayerbotAI* botAI) {
        return new MaidenOfVirtuePositionRangedBetweenPillarsAction(botAI);
    }
    static Action* maiden_of_virtue_set_grounding_totem(PlayerbotAI* botAI) {
        return new MaidenOfVirtueSetGroundingTotemAction(botAI);
    }

    // The Big Bad Wolf
    static Action* big_bad_wolf_position_boss(PlayerbotAI* botAI) {
        return new BigBadWolfPositionBossAction(botAI);
    }
    static Action* big_bad_wolf_little_red_riding_hood_run_away(PlayerbotAI* botAI) {
        return new BigBadWolfLittleRedRidingHoodRunAwayAction(botAI);
    }

    // Romulo and Julianne
    static Action* romulo_and_julianne_mark_target(PlayerbotAI* botAI) {
        return new RomuloAndJulianneMarkTargetAction(botAI);
    }

    // The Wizard of Oz
    static Action* wizard_of_oz_mark_target(PlayerbotAI* botAI) {
        return new WizardOfOzMarkTargetAction(botAI);
    }
    static Action* wizard_of_oz_scorch_strawman(PlayerbotAI* botAI) {
        return new WizardOfOzScorchStrawmanAction(botAI);
    }

    // The Curator
    static Action* the_curator_mark_astral_flare(PlayerbotAI* botAI) {
        return new TheCuratorMarkAstralFlareAction(botAI);
    }
    static Action* the_curator_position_boss(PlayerbotAI* botAI) {
        return new TheCuratorPositionBossAction(botAI);
    }
    static Action* the_curator_spread_ranged(PlayerbotAI* botAI) {
        return new TheCuratorSpreadRangedAction(botAI);
    }

    // Terestian Illhoof
    static Action* terestian_illhoof_mark_target(PlayerbotAI* botAI) {
        return new TerestianIllhoofMarkTargetAction(botAI);
    }

    // Shade of Aran
    static Action* shade_of_aran_run_away_from_arcane_explosion(PlayerbotAI* botAI) {
        return new ShadeOfAranRunAwayFromArcaneExplosionAction(botAI);
    }
    static Action* shade_of_aran_stop_moving_during_flame_wreath(PlayerbotAI* botAI) {
        return new ShadeOfAranStopMovingDuringFlameWreathAction(botAI);
    }
    static Action* shade_of_aran_mark_conjured_elemental(PlayerbotAI* botAI) {
        return new ShadeOfAranMarkConjuredElementalAction(botAI);
    }
    static Action* shade_of_aran_ranged_maintain_distance(PlayerbotAI* botAI) {
        return new ShadeOfAranRangedMaintainDistanceAction(botAI);
    }

    // Netherspite
    static Action* netherspite_block_red_beam(PlayerbotAI* botAI) {
        return new NetherspiteBlockRedBeamAction(botAI);
    }
    static Action* netherspite_block_blue_beam(PlayerbotAI* botAI) {
        return new NetherspiteBlockBlueBeamAction(botAI);
    }
    static Action* netherspite_block_green_beam(PlayerbotAI* botAI) {
        return new NetherspiteBlockGreenBeamAction(botAI);
    }
    static Action* netherspite_avoid_beam_and_void_zone(PlayerbotAI* botAI) {
        return new NetherspiteAvoidBeamAndVoidZoneAction(botAI);
    }
    static Action* netherspite_banish_phase_avoid_void_zone(PlayerbotAI* botAI) {
        return new NetherspiteBanishPhaseAvoidVoidZoneAction(botAI);
    }
    static Action* netherspite_manage_timers_and_trackers(PlayerbotAI* botAI) {
        return new NetherspiteManageTimersAndTrackersAction(botAI);
    }

    // Prince Malchezaar
    static Action* prince_malchezaar_enfeebled_bot_avoid_hazard(PlayerbotAI* botAI) {
        return new PrinceMalchezaarEnfeebledBotAvoidHazardAction(botAI);
    }
    static Action* prince_malchezaar_non_tank_avoid_infernal(PlayerbotAI* botAI) {
        return new PrinceMalchezaarNonTankAvoidInfernalAction(botAI);
    }
    static Action* prince_malchezaar_tanks_position_boss(PlayerbotAI* botAI) {
        return new PrinceMalchezaarTanksPositionBossAction(botAI);
    }

    // Nightbane
    static Action* nightbane_ground_phase_position_boss(PlayerbotAI* botAI) {
        return new NightbaneGroundPhaseTanksPositionBossAction(botAI);
    }
    static Action* nightbane_ground_phase_rotate_ranged_positions(PlayerbotAI* botAI) {
        return new NightbaneGroundPhaseCoordinateRangedMovementAction(botAI);
    }
    static Action* nightbane_control_pet_aggression(PlayerbotAI* botAI) {
        return new NightbaneControlPetAggressionAction(botAI);
    }
    static Action* nightbane_flight_phase_stack_and_move(PlayerbotAI* botAI) {
        return new NightbaneFlightPhaseStackAndMoveAction(botAI);
    }
    static Action* nightbane_teleport_back_to_terrace(PlayerbotAI* botAI) {
        return new NightbaneTeleportBackToTerraceAction(botAI);
    }
    static Action* nightbane_manage_timers_and_trackers(PlayerbotAI* botAI) {
        return new NightbaneManageTimersAndTrackersAction(botAI);
    }
};

#endif
