/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPSHARED_H
#define PLAYERBOTS_SWPSHARED_H

#include "Common.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <type_traits>

class Player;
class PlayerbotAI;
class Unit;

namespace SwpHelpers
{

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

enum class SwpSpells : uint32
{
    // Trash - Apocalypse Guard
    SPELL_INFERNAL_DEFENSE             = 46287,

    // Kalecgos
    SPELL_SPECTRAL_EXHAUSTION          = 44867,
    SPELL_SPECTRAL_BLAST_PORTAL        = 44866,
    SPELL_ARCANE_BUFFET                = 45018,
    SPELL_CURSE_OF_BOUNDLESS_AGONY     = 45032,
    SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC = 45034,
    SPELL_TELEPORT_SPECTRAL            = 46019,
    SPELL_SPECTRAL_REALM               = 46021,

    // Brutallus
    SPELL_METEOR_SLASH                 = 45150, // 120° cone
    SPELL_BURN                         = 46394, // Spread radius is 2y, no combat reaches added

    // Felmyst
    SPELL_SUMMON_DEMONIC_VAPOR         = 45391,
    SPELL_ENCAPSULATE                  = 45661,
    SPELL_GAS_NOVA                     = 45855,
    SPELL_FELMYST_SPEED_BURST          = 45495,
    SPELL_FOG_OF_CORRUPTION_CHARM      = 45717,

    // Eredar Twins
    SPELL_BLAZE                        = 45235,
    SPELL_CONFLAGRATION                = 45342,
    SPELL_FLAME_TOUCHED                = 45348,
    SPELL_FLAME_SEAR                   = 46771,

    // M'uru
    SPELL_DARKNESS                     = 45996,
    SPELL_DARKNESS_PRE_EFFECT          = 45999,
    SPELL_ENTROPIUS_DARKNESS           = 46269,
    SPELL_SHADOW_BOLT_VOLLEY           = 46082,
    SPELL_FEL_FIREBALL                 = 46101,
    SPELL_SPELL_FURY                   = 46102,
    SPELL_FLURRY                       = 46160,

    // Kil'jaeden <The Deceiver>
    SPELL_FIRE_BLOOM                   = 45641,
    SPELL_SHIELD_OF_THE_BLUE           = 45848,
    SPELL_DRAGON_BREATH_HASTE          = 45856,
    SPELL_DRAGON_BREATH_REVITALIZE     = 45860,
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT = 45839,
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS = 46605,
    SPELL_SHADOW_SPIKE                 = 46680,

    // Hunter
    SPELL_MISDIRECTION                 = 34477,
    SPELL_MISDIRECTION_AURA            = 35079, // the aura on the hunter

    // Mage
    SPELL_SPELLSTEAL                   = 30449,

    // Priest
    SPELL_DISPEL_MAGIC_RANK_1          = 527,
    SPELL_SHADOWFORM                   = 15473,
    SPELL_MASS_DISPEL                  = 32375,

    // Shaman
    SPELL_PURGE_RANK_1                 = 370,

    // Warlock
    SPELL_METAMORPHOSIS                = 47241,
};

enum class SwpNpcs : uint32
{
    // Trash
    NPC_APOCALYPSE_GUARD         = 25593,
    NPC_VOLATILE_FIEND           = 25851,

    // Kalecgos
    NPC_KALECGOS_DRAGON          = 24850,
    NPC_KALECGOS_HUMANOID        = 24891,

    // Brutallus
    NPC_BRUTALLUS                = 24882,

    // Felmyst
    NPC_FELMYST                  = 25038,
    NPC_DEMONIC_VAPOR            = 25265, // The vapor "head" that chases a player
    NPC_DEMONIC_VAPOR_TRAIL      = 25267,

    // Eredar Twins
    NPC_LADY_SACROLASH           = 25165,
    NPC_GRAND_WARLOCK_ALYTHESS   = 25166,

    // M'uru
    NPC_MURU                     = 25741,
    NPC_VOID_SENTINEL            = 25772,
    NPC_DARK_FIEND               = 25744,
    NPC_DARKNESS                 = 25879,
    NPC_SHADOWSWORD_BERSERKER    = 25798,
    NPC_SHADOWSWORD_FURY_MAGE    = 25799,
    NPC_VOID_SPAWN               = 25824,
    NPC_ENTROPIUS                = 25840,
    NPC_SINGULARITY              = 25855,

    // Kil'jaeden <The Deceiver>
    NPC_KILJAEDEN                = 25315,
    NPC_SHIELD_ORB               = 25502,
    NPC_HAND_OF_THE_DECEIVER     = 25588,
    NPC_POWER_OF_THE_BLUE_FLIGHT = 25653,
    NPC_SINISTER_REFLECTION      = 25708,
    NPC_ARMAGEDDON_TARGET        = 25735,
};

enum class SwpObjects : uint32
{
    // Kalecgos
    GO_SPECTRAL_RIFT = 187055,

    // Eredar Twins
    GO_BLAZE         = 187366,

    // Kil'jaeden <The Deceiver>
    GO_DRAGON_ORB_1  = 187869,
    GO_DRAGON_ORB_2  = 188114,
    GO_DRAGON_ORB_3  = 188115,
    GO_DRAGON_ORB_4  = 188116,
};

inline constexpr uint32 SWP_MAP_ID = 580;

// Steps short enough to navigate poor terrain, matching the standard in EncounterHelpers.
inline constexpr float PATH_STEP_DISTANCE = 3.5f;
inline constexpr float PATH_BACKWARD_STEP_DISTANCE = 2.25f;

// Ability reaches from SpellRange.dbc (MaxRangeHostile). _REACH is the distance from the caster to
// a target; _RADIUS is the area around the caster. Both are the raw dbc figures. A single-target
// cast counts both combat reaches, so using unmodified _REACH is conservative.
inline constexpr float MELEE_ABILITY_REACH = 5.0f;
inline constexpr float RANGED_ABILITY_REACH = 30.0f;
inline constexpr float HAMMER_OF_JUSTICE_REACH = 10.0f;
inline constexpr float ICY_TOUCH_REACH = 20.0f;
inline constexpr float CHARGE_REACH = 25.0f;
inline constexpr float WIND_SHEAR_REACH = 25.0f;
inline constexpr float SILENCING_SHOT_REACH = 35.0f;
inline constexpr float CONSECRATION_RADIUS = 8.0f;
inline constexpr float SHOCKWAVE_RADIUS = 10.0f;

// Radius for War Stomp (20549) and all 3 Arcane Torrent variants.
inline constexpr float SELF_AOE_RACIAL_RADIUS = 8.0f;
// Radius for Challenging Shout and Challenging Roar.
inline constexpr float TAUNT_SHOUT_RADIUS = 10.0f;

inline constexpr uint32 VOLATILE_FIEND_CACHE_INTERVAL_MS = 200;
// Felfire Fission (45779), the fiend's death explosion, hits within 10y and just murders melee
// bots (and me). This distance is farther since the fiends are running toward the raid.
inline constexpr float VOLATILE_FIEND_SAFE_DISTANCE = 20.0f;
// Don't try to reach targets if within this distance of a fiend. Works fine in practice since the
// gauntlet is always going forwards so nobody needs to go the other way to reach a target.
inline constexpr float VOLATILE_FIEND_APPROACH_SUPPRESSION_RADIUS = 25.0f;
inline constexpr float VOLATILE_FIEND_SEARCH_RADIUS =
    VOLATILE_FIEND_APPROACH_SUPPRESSION_RADIUS + 2.0f;
ObjectGuid FindSwpVolatileFiendGuid(Player* bot);
// Offset from the center of an arc for assigning positioning slots, filling outward and
// alternating sides. Slot 0 is the center when the count is odd and straddles it when even.
float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth);
// The minimum interval between spells cast by a charmed creature. This is in effect a manually
// enforced cooldown because the path being used for the spellcast skips cooldowns.
uint32 GetManualCastCooldown(uint32 spellId);
// Same as above, except for enforcing a GCD for abilities with no cooldowns.
uint32 GetManualCastGlobalCooldown(uint32 spellId);
// Misdirection on the tank, then Steady Shot on the target while it is up, to spend it.
bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank);
// One step along the bot's path to a point, stopping short of it by stopDistance. The step
// follows the path corner-by-corner, rather than aiming at the far end of it, so a bot can move
// around an obstacle between it and the point.
bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY);
// GetStepToPosition() from EncounterHelpers, but each step follows the path. Pass the unit being
// tanked as facing to walk the step backwards when it leads away from that unit. Returns false
// once the bot is within arrivalDist or when there is no path. Pass the bot's Z to MoveTo().
bool GetPathStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards);

}

#endif
