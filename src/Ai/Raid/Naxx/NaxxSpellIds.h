/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXXSPELLIDS_H
#define PLAYERBOTS_NAXXSPELLIDS_H

#include "PlayerbotAI.h"
#include <initializer_list>

// use src/server/scripts/Northrend/Naxxramas/naxxramas.h for CreatureId, NaxxramasSay, NaxxramasEvent, NaxxramasMisc
namespace NaxxSpellIds
{
    // Heigan
    static constexpr uint32 Eruption10 = 29371;      // cast by the floor gameobjects, not by the boss
    static constexpr uint32 PlagueCloud = 29350;     // 45s self-channel while Heigan is on his platform (fast dance)
    static constexpr uint32 TeleportSelf = 30211;
/*
    SPELL_SPELL_DISRUPTION          = 29310,
    SPELL_DECREPIT_FEVER            = 29998,
    SPELL_PLAGUE_CLOUD              = 29350,
    SPELL_TELEPORT_SELF             = 30211
*/

    // Grobbulus
    static constexpr uint32 PoisonCloud = 28240;

    // Thaddius polarity
    static constexpr uint32 PositiveCharge10 = 28059;
    static constexpr uint32 PositiveCharge25 = 28062;
    static constexpr uint32 PositiveChargeStack = 29659;
    static constexpr uint32 NegativeCharge10 = 28084;
    static constexpr uint32 NegativeCharge25 = 28085;
    static constexpr uint32 NegativeChargeStack = 29660;
/*
    SPELL_MAGNETIC_PULL                 = 28337,
    SPELL_TESLA_SHOCK                   = 28099,
    SPELL_SHOCK_VISUAL                  = 28159,

    // Stalagg
    SPELL_POWER_SURGE                   = 54529,
    SPELL_STALAGG_CHAIN                 = 28096,

    // Feugen
    SPELL_STATIC_FIELD                  = 28135,
    SPELL_FEUGEN_CHAIN                  = 28111,

    // Thaddius
    SPELL_POLARITY_SHIFT                = 28089,
    SPELL_BALL_LIGHTNING                = 28299,
    SPELL_CHAIN_LIGHTNING               = 28167,
    SPELL_BERSERK                       = 27680,
    SPELL_THADDIUS_VISUAL_LIGHTNING     = 28136,
    SPELL_THADDIUS_SPAWN_STUN           = 28160,

    SPELL_POSITIVE_CHARGE               = 28062,
    SPELL_POSITIVE_CHARGE_STACK         = 29659,
    SPELL_NEGATIVE_CHARGE               = 28085,
    SPELL_NEGATIVE_CHARGE_STACK         = 29660,
    SPELL_POSITIVE_POLARITY             = 28059,
    SPELL_NEGATIVE_POLARITY             = 28084
*/
    // Sapphiron
    static constexpr uint32 Icebolt10 = 28522;
    static constexpr uint32 Icebolt25 = 28526;
    static constexpr uint32 Chill10 = 28547;
    static constexpr uint32 Chill25 = 55699;
/*
    // Fight
    SPELL_FROST_AURA                = 28531,
    SPELL_CLEAVE                    = 19983,
    SPELL_TAIL_SWEEP                = 55697,
    SPELL_SUMMON_BLIZZARD           = 28560,
    SPELL_LIFE_DRAIN                = 28542,
    SPELL_BERSERK                   = 26662,

    // Ice block
    SPELL_ICEBOLT_CAST              = 28526,
    SPELL_ICEBOLT_TRIGGER           = 28522,
    SPELL_FROST_MISSILE             = 30101,
    SPELL_FROST_EXPLOSION           = 28524,

    // Visuals
    SPELL_SAPPHIRON_DIES            = 29357
*/
    // Gluth
    static constexpr uint32 Decimate10 = 28374;
    static constexpr uint32 Decimate25 = 54426;
    static constexpr uint32 Decimate25Alt = 28375;
    static constexpr uint32 MortalWound10 = 25646;
    static constexpr uint32 MortalWound25 = 54378;
/*
    SPELL_MORTAL_WOUND                  = 25646,
    SPELL_ENRAGE                        = 28371,
    SPELL_DECIMATE                      = 28374,
    SPELL_DECIMATE_DAMAGE               = 28375,
    SPELL_BERSERK                       = 26662,
    SPELL_INFECTED_WOUND                = 29306,
    SPELL_CHOW_SEARCHER                 = 28404
*/
    // Anub'Rekhan
    static constexpr uint32 LocustSwarm10 = 28785;
    static constexpr uint32 LocustSwarm10Alt = 28786;
    static constexpr uint32 LocustSwarm25 = 54021;  // 25-man Locust Swarm
/*
    SPELL_IMPALE                    = 28783,
    SPELL_LOCUST_SWARM              = 28785,
    SPELL_SUMMON_CORPSE_SCARABS_5   = 29105,
    SPELL_SUMMON_CORPSE_SCARABS_10  = 28864,
    SPELL_BERSERK                   = 26662
    ACHIEV_TIMED_START_EVENT        = 9891,
    EVENT_SPAWN_CRYPT_GUARDS_1      = 0,
    EVENT_BERSERK                   = 1,
    ////
    Position const cryptguardPositions[] = {
    { 3299.732f, -3502.489f, 287.077f, 2.378f },
    { 3299.086f, -3450.929f, 287.077f, 3.999f },
    { 3331.217f, -3476.607f, 287.074f, 3.269f }
};

*/
    // Loatheb
    static constexpr uint32 NecroticAura10 = 55593;
/*
    SPELL_NECROTIC_AURA                         = 55593,
    SPELL_SUMMON_SPORE                          = 29234,
    SPELL_DEATHBLOOM                            = 29865,
    SPELL_INEVITABLE_DOOM                       = 29204,
    SPELL_BERSERK                               = 26662
*/
    inline bool HasAnyAura(Unit* unit, std::initializer_list<uint32> spellIds)
    {
        if (!unit)
            return false;

        for (uint32 spellId : spellIds)
        {
            if (unit->HasAura(spellId))
                return true;
        }
        return false;
    }

    inline Aura* GetAnyAura(Unit* unit, std::initializer_list<uint32> spellIds)
    {
        if (!unit)
            return nullptr;

        for (uint32 spellId : spellIds)
        {
            if (Aura* aura = unit->GetAura(spellId))
                return aura;
        }
        return nullptr;
    }

    inline bool MatchesAnySpellId(SpellInfo const* info, std::initializer_list<uint32> spellIds)
    {
        if (!info)
            return false;

        for (uint32 spellId : spellIds)
        {
            if (info->Id == spellId)
                return true;
        }
        return false;
    }
}  // namespace NaxxSpellIds

#endif
