/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERCONTEXT_H
#define PLAYERBOTS_KARATRIGGERCONTEXT_H

#include "KaraTriggers.h"
#include "NamedObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        // Shared
        creators["karazhan bot is not in combat"] =
            &RaidKarazhanTriggerContext::karazhan_bot_is_not_in_combat;

        creators["karazhan enemies cast fear"] =
            &RaidKarazhanTriggerContext::karazhan_enemies_cast_fear;

        // Trash
        creators["mana warp is about to explode"] =
            &RaidKarazhanTriggerContext::mana_warp_is_about_to_explode;

        // Attumen the Huntsman
        creators["attumen the huntsman phase one active"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_phase_one_active;

        creators["attumen the huntsman phase two active"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_phase_two_active;

        creators["attumen the huntsman phase transition"] =
            &RaidKarazhanTriggerContext::attumen_the_huntsman_phase_transition;

        // Moroes
        creators["moroes should prioritize adds"] =
            &RaidKarazhanTriggerContext::moroes_should_prioritize_adds;

        // Maiden of Virtue
        creators["maiden of virtue boss engaged by tanks"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_boss_engaged_by_tanks;

        creators["maiden of virtue holy wrath deals chain damage"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_holy_wrath_deals_chain_damage;

        creators["maiden of virtue grounding totem consumes holy fire"] =
            &RaidKarazhanTriggerContext::maiden_of_virtue_grounding_totem_consumes_holy_fire;

        // The Big Bad Wolf
        creators["big bad wolf boss engaged by tank"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_boss_engaged_by_tank;

        creators["big bad wolf boss is chasing little red riding hood"] =
            &RaidKarazhanTriggerContext::big_bad_wolf_boss_is_chasing_little_red_riding_hood;

        // Romulo and Julianne
        creators["romulo and julianne both bosses revived"] =
            &RaidKarazhanTriggerContext::romulo_and_julianne_both_bosses_revived;

        // The Wizard of Oz
        creators["wizard of oz need target priority"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_need_target_priority;

        creators["wizard of oz strawman is vulnerable to fire"] =
            &RaidKarazhanTriggerContext::wizard_of_oz_strawman_is_vulnerable_to_fire;

        // The Curator
        creators["the curator astral flare spawned"] =
            &RaidKarazhanTriggerContext::the_curator_astral_flare_spawned;

        creators["the curator boss engaged by tanks"] =
            &RaidKarazhanTriggerContext::the_curator_boss_engaged_by_tanks;

        creators["the curator boss engaged by ranged"] =
            &RaidKarazhanTriggerContext::the_curator_boss_engaged_by_ranged;

        // Terestian Illhoof
        creators["terestian illhoof should prioritize chains"] =
            &RaidKarazhanTriggerContext::terestian_illhoof_need_target_priority;

        // Shade of Aran
        creators["shade of aran arcane explosion is casting"] =
            &RaidKarazhanTriggerContext::shade_of_aran_arcane_explosion_is_casting;

        creators["shade of aran flame wreath is active"] =
            &RaidKarazhanTriggerContext::shade_of_aran_flame_wreath_is_active;

        creators["shade of aran conjured elementals summoned"] =
            &RaidKarazhanTriggerContext::shade_of_aran_conjured_elementals_summoned;

        creators["shade of aran boss casts counterspell nearby"] =
            &RaidKarazhanTriggerContext::shade_of_aran_boss_casts_counterspell_nearby;

        // Netherspite
        creators["netherspite red beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_red_beam_is_active;

        creators["netherspite blue beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_blue_beam_is_active;

        creators["netherspite green beam is active"] =
            &RaidKarazhanTriggerContext::netherspite_green_beam_is_active;

        creators["netherspite bot is not beam blocker"] =
            &RaidKarazhanTriggerContext::netherspite_bot_is_not_beam_blocker;

        creators["netherspite boss is banished"] =
            &RaidKarazhanTriggerContext::netherspite_boss_is_banished;

        creators["netherspite should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::netherspite_need_to_manage_timers_and_trackers;

        // Prince Malchezaar
        creators["prince malchezaar bot is enfeebled"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_bot_is_enfeebled;

        creators["prince malchezaar engaged by non-tanks"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_engaged_by_non_tanks;

        creators["prince malchezaar boss engaged by tanks"] =
            &RaidKarazhanTriggerContext::prince_malchezaar_boss_engaged_by_tanks;

        // Nightbane
        creators["nightbane boss engaged by tanks"] =
            &RaidKarazhanTriggerContext::nightbane_boss_engaged_by_tanks;

        creators["nightbane ground phase engaged by ranged"] =
            &RaidKarazhanTriggerContext::nightbane_ranged_bots_are_in_charred_earth;

        creators["nightbane pets ignore collision to chase flying boss"] =
            &RaidKarazhanTriggerContext::nightbane_pets_ignore_collision_to_chase_flying_boss;

        creators["nightbane boss is flying"] =
            &RaidKarazhanTriggerContext::nightbane_boss_is_flying;

        creators["nightbane bot went out of bounds"] =
            &RaidKarazhanTriggerContext::nightbane_bot_went_out_of_bounds;

        creators["nightbane should manage timers and trackers"] =
            &RaidKarazhanTriggerContext::nightbane_need_to_manage_timers_and_trackers;
    }

private:
    // Shared
    static Trigger* karazhan_bot_is_not_in_combat(PlayerbotAI* botAI)
    {
        return new KarazhanBotIsNotInCombatTrigger(botAI);
    }
    static Trigger* karazhan_enemies_cast_fear(PlayerbotAI* botAI)
    {
        return new KarazhanEnemiesCastFearTrigger(botAI);
    }

    // Trash
    static Trigger* mana_warp_is_about_to_explode(PlayerbotAI* botAI)
    {
        return new ManaWarpIsAboutToExplodeTrigger(botAI);
    }

    // Attumen the Huntsman
    static Trigger* attumen_the_huntsman_phase_one_active(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanPhaseOneActiveTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_phase_two_active(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanPhaseTwoActiveTrigger(botAI);
    }
    static Trigger* attumen_the_huntsman_phase_transition(PlayerbotAI* botAI)
    {
        return new AttumenTheHuntsmanPhaseTransitionTrigger(botAI);
    }

    // Moroes
    static Trigger* moroes_should_prioritize_adds(PlayerbotAI* botAI)
    {
        return new MoroesShouldPrioritizeAddsTrigger(botAI);
    }

    // Maiden of Virtue
    static Trigger* maiden_of_virtue_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_holy_wrath_deals_chain_damage(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueHolyWrathDealsChainDamageTrigger(botAI);
    }
    static Trigger* maiden_of_virtue_grounding_totem_consumes_holy_fire(PlayerbotAI* botAI)
    {
        return new MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger(botAI);
    }

    // The Big Bad Wolf
    static Trigger* big_bad_wolf_boss_engaged_by_tank(PlayerbotAI* botAI)
    {
        return new BigBadWolfBossEngagedByTankTrigger(botAI);
    }
    static Trigger* big_bad_wolf_boss_is_chasing_little_red_riding_hood(PlayerbotAI* botAI)
    {
        return new BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(botAI);
    }
    // Romulo and Julianne
    static Trigger* romulo_and_julianne_both_bosses_revived(PlayerbotAI* botAI)
    {
        return new RomuloAndJulianneBothBossesRevivedTrigger(botAI);
    }

    // The Wizard of Oz
    static Trigger* wizard_of_oz_need_target_priority(PlayerbotAI* botAI)
    {
        return new WizardOfOzNeedTargetPriorityTrigger(botAI);
    }
    static Trigger* wizard_of_oz_strawman_is_vulnerable_to_fire(PlayerbotAI* botAI)
    {
        return new WizardOfOzStrawmanIsVulnerableToFireTrigger(botAI);
    }

    // The Curator
    static Trigger* the_curator_astral_flare_spawned(PlayerbotAI* botAI)
    {
        return new TheCuratorAstralFlareSpawnedTrigger(botAI);
    }
    static Trigger* the_curator_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new TheCuratorBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* the_curator_boss_engaged_by_ranged(PlayerbotAI* botAI)
    {
        return new TheCuratorBossEngagedByRangedTrigger(botAI);
    }

    // Terestian Illhoof
    static Trigger* terestian_illhoof_need_target_priority(PlayerbotAI* botAI)
    {
        return new TerestianIllhoofShouldPrioritizeChainsTrigger(botAI);
    }

    // Shade of Aran
    static Trigger* shade_of_aran_arcane_explosion_is_casting(PlayerbotAI* botAI)
    {
        return new ShadeOfAranArcaneExplosionIsCastingTrigger(botAI);
    }
    static Trigger* shade_of_aran_flame_wreath_is_active(PlayerbotAI* botAI)
    {
        return new ShadeOfAranFlameWreathIsActiveTrigger(botAI);
    }
    static Trigger* shade_of_aran_conjured_elementals_summoned(PlayerbotAI* botAI)
    {
        return new ShadeOfAranConjuredElementalsSummonedTrigger(botAI);
    }
    static Trigger* shade_of_aran_boss_casts_counterspell_nearby(PlayerbotAI* botAI)
    {
        return new ShadeOfAranBossCastsCounterspellNearbyTrigger(botAI);
    }

    // Netherspite
    static Trigger* netherspite_red_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteRedBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_blue_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteBlueBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_green_beam_is_active(PlayerbotAI* botAI)
    {
        return new NetherspiteGreenBeamIsActiveTrigger(botAI);
    }
    static Trigger* netherspite_bot_is_not_beam_blocker(PlayerbotAI* botAI)
    {
        return new NetherspiteBotIsNotBeamBlockerTrigger(botAI);
    }
    static Trigger* netherspite_boss_is_banished(PlayerbotAI* botAI)
    {
        return new NetherspiteBossIsBanishedTrigger(botAI);
    }
    static Trigger* netherspite_need_to_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NetherspiteShouldManageTimersAndTrackersTrigger(botAI);
    }

    // Prince Malchezaar
    static Trigger* prince_malchezaar_bot_is_enfeebled(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarBotIsEnfeebledTrigger(botAI);
    }
    static Trigger* prince_malchezaar_engaged_by_non_tanks(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarEngagedByNonTanksTrigger(botAI);
    }
    static Trigger* prince_malchezaar_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new PrinceMalchezaarBossEngagedByTanksTrigger(botAI);
    }

    // Nightbane
    static Trigger* nightbane_boss_engaged_by_tanks(PlayerbotAI* botAI)
    {
        return new NightbaneBossEngagedByTanksTrigger(botAI);
    }
    static Trigger* nightbane_ranged_bots_are_in_charred_earth(PlayerbotAI* botAI)
    {
        return new NightbaneGroundPhaseEngagedByRangedTrigger(botAI);
    }
    static Trigger* nightbane_pets_ignore_collision_to_chase_flying_boss(PlayerbotAI* botAI)
    {
        return new NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger(botAI);
    }
    static Trigger* nightbane_boss_is_flying(PlayerbotAI* botAI)
    {
        return new NightbaneBossIsFlyingTrigger(botAI);
    }
    static Trigger* nightbane_bot_went_out_of_bounds(PlayerbotAI* botAI)
    {
        return new NightbaneBotWentOutOfBoundsTrigger(botAI);
    }
    static Trigger* nightbane_need_to_manage_timers_and_trackers(PlayerbotAI* botAI)
    {
        return new NightbaneShouldManageTimersAndTrackersTrigger(botAI);
    }
};

#endif
