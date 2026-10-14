/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULACTIONCONTEXT_H
#define PLAYERBOTS_GRUULACTIONCONTEXT_H

#include "GruulActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        // General
        creators["gruul's lair reset encounter states"] =
            &RaidGruulsLairActionContext::gruuls_lair_reset_encounter_states;

        // High King Maulgar <Lord of the Ogres>
        creators["high king maulgar melee tanks position bosses"] =
            &RaidGruulsLairActionContext::high_king_maulgar_melee_tanks_position_bosses;

        creators["high king maulgar mage tank attack krosh"] =
            &RaidGruulsLairActionContext::high_king_maulgar_mage_tank_attack_krosh;

        creators["high king maulgar moonkin tank attack kiggler"] =
            &RaidGruulsLairActionContext::high_king_maulgar_moonkin_tank_attack_kiggler;

        creators["high king maulgar assign dps priority"] =
            &RaidGruulsLairActionContext::high_king_maulgar_assign_dps_priority;

        creators["high king maulgar run away from whirlwind"] =
            &RaidGruulsLairActionContext::high_king_maulgar_run_away_from_whirlwind;

        creators["high king maulgar back away from krosh"] =
            &RaidGruulsLairActionContext::high_king_maulgar_back_away_from_krosh;

        creators["high king maulgar banish fel stalker"] =
            &RaidGruulsLairActionContext::high_king_maulgar_banish_fel_stalker;

        creators["high king maulgar misdirect ogres to tanks"] =
            &RaidGruulsLairActionContext::high_king_maulgar_misdirect_ogres_to_tanks;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller tanks position boss"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_tanks_position_boss;

        creators["gruul the dragonkiller spread ranged"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_spread_ranged;

        creators["gruul the dragonkiller get out of cave in"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_get_out_of_cave_in;

        creators["gruul the dragonkiller shatter spread"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_shatter_spread;
    }

private:
    // General
    static Action* gruuls_lair_reset_encounter_states(PlayerbotAI* botAI) {
        return new GruulsLairResetEncounterStatesAction(botAI);
    }

    // High King Maulgar
    static Action* high_king_maulgar_melee_tanks_position_bosses(PlayerbotAI* botAI) {
        return new HighKingMaulgarMeleeTanksPositionBossesAction(botAI);
    }
    static Action* high_king_maulgar_mage_tank_attack_krosh(PlayerbotAI* botAI) {
        return new HighKingMaulgarMageTankAttackKroshAction(botAI);
    }
    static Action* high_king_maulgar_moonkin_tank_attack_kiggler(PlayerbotAI* botAI) {
        return new HighKingMaulgarMoonkinTankAttackKigglerAction(botAI);
    }
    static Action* high_king_maulgar_assign_dps_priority(PlayerbotAI* botAI) {
        return new HighKingMaulgarAssignDpsPriorityAction(botAI);
    }
    static Action* high_king_maulgar_run_away_from_whirlwind(PlayerbotAI* botAI) {
        return new HighKingMaulgarRunAwayFromWhirlwindAction(botAI);
    }
    static Action* high_king_maulgar_back_away_from_krosh(PlayerbotAI* botAI) {
        return new HighKingMaulgarBackAwayFromKroshAction(botAI);
    }
    static Action* high_king_maulgar_banish_fel_stalker(PlayerbotAI* botAI) {
        return new HighKingMaulgarBanishFelStalkerAction(botAI);
    }
    static Action* high_king_maulgar_misdirect_ogres_to_tanks(PlayerbotAI* botAI) {
        return new HighKingMaulgarMisdirectOgresToTanksAction(botAI);
    }

    // Gruul the Dragonkiller
    static Action* gruul_the_dragonkiller_tanks_position_boss(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerTanksPositionBossAction(botAI);
    }
    static Action* gruul_the_dragonkiller_spread_ranged(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerSpreadRangedAction(botAI);
    }
    static Action* gruul_the_dragonkiller_get_out_of_cave_in(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerGetOutOfCaveInAction(botAI);
    }
    static Action* gruul_the_dragonkiller_shatter_spread(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerShatterSpreadAction(botAI);
    }
};

#endif
