#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include "AiObject.h"
#include "Playerbots.h"
#include "Position.h"

enum KarazhanSpells
{
    // Maiden of Virtue
    SPELL_REPENTANCE                 = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD     = 30756,

    // Shade of Aran
    SPELL_FLAME_WREATH               = 30004,
    SPELL_AURA_FLAME_WREATH          = 29946,
    SPELL_ARCANE_EXPLOSION           = 29973,
    SPELL_WARLOCK_BANISH             = 18647, // Rank 2

    // Netherspite
    SPELL_GREEN_BEAM_DEBUFF          = 30422,
    SPELL_BLUE_BEAM_DEBUFF           = 30423,
    SPELL_NETHER_EXHAUSTION_RED      = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN    = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE     = 38639,
    SPELL_NETHERSPITE_BANISHED       = 39833,

    // Prince Malchezaar
    SPELL_ENFEEBLE                   = 30843,
};

enum KarazhanNpcs
{
    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

    // Terestian Illhoof
    NPC_KILREK                       = 17229,
    NPC_DEMON_CHAINS                 = 17248,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL           = 17167,

    // Netherspite
    NPC_VOID_ZONE                    = 16697,
    NPC_RED_PORTAL                   = 17369,
    NPC_BLUE_PORTAL                  = 17368,
    NPC_GREEN_PORTAL                 = 17367,

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL         = 17646,
};

extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION;
extern const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8];
extern const Position KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION;
extern const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4];
extern const Position KARAZHAN_THE_CURATOR_BOSS_POSITION;

class RaidKarazhanHelpers : public AiObject
{
public:
    explicit RaidKarazhanHelpers(PlayerbotAI* botAI) : AiObject(botAI) {}

    void MarkTargetWithSkull(Unit* /*target*/);
    Unit* GetFirstAliveUnit(const std::vector<Unit*>& /*units*/);
    Unit* GetFirstAliveUnitByEntry(uint32 /*entry*/);
    Unit* GetNearestPlayerInRadius(float /*radius*/ = 5.0f);
    bool IsFlameWreathActive();
    Position GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss);
    std::vector<Player*> GetRedBlockers();
    std::vector<Player*> GetBlueBlockers();
    std::vector<Player*> GetGreenBlockers();
    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers();
    std::vector<Unit*> GetAllVoidZones();
    bool IsSafePosition (float x, float y, float z,
         const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetSpawnedInfernals() const;
    bool IsStraightPathSafe(const Position& start, const Position& target, 
         const std::vector<Unit*>& hazards, float hazardRadius, float stepSize);
};

#endif
