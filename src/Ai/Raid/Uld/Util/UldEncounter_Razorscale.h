/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERRAZORSCALE_H
#define PLAYERBOTS_ULDENCOUNTERRAZORSCALE_H

#include "AiObject.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "UldData.h"

#include <array>
#include <cstddef>
#include <ctime>
#include <vector>

class GameObject;
class Player;
class PlayerbotAI;
class Unit;

class RazorscaleBossHelper : public AiObject
{
public:
    enum RazorscaleUnits : uint32
    {
        UNIT_RAZORSCALE          = 33186,
        UNIT_DARK_RUNE_SENTINEL  = 33846,
        UNIT_DARK_RUNE_WATCHER   = 33453,
        UNIT_DARK_RUNE_GUARDIAN  = 33388,
        UNIT_DEVOURING_FLAME     = 34188,
    };

    enum RazorscaleGameObjects : uint32
    {
        GO_RAZORSCALE_HARPOON_1 = 194519,
        GO_RAZORSCALE_HARPOON_2 = 194541,
        GO_RAZORSCALE_HARPOON_3 = 194542,
        GO_RAZORSCALE_HARPOON_4 = 194543,
    };

    enum RazorscaleSpells : uint32
    {
        SPELL_SENTINEL_WHIRLWIND = 63806,
        SPELL_STUN_AURA         = 62794,
        SPELL_FUSE_ARMOR        = 64821
    };

    static constexpr uint32 FUSEARMOR_THRESHOLD = 2;

    // The Devouring Flame stalker's tick (64704 / 64733) carries radius index 8. The clear radius adds
    // the margin a step needs to actually leave the patch rather than stopping on its edge.
    static constexpr float DEVOURING_FLAME_RADIUS = 5.0f;
    static constexpr float DEVOURING_FLAME_CLEAR_RADIUS = DEVOURING_FLAME_RADIUS + 2.0f;

    // The main tank's clear while she is airborne, so he can hold the Dark Rune adds away from the patches.
    static constexpr float DEVOURING_FLAME_AIRBORNE_TANK_SCALE = 2.3f;

    static constexpr float SENTINEL_AVOID_RADIUS = 8.0f;

    static constexpr float RAZORSCALE_FLYING_Z_THRESHOLD = 440.0f;
    static constexpr float RAZORSCALE_ARENA_CENTER_X = 587.54f;
    static constexpr float RAZORSCALE_ARENA_CENTER_Y = -175.04f;
    static constexpr float RAZORSCALE_ARENA_RADIUS = 30.0f;
    // A bot this far past the arena edge walks back in before anything else, to this far inside it.
    static constexpr float RAZORSCALE_ARENA_LEASH_MARGIN = 25.0f;
    static constexpr float RAZORSCALE_ARENA_RETURN_INSET = 10.0f;

    // Under this she stays on the ground for good. Above it a landing is only a harpoon knockdown.
    static constexpr float RAZORSCALE_PERMANENT_GROUND_HEALTH_PCT = 50.0f;

    // Her first landing spot. Ranged stack 20 yd off centre on Y for that one, so everyone stands behind her.
    static constexpr float RAZORSCALE_FIRST_LANDING_X = 588.0f;
    static constexpr float RAZORSCALE_FIRST_LANDING_Y = -166.0f;
    static constexpr float RAZORSCALE_FIRST_LANDING_Z = 391.1f;
    static constexpr float RAZORSCALE_FIRST_LANDING_TOLERANCE = 2.0f;
    static constexpr float RAZORSCALE_FIRST_LANDING_Z_TOLERANCE = 1.0f;
    static constexpr float RAZORSCALE_FIRST_LANDING_STACK_OFFSET_Y = 20.0f;
    static constexpr float RAZORSCALE_FIRST_LANDING_STACK_RADIUS = 18.0f;
    static constexpr float RAZORSCALE_RANGED_STACK_RADIUS = 12.0f;

    // Assist tank indexes that cover for a human main tank.
    static constexpr int RAZORSCALE_ASSIST_TANK_SLOTS = 3;

    // Flame Breath's frontal cone, and how far out a bot still counts as in it.
    static constexpr float FLAME_BREATH_CONE = static_cast<float>(M_PI) / 2.0f;
    static constexpr float FLAME_BREATH_RANGE = 40.0f;

    // Harpoon cooldown (seconds)
    static constexpr time_t HARPOON_COOLDOWN_DURATION = 5;

    static constexpr float HARPOON_SEARCH_RANGE = 200.0f;
    static constexpr std::size_t HARPOON_ENTRY_COUNT = 4;

    struct HarpoonData
    {
        uint32 gameObjectEntry;
    };

    explicit RazorscaleBossHelper(PlayerbotAI* botAI)
        : AiObject(botAI), _boss(nullptr) {}

    bool UpdateBossAI();
    Unit* GetBoss() const;

    bool IsGroundPhase() const;
    bool IsFlyingPhase() const;

    // Same phase reads off a boss the caller already holds, for code that must not run UpdateBossAI()
    // first: it reassigns the raid's tank roles as a side effect.
    static bool IsGroundPhaseFor(Unit* boss);
    static bool IsFlyingPhaseFor(Unit* boss);

    // Nearest live Devouring Flame patch within radius of the bot, or nullptr.
    static Unit* FindDevouringFlameNear(PlayerbotAI* botAI, float radius);

    // Centres of the live patches within radius of the bot. The dodge sweep tests every candidate
    // destination against the same set, and one grid search beats one per candidate.
    static void CollectDevouringFlames(Player* bot, float radius, std::vector<Position>& out);

    // True when one of the collected patches covers (x, y).
    static bool DevouringFlameBlocks(std::vector<Position> const& flames, float x, float y);

    // True when a Devouring Flame patch covers (x, y). Searched around the bot, so the radius has to
    // reach a destination he is not standing on yet as well as the patch's own reach around it.
    static bool DevouringFlameBlocks(Player* bot, float x, float y);

    static bool IsHarpoonReady(GameObject* harpoonGO);
    static void SetHarpoonOnCooldown(GameObject* harpoonGO);
    GameObject* FindNearestHarpoon(float x, float y, float z) const;

    static std::vector<HarpoonData> const& GetHarpoonData();

    void AssignRolesBasedOnHealth();
    bool AreRolesAssigned() const;
    bool CanSwapRoles() const;

private:
    Unit* _boss;

    // Seconds between role reassignments, tracked per bot.
    static constexpr std::time_t _roleSwapCooldown = 10;
};

// Per-bot cache of the lookups every Razorscale node repeats each tick, keyed on getMSTime() since a bot
// never ticks twice in one ms. Holds guids so a despawn between reads drops out instead of dangling.
class RazorscaleScan
{
public:
    explicit RazorscaleScan(PlayerbotAI* botAI) : botAI(botAI) {}

    // "find target" razorscale. Null until she has this bot on her threat list.
    Unit* Boss();

    // "nearest hostile npcs" and "possible targets no los", exactly as the values return them.
    GuidVector const& Hostiles();
    GuidVector const& PossibleTargets();

    // What FindNearestGameObject(entry, HARPOON_SEARCH_RANGE) returns for each harpoon entry, in
    // GetHarpoonData() order, off one grid visit. Empty guid where there is none.
    std::array<ObjectGuid, RazorscaleBossHelper::HARPOON_ENTRY_COUNT> const& NearestHarpoons();

private:
    PlayerbotAI* botAI;

    uint32 bossAtMs = 0;
    ObjectGuid boss;
    uint32 hostilesAtMs = 0;
    GuidVector hostiles;
    uint32 targetsAtMs = 0;
    GuidVector targets;
    uint32 harpoonsAtMs = 0;
    std::array<ObjectGuid, RazorscaleBossHelper::HARPOON_ENTRY_COUNT> harpoons;
};

// This bot's RazorscaleScan, held by the "razorscale scan" value.
RazorscaleScan& GetRazorscaleScan(PlayerbotAI* botAI);

// Only a ranged dps ever fires a harpoon. IsRanged() is true for healers too, hence the IsHeal test.
bool IsRazorscaleHarpoonCrew(PlayerbotAI* botAI, Player* bot);

// Closest harpoon to the bot (2D) that is built, unfired and off the local cooldown. A tie goes to
// the earlier GetHarpoonData() entry. Null when none is ready.
GameObject* GetRazorscaleClosestReadyHarpoon(PlayerbotAI* botAI);

// Most urgent add first: Sentinel (whirlwinds the raid) > Watcher (ranged caster) > Guardian. Lowest
// health Sentinel of two, so the raid finishes one instead of splitting across both.
Unit* GetRazorscaleAddKillTarget(PlayerbotAI* botAI);

// She sits on her first landing spot, where the ranged stack shifts off centre.
bool IsRazorscaleAtFirstLanding(Unit* boss);

// Skull target: the boss whenever she's on the floor, harpoon knockdowns included since she takes
// damage then. Otherwise the add above.
Unit* GetRazorscaleKillTarget(PlayerbotAI* botAI);

#endif
