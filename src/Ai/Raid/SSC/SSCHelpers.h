/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCHELPERS_H
#define PLAYERBOTS_SSCHELPERS_H

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"
#include <ctime>
#include <unordered_map>

namespace SerpentShrineCavernHelpers
{
    enum SerpentShrineCavernSpells
    {
        // Trash Mobs
        SPELL_TOXIC_POOL             = 38718,

        // Hydross the Unstable <Duke of Currents>
        SPELL_MARK_OF_HYDROSS_10     = 38215,
        SPELL_MARK_OF_HYDROSS_25     = 38216,
        SPELL_MARK_OF_HYDROSS_50     = 38217,
        SPELL_MARK_OF_HYDROSS_100    = 38218,
        SPELL_MARK_OF_HYDROSS_250    = 38231,
        SPELL_MARK_OF_HYDROSS_500    = 40584,
        SPELL_MARK_OF_CORRUPTION_10  = 38219,
        SPELL_MARK_OF_CORRUPTION_25  = 38220,
        SPELL_MARK_OF_CORRUPTION_50  = 38221,
        SPELL_MARK_OF_CORRUPTION_100 = 38222,
        SPELL_MARK_OF_CORRUPTION_250 = 38230,
        SPELL_MARK_OF_CORRUPTION_500 = 40583,
        SPELL_CORRUPTION             = 37961,

        // The Lurker Below
        SPELL_SPOUT_VISUAL = 37431,

        // Leotheras the Blind
        SPELL_LEOTHERAS_BANISHED     = 37546,
        SPELL_WHIRLWIND              = 37640,
        SPELL_WHIRLWIND_CHANNEL      = 37641,
        SPELL_METAMORPHOSIS          = 37673,
        SPELL_CHAOS_BLAST            = 37675,
        SPELL_INSIDIOUS_WHISPER      = 37676,

        // Lady Vashj <Coilfang Matron>
        SPELL_FEAR_WARD              =  6346,
        SPELL_MAGIC_BARRIER          = 38112,
        SPELL_POISON_BOLT            = 38253,
        SPELL_STATIC_CHARGE          = 38280,
        SPELL_ENTANGLE               = 38316,

        // Druid
        SPELL_CAT_FORM               =   768,
        SPELL_BEAR_FORM              =  5487,
        SPELL_DIRE_BEAR_FORM         =  9634,
        SPELL_TREE_OF_LIFE           = 33891,

        // Hunter
        SPELL_MISDIRECTION           = 35079,

        // Mage
        SPELL_SLOW                   = 31589,

        // Shaman
        SPELL_GROUNDING_TOTEM_EFFECT =  8178,

        // Warlock
        SPELL_CURSE_OF_EXHAUSTION    = 18223,
    };

    enum SerpentShrineCavernNPCs
    {
        // Trash Mobs
        NPC_WATER_ELEMENTAL_TOTEM    = 22236,

        // Hydross the Unstable <Duke of Currents>
        NPC_PURE_SPAWN_OF_HYDROSS    = 22035,
        NPC_TAINTED_SPAWN_OF_HYDROSS = 22036,

        // The Lurker Below
        NPC_COILFANG_GUARDIAN        = 21873,

        // Leotheras the Blind
        NPC_LEOTHERAS_THE_BLIND      = 21215,
        NPC_GREYHEART_SPELLBINDER    = 21806,
        NPC_INNER_DEMON              = 21857,
        NPC_SHADOW_OF_LEOTHERAS      = 21875,

        // Fathom-Lord Karathress
        NPC_SPITFIRE_TOTEM           = 22091,

        // Lady Vashj <Coilfang Matron>
        NPC_WORLD_INVISIBLE_TRIGGER  = 12999,
        NPC_LADY_VASHJ               = 21212,
        NPC_ENCHANTED_ELEMENTAL      = 21958,
        NPC_TAINTED_ELEMENTAL        = 22009,
        NPC_COILFANG_ELITE           = 22055,
        NPC_COILFANG_STRIDER         = 22056,
        NPC_TOXIC_SPOREBAT           = 22140,
        NPC_SPORE_DROP_TRIGGER       = 22207,
    };

    enum SerpentShrineCavernItems
    {
        // Lady Vashj <Coilfang Matron>
        ITEM_TAINTED_CORE            = 31088,
    };

    constexpr uint32 SSC_MAP_ID = 548;

    // Hydross the Unstable <Duke of Currents>
    extern const Position HYDROSS_FROST_TANK_POSITION;
    extern const Position HYDROSS_NATURE_TANK_POSITION;
    extern std::unordered_map<uint32, time_t> hydrossFrostDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> hydrossNatureDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> hydrossChangeToFrostPhaseTimer;
    extern std::unordered_map<uint32, time_t> hydrossChangeToNaturePhaseTimer;
    bool HasMarkOfHydrossAt100Percent(Player* bot);
    bool HasNoMarkOfHydross(Player* bot);
    bool HasMarkOfCorruptionAt100Percent(Player* bot);
    bool HasNoMarkOfCorruption(Player* bot);

    // The Lurker Below
    extern const Position LURKER_MAIN_TANK_POSITION;
    extern std::unordered_map<uint32, time_t> lurkerSpoutTimer;
    extern std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;
    bool IsLurkerCastingSpout(Unit* lurker);

    // Leotheras the Blind
    extern std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    extern std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;
    Unit* GetLeotherasHuman(Player* bot);
    Unit* GetPhase2LeotherasDemon(Player* bot);
    Unit* GetPhase3LeotherasDemon(Player* bot);
    Unit* GetActiveLeotherasDemon(Player* bot);
    Player* GetLeotherasDemonFormTank(Player* bot);

    // Fathom-Lord Karathress
    extern const Position KARATHRESS_TANK_POSITION;
    extern const Position TIDALVESS_TANK_POSITION;
    extern const Position SHARKKIS_TANK_POSITION;
    extern const Position CARIBDIS_TANK_POSITION;
    extern const Position CARIBDIS_HEALER_POSITION;
    extern const Position CARIBDIS_RANGED_DPS_POSITION;
    extern std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    // Morogrim Tidewalker
    extern const Position TIDEWALKER_PHASE_1_TANK_POSITION;
    extern const Position TIDEWALKER_PHASE_TRANSITION_WAYPOINT;
    extern const Position TIDEWALKER_PHASE_2_TANK_POSITION;
    extern const Position TIDEWALKER_PHASE_2_RANGED_POSITION;
    extern std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    extern std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    // Lady Vashj <Coilfang Matron>
    constexpr float VASHJ_PLATFORM_CENTER_Z = 42.902f;
    constexpr float VASHJ_PLATFORM_EDGE_Z = 41.097f;
    extern const Position VASHJ_PLATFORM_CENTER_POSITION;
    extern std::unordered_map<ObjectGuid, bool> hasReachedVashjRangedPosition;
    extern std::unordered_map<uint32, ObjectGuid> nearestTriggerGuid;
    extern std::unordered_map<ObjectGuid, Position> intendedLineup;
    extern std::unordered_map<uint32, time_t> lastImbueAttempt;
    extern std::unordered_map<ObjectGuid, time_t> lastCoreInInventoryTime;
    bool IsMainTankInSameSubgroup(Player* bot);
    bool IsLadyVashjInPhase1(PlayerbotAI* botAI);
    bool IsLadyVashjInPhase2(PlayerbotAI* botAI);
    bool IsLadyVashjInPhase3(PlayerbotAI* botAI);
    bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* botAI);
    Player* GetDesignatedCoreLooter(PlayerbotAI* botAI, Player* bot);
    Player* GetFirstTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
    Player* GetSecondTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
    Player* GetThirdTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
    Player* GetFourthTaintedCorePasser(PlayerbotAI* botAI, Player* bot);
    std::array<Player*, 5> GetCoreHandlers(PlayerbotAI* botAI, Player* bot);
    bool AnyRecentCoreInInventory(PlayerbotAI* botAI, Player* bot);
    struct GeneratorInfo { ObjectGuid guid; float x, y, z; };
    extern const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS;
    std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(
        Map* map, std::vector<uint32> const& generatorDbGuids);
    Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Unit* reference);
    GeneratorInfo const* GetNearestGeneratorToBot(
        Player* bot, std::vector<GeneratorInfo> const& generators);
}

#endif
