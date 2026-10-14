/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULTRIGGERCONTEXT_H
#define PLAYERBOTS_GRUULTRIGGERCONTEXT_H

#include "GruulTriggers.h"
#include "NamedObjectContext.h"

class RaidGruulsLairTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidGruulsLairTriggerContext() : NamedObjectContext<Trigger>()
    {
        // General
        creators["gruul's lair no encounter in progress"] =
            &RaidGruulsLairTriggerContext::gruuls_lair_no_encounter_in_progress;

        // High King Maulgar <Lord of the Ogres>
        creators["high king maulgar three ogres need melee tanks"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_three_ogres_need_melee_tanks;

        creators["high king maulgar krosh needs mage tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_krosh_needs_mage_tank;

        creators["high king maulgar kiggler needs moonkin tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_kiggler_needs_moonkin_tank;

        creators["high king maulgar determining kill order"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_determining_kill_order;

        creators["high king maulgar channeling whirlwind"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_channeling_whirlwind;

        creators["high king maulgar should stand back from krosh"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_should_stand_back_from_krosh;

        creators["high king maulgar wild fel stalker spawned"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_wild_fel_stalker_spawned;

        creators["high king maulgar pulling ogre council"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_pulling_ogre_council;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller should be tanked"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_should_be_tanked;

        creators["gruul the dragonkiller ranged should spread"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_ranged_should_spread;

        creators["gruul the dragonkiller in cave in"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_in_cave_in;

        creators["gruul the dragonkiller incoming shatter"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_incoming_shatter;
    }

private:
    // General
    static Trigger* gruuls_lair_no_encounter_in_progress(PlayerbotAI* botAI) {
        return new GruulsLairNoEncounterInProgressTrigger(botAI);
    }

    // High King Maulgar <Lord of the Ogres>
    static Trigger* high_king_maulgar_three_ogres_need_melee_tanks(PlayerbotAI* botAI) {
        return new HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(botAI);
    }
    static Trigger* high_king_maulgar_krosh_needs_mage_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKroshNeedsMageTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_kiggler_needs_moonkin_tank(PlayerbotAI* botAI) {
        return new HighKingMaulgarKigglerNeedsMoonkinTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_determining_kill_order(PlayerbotAI* botAI) {
        return new HighKingMaulgarDeterminingKillOrderTrigger(botAI);
    }
    static Trigger* high_king_maulgar_channeling_whirlwind(PlayerbotAI* botAI) {
        return new HighKingMaulgarChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* high_king_maulgar_should_stand_back_from_krosh(PlayerbotAI* botAI) {
        return new HighKingMaulgarShouldStandBackFromKroshTrigger(botAI);
    }
    static Trigger* high_king_maulgar_wild_fel_stalker_spawned(PlayerbotAI* botAI) {
        return new HighKingMaulgarWildFelStalkerSpawnedTrigger(botAI);
    }
    static Trigger* high_king_maulgar_pulling_ogre_council(PlayerbotAI* botAI) {
        return new HighKingMaulgarPullingOgreCouncilTrigger(botAI);
    }

    // Gruul the Dragonkiller
    static Trigger* gruul_the_dragonkiller_should_be_tanked(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerShouldBeTankedTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_ranged_should_spread(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_in_cave_in(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerInCaveInTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_incoming_shatter(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerIncomingShatterTrigger(botAI);
    }
};

#endif
