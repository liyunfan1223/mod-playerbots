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
        // Shared
        creators["gruul's lair no encounter in progress"] =
            &RaidGruulsLairTriggerContext::gruuls_lair_no_encounter_in_progress;

        // High King Maulgar <Lord of the Ogres>
        creators["high king maulgar three ogres need melee tanks"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_three_ogres_need_melee_tanks;

        creators["high king maulgar krosh needs mage tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_krosh_needs_mage_tank;

        creators["high king maulgar kiggler needs moonkin tank"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_kiggler_needs_moonkin_tank;

        creators["high king maulgar should assign dps priority"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_should_assign_dps_priority;

        creators["high king maulgar channeling whirlwind"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_channeling_whirlwind;

        creators["high king maulgar should stand back from krosh"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_should_stand_back_from_krosh;

        creators["high king maulgar wild fel stalker spawned"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_wild_fel_stalker_spawned;

        creators["high king maulgar hunter should misdirect"] =
            &RaidGruulsLairTriggerContext::high_king_maulgar_hunter_should_misdirect;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller should be tanked"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_should_be_tanked;

        creators["gruul the dragonkiller ranged should spread"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_ranged_should_spread;

        creators["gruul the dragonkiller bot in cave in"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_bot_in_cave_in;

        creators["gruul the dragonkiller incoming shatter"] =
            &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_incoming_shatter;
    }

private:
    // Shared
    static Trigger* gruuls_lair_no_encounter_in_progress(PlayerbotAI* botAI)
    {
        return new GruulsLairNoEncounterInProgressTrigger(botAI);
    }

    // High King Maulgar <Lord of the Ogres>
    static Trigger* high_king_maulgar_three_ogres_need_melee_tanks(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(botAI);
    }
    static Trigger* high_king_maulgar_krosh_needs_mage_tank(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarKroshNeedsMageTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_kiggler_needs_moonkin_tank(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarKigglerNeedsMoonkinTankTrigger(botAI);
    }
    static Trigger* high_king_maulgar_should_assign_dps_priority(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarShouldAssignDpsPriorityTrigger(botAI);
    }
    static Trigger* high_king_maulgar_channeling_whirlwind(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarChannelingWhirlwindTrigger(botAI);
    }
    static Trigger* high_king_maulgar_should_stand_back_from_krosh(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarShouldStandBackFromKroshTrigger(botAI);
    }
    static Trigger* high_king_maulgar_wild_fel_stalker_spawned(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarWildFelStalkerSpawnedTrigger(botAI);
    }
    static Trigger* high_king_maulgar_hunter_should_misdirect(PlayerbotAI* botAI)
    {
        return new HighKingMaulgarHunterShouldMisdirectTrigger(botAI);
    }

    // Gruul the Dragonkiller
    static Trigger* gruul_the_dragonkiller_should_be_tanked(PlayerbotAI* botAI)
    {
        return new GruulTheDragonkillerShouldBeTankedTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_ranged_should_spread(PlayerbotAI* botAI)
    {
        return new GruulTheDragonkillerRangedShouldSpreadTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_bot_in_cave_in(PlayerbotAI* botAI)
    {
        return new GruulTheDragonkillerBotInCaveInTrigger(botAI);
    }
    static Trigger* gruul_the_dragonkiller_incoming_shatter(PlayerbotAI* botAI)
    {
        return new GruulTheDragonkillerIncomingShatterTrigger(botAI);
    }
};

#endif
