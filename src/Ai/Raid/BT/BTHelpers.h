/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTHELPERS_H
#define PLAYERBOTS_BTHELPERS_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace BlackTempleHelpers
{

enum class BlackTempleSpells : uint32
{
    // High Warlord Naj'entus
    SPELL_IMPALING_SPINE            = 39837,
    SPELL_TIDAL_SHIELD              = 39872,

    // Supremus
    SPELL_SNARE_SELF                = 41922,

    // Teron Gorefiend
    SPELL_SHADOW_OF_DEATH           = 40251,
    SPELL_SPIRITUAL_VENGEANCE       = 40268,

    SPELL_SPIRIT_LANCE              = 40157,
    SPELL_SPIRIT_CHAINS             = 40175,
    SPELL_SPIRIT_VOLLEY             = 40314,
    SPELL_SPIRIT_STRIKE             = 40325,

    // Gurtogg Bloodboil
    SPELL_BOSS_FEL_RAGE             = 40594,
    SPELL_PLAYER_FEL_RAGE           = 40604,
    SPELL_BLOODBOIL                 = 42005,

    // Reliquary of Souls
    SPELL_DEADEN                    = 41410,
    SPELL_RUNE_SHIELD               = 41431,

    // Mother Shahraz
    SPELL_FATAL_ATTRACTION          = 41001,

    // Gathios the Shatterer
    SPELL_BLESSING_OF_PROTECTION    = 41450,
    SPELL_BLESSING_OF_SPELL_WARDING = 41451,
    SPELL_JUDGEMENT                 = 41467,
    SPELL_SEAL_OF_COMMAND           = 41469,
    SPELL_CONSECRATION              = 41541,

    // Veras Darkshadow
    SPELL_VANISH                    = 41476,

    // High Nethermancer Zerevor
    SPELL_DAMPEN_MAGIC              = 41478,
    SPELL_FLAMESTRIKE               = 41481,
    SPELL_BLIZZARD                  = 41482,

    // Illidan Stormrage <The Betrayer>
    SPELL_DEMON_TRANSFORM_1         = 40511,
    SPELL_DEMON_TRANSFORM_2         = 40398,
    SPELL_DEMON_TRANSFORM_3         = 40510,
    SPELL_DEMON_FORM                = 40506,
    SPELL_DARK_BARRAGE              = 40585,
    SPELL_SHADOW_PRISON             = 40647,
    SPELL_CAGED                     = 40695,
    SPELL_PARASITIC_SHADOWFIEND_1   = 41917, // cast by Illidan (primary infection)
    SPELL_PARASITIC_SHADOWFIEND_2   = 41914, // cast by Shadowfiend on contact (secondary infection)

    // Hunter
    SPELL_FROST_TRAP                = 13809,
    SPELL_MISDIRECTION              = 35079,

    // Shaman
    SPELL_EARTHBIND_TOTEM           = 2484,
};

enum class BlackTempleNpcs : uint32
{
    // Supremus
    NPC_SUPREMUS_VOLCANO      = 23085,

    // Shade of Akama
    NPC_ASHTONGUE_CHANNELER   = 23421,

    // Teron Gorefiend
    NPC_SHADOWY_CONSTRUCT     = 23111,

    // Illidan Stormrage <The Betrayer>
    NPC_FLAME_OF_AZZINOTH     = 22997,
    NPC_DEMON_FIRE            = 23069,
    NPC_ILLIDAN_DB_TARGET     = 23070,
    NPC_BLAZE                 = 23259,
    NPC_FLAME_CRASH           = 23336,
    NPC_SHADOW_DEMON          = 23375,
    NPC_PARASITIC_SHADOWFIEND = 23498,
};

enum class BlackTempleItems : uint32
{
    // High Warlord Naj'entus
    ITEM_NAJENTUS_SPINE = 32408,
};

enum class BlackTempleObjects : uint32
{
    // High Warlord Naj'entus
    GO_NAJENTUS_SPINE = 185584,

    // Illidan Stormrage <The Betrayer>
    GO_SHADOW_TRAP    = 185916,
};

enum class TankPositionState : uint8
{
    MovingToTransition = 0,
    MovingToFinal      = 1,
    Positioned         = 2,
    Unknown            = 255,
};

constexpr uint32 BLACK_TEMPLE_MAP_ID = 564;

// High Warlord Naj'entus
extern const Position NAJENTUS_TANK_POSITION;

// Supremus
extern std::unordered_map<uint32, time_t> supremusPhaseTimer;
bool HasSupremusVolcanoNearby(PlayerbotAI* botAI, Player* bot);

// Shade of Akama
extern const Position AKAMA_CHANNELER_POSITION;
extern std::unordered_set<ObjectGuid> hasReachedAkamaChannelerPosition;

// Teron Gorefiend
extern const Position GOREFIEND_TANK_POSITION;
extern const Position GOREFIEND_DIE_POSITION;

// Gurtogg Bloodboil
extern const Position GURTOGG_TANK_POSITION;
extern const Position GURTOGG_RANGED_POSITION;
extern const Position GURTOGG_SOAKER_POSITION;
extern std::unordered_map<uint32, time_t> gurtoggPhaseTimer;
std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot);
int GetGurtoggActiveRotationGroup(Unit* gurtogg);

// Mother Shahraz
extern const Position SHAHRAZ_TANK_POSITION;
extern const Position SHAHRAZ_TRANSITION_POSITION;
extern const Position SHAHRAZ_RANGED_POSITION;
extern std::unordered_map<ObjectGuid, TankPositionState> shahrazTankStep;
TankPositionState GetShahrazTankPositionState(Player* bot);

// Illidari Council
constexpr float COUNCIL_FLOOR_Z_THRESHOLD = 270.000f;
extern const std::array<Position, 4> GATHIOS_TANK_POSITIONS;
extern const Position MALANDE_TANK_POSITION;
extern const Position ZEREVOR_TANK_POSITION;
extern const std::array<Position, 2> ZEREVOR_HEALER_POSITIONS;
extern std::unordered_map<uint32, time_t> councilDpsWaitTimer;
extern std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
extern std::unordered_map<ObjectGuid, uint8> zerevorHealStep;
Player* GetZerevorMageTank(Player* bot);
bool HasDangerousCouncilAura(Unit* unit);

// Illidan Stormrage <The Betrayer>
extern const Position ILLIDAN_LANDING_POSITION;
extern const Position ILLIDAN_N_GRATE_POSITION;
extern const Position ILLIDAN_E_GRATE_POSITION;
extern const Position ILLIDAN_W_GRATE_POSITION;
extern const std::array<Position, 3> GRATE_POSITIONS;
extern const Position ILLIDAN_E_GLAIVE_WAITING_POSITION;
extern const std::array<Position, 7> E_GLAIVE_TANK_POSITIONS;
extern const Position ILLIDAN_W_GLAIVE_WAITING_POSITION;
extern const std::array<Position, 7> W_GLAIVE_TANK_POSITIONS;
extern std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
extern std::unordered_map<ObjectGuid, ObjectGuid> illidanShadowTrapGuid;
extern std::unordered_map<ObjectGuid, Position> illidanShadowTrapDestination;
extern std::unordered_map<uint32, int> illidanLastPhase;
extern std::unordered_map<uint32, time_t> illidanBossDpsWaitTimer;
extern std::unordered_map<uint32, time_t> illidanFlameDpsWaitTimer;
extern std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
extern std::unordered_map<uint32, ObjectGuid> westFlameGuid;
int GetIllidanPhase(Unit* illidan);
std::vector<Unit*> GetAllFlameCrashes(Player* bot);
std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot);
Player* GetIllidanWarlockTank(Player* bot);
bool HasParasiticShadowfiend(Player* member);
Player* GetIllidanTrapperHunter(Player* bot);
Player* GetBotWithParasiticShadowfiend(Player* bot);
struct EyeBlastDangerArea
{
    Position start;
    Position end;
    float width;
};
EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot);
bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area);
GameObject* FindNearestTrap(PlayerbotAI* botAI, Player* bot);

}

#endif
