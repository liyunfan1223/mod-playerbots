/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAMPTRIGGERCONTEXT_H
#define PLAYERBOTS_RAMPTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "RampTriggers.h"

class TbcDungeonHellfireRampartsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonHellfireRampartsTriggerContext()
    {
        // Watchkeeper Gargolmar
        creators["gargolmar hellfire watchers are active"] =
            &TbcDungeonHellfireRampartsTriggerContext::gargolmar_hellfire_watchers_are_active;

        // Omor the Unscarred
        creators["omor treachery aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_treachery_aura;

        creators["omor tank has treachery aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_tank_has_treachery_aura;

        creators["omor ranged spread"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_ranged_spread;

        creators["omor fiendish hound is active"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_fiendish_hound_is_active;

        // Vazruden & Nazan
        creators["vazruden tank position boss"] =
            &TbcDungeonHellfireRampartsTriggerContext::vazruden_tank_position_boss;

        creators["vazruden boss is active"] =
            &TbcDungeonHellfireRampartsTriggerContext::vazruden_boss_is_active;

        creators["nazan boss tremor totem"] =
            &TbcDungeonHellfireRampartsTriggerContext::nazan_boss_tremor_totem;

        creators["nazan boss fire resistance totem"] =
            &TbcDungeonHellfireRampartsTriggerContext::nazan_boss_fire_resistance_totem;

        creators["nazan boss fire resistance aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::nazan_boss_fire_resistance_aura;
    }

private:
    // Watchkeeper Gargolmar
    static Trigger* gargolmar_hellfire_watchers_are_active(PlayerbotAI* botAI)
    {
        return new GargolmarHellfireWatchersAreActiveTrigger(botAI);
    }

    // Omor the Unscarred
    static Trigger* omor_treachery_aura(PlayerbotAI* botAI)
    {
        return new OmorTreacheryAuraTrigger(botAI);
    }

    static Trigger* omor_tank_has_treachery_aura(PlayerbotAI* botAI)
    {
        return new OmorTankHasTreacheryAuraTrigger(botAI);
    }

    static Trigger* omor_ranged_spread(PlayerbotAI* botAI)
    {
        return new OmorRangedSpreadTrigger(botAI);
    }

    static Trigger* omor_fiendish_hound_is_active(PlayerbotAI* botAI)
    {
        return new OmorFiendishHoundIsActiveTrigger(botAI);
    }

    // Vazruden & Nazan
    static Trigger* vazruden_tank_position_boss(PlayerbotAI* botAI)
    {
        return new VazrudenTankPositionBossTrigger(botAI);
    }

    static Trigger* vazruden_boss_is_active(PlayerbotAI* botAI)
    {
        return new VazrudenBossIsActiveTrigger(botAI);
    }

    static Trigger* nazan_boss_tremor_totem(PlayerbotAI* botAI)
    {
        return new NazanBossTremorTotemTrigger(botAI);
    }

    static Trigger* nazan_boss_fire_resistance_totem(PlayerbotAI* botAI)
    {
        return new NazanBossFireResistanceTotemTrigger(botAI);
    }

    static Trigger* nazan_boss_fire_resistance_aura(PlayerbotAI* botAI)
    {
        return new NazanBossFireResistanceAuraTrigger(botAI);
    }
};

#endif
