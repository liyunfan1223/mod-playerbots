/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPACTIONCONTEXT_H
#define PLAYERBOTS_RAMPACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "RampActions.h"

class TbcDungeonHellfireRampartsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonHellfireRampartsActionContext()
    {
        // Watchkeeper Gargolmar
        creators["gargolmar mark hellfire watchers"] =
            &TbcDungeonHellfireRampartsActionContext::gargolmar_mark_hellfire_watchers;

        // Omor the Unscarred
        creators["omor ranged spread"] =
            &TbcDungeonHellfireRampartsActionContext::omor_ranged_spread;

        creators["omor treachery aura flee from players"] =
            &TbcDungeonHellfireRampartsActionContext::omor_treachery_aura_flee_from_players;

        creators["omor treachery aura flee from tank"] =
            &TbcDungeonHellfireRampartsActionContext::omor_treachery_aura_flee_from_tank;

        creators["omor mark fiendish hound"] =
            &TbcDungeonHellfireRampartsActionContext::omor_mark_fiendish_hound;

        // Vazruden & Nazan
        creators["vazruden tank position boss"] =
            &TbcDungeonHellfireRampartsActionContext::vazruden_tank_position_boss;

        creators["vazruden mark boss"] =
            &TbcDungeonHellfireRampartsActionContext::vazruden_mark_boss;

        creators["nazan set tremor totem"] =
            &TbcDungeonHellfireRampartsActionContext::nazan_set_tremor_totem;

        creators["nazan set fire resistance totem"] =
            &TbcDungeonHellfireRampartsActionContext::nazan_set_fire_resistance_totem;

        creators["nazan set fire resistance aura"] =
            &TbcDungeonHellfireRampartsActionContext::nazan_set_fire_resistance_aura;
    }

private:
    // Watchkeeper Gargolmar
    static Action* gargolmar_mark_hellfire_watchers(PlayerbotAI* botAI) {
        return new GargolmarMarkHellfireWatchersAction(botAI);
    }

    // Omor the Unscarred
    static Action* omor_ranged_spread(PlayerbotAI* botAI) {
        return new OmorRangedSpreadAction(botAI);
    }

    static Action* omor_treachery_aura_flee_from_players(PlayerbotAI* botAI) {
        return new OmorTreacheryAuraFleeFromPlayersAction(botAI);
    }

    static Action* omor_treachery_aura_flee_from_tank(PlayerbotAI* botAI) {
        return new OmorTreacheryAuraFleeFromTankAction(botAI);
    }

    static Action* omor_mark_fiendish_hound(PlayerbotAI* botAI) {
        return new OmorMarkFiendishHoundAction(botAI);
    }

    // Vazruden & Nazan
    static Action* vazruden_tank_position_boss(PlayerbotAI* botAI) {
        return new VazrudenTankPositionBossAction(botAI);
    }

    static Action* vazruden_mark_boss(PlayerbotAI* botAI) {
        return new VazrudenMarkBossAction(botAI);
    }

    static Action* nazan_set_tremor_totem(PlayerbotAI* botAI) {
        return new NazanSetTremorTotemAction(botAI);
    }

    static Action* nazan_set_fire_resistance_totem(PlayerbotAI* botAI) {
        return new NazanSetFireResistanceTotemAction(botAI);
    }

    static Action* nazan_set_fire_resistance_aura(PlayerbotAI* botAI) {
        return new NazanSetFireResistanceAuraAction(botAI);
    }
};

#endif
