#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include "AiObject.h"
#include "Playerbots.h"
#include "MovementActions.h"

enum KarazhanSpells
{
    // Maiden of Virtue
    SPELL_REPENTANCE                = 29511,

    // Opera Event
    SPELL_LITTLE_RED_RIDING_HOOD    = 30756,
    SPELL_FEAR                      = 6215,  // Rank 3

    // Shade of Aran
    SPELL_FLAME_WREATH_CAST         = 30004,
    SPELL_FLAME_WREATH_AURA         = 29946,
    SPELL_ARCANE_EXPLOSION          = 29973,
    SPELL_WARLOCK_BANISH            = 18647, // Rank 2

    // Netherspite
    SPELL_GREEN_BEAM_DEBUFF         = 30422,
    SPELL_BLUE_BEAM_DEBUFF          = 30423,
    SPELL_NETHER_EXHAUSTION_RED     = 38637,
    SPELL_NETHER_EXHAUSTION_GREEN   = 38638,
    SPELL_NETHER_EXHAUSTION_BLUE    = 38639,
    SPELL_NETHERSPITE_BANISHED      = 39833,

    // Prince Malchezaar
    SPELL_ENFEEBLE                  = 30843,

    // Nightbane
    SPELL_CHARRED_EARTH             = 30129
};

enum KarazhanNpcs
{
    // Attumen the Huntsman
    NPC_ATTUMEN_THE_HUNTSMAN        = 16152, // Mounted ID

    // Terestian Illhoof
    NPC_KILREK                      = 17229,
    NPC_DEMON_CHAINS                = 17248,

    // Shade of Aran
    NPC_CONJURED_ELEMENTAL          = 17167,

    // Netherspite
    NPC_VOID_ZONE                   = 16697,
    NPC_RED_PORTAL                  = 17369,
    NPC_BLUE_PORTAL                 = 17368,
    NPC_GREEN_PORTAL                = 17367,

    // Prince Malchezaar
    NPC_NETHERSPITE_INFERNAL        = 17646,

    // Nightbane
    NPC_RESTLESS_SKELETON           = 17261,
};

const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION = Position(-10945.881f, -2103.7817f, 92.71163f);
const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8] =
{
    { -10931.178f, -2116.58f, 92.1787f },
    { -10925.828f, -2102.425f, 92.18016f },
    { -10933.089f, -2088.5017f, 92.18028f },
    { -10947.59f, -2082.8147f, 92.18024f },
    { -10960.912f, -2090.4368f, 92.17964f },
    { -10966.017f, -2105.288f, 92.17582f },
    { -10959.242f, -2119.6172f, 92.18062f },
    { -10944.495f, -2123.857f, 92.18021f },
};

const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4] = {
    { -10913.391f, -1773.5083f, 90.47706f },
    { -10875.456f, -1779.0358f, 90.47706f },
    { -10872.281f, -1751.6376f, 90.47716f },
    { -10910.492f, -1747.401f, 90.477165f },
};

const Position KARAZHAN_THE_CURATOR_BOSS_POSITION = Position(-11139.463f, -1884.6451f, 165.76564f);

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
    std::vector<Player*> GetBlueBlockers();
    std::vector<Player*> GetGreenBlockers();
    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers();
    std::vector<Unit*> GetAllVoidZones();
    bool IsSafePosition (float x, float y, float z,
         const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetSpawnedInfernals() const;
};

#endif
