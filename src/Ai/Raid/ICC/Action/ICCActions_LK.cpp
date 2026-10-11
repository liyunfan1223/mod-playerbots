/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCScripts.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

static bool IsLkShambling(uint32 entry)
{
    return entry == NPC_SHAMBLING_HORROR1 || entry == NPC_SHAMBLING_HORROR2 ||
           entry == NPC_SHAMBLING_HORROR3 || entry == NPC_SHAMBLING_HORROR4;
}

static bool IsLkRagingSpirit(uint32 entry)
{
    return entry == NPC_RAGING_SPIRIT1 || entry == NPC_RAGING_SPIRIT2 ||
           entry == NPC_RAGING_SPIRIT3 || entry == NPC_RAGING_SPIRIT4;
}

static bool IsLkValkyr(Unit* unit)
{
    if (!unit)
        return false;

    uint32 const entry = unit->GetEntry();
    return entry == NPC_VALKYR_SHADOWGUARD1 || entry == NPC_VALKYR_SHADOWGUARD2 ||
           entry == NPC_VALKYR_SHADOWGUARD3 || entry == NPC_VALKYR_SHADOWGUARD4;
}

static bool IsLkVileSpirit(Unit* unit)
{
    if (!unit)
        return false;

    uint32 const entry = unit->GetEntry();
    return entry == NPC_VILE_SPIRIT1 || entry == NPC_VILE_SPIRIT2 ||
           entry == NPC_VILE_SPIRIT3 || entry == NPC_VILE_SPIRIT4;
}

static bool IsLkWickedSpirit(uint32 entry)
{
    return entry == NPC_WICKED_SPIRIT1 || entry == NPC_WICKED_SPIRIT2 ||
           entry == NPC_WICKED_SPIRIT3 || entry == NPC_WICKED_SPIRIT4;
}

static bool IsIceSphere(uint32 entry)
{
    return entry == NPC_ICE_SPHERE1 || entry == NPC_ICE_SPHERE2 ||
           entry == NPC_ICE_SPHERE3 || entry == NPC_ICE_SPHERE4;
}

static bool IsLkCollectibleAdd(Unit* unit)
{
    if (!unit || !unit->IsAlive())
        return false;

    uint32 const entry = unit->GetEntry();
    return IsLkShambling(entry) || IsLkRagingSpirit(entry) ||
           entry == NPC_DRUDGE_GHOUL1 || entry == NPC_DRUDGE_GHOUL2 ||
           entry == NPC_DRUDGE_GHOUL3 || entry == NPC_DRUDGE_GHOUL4;
}

static bool IsHeroicLk(Difficulty diff)
{
    return diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC;
}

static float GetDefileEffectiveRadius(Unit const* defile, Difficulty diff)
{
    static constexpr float BASE_RADIUS = 6.0f;

    for (uint32 const auraId : DEFILE_AURAS)
    {
        Aura const* grow = defile->GetAura(auraId);
        if (!grow)
            continue;

        float const multiplier =
            (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_10MAN_NORMAL) ? 1.4f : 0.95f;

        return BASE_RADIUS + grow->GetStackAmount() * multiplier;
    }

    return BASE_RADIUS;
}

// Compute the centroid of alive group members for a stable reference point.
// Unlike a single reference player, the centroid barely shifts when one
// player dies, preventing the raid from oscillating between positions.
static Position ComputeGroupCentroid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    float sumX = 0.0f;
    float sumY = 0.0f;
    int count = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld())
            continue;

        sumX += member->GetPositionX();
        sumY += member->GetPositionY();
        ++count;
    }

    if (count == 0)
        return Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    return Position(sumX / count, sumY / count, bot->GetPositionZ());
}

static Position const& SelectClosestOf3(Position const& ref, Position const& p1, Position const& p2, Position const& p3)
{
    float const dx1 = ref.GetPositionX() - p1.GetPositionX();
    float const dy1 = ref.GetPositionY() - p1.GetPositionY();
    float const d1 = dx1 * dx1 + dy1 * dy1;

    float const dx2 = ref.GetPositionX() - p2.GetPositionX();
    float const dy2 = ref.GetPositionY() - p2.GetPositionY();
    float const d2 = dx2 * dx2 + dy2 * dy2;

    float const dx3 = ref.GetPositionX() - p3.GetPositionX();
    float const dy3 = ref.GetPositionY() - p3.GetPositionY();
    float const d3 = dx3 * dx3 + dy3 * dy3;

    if (d2 < d1 && d2 < d3)
        return p2;
    if (d3 < d1 && d3 < d2)
        return p3;
    return p1;
}

// AoE taunt — returns true if a spell was cast
static bool CastAoeTaunt(PlayerbotAI* botAI, Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            if (botAI->CastSpell("challenging shout", bot))
                return true;
            break;
        case CLASS_DRUID:
            if (botAI->CastSpell("challenging roar", bot))
                return true;
            break;
        default:
            break;
    }
    return false;
}

bool IccLichKingShadowTrapAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss || !PlayerbotAI::IsTank(bot))
        return false;

    Difficulty const diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig.EnableICCBuffs && boss->IsInCombat() && IsHeroicLk(diff))
        IccApplyHeroicBuffToMember(bot, true, false);

    static constexpr float CIRCLE_RADIUS = 20.0f;
    static constexpr float SAFE_DISTANCE = 12.0f;
    static constexpr float SEARCH_DISTANCE = SAFE_DISTANCE + 5.0f;
    static constexpr int TEST_POSITIONS = 16;
    static constexpr float ANGLE_STEP = 2.0f * float(M_PI) / TEST_POSITIONS;

    float const centerX = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionX();
    float const centerY = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionY();
    float const centerZ = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionZ();

    // Collect shadow traps within search range
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> nearbyTraps;

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOW_TRAP)
            continue;

        if (bot->GetDistance(unit) < SEARCH_DISTANCE)
            nearbyTraps.push_back(unit);
    }

    if (nearbyTraps.empty())
        return false;

    // Check whether the current position is already safe
    auto isSafePosition = [&](float x, float y) -> bool
    {
        for (Unit const* trap : nearbyTraps)
        {
            if (trap->GetDistance2d(x, y) < SAFE_DISTANCE)
                return false;
        }
        return true;
    };

    if (isSafePosition(bot->GetPositionX(), bot->GetPositionY()))
        return false;

    // Walk clockwise around the centre circle to find the nearest safe spot
    float const currentAngle = std::atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);

    for (int i = 1; i <= TEST_POSITIONS + 1; ++i)
    {
        // Last iteration tests the diametrically opposite point as a fallback
        float const testAngle = (i <= TEST_POSITIONS)
            ? currentAngle - ANGLE_STEP * i
            : currentAngle + float(M_PI);

        float testX = centerX + std::cos(testAngle) * CIRCLE_RADIUS;
        float testY = centerY + std::sin(testAngle) * CIRCLE_RADIUS;
        float testZ = centerZ;

        bot->UpdateAllowedPositionZ(testX, testY, testZ);

        if (!bot->IsWithinLOS(testX, testY, testZ))
            continue;

        if (!isSafePosition(testX, testY))
            continue;

        return MoveTo(bot->GetMapId(), testX, testY, testZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IccLichKingNecroticPlagueAction::Execute(Event /*event*/)
{
    if (!botAI->HasAura("Necrotic Plague", bot))
        return false;

    static constexpr float DELIVER_RANGE = 3.0f;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Prefer shamblings tanked by the assist tank so plague is delivered
    // to the correct add pile. Fall back to nearest if none qualify.
    Unit* closestHorror = nullptr;
    float minDist = 150.0f;
    Unit* fallbackHorror = nullptr;
    float fallbackDist = 150.0f;

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!IsLkShambling(unit->GetEntry()))
            continue;

        float const dist = bot->GetDistance(unit);

        Unit* victim = unit->GetVictim();
        if (victim && PlayerbotAI::IsAssistTank(victim->ToPlayer()))
        {
            if (dist < minDist)
            {
                minDist = dist;
                closestHorror = unit;
            }
        }
        else if (dist < fallbackDist)
        {
            fallbackDist = dist;
            fallbackHorror = unit;
        }
    }

    if (!closestHorror)
    {
        closestHorror = fallbackHorror;
        minDist = fallbackDist;
    }

    // No alive Horror — stop moving so healers can dispel immediately.
    // Keeping the plague active with no valid target risks a raid wipe.
    if (!closestHorror)
    {
        bot->StopMoving();
        bot->AttackStop();
        bot->SetTarget(ObjectGuid::Empty);
        return true;
    }

    // Already in delivery range — hold position for dispel
    if (minDist <= DELIVER_RANGE)
    {
        bot->StopMoving();
        return true;
    }

    // Move toward the Horror, suppressing combat so we don't get sidetracked
    bot->AttackStop();
    bot->SetTarget(ObjectGuid::Empty);

    // During winter (boss <= 71% HP) approach from behind the shambling to
    // avoid its frontal Shockwave.
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    bool const duringWinter = boss && !boss->HealthAbovePct(71);

    if (duringWinter)
    {
        Unit* victim = closestHorror->GetVictim();
        if (victim)
        {
            static constexpr float BEHIND_OFFSET = 2.0f;
            float const vecX = closestHorror->GetPositionX() - victim->GetPositionX();
            float const vecY = closestHorror->GetPositionY() - victim->GetPositionY();
            float const vecLen = std::hypot(vecX, vecY);

            if (vecLen > 0.1f)
            {
                float const dirX = vecX / vecLen;
                float const dirY = vecY / vecLen;

                // Try directly behind, then slight left/right arcs
                static constexpr std::array<float, 5> Arcs = {0.0f, 0.35f, -0.35f, 0.70f, -0.70f};

                for (float const arc : Arcs)
                {
                    float const cosA = std::cos(arc);
                    float const sinA = std::sin(arc);
                    float const rotX = dirX * cosA - dirY * sinA;
                    float const rotY = dirX * sinA + dirY * cosA;

                    float const destX = closestHorror->GetPositionX() + rotX * BEHIND_OFFSET;
                    float const destY = closestHorror->GetPositionY() + rotY * BEHIND_OFFSET;

                    if (bot->IsWithinLOS(destX, destY, closestHorror->GetPositionZ()))
                    {
                        return MoveTo(bot->GetMapId(), destX, destY, closestHorror->GetPositionZ(),
                                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
                    }
                }
            }
        }
    }

    return MoveTo(bot->GetMapId(), closestHorror->GetPositionX(), closestHorror->GetPositionY(),
                  closestHorror->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IccLichKingWinterAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    Difficulty const diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig.EnableICCBuffs && boss->IsInCombat() && IsHeroicLk(diff))
        IccApplyHeroicBuffToMember(bot, true, true);

    // Speed boost to help escape the inward push
    if (bot->GetDistance2d(boss) < 35.0f && !bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

    // Heroic: if more than 2 Raging Spirits are alive, kill the one with the
    // highest HP. Only the main tank does this to avoid simultaneous actions.
    if (PlayerbotAI::IsMainTank(bot) && IsHeroicLk(diff))
    {
        GuidVector const& heroicNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        std::vector<Unit*> spirits;

        for (ObjectGuid const& guid : heroicNpcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            if (IsLkRagingSpirit(unit->GetEntry()))
                spirits.push_back(unit);
        }

        if (spirits.size() > 2)
        {
            Unit* killTarget = *std::max_element(spirits.begin(), spirits.end(),
                [](Unit const* a, Unit const* b) { return a->GetHealth() < b->GetHealth(); });
            bot->Kill(bot, killTarget);
        }
    }

    // Skull marking during Remorseless Winter.
    // Priority: Shambling Horror > Raging Spirit (both on MT within 10 yd).
    // Only re-mark when the current skull target is dead/gone.
    if (Group* group = bot->GetGroup())
    {
        static constexpr float MARK_RANGE = 10.0f;

        Unit* currentSkull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
        bool const currentValid = currentSkull && currentSkull->IsAlive() && (IsLkShambling(currentSkull->GetEntry()) || IsLkRagingSpirit(currentSkull->GetEntry()));

        if (!currentValid)
        {
            Unit* mainTank = AI_VALUE(Unit*, "main tank");
            GuidVector const& winterNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");

            Unit* bestShambling = nullptr;
            Unit* bestSpirit    = nullptr;

            for (ObjectGuid const& guid : winterNpcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;

                if (!mainTank || unit->GetVictim() != mainTank)
                    continue;

                if (mainTank->GetDistance(unit) > MARK_RANGE)
                    continue;

                uint32 const entry = unit->GetEntry();
                if (!bestShambling && IsLkShambling(entry))
                    bestShambling = unit;
                else if (!bestSpirit && IsLkRagingSpirit(entry))
                    bestSpirit = unit;

                if (bestShambling)
                    break;
            }

            Unit* markTarget = bestShambling ? bestShambling : bestSpirit;

            if (markTarget)
                group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), markTarget->GetGUID());
            else if (!group->GetTargetIcon(RtiTargetValue::skullIndex).IsEmpty())
                group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), ObjectGuid::Empty);
        }
    }

    // Teleport back onto the platform if the engine pushed us through
    FixPlatformPosition();

    // Staging during Remorseless Winter cast: non-tanks converge on ONE shared
    // safe vile spirit anchor and hold for 4s before falling through to the
    // final-spot logic. The chosen anchor is locked-in on first detection of
    // the cast so all bots agree even if defile state shifts mid-cast.
    // Among safe slots VILE_SPIRIT1 and VILE_SPIRIT3, picks
    // the one closest to the group centroid; VILE_SPIRIT2 is fallback only.
    auto& s_winterStage = IcecrownHelpers::IccState(boss->GetInstanceId()).winterStage;
    ObjectGuid const winterKey = boss->GetGUID();

    bool const bossCastingWinter = IccBossCastingRemorselessWinter(boss);

    static constexpr uint32 STAGE_DURATION_MS = 4000;

    // Enter staging block if boss is casting Winter OR a staging entry exists
    // that hasn't yet expired. The cast ends before the aura applies, so we
    // can't rely on bossCastingWinter staying true for the full window.
    uint32 const now = getMSTime();
    auto winterIt = s_winterStage.find(winterKey);
    bool const stageActive = winterIt != s_winterStage.end() &&
                             getMSTimeDiff(winterIt->second.startMs, now) < STAGE_DURATION_MS;

    if (!PlayerbotAI::IsTank(bot) && (bossCastingWinter || stageActive))
    {
        if (winterIt == s_winterStage.end())
        {
            Position const* candidates[2] = {&ICC_LK_VILE_SPIRIT1_POSITION,
                                             &ICC_LK_VILE_SPIRIT3_POSITION};
            Position const centroid = ComputeGroupCentroid(bot);

            Position const* chosen = nullptr;
            float bestDist = std::numeric_limits<float>::max();
            for (Position const* slot : candidates)
            {
                if (!IsPositionSafeFromDefile(slot->GetPositionX(), slot->GetPositionY(),
                                              PLATFORM_Z, 3.0f))
                    continue;

                if (!IsPositionSafeFromShadowTraps(slot->GetPositionX(), slot->GetPositionY()))
                    continue;

                float const d = std::hypot(centroid.GetPositionX() - slot->GetPositionX(),
                                           centroid.GetPositionY() - slot->GetPositionY());
                if (d < bestDist)
                {
                    bestDist = d;
                    chosen = slot;
                }
            }

            if (!chosen &&
                IsPositionSafeFromDefile(ICC_LK_VILE_SPIRIT2_POSITION.GetPositionX(),
                                         ICC_LK_VILE_SPIRIT2_POSITION.GetPositionY(),
                                         PLATFORM_Z, 3.0f) &&
                IsPositionSafeFromShadowTraps(ICC_LK_VILE_SPIRIT2_POSITION.GetPositionX(),
                                              ICC_LK_VILE_SPIRIT2_POSITION.GetPositionY()))
                chosen = &ICC_LK_VILE_SPIRIT2_POSITION;

            winterIt = s_winterStage.emplace(winterKey, IcecrownHelpers::WinterStageState{now, chosen}).first;
        }

        uint32 const elapsed = getMSTimeDiff(winterIt->second.startMs, now);
        Position const* stagePos = winterIt->second.pos;

        if (stagePos && elapsed < STAGE_DURATION_MS)
        {
            static constexpr float STAGE_TOLERANCE = 3.0f;
            auto& s_stageInbound = IcecrownHelpers::IccState(bot->GetInstanceId()).winterStageInbound;
            ObjectGuid const stageKey = bot->GetGUID();
            float const dist = bot->GetDistance2d(stagePos->GetPositionX(), stagePos->GetPositionY());
            if (dist > STAGE_TOLERANCE)
            {
                if (!s_stageInbound[stageKey])
                {
                    bot->AttackStop();
                    bot->CastStop();
                    bot->SetTarget(ObjectGuid::Empty);
                    context->GetValue<Unit*>("current target")->Set(nullptr);
                    botAI->Reset();
                    s_stageInbound[stageKey] = true;
                }
                TryMoveToPosition(stagePos->GetPositionX(), stagePos->GetPositionY(), PLATFORM_Z, true);
            }
            else
            {
                s_stageInbound[stageKey] = false;
                bot->StopMoving();
            }
            return true;
        }
    }
    else if (!bossCastingWinter && !stageActive)
    {
        // Only clear when no staging window is active (timer expired AND not
        // re-casting). This keeps the staging entry alive across the brief
        // gap between cast end and aura application, and prevents tanks from
        // wiping the entry mid-staging (tanks always hit this branch).
        s_winterStage.erase(winterKey);
    }

    // Defile evacuation has absolute priority over everything else
    if (!IsPositionSafeFromDefile(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 3.0f))
    {
        Position const& safePos = PlayerbotAI::IsTank(bot)
            ? *GetMainTankPosition()
            : *GetMainTankRangedPosition();

        TryMoveToPosition(safePos.GetPositionX(), safePos.GetPositionY(), PLATFORM_Z, true);
        return true;
    }

    // If any ice sphere is targeting this bot, move to the midpoint between the
    // melee and ranged frost positions
    if (!PlayerbotAI::IsTank(bot))
    {
        GuidVector const& sphereNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (ObjectGuid const& guid : sphereNpcs)
        {
            Unit* sphere = botAI->GetUnit(guid);
            if (!sphere || !sphere->IsAlive() || !IsIceSphere(sphere->GetEntry()))
                continue;

            if (sphere->GetVictim() != bot)
                continue;

            Position const& meleePos  = *GetMainTankPosition();
            Position const& rangedPos = *GetMainTankRangedPosition();
            float const midX = (meleePos.GetPositionX() + rangedPos.GetPositionX()) * 0.5f;
            float const midY = (meleePos.GetPositionY() + rangedPos.GetPositionY()) * 0.5f;

            static constexpr float SPHERE_HOLD_TOLERANCE = 2.0f;
            if (bot->GetDistance2d(midX, midY) > SPHERE_HOLD_TOLERANCE)
                TryMoveToPosition(midX, midY, PLATFORM_Z, true);
            else
                bot->StopMoving();
            return false;
        }
    }

    ClearInvalidTarget();
    HandlePetManagement();

    if (PlayerbotAI::IsTank(bot))
        HandleTankPositioning();
    else if (!PlayerbotAI::IsRanged(bot))
    {
        // Non-tank melee:
        //   - MT at the frost anchor with an add (shambling/spirit) within
        //     5y of him: adds are in place, flank them.
        //   - Otherwise: wait at midpoint between melee and ranged anchors
        //     (the ice sphere hold spot), close enough to engage once adds
        //     settle, far enough not to eat frontals while MT drags them.
        // Never pulled to MT/melee anchor.
        Unit* mainTank = AI_VALUE(Unit*, "main tank");
        bool addNearTank = false;
        bool tankInPlace = false;

        if (mainTank && mainTank->IsAlive())
        {
            static constexpr float TANK_IN_PLACE_RANGE = 10.0f;
            Position const& frostPos = *GetMainTankPosition();
            tankInPlace = mainTank->GetDistance2d(frostPos.GetPositionX(),
                                                  frostPos.GetPositionY()) <= TANK_IN_PLACE_RANGE;

            GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (ObjectGuid const& guid : npcs)
            {
                Unit* add = botAI->GetUnit(guid);
                if (!add || !add->IsAlive())
                    continue;
                if (!IsLkShambling(add->GetEntry()) && !IsLkRagingSpirit(add->GetEntry()))
                    continue;
                if (add->GetDistance2d(mainTank) <= 5.0f)
                {
                    addNearTank = true;
                    break;
                }
            }
        }

        if (addNearTank && tankInPlace)
            HandleMeleePositioning();
        else
        {
            Position const& meleePos  = *GetMainTankPosition();
            Position const& rangedPos = *GetMainTankRangedPosition();
            float const midX = (meleePos.GetPositionX() + rangedPos.GetPositionX()) * 0.5f;
            float const midY = (meleePos.GetPositionY() + rangedPos.GetPositionY()) * 0.5f;

            static constexpr float MID_TOLERANCE = 2.0f;
            if (bot->GetDistance2d(midX, midY) > MID_TOLERANCE)
                TryMoveToPosition(midX, midY, PLATFORM_Z, true);
            else
                bot->StopMoving();
            return false;
        }
    }
    else
        HandleRangedPositioning();

    return false;
}

bool IccLichKingWinterAction::FixPlatformPosition()
{
    if (std::abs(bot->GetPositionZ() - PLATFORM_Z) > 1.0f)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                        PLATFORM_Z, bot->GetOrientation());
        return true;
    }

    return false;
}

bool IccLichKingWinterAction::ClearInvalidTarget()
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    bool doClear = false;

    // Bots must not fight the boss during Remorseless Winter
    if (boss && currentTarget == boss)
        doClear = true;

    // Tanks must not chase ice spheres
    if (PlayerbotAI::IsTank(bot) && IsIceSphere(currentTarget->GetEntry()))
        doClear = true;

    if (doClear)
    {
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->AttackStop();
        bot->SetTarget(ObjectGuid::Empty);
        return true;
    }

    return false;
}

Position const* IccLichKingWinterAction::GetMainTankPosition()
{
    // Use group centroid so the chosen frost slot follows the majority of the
    // raid, not the tanks. Non-tanks can't survive Remorseless Winter ticks
    // while traversing the platform, so dragging them to a far slot just
    // because tanks parked there gets them killed. Tanks tolerate a few
    // ticks and will reroute to the majority slot too.
    Position ref = ComputeGroupCentroid(bot);

    return &SelectClosestOf3(ref, ICC_LK_FROST1_POSITION, ICC_LK_FROST2_POSITION, ICC_LK_FROST3_POSITION);
}

Position const* IccLichKingWinterAction::GetMainTankRangedPosition()
{
    Position const* melee = GetMainTankPosition();

    if (melee == &ICC_LK_FROST1_POSITION)
        return &ICC_LK_FROSTR1_POSITION;
    if (melee == &ICC_LK_FROST2_POSITION)
        return &ICC_LK_FROSTR2_POSITION;

    return &ICC_LK_FROSTR3_POSITION;
}

bool IccLichKingWinterAction::IsPositionSafeFromDefile(float x, float y, float /*z*/, float minSafeDistance) const
{
    static constexpr float BASE_RADIUS = 6.0f;
    static constexpr float SAFETY_MARGIN = 3.0f;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != DEFILE_NPC_ID)
            continue;

        float currentRadius = BASE_RADIUS;
        for (uint32 const auraId : DEFILE_AURAS)
        {
            Aura* grow = unit->GetAura(auraId);
            if (!grow)
                continue;

            float const growthMultiplier =
                (bot->GetRaidDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC ||
                 bot->GetRaidDifficulty() == RAID_DIFFICULTY_10MAN_NORMAL)
                    ? 1.4f
                    : 0.95f;

            currentRadius = BASE_RADIUS + grow->GetStackAmount() * growthMultiplier;
            break;
        }

        float const dx = x - unit->GetPositionX();
        float const dy = y - unit->GetPositionY();

        if (std::sqrt(dx * dx + dy * dy) < currentRadius + SAFETY_MARGIN + minSafeDistance)
            return false;
    }

    return true;
}

bool IccLichKingWinterAction::IsPositionSafeFromShadowTraps(float x, float y) const
{
    // Shadow trap has a fixed 12 yd lethal radius (matches the SAFE_DISTANCE
    // used by IccLichKingShadowTrapAction).
    static constexpr float TRAP_SAFE_RADIUS = 12.0f;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOW_TRAP)
            continue;

        float const dx = x - unit->GetPositionX();
        float const dy = y - unit->GetPositionY();

        if (std::sqrt(dx * dx + dy * dy) < TRAP_SAFE_RADIUS)
            return false;
    }

    return true;
}

bool IccLichKingWinterAction::TryMoveToPosition(float targetX, float targetY, float targetZ, bool forced)
{
    float const dx = targetX - bot->GetPositionX();
    float const dy = targetY - bot->GetPositionY();
    float const dist = std::hypot(dx, dy);

    if (dist < 0.1f)
        return true;

    MovementPriority const priority = forced
        ? MovementPriority::MOVEMENT_FORCED
        : MovementPriority::MOVEMENT_COMBAT;

    if (IsPositionSafeFromDefile(targetX, targetY, targetZ, 3.0f) &&
        IsPositionSafeFromShadowTraps(targetX, targetY) &&
        bot->IsWithinLOS(targetX, targetY, targetZ))
    {
        // Sample the straight-line path for defile and shadow trap crossings
        bool pathSafe = true;
        static constexpr float PATH_CHECK_INTERVAL = 3.0f;
        int const numChecks = std::max(1, static_cast<int>(dist / PATH_CHECK_INTERVAL));

        for (int i = 1; i < numChecks; ++i)
        {
            float const t = static_cast<float>(i) / static_cast<float>(numChecks);
            float const checkX = bot->GetPositionX() + dx * t;
            float const checkY = bot->GetPositionY() + dy * t;

            if (!IsPositionSafeFromDefile(checkX, checkY, targetZ, 2.0f) ||
                !IsPositionSafeFromShadowTraps(checkX, checkY))
            {
                pathSafe = false;
                break;
            }
        }

        if (pathSafe)
        {
            MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true, priority, true, false);
            return true;
        }
        // Path crosses defile or shadow trap — fall through to arc-stepping logic
    }

    // Arc-stepping: rotate around the movement direction to find a way around defile/traps
    static constexpr int STEPS = 8;
    static constexpr float STEP_DIST = 8.0f;

    float const dirX = dx / dist;
    float const dirY = dy / dist;

    for (int i = 1; i <= STEPS; ++i)
    {
        for (int sign : {1, -1})
        {
            float const arc = float(M_PI) / STEPS * i * sign;
            float const cosA = std::cos(arc);
            float const sinA = std::sin(arc);

            float const testX = bot->GetPositionX() + (dirX * cosA - dirY * sinA) * STEP_DIST;
            float const testY = bot->GetPositionY() + (dirX * sinA + dirY * cosA) * STEP_DIST;

            if (!IsPositionSafeFromDefile(testX, testY, targetZ, 3.0f))
                continue;
            if (!IsPositionSafeFromShadowTraps(testX, testY))
                continue;
            if (!bot->IsWithinLOS(testX, testY, targetZ))
                continue;

            MoveTo(bot->GetMapId(), testX, testY, targetZ, false, false, false, true, priority, true, false);
            return false;
        }
    }

    // Probe along the direct path, shortening the distance until safe
    static constexpr float PROBE_STEP = 5.0f;

    for (float probeLen = dist - PROBE_STEP; probeLen >= PROBE_STEP; probeLen -= PROBE_STEP)
    {
        float probeX = bot->GetPositionX() + dirX * probeLen;
        float probeY = bot->GetPositionY() + dirY * probeLen;
        float probeZ = targetZ;
        bot->UpdateAllowedPositionZ(probeX, probeY, probeZ);

        if (!IsPositionSafeFromDefile(probeX, probeY, probeZ, 3.0f))
            continue;
        if (!IsPositionSafeFromShadowTraps(probeX, probeY))
            continue;
        if (!bot->IsWithinLOS(probeX, probeY, probeZ))
            continue;

        MoveTo(bot->GetMapId(), probeX, probeY, probeZ,
               false, false, false, true, priority, true, false);
        return false;
    }

    // Small nudge forward
    {
        float nudgeX = bot->GetPositionX() + dirX * 2.0f;
        float nudgeY = bot->GetPositionY() + dirY * 2.0f;
        float nudgeZ = targetZ;
        bot->UpdateAllowedPositionZ(nudgeX, nudgeY, nudgeZ);

        if (bot->IsWithinLOS(nudgeX, nudgeY, nudgeZ))
        {
            MoveTo(bot->GetMapId(), nudgeX, nudgeY, nudgeZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
            return false;
        }
    }

    // Last resort: move away from the nearest defile to get to safety first
    {
        GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        Unit* nearestDefile = nullptr;
        float nearestDefileDist = FLT_MAX;

        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != DEFILE_NPC_ID)
                continue;

            float const d = bot->GetDistance2d(unit);
            if (d < nearestDefileDist)
            {
                nearestDefileDist = d;
                nearestDefile = unit;
            }
        }

        if (nearestDefile)
        {
            float const awayX = bot->GetPositionX() - nearestDefile->GetPositionX();
            float const awayY = bot->GetPositionY() - nearestDefile->GetPositionY();
            float const awayLen = std::hypot(awayX, awayY);

            if (awayLen > 0.1f)
            {
                float const escapeX = bot->GetPositionX() + (awayX / awayLen) * 5.0f;
                float const escapeY = bot->GetPositionY() + (awayY / awayLen) * 5.0f;

                MoveTo(bot->GetMapId(), escapeX, escapeY, targetZ,
                       false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
                return false;
            }
        }
    }

    // No defile nearby — safe to move directly to the target.

    if (bot->IsWithinLOS(targetX, targetY, targetZ))
    {
        botAI->Reset();
        MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IccLichKingWinterAction::IsValidCollectibleAdd(Unit* unit) const
{
    return IsLkCollectibleAdd(unit);
}

bool IccLichKingWinterAction::HandleTankPositioning()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    // Both tanks converge on the same frost position
    Position const& frostPos = *GetMainTankPosition();
    static constexpr float FROST_AT_POS_TOLERANCE = 3.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        float const dist = bot->GetDistance2d(frostPos.GetPositionX(), frostPos.GetPositionY());
        auto& s_mtInbound = IcecrownHelpers::IccState(bot->GetInstanceId()).mtAddInbound;
        ObjectGuid const mtKey = bot->GetGUID();

        // Hysteresis: converge to 3y once, then add management owns movement
        // within a 12y leash. A single threshold flip-flops: orienting steps
        // push past 3y and the walk-back (with its reset) yanks him straight
        // back every other tick.
        static constexpr float FROST_LEASH = 12.0f;

        if (dist > (s_mtInbound[mtKey] ? FROST_AT_POS_TOLERANCE : FROST_LEASH))
        {
            if (!s_mtInbound[mtKey])
            {
                bot->AttackStop();
                bot->CastStop();
                bot->SetTarget(ObjectGuid::Empty);
                context->GetValue<Unit*>("current target")->Set(nullptr);
                botAI->Reset();
                s_mtInbound[mtKey] = true;
            }
            TryMoveToPosition(frostPos.GetPositionX(), frostPos.GetPositionY(), PLATFORM_Z, true);
            return false;
        }

        s_mtInbound[mtKey] = false;
        return HandleMainTankAddManagement(boss, &frostPos);
    }

    if (PlayerbotAI::IsAssistTank(bot))
        return HandleAssistTankAddManagement(boss, &frostPos);

    return false;
}

bool IccLichKingWinterAction::HandleMeleePositioning()
{
    // Frost engagement zone: bots within MELEE_FLANK_RADIUS of the frost anchor
    // are considered "engaged" and trusted to flank. Outside, they're inbound
    // and may need to wait at the ranged anchor for MT to gain control.
    static constexpr float MELEE_FLANK_RADIUS = 10.0f;
    Position const& tankPos = *GetMainTankPosition();
    float const distToPos = bot->GetDistance2d(tankPos.GetPositionX(), tankPos.GetPositionY());

    // Transition guard: only retreat to ranged frost position if the bot is
    // still inbound (outside the engagement zone) AND a nearby Shambling is
    // not yet under MT control. Once engaged, trust flank logic — no retreats.
    if (distToPos > MELEE_FLANK_RADIUS)
    {
        Unit* mainTank = AI_VALUE(Unit*, "main tank");
        if (mainTank && mainTank->IsAlive())
        {
            static constexpr float NEARBY_SHAMBLING_RANGE = 10.0f;
            GuidVector const& transNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            bool nearbyShamblingLoose = false;

            for (ObjectGuid const& guid : transNpcs)
            {
                Unit* add = botAI->GetUnit(guid);
                if (!add || !add->IsAlive())
                    continue;

                if (!IsLkShambling(add->GetEntry()))
                    continue;

                if (bot->GetExactDist2d(add) > NEARBY_SHAMBLING_RANGE)
                    continue;

                Unit* victim = add->GetVictim();
                if (!victim || victim->GetGUID() != mainTank->GetGUID())
                {
                    nearbyShamblingLoose = true;
                    break;
                }
            }

            if (nearbyShamblingLoose)
            {
                Position const& rangedPos = *GetMainTankRangedPosition();
                TryMoveToPosition(rangedPos.GetPositionX(), rangedPos.GetPositionY(), PLATFORM_Z, true);
                return false;
            }
        }
    }

    // No re-anchor to tank position: flanking owns positioning. Pulling melee
    // toward MT during winter puts bots in front of shamblings (frontal cone).

    // Only adds within MT_ADD_RANGE of the main tank are valid melee targets.
    // Adds further out are not under tank control — engaging them puts bots
    // in front of unrooted shamblings.
    static constexpr float MT_ADD_RANGE = 5.0f;
    Unit* mainTank = AI_VALUE(Unit*, "main tank");

    auto addNearMT = [&](Unit* add) -> bool
    {
        return mainTank && mainTank->IsAlive() && add &&
               add->GetDistance2d(mainTank) <= MT_ADD_RANGE;
    };

    // Acquire a valid add target
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || !currentTarget->IsAlive() ||
        !IsLkCollectibleAdd(currentTarget) || IsIceSphere(currentTarget->GetEntry()) ||
        !addNearMT(currentTarget))
    {
        Unit* newTarget = nullptr;

        // Priority: skull-marked target (only if near MT)
        if (Group* group = bot->GetGroup())
        {
            Unit* skull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
            if (skull && skull->IsAlive() && IsLkCollectibleAdd(skull) && addNearMT(skull))
                newTarget = skull;
        }

        // Fallback: nearest valid add within MT range
        if (!newTarget)
        {
            GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            float closestDist = FLT_MAX;

            for (ObjectGuid const& guid : npcs)
            {
                Unit* add = botAI->GetUnit(guid);
                if (!IsLkCollectibleAdd(add))
                    continue;
                if (!addNearMT(add))
                    continue;

                float const d = bot->GetDistance(add);
                if (d < closestDist)
                {
                    closestDist = d;
                    newTarget = add;
                }
            }
        }

        if (!newTarget)
            return false;

        context->GetValue<Unit*>("current target")->Set(newTarget);
        bot->SetTarget(newTarget->GetGUID());
        currentTarget = newTarget;
    }

    // A flank slot must be out of the frontal cone of EVERY nearby shambling
    // and spirit, not just the current target: their frontals hit whoever
    // stands in them regardless of who the bot is attacking.
    GuidVector const& coneNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    auto inAddFrontalCone = [&](float x, float y) -> bool
    {
        static constexpr float ADD_CONE_RANGE = 15.0f;
        static constexpr float ADD_FRONTAL_ARC = 2.0f * float(M_PI) / 3.0f;
        Position const slot(x, y);

        for (ObjectGuid const& guid : coneNpcs)
        {
            Unit* add = botAI->GetUnit(guid);
            if (!add || !add->IsAlive())
                continue;

            uint32 const entry = add->GetEntry();
            if (!IsLkShambling(entry) && !IsLkRagingSpirit(entry))
                continue;

            if (add->GetExactDist2d(x, y) > ADD_CONE_RANGE)
                continue;

            if (add->HasInArc(ADD_FRONTAL_ARC, &slot))
                return true;
        }

        return false;
    };

    // Settle band: if a candidate flank slot is safe AND the bot is already
    // within FLANK_SETTLE_DIST of it, stay put and attack — no per-tick step
    // (prevents back-and-forth jitter near the slot).
    static constexpr float FLANK_SETTLE_DIST = 2.0f;

    // Position behind the current melee target using mainTank→add vector
    if (mainTank && mainTank->IsAlive())
    {
        float const vecX = currentTarget->GetPositionX() - mainTank->GetPositionX();
        float const vecY = currentTarget->GetPositionY() - mainTank->GetPositionY();
        float const vecLen = std::hypot(vecX, vecY);

        if (vecLen > 0.1f)
        {
            float const dirX = vecX / vecLen;
            float const dirY = vecY / vecLen;

            static constexpr std::array<float, 5> Arcs = {0.0f, 0.35f, -0.35f, 0.70f, -0.70f};

            for (float const arc : Arcs)
            {
                float const cosA = std::cos(arc);
                float const sinA = std::sin(arc);
                float const rotX = dirX * cosA - dirY * sinA;
                float const rotY = dirX * sinA + dirY * cosA;

                float const destX = currentTarget->GetPositionX() + rotX * BEHIND_DISTANCE;
                float const destY = currentTarget->GetPositionY() + rotY * BEHIND_DISTANCE;

                float const bDx = destX - bot->GetPositionX();
                float const bDy = destY - bot->GetPositionY();
                float const bDist = std::hypot(bDx, bDy);

                if (!IsPositionSafeFromDefile(destX, destY, bot->GetPositionZ(), 2.0f))
                    continue;
                if (!bot->IsWithinLOS(destX, destY, bot->GetPositionZ()))
                    continue;
                if (inAddFrontalCone(destX, destY))
                    continue;

                if (bDist < FLANK_SETTLE_DIST)
                {
                    bot->SetFacingToObject(currentTarget);
                    Attack(currentTarget);
                    return false;
                }

                float const step = std::min(1.25f, bDist);
                TryMoveToPosition(bot->GetPositionX() + (bDx / bDist) * step, bot->GetPositionY() + (bDy / bDist) * step, bot->GetPositionZ(), true);
                return false;
            }
        }
    }

    // Fallback: stored orientation-based angles
    static constexpr std::array<float, 6> BehindAngles =
    {
        float(M_PI),           // 180° directly behind
        float(M_PI) * 0.75f,   // 135° left-rear flank
        float(M_PI) * 1.25f,   // 225° right-rear flank
        float(M_PI) * 0.5f,    // 90°  left flank
        float(M_PI) * 1.5f,    // 270° right flank
        float(M_PI) * 0.875f,  // 157° closer left-rear
    };

    for (float const angleOffset : BehindAngles)
    {
        float const angle = currentTarget->GetOrientation() + angleOffset;
        float const destX = currentTarget->GetPositionX() + std::cos(angle) * BEHIND_DISTANCE;
        float const destY = currentTarget->GetPositionY() + std::sin(angle) * BEHIND_DISTANCE;

        float const bDx = destX - bot->GetPositionX();
        float const bDy = destY - bot->GetPositionY();
        float const bDist = std::hypot(bDx, bDy);

        if (!IsPositionSafeFromDefile(destX, destY, bot->GetPositionZ(), 2.0f))
            continue;
        if (!bot->IsWithinLOS(destX, destY, bot->GetPositionZ()))
            continue;
        if (inAddFrontalCone(destX, destY))
            continue;

        if (bDist < FLANK_SETTLE_DIST)
        {
            bot->SetFacingToObject(currentTarget);
            Attack(currentTarget);
            return false;
        }

        float const step = std::min(1.25f, bDist);
        TryMoveToPosition(bot->GetPositionX() + (bDx / bDist) * step,
                          bot->GetPositionY() + (bDy / bDist) * step,
                          bot->GetPositionZ(), true);
        return false;
    }

    bot->SetFacingToObject(currentTarget);
    Attack(currentTarget);
    return false;
}

bool IccLichKingWinterAction::HandleRangedPositioning()
{
    Position const& targetPos = *GetMainTankRangedPosition();

    // Evacuate defile first
    if (!IsPositionSafeFromDefile(bot->GetPositionX(), bot->GetPositionY(),
                                  bot->GetPositionZ(), 3.0f))
    {
        TryMoveToPosition(targetPos.GetPositionX(), targetPos.GetPositionY(), PLATFORM_Z, true);
        return false;
    }

    // Move to ranged frost position. Clear target + reset only on the FIRST
    // tick of the inbound phase — otherwise we cancel the bot's own movement
    // every tick and it ends up walking 1y per cycle.
    auto& s_rangedInbound = IcecrownHelpers::IccState(bot->GetInstanceId()).rangedInbound;
    ObjectGuid const rangedKey = bot->GetGUID();
    bool const farFromAnchor =
        bot->GetDistance2d(targetPos.GetPositionX(), targetPos.GetPositionY()) > 2.0f;

    if (farFromAnchor)
    {
        bool const wasInbound = s_rangedInbound[rangedKey];
        if (!wasInbound)
        {
            bot->AttackStop();
            bot->CastStop();
            bot->SetTarget(ObjectGuid::Empty);
            context->GetValue<Unit*>("current target")->Set(nullptr);
            botAI->Reset();
            s_rangedInbound[rangedKey] = true;
        }
        TryMoveToPosition(targetPos.GetPositionX(), targetPos.GetPositionY(), PLATFORM_Z, true);
        // Skip target acquisition while inbound — Attack() would cancel movement.
        return false;
    }

    s_rangedInbound[rangedKey] = false;

    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    static constexpr float SPHERE_RANGE = 30.0f;

    // Keep attacking current sphere if still alive and in range
    Unit* cur = bot->GetVictim();
    if (cur && cur->IsAlive() && IsIceSphere(cur->GetEntry()) &&
        bot->GetDistance(cur) <= SPHERE_RANGE)
    {
        bot->SetFacingToObject(cur);
        Attack(cur);
        return false;
    }

    // Drop stale sphere target
    if (cur && IsIceSphere(cur->GetEntry()))
    {
        bot->AttackStop();
        bot->SetTarget(ObjectGuid::Empty);
    }

    // Find the nearest sphere within range
    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* nearestSphere = nullptr;
    float nearestDist = SPHERE_RANGE;

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !IsIceSphere(unit->GetEntry()))
            continue;

        float const d = bot->GetDistance(unit);
        if (d < nearestDist)
        {
            nearestDist = d;
            nearestSphere = unit;
        }
    }

    if (nearestSphere)
    {
        bot->SetTarget(nearestSphere->GetGUID());
        bot->SetFacingToObject(nearestSphere);
        Attack(nearestSphere);
        return true;
    }

    // No spheres — engage skull-marked add or nearest valid add
    Unit* addTarget = nullptr;
    if (Group* group = bot->GetGroup())
    {
        Unit* skull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
        if (skull && skull->IsAlive() && IsLkCollectibleAdd(skull))
            addTarget = skull;
    }

    if (!addTarget)
    {
        float closestDist = SPHERE_RANGE;
        for (ObjectGuid const& guid : npcs)
        {
            Unit* add = botAI->GetUnit(guid);
            if (!IsLkCollectibleAdd(add))
                continue;

            float const d = bot->GetDistance(add);
            if (d < closestDist)
            {
                closestDist = d;
                addTarget = add;
            }
        }
    }

    if (addTarget)
    {
        context->GetValue<Unit*>("current target")->Set(addTarget);
        bot->SetTarget(addTarget->GetGUID());
        bot->SetFacingToObject(addTarget);
        Attack(addTarget);
        return true;
    }

    return false;
}

bool IccLichKingWinterAction::HandleMainTankAddManagement(Unit*, Position const* frostPos)
{
    static constexpr float ENGAGE_RADIUS = 12.0f;
    static constexpr float TAUNT_RADIUS = 30.0f;
    static constexpr int AOE_TAUNT_MIN = 2;

    bool hasAliveAssistTank = false;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->IsAlive() && member != bot && PlayerbotAI::IsAssistTank(member))
            {
                hasAliveAssistTank = true;
                break;
            }
        }
    }

    GuidVector const& targets = AI_VALUE(GuidVector, "possible targets");

    Unit* targetAdd = nullptr;
    int nearbyCount = 0;

    auto typeRank = [](Unit* u) -> int
    {
        uint32 const e = u->GetEntry();
        if (IsLkShambling(e))
            return 2;
        if (IsLkRagingSpirit(e))
            return 1;
        return 0;
    };

    auto rescueRank = [&](Unit* u) -> int
    {
        Unit* v = u->GetVictim();
        return v && v->IsPlayer() && !PlayerbotAI::IsTank(v->ToPlayer()) ? 1 : 0;
    };

    auto better = [&](Unit* cand, Unit* cur) -> bool
    {
        if (!cur)
            return true;
        int const tc = typeRank(cand);
        int const tr = typeRank(cur);
        if (tc != tr)
            return tc > tr;
        int const rc = rescueRank(cand);
        int const rr = rescueRank(cur);
        if (rc != rr)
            return rc > rr;
        return bot->GetDistance(cand) < bot->GetDistance(cur);
    };

    for (ObjectGuid const& guid : targets)
    {
        Unit* add = botAI->GetUnit(guid);
        if (!IsLkCollectibleAdd(add))
            continue;

        float const addDist = bot->GetDistance(add);
        float const maxEngage = hasAliveAssistTank ? ENGAGE_RADIUS : 25.0f;

        if (addDist <= TAUNT_RADIUS)
            ++nearbyCount;

        Unit* victim = add->GetVictim();

        // Taunt pass: all adds in range that are NOT already on MT
        bool const onMT = victim && victim->IsPlayer() && PlayerbotAI::IsMainTank(victim->ToPlayer());
        if (!onMT && addDist <= TAUNT_RADIUS)
            IccCastClassTaunt(bot, botAI, add);

        // Priority taunt: adds on the assist tank within 10 yd
        bool const onAT = victim && victim->IsPlayer() && PlayerbotAI::IsAssistTank(victim->ToPlayer());
        if (onAT && addDist <= 10.0f)
            IccCastClassTaunt(bot, botAI, add);

        if (nearbyCount >= AOE_TAUNT_MIN)
            CastAoeTaunt(botAI, bot);

        if (addDist > maxEngage)
            continue;

        if (better(add, targetAdd))
            targetAdd = add;
    }

    if (!targetAdd)
    {
        Unit* cur = bot->GetVictim();
        if (cur && !IsLkCollectibleAdd(cur))
            bot->SetTarget(ObjectGuid::Empty);
        return false;
    }

    {
        static constexpr float HOLD_REACH = 3.0f;
        static constexpr float HOLD_TOL   = 1.5f;

        Position const& rangedPos = *GetMainTankRangedPosition();

        // Only step when some add on us is deeper (farther from the ranged
        // anchor) than we are. Chasing centroid + away unconditionally is a
        // treadmill: adds follow every step, dragging the centroid along.
        float const botRadial = bot->GetDistance2d(rangedPos.GetPositionX(), rangedPos.GetPositionY());
        bool behindAll = true;

        float sumX = 0.0f;
        float sumY = 0.0f;
        int count = 0;
        for (ObjectGuid const& guid : targets)
        {
            Unit* add = botAI->GetUnit(guid);
            if (!add || !add->IsAlive())
                continue;

            uint32 const entry = add->GetEntry();
            if (!IsLkShambling(entry) && !IsLkRagingSpirit(entry))
                continue;

            if (add->GetVictim() != bot)
                continue;

            sumX += add->GetPositionX();
            sumY += add->GetPositionY();
            ++count;

            if (add->GetDistance2d(rangedPos.GetPositionX(), rangedPos.GetPositionY()) > botRadial + 1.0f)
                behindAll = false;
        }

        float cx = count > 0 ? sumX / count : targetAdd->GetPositionX();
        float cy = count > 0 ? sumY / count : targetAdd->GetPositionY();

        float awayNx = cx - rangedPos.GetPositionX();
        float awayNy = cy - rangedPos.GetPositionY();
        float const awayLen = std::hypot(awayNx, awayNy);

        if (awayLen > 0.5f)
        {
            awayNx /= awayLen;
            awayNy /= awayLen;
        }
        else
        {
            float const fdx = frostPos->GetPositionX() - rangedPos.GetPositionX();
            float const fdy = frostPos->GetPositionY() - rangedPos.GetPositionY();
            float const flen = std::hypot(fdx, fdy);
            awayNx = flen > 0.01f ? fdx / flen : 1.0f;
            awayNy = flen > 0.01f ? fdy / flen : 0.0f;
        }

        float const holdX = cx + awayNx * HOLD_REACH;
        float const holdY = cy + awayNy * HOLD_REACH;

        if (!behindAll && bot->GetDistance2d(holdX, holdY) > HOLD_TOL)
            TryMoveToPosition(holdX, holdY, PLATFORM_Z, false);
    }

    bot->SetTarget(targetAdd->GetGUID());
    bot->SetFacingToObject(targetAdd);
    Attack(targetAdd);
    return false;
}

bool IccLichKingWinterAction::HandleAssistTankAddManagement(Unit*, Position const* frostPos)
{
    static constexpr float FROST_TOL = 3.0f;
    static constexpr float MELE_RANGE = 5.0f;
    static constexpr float TAUNT_RADIUS = 30.0f;

    GuidVector const& targets = AI_VALUE(GuidVector, "possible targets");

    Unit* mainTank = AI_VALUE(Unit*, "main tank");

    std::vector<Unit*> addsOnUs;
    std::vector<Unit*> addsLoose;

    for (ObjectGuid const& guid : targets)
    {
        Unit* add = botAI->GetUnit(guid);
        if (!IsLkCollectibleAdd(add))
            continue;

        Unit* victim = add->GetVictim();

        // Skip adds already securely held by the main tank
        if (mainTank && victim && victim->GetGUID() == mainTank->GetGUID())
            continue;

        if (victim == bot)
            addsOnUs.push_back(add);
        else
            addsLoose.push_back(add);
    }

    // adds are on us — walk back to frost position
    if (!addsOnUs.empty())
    {
        float const distToFrost = bot->GetExactDist2d(frostPos->GetPositionX(),
                                                       frostPos->GetPositionY());
        if (distToFrost > FROST_TOL)
            TryMoveToPosition(frostPos->GetPositionX(), frostPos->GetPositionY(), PLATFORM_Z, true);

        // Maintain combat target on the way back
        Unit* currentTarget = bot->GetVictim();
        bool keepCurrent = false;

        if (currentTarget && currentTarget->IsAlive())
        {
            for (Unit* add : addsOnUs)
            {
                if (add->GetGUID() == currentTarget->GetGUID())
                {
                    keepCurrent = true;
                    break;
                }
            }
        }

        if (!keepCurrent)
        {
            currentTarget = nullptr;

            // Prefer shambling as combat target
            for (Unit* add : addsOnUs)
            {
                if (IsLkShambling(add->GetEntry()))
                {
                    currentTarget = add;
                    break;
                }
            }

            if (!currentTarget)
                currentTarget = addsOnUs.front();
        }

        if (currentTarget)
        {
            bot->SetTarget(currentTarget->GetGUID());
            bot->SetFacingToObject(currentTarget);
            Attack(currentTarget);
        }

        return false;
    }

    // return to frost position if we drifted
    float const distToFrost = bot->GetExactDist2d(frostPos->GetPositionX(),
                                                   frostPos->GetPositionY());
    if (distToFrost > FROST_TOL)
    {
        botAI->Reset();
        TryMoveToPosition(frostPos->GetPositionX(), frostPos->GetPositionY(), PLATFORM_Z, true);

        Unit* cur = bot->GetVictim();
        if (cur && !IsLkCollectibleAdd(cur))
            bot->SetTarget(ObjectGuid::Empty);
        return false;
    }

    // at frost position, collect the nearest loose add
    Unit* targetAdd = nullptr;
    float closestDist = FLT_MAX;

    // Priority 1: rescue add attacking a non-tank
    for (Unit* add : addsLoose)
    {
        Unit* victim = add->GetVictim();
        if (victim && victim->IsPlayer() && !PlayerbotAI::IsTank(victim->ToPlayer()))
        {
            float const dist = bot->GetDistance(add);
            if (dist < closestDist)
            {
                closestDist = dist;
                targetAdd = add;
            }
        }
    }

    // Priority 2: any loose add
    if (!targetAdd)
    {
        for (Unit* add : addsLoose)
        {
            float const dist = bot->GetDistance(add);
            if (dist < closestDist)
            {
                closestDist = dist;
                targetAdd = add;
            }
        }
    }

    if (!targetAdd)
    {
        Unit* cur = bot->GetVictim();
        if (cur && !IsLkCollectibleAdd(cur))
            bot->SetTarget(ObjectGuid::Empty);

        // No loose adds — idle at frost pos but allow lower-priority actions to run
        return false;
    }

    IccCastClassTaunt(bot, botAI, targetAdd);

    // Also taunt other loose adds in range
    for (Unit* add : addsLoose)
    {
        if (add == targetAdd)
            continue;
        if (bot->GetExactDist2d(add) <= TAUNT_RADIUS)
            IccCastClassTaunt(bot, botAI, add);
    }

    // AoE taunt if 2+ loose adds are close enough
    {
        int nearbyLoose = 0;
        for (Unit* add : addsLoose)
        {
            if (bot->GetExactDist2d(add) <= TAUNT_RADIUS)
                ++nearbyLoose;
        }
        if (nearbyLoose >= 2)
            CastAoeTaunt(botAI, bot);
    }

    bot->SetTarget(targetAdd->GetGUID());

    if (closestDist > MELE_RANGE)
    {
        botAI->Reset();
        TryMoveToPosition(targetAdd->GetPositionX(), targetAdd->GetPositionY(), PLATFORM_Z, false);
    }
    else
    {
        bot->SetFacingToObject(targetAdd);
        Attack(targetAdd);
    }

    return false;
}

bool IccLichKingWinterAction::HandlePetManagement()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    CharmInfo* ci = pet->GetCharmInfo();
    if (!ci)
        return false;

    if (PlayerbotAI::IsHeal(bot))
    {
        if (ci->GetCommandState() != COMMAND_FOLLOW)
        {
            pet->AttackStop();
            pet->CastStop();
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
            ci->SetCommandState(COMMAND_FOLLOW);
            ci->SetIsCommandAttack(false);
            ci->SetIsAtStay(false);
            ci->SetIsReturning(true);
            ci->SetIsCommandFollow(true);
            ci->SetIsFollowing(false);
            ci->RemoveStayPosition();
        }
        return false;
    }

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Unit* bestTarget = nullptr;
    int bestPriority = -1;
    float bestHpPct = 101.0f;

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 const entry = unit->GetEntry();
        int priority = -1;

        if (IsLkShambling(entry))
            priority = 3;
        else if (IsLkRagingSpirit(entry))
            priority = 2;
        else if (IsIceSphere(entry))
            priority = 1;

        if (priority < 0)
            continue;

        float const hp = unit->GetHealthPct();
        if (priority > bestPriority || (priority == bestPriority && hp < bestHpPct))
        {
            bestPriority = priority;
            bestHpPct = hp;
            bestTarget = unit;
        }
    }

    if (!bestTarget)
        return false;

    // Skip if the pet is already attacking the right unit
    if (pet->GetVictim() == bestTarget)
        return false;

    ci->SetIsCommandAttack(true);
    ci->SetIsAtStay(false);
    ci->SetIsReturning(false);
    ci->SetIsCommandFollow(false);
    ci->SetIsFollowing(false);
    ci->SetCommandState(COMMAND_ATTACK);

    if (pet->IsAIEnabled)
    {
        pet->AI()->AttackStart(bestTarget);
        pet->GetMotionMaster()->MoveChase(bestTarget);
    }

    return true;
}

bool IccLichKingAddsAction::Execute(Event /*event*/)
{
    // Being carried by a Val'kyr — no actions possible
    if (bot->HasAura(SPELL_HARVEST_SOUL_VALKYR))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    Difficulty const diff = bot->GetRaidDifficulty();
    bool const hasPlague = botAI->HasAura("Necrotic Plague", bot);
    Unit* const terenas = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);

    // Heroic cheat buffs — apply to all group members
    Creature* lichKing = bot->FindNearestCreature(NPC_THE_LICH_KING, 100.0f);
    if (lichKing && lichKing->IsInCombat() && sPlayerbotAIConfig.EnableICCBuffs && IsHeroicLk(diff))
    {
        Group* buffGroup = bot->GetGroup();
        if (buffGroup)
        {
            for (GroupReference* itr = buffGroup->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !member->IsInWorld())
                    continue;

                IccApplyHeroicBuffToMember(member, true, true);

                if (boss && boss->HealthBelowPct(60) && boss->HealthAbovePct(40) &&
                    !member->HasAura(SPELL_EMPOWERED_BLOOD))
                    member->AddAura(SPELL_EMPOWERED_BLOOD, member);
            }
        }
    }

    // Skull marking: phase 1 marks the boss; phase 2+ prioritises Raging Spirits.
    // Val'kyr marking owns skull while any Val'kyr is actively grabbing — skip here.
    if (Group* group = bot->GetGroup())
    {
        bool anyValkyrGrabbing = false;
        {
            GuidVector const& nearbyNpcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (ObjectGuid const& guid : nearbyNpcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && unit->IsAlive() && IsLkValkyr(unit) &&
                    unit->HasAura(SPELL_HARVEST_SOUL_VALKYR) &&
                    (!IsHeroicLk(diff) || unit->HealthAbovePct(49)))
                {
                    anyValkyrGrabbing = true;
                    break;
                }
            }
        }

        if (!anyValkyrGrabbing)
        {
            // Phase 3 (boss < 40%, non-winter): skull priority is Raging Spirit > Vile Spirit > boss.
            // During Vile Spirit windows, melee can't reach the spirits — mark boss on cross
            // and route melee DPS there.
            bool phase3 = false;
            if (boss && boss->HealthBelowPct(40) && !IccBossHasRemorselessWinter(boss))
            {
                phase3 = true;

                Unit* currentSkull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
                bool const ragingMarked = currentSkull && currentSkull->IsAlive() &&
                                          IsLkRagingSpirit(currentSkull->GetEntry());

                GuidVector const& p3Npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

                Unit* nearestRaging = nullptr;
                float nearestRagingDist = std::numeric_limits<float>::max();

                for (ObjectGuid const& guid : p3Npcs)
                {
                    Unit* unit = botAI->GetUnit(guid);
                    if (!unit || !unit->IsAlive())
                        continue;

                    if (!IsLkRagingSpirit(unit->GetEntry()))
                        continue;

                    float const dist = bot->GetDistance(unit);
                    if (dist < nearestRagingDist)
                    {
                        nearestRagingDist = dist;
                        nearestRaging = unit;
                    }
                }

                static constexpr uint8 CROSS_ICON = RtiTargetValue::crossIndex;

                // Priority: Raging Spirit > boss. Vile Spirits are not marked
                // (handled positionally by HandleVileSpiritMechanics + AT chase
                // + hunter frost trap).
                if (nearestRaging)
                {
                    if (!ragingMarked && group->GetTargetIcon(RtiTargetValue::skullIndex) != nearestRaging->GetGUID())
                        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), nearestRaging->GetGUID());
                }
                else if (group->GetTargetIcon(RtiTargetValue::skullIndex) != boss->GetGUID())
                {
                    group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
                }

                // Cross marker is no longer used for vile windows — clear if set
                if (!group->GetTargetIcon(CROSS_ICON).IsEmpty())
                    group->SetTargetIcon(CROSS_ICON, bot->GetGUID(), ObjectGuid::Empty);

                // Per-bot RTI: melee non-tanks always target skull (boss or raging)
                if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot))
                    context->GetValue<std::string>("rti")->Set("skull");
            }

            if (phase3)
            {
                // skip default skull logic
            }
            else if (boss && boss->HealthAbovePct(71))
            {
                if (group->GetTargetIcon(RtiTargetValue::skullIndex) != boss->GetGUID())
                    group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
            }
            else if (boss)
            {
                Unit* currentSkull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
                bool const spiritMarked = currentSkull && currentSkull->IsAlive() &&
                                          IsLkRagingSpirit(currentSkull->GetEntry());

                if (!spiritMarked)
                {
                    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
                    Unit* nearestSpirit = nullptr;
                    float nearestDist = std::numeric_limits<float>::max();

                    for (ObjectGuid const& guid : npcs)
                    {
                        Unit* unit = botAI->GetUnit(guid);
                        if (!unit || !unit->IsAlive())
                            continue;

                        if (!IsLkRagingSpirit(unit->GetEntry()))
                            continue;

                        float const dist = bot->GetDistance(unit);
                        if (dist < nearestDist)
                        {
                            nearestDist = dist;
                            nearestSpirit = unit;
                        }
                    }

                    if (nearestSpirit)
                    {
                        if (group->GetTargetIcon(RtiTargetValue::skullIndex) != nearestSpirit->GetGUID())
                            group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), nearestSpirit->GetGUID());
                    }
                    else if (group->GetTargetIcon(RtiTargetValue::skullIndex) != boss->GetGUID())
                    {
                        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
                    }
                }
            }
        }
    }

    // Val'kyr edge-dive (ongoing)
    if (bot->HasAura(SPELL_VALKYR_CARRY))
    {
        if (bot->GetPositionZ() > 779.0f)
            return JumpTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), 740.01f);

        bot->Kill(bot, bot);
        return true;
    }

    // Detect bots fallen off edge and initiate dive
    if (boss && boss->GetHealthPct() < 70.0f && boss->GetHealthPct() > 40.0f &&
        !IccBossHasRemorselessWinter(boss))
    {
        static constexpr float PLATFORM_CENTER_X = 503.0f;
        static constexpr float PLATFORM_CENTER_Y = -2124.0f;
        static constexpr float PLATFORM_EDGE_MIN = 52.0f;
        static constexpr float PLATFORM_EDGE_MAX = 70.0f;
        static constexpr float PLATFORM_MIN_Z    = 844.0f;

        float const dx = bot->GetPositionX() - PLATFORM_CENTER_X;
        float const dy = bot->GetPositionY() - PLATFORM_CENTER_Y;
        float const platDist = std::hypot(dx, dy);

        if (platDist > PLATFORM_EDGE_MIN && platDist < PLATFORM_EDGE_MAX &&
            bot->GetPositionZ() > PLATFORM_MIN_Z)
        {
            bot->AddAura(SPELL_VALKYR_CARRY, bot);
            return JumpTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), 740.01f);
        }
    }

    if (HandleTeleportationFixes(diff, terenas))
        return true;

    HandleHeroicNonTankPositioning(diff, terenas);
    HandleSpiritMarkingAndTargeting(diff, terenas);

    if (terenas)
        return false;

    // Normal encounter flow
    if (HandleQuakeMechanics(boss))
        return true;

    HandleShamblingHorrors(boss, hasPlague);

    if (HandleAssistTankAddManagement(boss, diff))
        return true;

    if (HandleRagingSpiritFlanking(boss, hasPlague, diff))
        return true;

    HandleMeleePositioning(boss, hasPlague, diff);
    HandleMainTankTargeting(boss, diff);
    HandleNonTankHeroicPositioning(boss, diff, hasPlague);
    HandleRangedPositioning(boss, hasPlague, diff);
    if (HandleDefileMechanics(boss, diff))
        return true;
    if (HandleCenterStacking(boss, diff))
        return true;
    HandleValkyrMechanics(diff);
    HandleVileSpiritMechanics();
    HandleIceSphereMechanics();

    return false;
}

bool IccLichKingAddsAction::HandleTeleportationFixes(Difficulty diff, Unit* terenas)
{
    static constexpr float PLATFORM_Z  = 840.857f;
    static constexpr float SPIRIT_Z    = 1049.865f;
    static constexpr float MAX_Y_DRIFT = 200.0f;
    static constexpr float MAX_Z_DIFF  = 1.0f;
    static constexpr float SPIRIT_Z_TOLERANCE = 5.0f;

    // Normal mode: snap back if teleported far outside the encounter area
    // (Harvest Soul victim exits Frostmourne room). Land on the main tank,
    // falling back to assist tank, then the fixed adds anchor. Iterate group
    // directly so distance/LOS gating in PartyMemberValue::Check doesn't hide
    // a far-away main tank.
    if (!IsHeroicLk(diff) && std::abs(bot->GetPositionY() - (-2095.7915f)) > MAX_Y_DRIFT)
    {
        Player* mainTank = nullptr;
        Player* assistTank = nullptr;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive())
                    continue;
                if (!mainTank && PlayerbotAI::IsMainTank(member))
                    mainTank = member;
                else if (!assistTank && PlayerbotAI::IsAssistTank(member))
                    assistTank = member;
            }
        }
        Unit* anchor = mainTank ? static_cast<Unit*>(mainTank) : static_cast<Unit*>(assistTank);

        if (anchor)
            bot->TeleportTo(bot->GetMapId(), anchor->GetPositionX(), anchor->GetPositionY(),
                            anchor->GetPositionZ(), bot->GetOrientation());
        else
            bot->TeleportTo(bot->GetMapId(),
                            ICC_LICH_KING_ADDS_POSITION.GetPositionX(),
                            ICC_LICH_KING_ADDS_POSITION.GetPositionY(),
                            ICC_LICH_KING_ADDS_POSITION.GetPositionZ(),
                            bot->GetOrientation());
        return true;
    }

    // Fix bots going underground (buggy ice-platform collisions)
    if (!botAI->GetAura("Harvest Soul", bot, false, false) &&
        !botAI->GetAura("Harvest Souls", bot, false, false) &&
        std::abs(bot->GetPositionZ() - PLATFORM_Z) > MAX_Z_DIFF)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                        PLATFORM_Z, bot->GetOrientation());
        return true;
    }

    // Heroic: keep bot near main tank during Harvest Soul (fall back to Terenas).
    // Resolve main tank via group iteration so distance/LOS gating in
    // PartyMemberValue::Check doesn't hide a far-away main tank.
    if (terenas && botAI->GetAura("Harvest Soul", bot, false, false) &&
        std::abs(bot->GetPositionZ() - SPIRIT_Z) > SPIRIT_Z_TOLERANCE)
    {
        Player* mainTank = nullptr;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && PlayerbotAI::IsMainTank(member))
                {
                    mainTank = member;
                    break;
                }
            }
        }
        Unit* anchor = mainTank ? static_cast<Unit*>(mainTank) : terenas;
        bot->TeleportTo(bot->GetMapId(), anchor->GetPositionX(), anchor->GetPositionY(),
                        SPIRIT_Z, bot->GetOrientation());
        return true;
    }

    return false;
}

bool IccLichKingSpiritBombAction::IsBombThreatActive(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !PlayerbotAI::IsMainTank(bot))
        return false;

    Difficulty const diff = bot->GetMap() ? bot->GetMap()->GetDifficulty() : RAID_DIFFICULTY_10MAN_NORMAL;
    if (!IsHeroicLk(diff))
        return false;

    if (!bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f))
        return false;

    GuidVector const& npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SPIRIT_BOMB)
            return true;
    }

    return false;
}

bool IccLichKingSpiritBombAction::Execute(Event)
{
    Difficulty const diff = bot->GetMap() ? bot->GetMap()->GetDifficulty() : RAID_DIFFICULTY_10MAN_NORMAL;
    Unit* terenas = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);

    if (!PlayerbotAI::IsMainTank(bot) || !terenas || !IsHeroicLk(diff))
        return false;

    // Snap back to spirit-room Z if glitched through the floor
    static constexpr float SPIRIT_Z = 1049.865f;
    static constexpr float SPIRIT_Z_TOLERANCE = 5.0f;
    if (std::abs(bot->GetPositionZ() - SPIRIT_Z) > SPIRIT_Z_TOLERANCE)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                        SPIRIT_Z, bot->GetOrientation());
        return true;
    }

    static constexpr float SAFE_DIST = 14.0f;
    static constexpr float SAFE_HEIGHT = 12.0f;
    static constexpr float DENSITY_RADIUS = 10.0f;
    static constexpr float DENSITY_PENALTY = 10.0f;
    static constexpr float MAX_HEIGHT_DIFF = 8.0f;
    static constexpr uint32 UNSAFE_MEM_MS = 15000;

    IcecrownHelpers::IccInstanceState& st = IcecrownHelpers::IccState(bot->GetInstanceId());
    float& lastUnsafeX = st.defileLastUnsafeX;
    float& lastUnsafeY = st.defileLastUnsafeY;
    uint32& lastUnsafeTime = st.defileLastUnsafeTime;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> bombs;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SPIRIT_BOMB)
            bombs.push_back(unit);
    }

    if (bombs.empty())
        return false;

    // Check current position safety
    bool currentlySafe = true;
    for (Unit const* bomb : bombs)
    {
        float const hDist = std::hypot(bot->GetPositionX() - bomb->GetPositionX(),
                                       bot->GetPositionY() - bomb->GetPositionY());
        float const vDist = std::abs(bot->GetPositionZ() - bomb->GetPositionZ());

        if (hDist < SAFE_DIST && vDist <= SAFE_HEIGHT)
        {
            currentlySafe = false;
            break;
        }
    }

    if (currentlySafe)
        return true;

    lastUnsafeX = bot->GetPositionX();
    lastUnsafeY = bot->GetPositionY();
    lastUnsafeTime = getMSTime();

    // Search for the best nearby safe position
    static constexpr std::array<float, 5> SearchDistances = {6.0f, 10.0f, 15.0f, 20.0f, 25.0f};
    static constexpr int ANGLE_COUNT = 36;

    float bestScore = -std::numeric_limits<float>::max();
    float bestX = 0.0f;
    float bestY = 0.0f;
    float bestZ = 0.0f;
    bool found = false;

    for (float const radius : SearchDistances)
    {
        for (int i = 0; i < ANGLE_COUNT; ++i)
        {
            float const angle = i * 2.0f * float(M_PI) / ANGLE_COUNT;
            float const testX = bot->GetPositionX() + radius * std::cos(angle);
            float const testY = bot->GetPositionY() + radius * std::sin(angle);
            float testZ = bot->GetPositionZ();

            bot->UpdateAllowedPositionZ(testX, testY, testZ);

            if (std::abs(testZ - bot->GetPositionZ()) >= MAX_HEIGHT_DIFF)
                continue;
            if (!bot->IsWithinLOS(testX, testY, testZ))
                continue;

            bool posSafe = true;
            float minHDist = std::numeric_limits<float>::max();
            int nearbyCount = 0;

            for (Unit const* bomb : bombs)
            {
                float const hDist = std::hypot(testX - bomb->GetPositionX(),
                                               testY - bomb->GetPositionY());
                float const vDist = std::abs(testZ - bomb->GetPositionZ());

                minHDist = std::min(minHDist, hDist);

                if (hDist < DENSITY_RADIUS)
                    ++nearbyCount;

                if (hDist < SAFE_DIST && vDist <= SAFE_HEIGHT)
                {
                    posSafe = false;
                    break;
                }
            }

            if (!posSafe)
                continue;

            if (lastUnsafeTime != 0 && (getMSTime() - lastUnsafeTime) < UNSAFE_MEM_MS &&
                std::hypot(testX - lastUnsafeX, testY - lastUnsafeY) < SAFE_DIST)
                continue;

            float const distBonus = std::max(0.0f, 30.0f - radius);
            float const score = minHDist - nearbyCount * DENSITY_PENALTY + distBonus;

            if (score > bestScore)
            {
                bestScore = score;
                bestX = testX;
                bestY = testY;
                bestZ = testZ;
                found = true;
            }
        }

        if (found && radius <= 15.0f)
            break;
    }

    // Fallback: surrounded with no clean path. Pick direction where the
    // blocking bomb is highest on Z axis and move thru there
    if (!found)
    {
        float bestEscapeZ = std::numeric_limits<float>::max();
        for (float const radius : SearchDistances)
        {
            for (int i = 0; i < ANGLE_COUNT; ++i)
            {
                float const angle = i * 2.0f * float(M_PI) / ANGLE_COUNT;
                float const testX = bot->GetPositionX() + radius * std::cos(angle);
                float const testY = bot->GetPositionY() + radius * std::sin(angle);
                float testZ = bot->GetPositionZ();

                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                if (std::abs(testZ - bot->GetPositionZ()) >= MAX_HEIGHT_DIFF)
                    continue;
                if (!bot->IsWithinLOS(testX, testY, testZ))
                    continue;

                // Highest blocking bomb on the line from bot to test pos
                float worstBombZ = -std::numeric_limits<float>::max();
                bool blocked = false;
                for (Unit const* bomb : bombs)
                {
                    float const hDist = std::hypot(testX - bomb->GetPositionX(),
                                                   testY - bomb->GetPositionY());
                    if (hDist < SAFE_DIST)
                    {
                        blocked = true;
                        worstBombZ = std::max(worstBombZ, bomb->GetPositionZ());
                    }
                }

                if (!blocked)
                    continue;

                if (worstBombZ < bestEscapeZ)
                {
                    bestEscapeZ = worstBombZ;
                    bestX = testX;
                    bestY = testY;
                    bestZ = testZ;
                    found = true;
                }
            }

            if (found)
                break;
        }
    }

    if (found && bot->IsWithinLOS(bestX, bestY, bestZ) &&
        std::abs(bestZ - bot->GetPositionZ()) <= MAX_HEIGHT_DIFF)
    {
        MoveTo(bot->GetMapId(), bestX, bestY, bestZ,
               false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return true;
}

bool IccLichKingAddsAction::HandleHeroicNonTankPositioning(Difficulty diff, Unit* terenas)
{
    if (!terenas || PlayerbotAI::IsMainTank(bot) || !IsHeroicLk(diff))
        return false;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
        return false;

    // Stack on main tank — 3-yard threshold avoids micro-jitter
    if (bot->GetExactDist2d(mainTank) > 3.0f)
    {
        MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(),
               mainTank->GetPositionZ(), false, false, false, true,
               MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool IccLichKingAddsAction::HandleSpiritMarkingAndTargeting(Difficulty diff, Unit* terenas)
{
    if (!terenas || PlayerbotAI::IsMainTank(bot) || !IsHeroicLk(diff))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static constexpr uint8 STAR_ICON = RtiTargetValue::starIndex;
    static constexpr float MAX_Z_DIFF = 20.0f;

    auto const spiritTargetsGroup = [&](Unit* spirit) -> bool
    {
        Unit* victim = spirit->GetVictim();
        if (!victim || !victim->IsPlayer())
            return false;

        Group* g = victim->ToPlayer()->GetGroup();
        return g && g->GetGUID() == group->GetGUID();
    };

    Unit* currentMark = botAI->GetUnit(group->GetTargetIcon(STAR_ICON));
    bool const needNewMark = !currentMark || !currentMark->IsAlive();
    bool const markOnTarget = currentMark && currentMark->IsAlive() && spiritTargetsGroup(currentMark);

    if (needNewMark || !markOnTarget)
    {
        GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

        Unit* prioritySpirit = nullptr;
        Unit* nearestSpirit = nullptr;
        float priorityDist = 100.0f;
        float nearestDist = 100.0f;

        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || !unit->isTargetableForAttack())
                continue;

            if (!IsLkWickedSpirit(unit->GetEntry()))
                continue;

            if (std::abs(unit->GetPositionZ() - bot->GetPositionZ()) > MAX_Z_DIFF)
                continue;

            float const dist = bot->GetDistance(unit);
            if (spiritTargetsGroup(unit) && dist < priorityDist)
            {
                prioritySpirit = unit;
                priorityDist = dist;
            }
            if (dist < nearestDist)
            {
                nearestSpirit = unit;
                nearestDist = dist;
            }
        }

        Unit* toMark = prioritySpirit ? prioritySpirit : nearestSpirit;
        if (toMark && (needNewMark || (prioritySpirit && !markOnTarget)))
            group->SetTargetIcon(STAR_ICON, bot->GetGUID(), toMark->GetGUID());
    }

    // Ranged DPS focus the star target
    if (PlayerbotAI::IsRangedDps(bot))
    {
        context->GetValue<std::string>("rti")->Set("star");

        Unit* starTarget = botAI->GetUnit(group->GetTargetIcon(STAR_ICON));
        if (starTarget && starTarget->IsAlive())
        {
            bot->SetTarget(starTarget->GetGUID());
            bot->SetFacingToObject(starTarget);
            Attack(starTarget);
            bot->Kill(bot, starTarget);
            return true;
        }
    }

    return false;
}

bool IccLichKingAddsAction::HandleQuakeMechanics(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_QUAKE))
        return false;

    static constexpr float QUAKE_MIN = 35.0f;
    static constexpr float QUAKE_MAX = 45.0f;
    static constexpr float QUAKE_TARGET = 40.0f;

    float const dist = bot->GetExactDist2d(boss);
    if (dist >= QUAKE_MIN && dist <= QUAKE_MAX)
        return false;

    // Interrupt active channel before repositioning
    bot->CastStop();

    float const dx = bot->GetPositionX() - boss->GetPositionX();
    float const dy = bot->GetPositionY() - boss->GetPositionY();
    float const len = std::hypot(dx, dy);
    if (len < 0.01f)
        return false;

    float const targetX = boss->GetPositionX() + (dx / len) * QUAKE_TARGET;
    float const targetY = boss->GetPositionY() + (dy / len) * QUAKE_TARGET;

    return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

bool IccLichKingAddsAction::HandleRagingSpiritFlanking(Unit* boss, bool hasPlague, Difficulty diff)
{
    if (!boss || PlayerbotAI::IsTank(bot) || hasPlague)
        return false;
    if (IccBossHasRemorselessWinter(boss))
        return false;
    if (bot->GetVehicle())
        return false;
    if (bot->HasAura(SPELL_HARVEST_SOUL_VALKYR) || bot->HasAura(SPELL_VALKYR_CARRY))
        return false;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Designated trap hunter (lowest-GUID alive bot hunter) skips flanking when
    // vile spirits are active so HandleVileSpiritMechanics can place the hunter
    // at center for Frost Trap duty. On heroic, raging + vile coexist during P3
    // and would otherwise keep the hunter stuck in flanking.
    if (bot->getClass() == CLASS_HUNTER)
    {
        bool vileAlive = false;
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && IsLkVileSpirit(unit))
            {
                vileAlive = true;
                break;
            }
        }

        if (vileAlive)
        {
            if (Group* hunterGroup = bot->GetGroup())
            {
                ObjectGuid bestGuid;
                for (GroupReference* ref = hunterGroup->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (!member || !member->IsAlive())
                        continue;
                    if (member->getClass() != CLASS_HUNTER)
                        continue;
                    if (!GET_PLAYERBOT_AI(member))
                        continue;

                    if (bestGuid.IsEmpty() || member->GetGUID() < bestGuid)
                        bestGuid = member->GetGUID();
                }
                if (!bestGuid.IsEmpty() && bot->GetGUID() == bestGuid)
                    return false;
            }
        }
    }

    std::vector<Unit*> spirits;
    Unit* nearestSpirit = nullptr;
    float nearestDist = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        if (!IsLkRagingSpirit(unit->GetEntry()))
            continue;

        float const dist = bot->GetExactDist2d(unit);
        if (dist > 40.0f)
            continue;

        spirits.push_back(unit);
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearestSpirit = unit;
        }
    }

    if (spirits.empty())
        return false;

    bool const isRanged = PlayerbotAI::IsRanged(bot);
    static constexpr float RANGED_SAFE_DIST = 15.0f;

    auto IsSafeFromAllSpirits = [&](float x, float y) -> bool
    {
        for (Unit* spirit : spirits)
        {
            float const dx = x - spirit->GetPositionX();
            float const dy = y - spirit->GetPositionY();
            float const d = std::hypot(dx, dy);

            if (isRanged && d < RANGED_SAFE_DIST)
                return false;

            if (d < 0.1f)
                return false;

            // Cone test: dot of spirit's facing vs vector spirit->point.
            // dot > 0 means point is in front half of spirit.
            float const dot = (std::cos(spirit->GetOrientation()) * dx +
                               std::sin(spirit->GetOrientation()) * dy) / d;
            if (dot > 0.0f)
                return false;
        }
        return true;
    };

    // Already safe from every spirit -- nothing to do
    if (IsSafeFromAllSpirits(bot->GetPositionX(), bot->GetPositionY()))
        return false;

    std::vector<Unit*> defiles;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)
            defiles.push_back(unit);
    }

    auto IsDefileSafe = [&](float x, float y) -> bool
    {
        static constexpr float SAFETY_MARGIN = 2.0f;
        for (Unit const* defile : defiles)
        {
            float const radius = GetDefileEffectiveRadius(defile, diff);
            float const d = std::hypot(x - defile->GetPositionX(), y - defile->GetPositionY());
            if (d < radius + SAFETY_MARGIN)
                return false;
        }
        return true;
    };

    static constexpr float MELEE_STEP = 7.0f;
    static constexpr float RANGED_STEP = 5.0f;
    float const stepSize = isRanged ? RANGED_STEP : MELEE_STEP;

    auto TryDest = [&](float destX, float destY) -> bool
    {
        float destZ = nearestSpirit->GetPositionZ();
        bot->UpdateAllowedPositionZ(destX, destY, destZ);

        if (!IsSafeFromAllSpirits(destX, destY))
            return false;
        if (!IsDefileSafe(destX, destY))
            return false;
        if (!bot->IsWithinLOS(destX, destY, destZ))
            return false;

        float const bDx = destX - bot->GetPositionX();
        float const bDy = destY - bot->GetPositionY();
        float const bDist = std::hypot(bDx, bDy);
        if (bDist < 0.5f)
            return false;

        // Ranged: don't chase spirits across the room. Reject destinations that
        // require more than one short hop.
        if (isRanged && bDist > RANGED_STEP * 1.5f)
            return false;

        float const step = std::min(stepSize, bDist);
        float const stepX = bot->GetPositionX() + (bDx / bDist) * step;
        float const stepY = bot->GetPositionY() + (bDy / bDist) * step;
        float stepZ = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(stepX, stepY, stepZ);

        MoveTo(bot->GetMapId(), stepX, stepY, stepZ,
               false, false, false, true,
               MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    };

    static constexpr int ANGLE_STEPS = 24;

    if (isRanged)
    {
        // Step away from current bot position. Prefer the direction directly away
        // from the nearest spirit, then sweep outward in alternating angles.
        float const awayX = bot->GetPositionX() - nearestSpirit->GetPositionX();
        float const awayY = bot->GetPositionY() - nearestSpirit->GetPositionY();
        float const awayLen = std::hypot(awayX, awayY);
        float const baseAngle = (awayLen > 0.01f) ? std::atan2(awayY, awayX) : 0.0f;

        for (int i = 0; i < ANGLE_STEPS; ++i)
        {
            int const sign = (i % 2 == 0) ? 1 : -1;
            int const offset = (i + 1) / 2;
            float const angle = baseAngle + sign * offset * (2.0f * float(M_PI) / ANGLE_STEPS);

            float const destX = bot->GetPositionX() + std::cos(angle) * RANGED_STEP;
            float const destY = bot->GetPositionY() + std::sin(angle) * RANGED_STEP;

            if (TryDest(destX, destY))
                return true;
        }

        return false;
    }

    // Melee: search rings around the nearest spirit
    static constexpr std::array<float, 4> MeleeRings = {4.0f, 6.0f, 9.0f, 13.0f};

    for (float const radius : MeleeRings)
    {
        for (int i = 0; i < ANGLE_STEPS; ++i)
        {
            float const angle = (i * 2.0f * float(M_PI)) / ANGLE_STEPS;
            float const destX = nearestSpirit->GetPositionX() + std::cos(angle) * radius;
            float const destY = nearestSpirit->GetPositionY() + std::sin(angle) * radius;

            if (TryDest(destX, destY))
                return true;
        }
    }

    return false;
}

bool IccLichKingAddsAction::HandleShamblingHorrors(Unit* /*boss*/, bool /*hasPlague*/)
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !IsLkShambling(unit->GetEntry()))
            continue;

        if (botAI->HasAura("Enrage", unit))
        {
            botAI->CastSpell("Tranquilizing Shot", unit);
            return true;
        }
    }

    return false;
}

bool IccLichKingAddsAction::HandleAssistTankAddManagement(Unit* boss, Difficulty diff)
{
    if (!PlayerbotAI::IsAssistTank(bot) || !boss)
        return false;

    // Below 71%: stun all shamblings until winter starts so they don't
    // shockwave the raid during the transition gap.
    if (boss->HealthBelowPct(72) && boss->HealthAbovePct(70))
    {
        if (!IccBossHasRemorselessWinter(boss))
        {
            GuidVector const& stunTargets = AI_VALUE(GuidVector, "possible targets");
            for (ObjectGuid const& guid : stunTargets)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;
                if (!IsLkShambling(unit->GetEntry()))
                    continue;
                if (!unit->HasAura(SPELL_HAMMER_OF_JUSTICE))
                    bot->AddAura(SPELL_HAMMER_OF_JUSTICE, unit);
            }
        }

        return false;
    }

    Position const& holdPos = IsHeroicLk(diff)
        ? ICC_LICH_KING_ASSISTHC_POSITION
        : ICC_LICH_KING_ADDS_POSITION;

    // Categorise visible adds
    GuidVector const& targets = AI_VALUE(GuidVector, "possible targets");

    std::vector<Unit*> addsOnUs;
    std::vector<Unit*> addsElsewhere;

    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsLkCollectibleAdd(unit))
            continue;

        if (unit->GetVictim() == bot)
            addsOnUs.push_back(unit);
        else
            addsElsewhere.push_back(unit);
    }

    bool const isHeroic = IsHeroicLk(diff);

    // Non-winter Raging Spirit pickup: AT grabs ALL spirits before joining MT.
    if (!IccBossHasRemorselessWinter(boss))
    {
        std::vector<Unit*> looseSpirits;
        std::vector<Unit*> spiritsOnUs;

        for (Unit* add : addsOnUs)
        {
            if (IsLkRagingSpirit(add->GetEntry()))
                spiritsOnUs.push_back(add);
        }

        for (Unit* add : addsElsewhere)
        {
            if (!IsLkRagingSpirit(add->GetEntry()))
                continue;
            if (bot->GetExactDist2d(add) > 60.0f)
                continue;
            looseSpirits.push_back(add);
        }

        if (!looseSpirits.empty() || !spiritsOnUs.empty())
        {
            // Taunt every loose spirit that we can reach
            Unit* nearestLoose = nullptr;
            float nearestLooseDist = FLT_MAX;
            for (Unit* spirit : looseSpirits)
            {
                IccCastClassTaunt(bot, botAI,spirit);
                float const d = bot->GetExactDist2d(spirit);
                if (d < nearestLooseDist)
                {
                    nearestLooseDist = d;
                    nearestLoose = spirit;
                }
            }

            // If any spirit is loose, go fetch the nearest loose one (do not drag yet)
            if (nearestLoose)
            {
                bot->SetTarget(nearestLoose->GetGUID());
                bot->SetFacingToObject(nearestLoose);
                Attack(nearestLoose);

                if (nearestLooseDist > 4.0f)
                {
                    float const dx = nearestLoose->GetPositionX() - bot->GetPositionX();
                    float const dy = nearestLoose->GetPositionY() - bot->GetPositionY();
                    float const len = std::hypot(dx, dy);
                    if (len > 0.1f)
                    {
                        float const step = std::min(7.0f, len - 3.0f);
                        if (step > 0.0f)
                        {
                            float const goalX = bot->GetPositionX() + (dx / len) * step;
                            float const goalY = bot->GetPositionY() + (dy / len) * step;
                            float goalZ = bot->GetPositionZ();
                            bot->UpdateAllowedPositionZ(goalX, goalY, goalZ);
                            MoveTo(bot->GetMapId(), goalX, goalY, goalZ,
                                   false, false, false, true,
                                   MovementPriority::MOVEMENT_FORCED, true, false);
                        }
                    }
                }
                return false;
            }

            // All spirits are on us: drag the pack toward boss
            Unit* nearestOnUs = nullptr;
            float nearestOnUsDist = FLT_MAX;
            for (Unit* spirit : spiritsOnUs)
            {
                float const d = bot->GetExactDist2d(spirit);
                if (d < nearestOnUsDist)
                {
                    nearestOnUsDist = d;
                    nearestOnUs = spirit;
                }
            }

            if (nearestOnUs)
            {
                bot->SetTarget(nearestOnUs->GetGUID());
                bot->SetFacingToObject(nearestOnUs);
                Attack(nearestOnUs);

                if (bot->GetExactDist2d(boss) > 5.0f)
                {
                    float const dx = boss->GetPositionX() - bot->GetPositionX();
                    float const dy = boss->GetPositionY() - bot->GetPositionY();
                    float const len = std::hypot(dx, dy);
                    if (len > 0.1f)
                    {
                        float const step = std::min(5.0f, len - 4.0f);
                        if (step > 0.0f)
                        {
                            float const goalX = bot->GetPositionX() + (dx / len) * step;
                            float const goalY = bot->GetPositionY() + (dy / len) * step;
                            float goalZ = bot->GetPositionZ();
                            bot->UpdateAllowedPositionZ(goalX, goalY, goalZ);
                            MoveTo(bot->GetMapId(), goalX, goalY, goalZ,
                                   false, false, false, true,
                                   MovementPriority::MOVEMENT_FORCED, true, false);
                        }
                    }
                }
                return false;
            }
        }
    }

    // Hold position during phase 1 (boss above 70%)
    if (boss->HealthAbovePct(70))
    {
        if (!isHeroic)
        {
            if (bot->GetExactDist2d(holdPos) > 5.0f)
            {
                MoveTo(bot->GetMapId(), holdPos.GetPositionX(), holdPos.GetPositionY(),
                       holdPos.GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
        else
        {
            Unit* mainTankStep1 = AI_VALUE(Unit*, "main tank");
            if (mainTankStep1 && mainTankStep1->IsAlive())
            {
                float const distToMt = bot->GetExactDist2d(mainTankStep1);

                if (distToMt < 20.0f || distToMt > 30.0f)
                {
                    float const axX = holdPos.GetPositionX() - mainTankStep1->GetPositionX();
                    float const axY = holdPos.GetPositionY() - mainTankStep1->GetPositionY();
                    float const axLen = std::hypot(axX, axY);

                    if (axLen > 0.01f)
                    {
                        float const targetDist = std::max(20.0f, std::min(30.0f, distToMt));
                        float const goalX = mainTankStep1->GetPositionX() + (axX / axLen) * targetDist;
                        float const goalY = mainTankStep1->GetPositionY() + (axY / axLen) * targetDist;
                        MoveTo(bot->GetMapId(), goalX, goalY, holdPos.GetPositionZ(),
                               false, false, false, true,
                               MovementPriority::MOVEMENT_FORCED, true, false);
                    }
                }
            }
        }
    }

    // Taunt loose adds: shamblings first, then others; nearest wins within tier.
    Unit* tauntTargetShambling = nullptr;
    float tauntDistShambling = FLT_MAX;
    Unit* tauntTargetOther = nullptr;
    float tauntDistOther = FLT_MAX;

    for (Unit* add : addsElsewhere)
    {
        float const dist = bot->GetExactDist2d(add);
        if (dist > 30.0f)
            continue;

        if (IsLkShambling(add->GetEntry()))
        {
            if (dist < tauntDistShambling)
            {
                tauntDistShambling = dist;
                tauntTargetShambling = add;
            }
        }
        else if (dist < tauntDistOther)
        {
            tauntDistOther = dist;
            tauntTargetOther = add;
        }
    }

    Unit* tauntTarget = tauntTargetShambling ? tauntTargetShambling : tauntTargetOther;
    if (tauntTarget)
        IccCastClassTaunt(bot, botAI,tauntTarget);

    // No adds at all — stay at hold position
    if (addsOnUs.empty() && addsElsewhere.empty())
        return boss->HealthAbovePct(70);

    // Target priority by mob type (stable, not victim-dependent):
    //   Tier 1 — Shambling Horror
    //   Tier 0 — Ghoul / Raging Spirit / other collectible add
    // Sticky: never switch away from a same-or-higher tier target.
    auto TargetTier = [](Unit* t) -> int
    {
        if (!t || !t->IsAlive())
            return -1;
        if (!IsLkCollectibleAdd(t))
            return -1;
        return IsLkShambling(t->GetEntry()) ? 1 : 0;
    };

    Unit* currentTarget = bot->GetVictim();
    int const currentTier = TargetTier(currentTarget);

    // Current target is a valid add — keep it unless a higher-tier add exists
    if (currentTier >= 0)
    {
        if (currentTier < 1)
        {
            for (Unit* add : addsOnUs)
            {
                if (IsLkShambling(add->GetEntry()))
                {
                    bot->SetTarget(add->GetGUID());
                    bot->SetFacingToObject(add);
                    Attack(add);
                    currentTarget = add;
                    break;
                }
            }
        }

        if (currentTarget)
        {
            bot->SetFacingToObject(currentTarget);
            Attack(currentTarget);
        }
    }
    else
    {
        // Current target is dead/invalid — pick a new one.
        // Prefer adds already on us, then nearby adds within melee reach.
        Unit* bestTarget = nullptr;
        int bestTier = -1;
        float bestDist = FLT_MAX;

        for (Unit* add : addsOnUs)
        {
            int const tier = TargetTier(add);
            if (tier < 0)
                continue;
            float const dist = bot->GetExactDist2d(add);
            if (tier > bestTier || (tier == bestTier && dist < bestDist))
            {
                bestTier = tier;
                bestDist = dist;
                bestTarget = add;
            }
        }

        if (!bestTarget)
        {
            for (Unit* add : addsElsewhere)
            {
                int const tier = TargetTier(add);
                if (tier < 0)
                    continue;
                float const dist = bot->GetExactDist2d(add);
                if (dist > 8.0f)
                    continue;
                if (tier > bestTier || (tier == bestTier && dist < bestDist))
                {
                    bestTier = tier;
                    bestDist = dist;
                    bestTarget = add;
                }
            }
        }

        if (bestTarget)
        {
            bot->SetTarget(bestTarget->GetGUID());
            bot->SetFacingToObject(bestTarget);
            Attack(bestTarget);
        }
    }

    // Face the nearest high-priority add (even when not actively attacking)
    {
        Unit* faceTarget = nullptr;
        int faceTier = -1;
        float faceDist = FLT_MAX;

        auto const& faceList = addsOnUs.empty() ? addsElsewhere : addsOnUs;
        for (Unit* add : faceList)
        {
            int const tier = TargetTier(add);
            if (tier < 0)
                continue;
            float const dist = bot->GetExactDist2d(add);
            if (tier > faceTier || (tier == faceTier && dist < faceDist))
            {
                faceTier = tier;
                faceDist = dist;
                faceTarget = add;
            }
        }

        if (faceTarget)
            bot->SetFacingToObject(faceTarget);
    }

    // Heroic: rotate any Shambling that is facing the MT
    if (isHeroic && !addsOnUs.empty())
    {
        Unit* mainTank = AI_VALUE(Unit*, "main tank");
        if (mainTank && mainTank->IsAlive())
        {
            Unit* worstShambling = nullptr;
            float worstDot = -2.0f;

            for (Unit* add : addsOnUs)
            {
                if (!IsLkShambling(add->GetEntry()))
                    continue;

                float const toMtX = mainTank->GetPositionX() - add->GetPositionX();
                float const toMtY = mainTank->GetPositionY() - add->GetPositionY();
                float const toMtLen = std::hypot(toMtX, toMtY);
                if (toMtLen < 0.01f)
                    continue;

                float const dot = (std::cos(add->GetOrientation()) * toMtX +
                                   std::sin(add->GetOrientation()) * toMtY) / toMtLen;
                if (dot > worstDot)
                {
                    worstDot = dot;
                    worstShambling = add;
                }
            }

            // dot > 0.3: Shambling is facing toward MT — reposition to turn it away
            if (worstShambling && worstDot > 0.3f)
            {
                float const axisX = worstShambling->GetPositionX() - mainTank->GetPositionX();
                float const axisY = worstShambling->GetPositionY() - mainTank->GetPositionY();
                float const axisLen = std::hypot(axisX, axisY);

                if (axisLen > 0.01f)
                {
                    float const goalX = worstShambling->GetPositionX() + (axisX / axisLen) * 5.0f;
                    float const goalY = worstShambling->GetPositionY() + (axisY / axisLen) * 5.0f;

                    if (std::hypot(goalX - bot->GetPositionX(), goalY - bot->GetPositionY()) > 1.0f)
                    {
                        MoveTo(bot->GetMapId(), goalX, goalY, bot->GetPositionZ(),
                               false, false, false, true,
                               MovementPriority::MOVEMENT_COMBAT, true, false);
                    }
                }
            }
        }
    }

    return false;
}

bool IccLichKingAddsAction::HandleMeleePositioning(Unit* boss, bool hasPlague, Difficulty diff)
{
    if (!boss || !PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsAssistTank(bot) ||
        boss->HealthBelowPct(71) || hasPlague || IsHeroicLk(diff))
        return false;

    float const distToPos = bot->GetDistance(ICC_LICH_KING_MELEE_POSITION);
    if (distToPos <= 6.0f)
        return false;

    if (!PlayerbotAI::IsMainTank(bot))
    {
        MoveTo(bot->GetMapId(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionX(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionY(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionZ(),
               false, false, false, true,
               MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    // Main tank: step toward position in 3-yard increments to avoid overshooting
    if (boss->GetVictim() != bot)
        return false;

    float const dx = ICC_LICH_KING_MELEE_POSITION.GetPositionX() - bot->GetPositionX();
    float const dy = ICC_LICH_KING_MELEE_POSITION.GetPositionY() - bot->GetPositionY();
    float const len = std::hypot(dx, dy);
    if (len < 0.1f)
        return false;

    float const step = std::min(3.0f, len - 1.0f);
    if (step <= 0.0f)
    {
        MoveTo(bot->GetMapId(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionX(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionY(),
               bot->GetPositionZ(),
               false, false, false, true,
               MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    MoveTo(bot->GetMapId(),
           bot->GetPositionX() + (dx / len) * step,
           bot->GetPositionY() + (dy / len) * step,
           bot->GetPositionZ(),
           false, false, false, true,
           MovementPriority::MOVEMENT_FORCED, true, false);

    return true;
}

bool IccLichKingAddsAction::HandleMainTankTargeting(Unit* boss, Difficulty diff)
{
    if (!PlayerbotAI::IsMainTank(bot) || !boss)
        return false;

    if (boss->GetVictim() == bot)
        return false;

    // Non-winter Raging Spirit case: stay on boss only when an assist tank is alive
    // to take the spirit. If AT is dead, fall through so MT can swap.
    if (!IccBossHasRemorselessWinter(boss))
    {
        GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        bool spiritAlive = false;
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && IsLkRagingSpirit(unit->GetEntry()))
            {
                spiritAlive = true;
                break;
            }
        }

        if (spiritAlive)
        {
            bool assistTankAlive = false;
            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
                {
                    Player* member = itr->GetSource();
                    if (member && member->IsAlive() && member != bot &&
                        PlayerbotAI::IsAssistTank(member))
                    {
                        assistTankAlive = true;
                        break;
                    }
                }
            }

            if (assistTankAlive)
            {
                bot->SetTarget(boss->GetGUID());
                bot->SetFacingToObject(boss);
                Attack(boss);
                return true;
            }
        }
    }

    if (!IsHeroicLk(diff) || boss->HealthBelowPct(71))
        return false;

    bot->SetTarget(boss->GetGUID());
    bot->SetFacingToObject(boss);
    Attack(boss);

    return true;
}

bool IccLichKingAddsAction::HandleNonTankHeroicPositioning(Unit* boss, Difficulty diff, bool hasPlague)
{
    if (PlayerbotAI::IsTank(bot) || !boss || !IsHeroicLk(diff))
        return false;

    if (boss->HealthBelowPct(71) || hasPlague)
        return false;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
        return false;

    float const distToTank = bot->GetDistance2d(mainTank);

    if (bot->getClass() == CLASS_HUNTER)
    {
        // Hunters stay within 10 yd of MT but away from the assist tank position
        if (distToTank > 10.0f)
        {
            float goalX = mainTank->GetPositionX();
            float goalY = mainTank->GetPositionY();

            float const atDx = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionX() - mainTank->GetPositionX();
            float const atDy = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionY() - mainTank->GetPositionY();
            float const atLen = std::hypot(atDx, atDy);
            if (atLen > 0.01f)
            {
                goalX -= (atDx / atLen) * 8.0f;
                goalY -= (atDy / atLen) * 8.0f;
            }

            MoveTo(bot->GetMapId(), goalX, goalY, bot->GetPositionZ(),
                   false, false, false, true,
                   MovementPriority::MOVEMENT_FORCED, true, false);
            return true;
        }
    }
    else
    {
        // Everyone else stacks on main tank
        if (distToTank > 3.0f)
        {
            MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(),
                   bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_FORCED, true, false);
            return true;
        }
    }

    return false;
}

bool IccLichKingAddsAction::HandleRangedPositioning(Unit* boss, bool hasPlague, Difficulty diff)
{
    if (!boss || !PlayerbotAI::IsRanged(bot) || boss->HealthBelowPct(71) ||
        hasPlague || IsHeroicLk(diff))
        return false;

    if (bot->GetDistance(ICC_LICH_KING_RANGED_POSITION) > 2.0f)
    {
        MoveTo(bot->GetMapId(),
               ICC_LICH_KING_RANGED_POSITION.GetPositionX(),
               ICC_LICH_KING_RANGED_POSITION.GetPositionY(),
               ICC_LICH_KING_RANGED_POSITION.GetPositionZ(),
               false, false, false, true,
               MovementPriority::MOVEMENT_FORCED, true, false);
        return true;
    }

    return false;
}

bool IccLichKingAddsAction::HandleCenterStacking(Unit* boss, Difficulty diff)
{
    if (!boss || !boss->HealthBelowPct(67) || IccBossHasRemorselessWinter(boss))
        return false;

    // Defile target: let HandleDefileMechanics() handle movement
    // (perpendicular run). Don't override with slot/center movement.
    auto const& defileInfo = IcecrownHelpers::IccState(bot->GetInstanceId()).defileCast;
    if (!defileInfo.targetGuid.IsEmpty() &&
        getMSTimeDiff(defileInfo.castTime, getMSTime()) <= 3000 &&
        defileInfo.targetGuid == bot->GetGUID())
    {
        return false;
    }

    // Marked Val'kyrs (Skull/Cross/Star) are being kited by assist - bots
    // assigned to them must not be locked to the center stack.
    if (Group* group = bot->GetGroup())
    {
        static constexpr std::array<uint8, 3> ValkyrIcons = {RtiTargetValue::skullIndex, RtiTargetValue::crossIndex,
                                                             RtiTargetValue::starIndex};
        for (uint8 const iconIdx : ValkyrIcons)
        {
            Unit* marked = botAI->GetUnit(group->GetTargetIcon(iconIdx));
            if (marked && marked->IsAlive() && IsLkValkyr(marked))
                return false;
        }
    }

    static constexpr float HUNTER_DISTANCE = 8.0f;
    static constexpr float OTHER_DISTANCE = 5.0f;
    // Worst-case stack radius: hunters sit furthest out, plus safety margin.
    // Defile reach = dist - radius; unsafe if reach < stack + safety.
    static constexpr float STACK_SAFETY = 3.0f;
    static constexpr float STACK_DEFILE_BUFFER = HUNTER_DISTANCE + STACK_SAFETY;

    bool const lastPhase   = boss->HealthBelowPct(37);
    bool const spiritPhase = boss->HealthBelowPct(43) && boss->HealthAbovePct(37);

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> defiles;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (IsLkRagingSpirit(unit->GetEntry()) || IsLkVileSpirit(unit) ||
            IsIceSphere(unit->GetEntry()))
            return false;

        if (unit->GetEntry() == DEFILE_NPC_ID)
            defiles.push_back(unit);
    }

    Position dest;
    if (spiritPhase || lastPhase)
    {
        // Phase 3: pick the safe vile-spirit slot closest to the bot.
        Position const slots[3] = {
            ICC_LK_VILE_SPIRIT1_POSITION,
            ICC_LK_VILE_SPIRIT2_POSITION,
            ICC_LK_VILE_SPIRIT3_POSITION,
        };

        auto IsSlotSafe = [&](Position const& slot) -> bool
        {
            for (Unit const* defile : defiles)
            {
                float const radius = GetDefileEffectiveRadius(defile, diff);
                float const d = std::hypot(slot.GetPositionX() - defile->GetPositionX(),
                                           slot.GetPositionY() - defile->GetPositionY());
                if (d < radius + STACK_DEFILE_BUFFER)
                    return false;
            }
            return true;
        };

        // Among safe slots (0 and 2), pick the one closest to the group
        // centroid. Centroid is identical for every bot in the raid so all
        // bots converge on the same anchor. Cached per boss GUID for 2s to
        // dampen flicker if defile state shifts between ticks.
        auto& s_stackChoice = IcecrownHelpers::IccState(boss->GetInstanceId()).mtStackChoice;
        static constexpr uint32 STACK_CHOICE_TTL_MS = 2000;
        ObjectGuid const stackKey = boss->GetGUID();

        uint32 const now = getMSTime();
        int chosen = -1;
        auto cacheIt = s_stackChoice.find(stackKey);
        if (cacheIt != s_stackChoice.end() &&
            getMSTimeDiff(cacheIt->second.evaluatedMs, now) < STACK_CHOICE_TTL_MS)
        {
            chosen = cacheIt->second.slotIdx;
        }
        else
        {
            Position const centroid = ComputeGroupCentroid(bot);
            float bestDist = std::numeric_limits<float>::max();
            for (int const i : {0, 2})
            {
                if (!IsSlotSafe(slots[i]))
                    continue;

                float const d = std::hypot(centroid.GetPositionX() - slots[i].GetPositionX(),
                                           centroid.GetPositionY() - slots[i].GetPositionY());
                if (d < bestDist)
                {
                    bestDist = d;
                    chosen = i;
                }
            }
            s_stackChoice[stackKey] = {now, chosen};
        }

        if (chosen < 0)
            return false;

        dest = slots[chosen];
    }
    else
    {
        // Phase 2: stack on platform center if it's safe from defiles.
        for (Unit const* defile : defiles)
        {
            float const radius = GetDefileEffectiveRadius(defile, diff);
            float const d = std::hypot(
                defile->GetPositionX() - ICC_LICH_KING_CENTER_POSITION.GetPositionX(),
                defile->GetPositionY() - ICC_LICH_KING_CENTER_POSITION.GetPositionY());
            if (d < radius + STACK_DEFILE_BUFFER)
                return false;
        }
        dest = ICC_LICH_KING_CENTER_POSITION;
    }

    // Main tank only moves to the stack point if he's still the boss's victim.
    if (PlayerbotAI::IsMainTank(bot) && boss->GetVictim() != bot)
        return false;

    float const distToDest = bot->GetDistance2d(dest.GetPositionX(), dest.GetPositionY());
    float const threshold = (bot->getClass() == CLASS_HUNTER) ? HUNTER_DISTANCE : OTHER_DISTANCE;
    if (distToDest <= threshold)
        return false;

    auto const [stepX, stepY] = DefileAwareStep(dest.GetPositionX(), dest.GetPositionY(), defiles, diff);
    float stepZ = dest.GetPositionZ();
    bot->UpdateAllowedPositionZ(stepX, stepY, stepZ);

    MoveTo(bot->GetMapId(), stepX, stepY, stepZ,
           false, false, false, true,
           MovementPriority::MOVEMENT_FORCED, true, false);
    return true;
}

bool IccLichKingAddsAction::HandleDefileMechanics(Unit* boss, Difficulty diff)
{
    if (!boss)
        return false;

    static constexpr float SAFETY_MARGIN = 3.0f;
    static constexpr float MOVE_DISTANCE = 5.0f;
    static constexpr float FIXED_Z = 840.857f;
    static constexpr float MAX_HEIGHT_DIFF = 5.0f;
    static constexpr int ANGLE_TESTS = 16;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> defiles;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)
            defiles.push_back(unit);
    }

    if (!defiles.empty())
    {
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();

        bool needToMove = false;
        for (Unit const* defile : defiles)
        {
            float const radius = GetDefileEffectiveRadius(defile, diff);
            float const dist = std::hypot(botX - defile->GetPositionX(),
                                          botY - defile->GetPositionY());
            if (dist < radius + SAFETY_MARGIN)
            {
                needToMove = true;
                break;
            }
        }

        if (needToMove)
        {
            float bestScore = 0.0f;
            float bestAngle = 0.0f;
            bool found = false;

            for (int i = 0; i < ANGLE_TESTS; ++i)
            {
                float const angle = i * float(M_PI) / 8.0f;
                float const testX = botX + MOVE_DISTANCE * std::cos(angle);
                float const testY = botY + MOVE_DISTANCE * std::sin(angle);
                float testZ = FIXED_Z;

                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                if (!bot->IsWithinLOS(testX, testY, testZ) ||
                    std::abs(testZ - bot->GetPositionZ()) >= MAX_HEIGHT_DIFF)
                    continue;

                float minDefileDist = std::numeric_limits<float>::max();
                for (Unit const* defile : defiles)
                {
                    float const d = std::hypot(testX - defile->GetPositionX(),
                                               testY - defile->GetPositionY());
                    minDefileDist = std::min(minDefileDist, d);
                }

                float const bossProximity = 100.0f - std::min(100.0f, boss->GetDistance2d(testX, testY));
                float const score = minDefileDist + bossProximity * 0.5f;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestAngle = angle;
                    found = true;
                }
            }

            if (found)
            {
                float moveX = botX + MOVE_DISTANCE * std::cos(bestAngle);
                float moveY = botY + MOVE_DISTANCE * std::sin(bestAngle);
                float moveZ = FIXED_Z;

                bot->CastStop();
                bot->UpdateAllowedPositionZ(moveX, moveY, moveZ);
                MoveTo(bot->GetMapId(), moveX, moveY, moveZ,
                       false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                return true;
            }
        }
    }

    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(DEFILE_CAST_ID))
        return false;

    // Boss casting Defile — only the targeted player runs out. Target is
    // stamped by IccLichKingListenerScript at OnSpellPrepare time (cast start).
    auto const& info = IcecrownHelpers::IccState(bot->GetInstanceId()).defileCast;
    if (info.targetGuid.IsEmpty() || getMSTimeDiff(info.castTime, getMSTime()) > 3000)
        return false;

    Player* target = ObjectAccessor::FindPlayer(info.targetGuid);
    if (!target || !target->IsAlive())
        return false;

    // Main tank yells once per cast.
    uint32& lastYellMs = IcecrownHelpers::IccState(bot->GetInstanceId()).lkLastYellMs;
    if (PlayerbotAI::IsMainTank(bot) && info.castTime != lastYellMs)
    {
        botAI->Yell("Defile on " + target->GetName() + " - move to the edge!");
        lastYellMs = info.castTime;
    }

    if (!target->HasAura(SPELL_NITRO_BOOSTS))
        target->AddAura(SPELL_NITRO_BOOSTS, target);

    // Real players run themselves; only the targeted bot moves.
    if (target != bot)
        return false;

    // During Vile Spirit phase, run perpendicular (left/right) to the line from
    // spirit centroid -> this bot, so defile drops sideways instead of into the
    // raid stack or into the spirit cluster.
    float spiritSumX = 0.0f;
    float spiritSumY = 0.0f;
    uint32 spiritCount = 0;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !IsLkVileSpirit(unit))
            continue;
        spiritSumX += unit->GetPositionX();
        spiritSumY += unit->GetPositionY();
        ++spiritCount;
    }

    float baseAngle;
    if (spiritCount > 0)
    {
        float const cx = spiritSumX / spiritCount;
        float const cy = spiritSumY / spiritCount;
        float const sx = bot->GetPositionX() - cx;
        float const sy = bot->GetPositionY() - cy;
        float const sLen = std::hypot(sx, sy);
        if (sLen < 0.01f)
            baseAngle = (bot->GetGUID().GetCounter() % 16) * (float(M_PI) / 8.0f);
        else
        {
            float const radial = std::atan2(sy, sx);
            // Pick left or right perpendicular based on bot GUID for stable choice
            float const sign = (bot->GetGUID().GetCounter() & 1) ? 1.0f : -1.0f;
            baseAngle = radial + sign * (float(M_PI) / 2.0f);
        }
    }
    else
    {
        Position centroid = ComputeGroupCentroid(bot);
        float const dx = bot->GetPositionX() - centroid.GetPositionX();
        float const dy = bot->GetPositionY() - centroid.GetPositionY();
        float const len = std::hypot(dx, dy);
        if (len < 0.01f)
            baseAngle = (bot->GetGUID().GetCounter() % 16) * (float(M_PI) / 8.0f);
        else
            baseAngle = std::atan2(dy, dx);
    }

    // Reject candidates landing inside any defile (radius from grow stacks).
    auto IsSafeFromDefiles = [&](float x, float y) -> bool
    {
        for (Unit const* defile : defiles)
        {
            float const radius = GetDefileEffectiveRadius(defile, diff);
            float const d = std::hypot(x - defile->GetPositionX(),
                                       y - defile->GetPositionY());
            if (d < radius + SAFETY_MARGIN)
                return false;
        }
        return true;
    };

    // Try base direction first, then fan out in +/- 22.5 deg increments.
    float destX = 0.0f;
    float destY = 0.0f;
    bool found = false;
    for (int offset = 0; offset <= 8 && !found; ++offset)
    {
        for (int dir : {-1, 1})
        {
            if (offset == 0 && dir > 0)
                continue;

            float const angle = baseAngle + dir * offset * float(M_PI) / 8.0f;
            float const tx = bot->GetPositionX() + 10.0f * std::cos(angle);
            float const ty = bot->GetPositionY() + 10.0f * std::sin(angle);
            if (!IsSafeFromDefiles(tx, ty))
                continue;

            destX = tx;
            destY = ty;
            found = true;
            break;
        }
    }

    if (!found)
        return false;

    float destZ = bot->GetPositionZ();
    bot->UpdateAllowedPositionZ(destX, destY, destZ);

    bot->CastStop();
    MoveTo(bot->GetMapId(), destX, destY, destZ,
           false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    return true;
}

bool IccLichKingAddsAction::HandleValkyrMechanics(Difficulty diff)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> grabbingValkyrs;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !IsLkValkyr(unit))
            continue;

        // Heroic: only target Val'kyrs above 49% health
        bool const isGrabbing = unit->HasAura(SPELL_HARVEST_SOUL_VALKYR) &&
                                (!IsHeroicLk(diff) || unit->HealthAbovePct(49));

        if (isGrabbing)
            grabbingValkyrs.push_back(unit);
    }

    // No active valkyrs — reset bots still targeting an excluded valkyr
    if (grabbingValkyrs.empty() || (boss && boss->HealthBelowPct(40)))
    {
        Unit* currentTarget = bot->GetVictim();
        if (currentTarget && IsLkValkyr(currentTarget) && boss)
        {
            bot->SetTarget(boss->GetGUID());
            context->GetValue<std::string>("rti")->Set("skull");
        }
        return false;
    }

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    // Defile target: let HandleDefileMechanics() handle movement
    // (perpendicular run). Don't override with Val'kyr chase.
    auto const& defileInfo = IcecrownHelpers::IccState(bot->GetInstanceId()).defileCast;
    if (!defileInfo.targetGuid.IsEmpty() && getMSTimeDiff(defileInfo.castTime, getMSTime()) <= 3000 &&
        defileInfo.targetGuid == bot->GetGUID())
        return false;

    HandleValkyrMarking(grabbingValkyrs, diff);
    HandleValkyrAssignment(grabbingValkyrs);

    return true;
}

bool IccLichKingAddsAction::HandleValkyrMarking(std::vector<Unit*> const& grabbingValkyrs,
                                                 Difficulty diff)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> sorted = grabbingValkyrs;
    std::sort(sorted.begin(), sorted.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    static constexpr std::array<uint8, 3> Icons = {RtiTargetValue::skullIndex, RtiTargetValue::crossIndex,
                                                   RtiTargetValue::starIndex};

    // Heroic: clear stale markers for Val'kyrs no longer grabbing or at wrong Z
    if (IsHeroicLk(diff))
    {
        for (uint8 const iconIdx : Icons)
        {
            Unit* marked = botAI->GetUnit(group->GetTargetIcon(iconIdx));
            if (!marked || !IsLkValkyr(marked))
                continue;

            bool const stale = !marked->HasAura(SPELL_HARVEST_SOUL_VALKYR) ||
                               std::abs(marked->GetPositionZ() - bot->GetPositionZ()) > 5.0f;
            if (stale)
                group->SetTargetIcon(iconIdx, bot->GetGUID(), ObjectGuid::Empty);
        }
    }

    // Clear icon slots beyond the current Val'kyr count
    for (size_t i = sorted.size(); i < Icons.size(); ++i)
    {
        if (!group->GetTargetIcon(Icons[i]).IsEmpty())
            group->SetTargetIcon(Icons[i], bot->GetGUID(), ObjectGuid::Empty);
    }

    // Assign an icon to each active Val'kyr.
    // Skip skull if a Raging Spirit currently owns it.
    for (size_t i = 0; i < sorted.size() && i < Icons.size(); ++i)
    {
        uint8 const iconIdx = Icons[i];

        if (iconIdx == 7)
        {
            Unit* currentSkull = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
            if (currentSkull && currentSkull->IsAlive() && IsLkRagingSpirit(currentSkull->GetEntry()))
                continue;
        }

        Unit* marked = botAI->GetUnit(group->GetTargetIcon(iconIdx));
        if (!marked || marked != sorted[i])
            group->SetTargetIcon(iconIdx, bot->GetGUID(), sorted[i]->GetGUID());
    }

    return true;
}

bool IccLichKingAddsAction::HandleValkyrAssignment(std::vector<Unit*> const& grabbingValkyrs)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (boss && boss->HealthBelowPct(40))
        return false;

    std::vector<Unit*> valid;
    for (Unit* valkyr : grabbingValkyrs)
    {
        if (valkyr && valkyr->IsAlive() && valkyr->HasAura(SPELL_HARVEST_SOUL_VALKYR))
            valid.push_back(valkyr);
    }

    if (valid.empty())
        return false;

    std::sort(valid.begin(), valid.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    // Build sorted list of non-main-tank members for deterministic assignment
    std::vector<Player*> assistMembers;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && !PlayerbotAI::IsMainTank(member))
            assistMembers.push_back(member);
    }

    if (assistMembers.empty())
        return false;

    std::sort(assistMembers.begin(), assistMembers.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto const it = std::find(assistMembers.begin(), assistMembers.end(), bot);
    if (it == assistMembers.end())
        return false;

    size_t const myIndex = std::distance(assistMembers.begin(), it);
    auto const groupSizes = IccBalancedGroupSizes(assistMembers.size(), valid.size());
    size_t const valkyrIndex = IccAssignedBucketIndex(myIndex, groupSizes);

    if (valkyrIndex >= valid.size())
        return false;

    Unit* myValkyr = valid[valkyrIndex];
    context->GetValue<std::string>("rti")->Set(IccRtiNameForBucket(valkyrIndex));

    Attack(myValkyr);

    Difficulty const diff = bot->GetRaidDifficulty();
    if (sPlayerbotAIConfig.EnableICCBuffs && IsHeroicLk(diff) &&
        !myValkyr->HasAura(SPELL_HAMMER_OF_JUSTICE))
        bot->AddAura(SPELL_HAMMER_OF_JUSTICE, myValkyr);

    ApplyCCToValkyr(myValkyr);

    return true;
}

std::pair<float, float> IccLichKingAddsAction::DefileAwareStep(float tx, float ty,
                                                       std::vector<Unit*> const& defiles,
                                                       Difficulty diff)
{
    float const px = bot->GetPositionX();
    float const py = bot->GetPositionY();
    float const ddx = tx - px;
    float const ddy = ty - py;
    float const fullLen = std::hypot(ddx, ddy);
    if (fullLen < 0.01f)
        return {tx, ty};

    static constexpr float STEP_DISTANCE = 10.0f;
    static constexpr float MARGIN = 2.0f;
    static constexpr int SAMPLES = 5;

    auto PathSafe = [&](float ex, float ey) -> bool
    {
        for (int s = 1; s <= SAMPLES; ++s)
        {
            float const t = float(s) / float(SAMPLES);
            float const sx = px + (ex - px) * t;
            float const sy = py + (ey - py) * t;
            for (Unit const* defile : defiles)
            {
                float const radius = GetDefileEffectiveRadius(defile, diff);
                float const d = std::hypot(sx - defile->GetPositionX(), sy - defile->GetPositionY());
                if (d < radius + MARGIN)
                    return false;
            }
        }
        return true;
    };

    float const stepLen = std::min(STEP_DISTANCE, fullLen);
    float const baseAngle = std::atan2(ddy, ddx);

    for (int offset = 0; offset <= 8; ++offset)
    {
        for (int dir : {-1, 1})
        {
            if (offset == 0 && dir > 0)
                continue;
            float const angle = baseAngle + dir * offset * float(M_PI) / 8.0f;
            float const ex = px + stepLen * std::cos(angle);
            float const ey = py + stepLen * std::sin(angle);
            if (PathSafe(ex, ey))
                return {ex, ey};
        }
    }

    return {tx, ty};
}

bool IccLichKingAddsAction::HandleVileSpiritMechanics()
{
    static constexpr float ARRIVE_TOLERANCE = 4.0f;

    // Defile target: let HandleDefileMechanics() handle movement
    // (perpendicular run). Don't override with spirit chase or slot
    // movement.
    auto const& defileInfo = IcecrownHelpers::IccState(bot->GetInstanceId()).defileCast;
    if (!defileInfo.targetGuid.IsEmpty() && getMSTimeDiff(defileInfo.castTime, getMSTime()) <= 3000 &&
        defileInfo.targetGuid == bot->GetGUID())
        return false;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    uint32 spiritCount = 0;
    std::vector<Unit*> spirits;
    std::vector<Unit*> defiles;

    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (IsLkVileSpirit(unit))
        {
            ++spiritCount;
            spirits.push_back(unit);
        }
        else if (unit->GetEntry() == DEFILE_NPC_ID)
        {
            defiles.push_back(unit);
        }
    }

    // Shared raid-wide slot choice. All bots converge on the same position so
    // they stay stacked. Reset when no spirits are alive. Keyed per-instance to
    // avoid cross-instance pollution when multiple ICCs run simultaneously.
    int& sharedSlot = IcecrownHelpers::IccState(bot->GetInstanceId()).lkSharedSlot;

    if (spiritCount == 0)
    {
        sharedSlot = -1;
        return false;
    }

    Difficulty const diff = bot->GetRaidDifficulty();
    auto IsSlotSafeFromDefile = [&](Position const& slot) -> bool
    {
        static constexpr float SAFETY_MARGIN = 2.0f;
        for (Unit const* defile : defiles)
        {
            float const radius = GetDefileEffectiveRadius(defile, diff);
            float const d =
                std::hypot(slot.GetPositionX() - defile->GetPositionX(), slot.GetPositionY() - defile->GetPositionY());
            if (d < radius + SAFETY_MARGIN)
                return false;
        }
        return true;
    };

    static constexpr float SPIRIT_NEAR_SLOT = 15.0f;
    auto IsSlotSafeFromSpirits = [&](Position const& slot) -> bool
    {
        for (Unit const* spirit : spirits)
        {
            float const d =
                std::hypot(slot.GetPositionX() - spirit->GetPositionX(), slot.GetPositionY() - spirit->GetPositionY());
            if (d < SPIRIT_NEAR_SLOT)
                return false;
        }
        return true;
    };

    Position const slots[3] = {
        ICC_LK_VILE_SPIRIT1_POSITION,
        ICC_LK_VILE_SPIRIT2_POSITION,
        ICC_LK_VILE_SPIRIT3_POSITION,
    };

    // Spirits beyond this radius from bot = previous wave still alive, block move
    static constexpr float OLD_SPIRIT_RADIUS = 20.0f;
    // Priority order: pos1 (0), pos3 (2), pos2 (1)
    static constexpr int SLOT_PRIORITY[3] = {0, 2, 1};

    auto HasOldSpirits = [&]() -> bool
    {
        for (Unit const* spirit : spirits)
        {
            float const d = std::hypot(bot->GetPositionX() - spirit->GetPositionX(),
                                       bot->GetPositionY() - spirit->GetPositionY());
            if (d > OLD_SPIRIT_RADIUS)
                return true;
        }
        return false;
    };

    if (sharedSlot >= 0 && sharedSlot < 3)
    {
        bool const defileHit = !IsSlotSafeFromDefile(slots[sharedSlot]);
        bool const spiritHit = !IsSlotSafeFromSpirits(slots[sharedSlot]);

        if (defileHit)
        {
            // Defile is fatal — move regardless of old spirits
            sharedSlot = -1;
        }
        else if (spiritHit)
        {
            // New spirits at current slot; wait if previous-wave spirits still alive
            if (HasOldSpirits())
                return false;

            sharedSlot = -1;
        }
        else if (sharedSlot == 1)
        {
            // On fallback slot (pos2); upgrade to pos1 or pos3 if now safe
            for (int pri : SLOT_PRIORITY)
            {
                if (pri == 1)
                    break;
                if (IsSlotSafeFromDefile(slots[pri]) && IsSlotSafeFromSpirits(slots[pri]))
                {
                    sharedSlot = pri;
                    break;
                }
            }
        }
    }

    if (sharedSlot < 0)
    {
        for (int pri : SLOT_PRIORITY)
        {
            if (!IsSlotSafeFromDefile(slots[pri]))
                continue;
            if (!IsSlotSafeFromSpirits(slots[pri]))
                continue;
            sharedSlot = pri;
            break;
        }

        if (sharedSlot < 0)
            return false;
    }

    if (sharedSlot < 0 || sharedSlot >= 3)
        return false;

    int const chosen = sharedSlot;
    Position const& slotPos = slots[chosen];

    // Assist tank: stacks with the raid at the chosen vile slot, but allowed
    // a 40y leash to intercept the spirit nearest the slot. Keeps nitro boost
    // for chase speed.
    if (PlayerbotAI::IsAssistTank(bot))
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);
        if (!bot->HasAura(SPELL_PAIN_SUPPRESION))
            bot->AddAura(SPELL_PAIN_SUPPRESION, bot);
        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        static constexpr float LEASH_RADIUS = 40.0f;

        float const anchorX = slotPos.GetPositionX();
        float const anchorY = slotPos.GetPositionY();

        // Find spirit closest to the slot (not to bot) — that's the spirit
        // most likely to reach the raid stack first.
        Unit* chaseTarget = nullptr;
        float chaseTargetDist = std::numeric_limits<float>::max();
        for (Unit* spirit : spirits)
        {
            float const d = std::hypot(spirit->GetPositionX() - anchorX, spirit->GetPositionY() - anchorY);
            if (d < chaseTargetDist)
            {
                chaseTargetDist = d;
                chaseTarget = spirit;
            }
        }

        float ax = anchorX;
        float ay = anchorY;
        if (chaseTarget)
        {
            float tx = chaseTarget->GetPositionX();
            float ty = chaseTarget->GetPositionY();
            float const dxc = tx - anchorX;
            float const dyc = ty - anchorY;
            float const lenc = std::hypot(dxc, dyc);
            if (lenc > LEASH_RADIUS)
            {
                // Spirit too far from slot — clamp chase point to leash radius
                tx = anchorX + dxc * LEASH_RADIUS / lenc;
                ty = anchorY + dyc * LEASH_RADIUS / lenc;
            }
            ax = tx;
            ay = ty;
        }

        float az = slotPos.GetPositionZ();

        float const distToAnchor = std::hypot(bot->GetPositionX() - ax, bot->GetPositionY() - ay);
        if (distToAnchor <= ARRIVE_TOLERANCE)
            return false;

        auto const [sx, sy] = DefileAwareStep(ax, ay, defiles, diff);
        float sz = az;
        bot->UpdateAllowedPositionZ(sx, sy, sz);

        return MoveTo(bot->GetMapId(), sx, sy, sz, false, false, false, true, MovementPriority::MOVEMENT_FORCED, true,
                      false);
    }

    // Hunter trap duty: only the lowest-GUID alive bot hunter handles it so
    // multiple hunters don't pile up at center. Real-player hunters are skipped
    // — they may not know the strategy, so a bot owns the role.
    bool isDesignatedHunter = false;
    if (bot->getClass() == CLASS_HUNTER)
    {
        Group* hunterGroup = bot->GetGroup();
        if (hunterGroup)
        {
            ObjectGuid bestGuid;
            for (GroupReference* ref = hunterGroup->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive())
                    continue;
                if (member->getClass() != CLASS_HUNTER)
                    continue;
                if (!GET_PLAYERBOT_AI(member))
                    continue;

                if (bestGuid.IsEmpty() || member->GetGUID() < bestGuid)
                    bestGuid = member->GetGUID();
            }
            isDesignatedHunter = (!bestGuid.IsEmpty() && bot->GetGUID() == bestGuid);
        }
    }

    // Skip the spirit-flee logic entirely if boss is casting Harvest Soul(s).
    // The harvested player must stay alive — flee movement breaks range for
    // healers and gets the soul victim killed, buffing LK and wiping raid.
    Unit* Boss = AI_VALUE2(Unit*, "find target", "the lich king");
    bool const bossCastingHarvest = Boss && Boss->HasUnitState(UNIT_STATE_CASTING) &&
        (Boss->FindCurrentSpellBySpellId(SPELL_HARVEST_SOUL_LK) ||
         Boss->FindCurrentSpellBySpellId(SPELL_HARVEST_SOULS_LK_25) ||
         Boss->FindCurrentSpellBySpellId(SPELL_HARVEST_SOULS_LK_H1) ||
         Boss->FindCurrentSpellBySpellId(SPELL_HARVEST_SOULS_LK_H2) ||
         Boss->FindCurrentSpellBySpellId(SPELL_HARVEST_SOULS_LK_H3));

    if (!PlayerbotAI::IsTank(bot) && !bossCastingHarvest)
    {
        // Flee to MT if a spirit is targeting this bot OR is within FLEE_RANGE.
        // Either condition is enough — proximity catches spirits that haven't
        // committed a target yet, targeting catches faraway chasers.
        static constexpr float FLEE_RANGE = 15.0f;

        Unit* chaser = nullptr;
        for (Unit* spirit : spirits)
        {
            bool const isTargetingBot = spirit->GetVictim() &&
                                        spirit->GetVictim()->GetGUID() == bot->GetGUID();
            bool const isClose = bot->GetDistance2d(spirit) < FLEE_RANGE;
            if (isTargetingBot || isClose)
            {
                chaser = spirit;
                break;
            }
        }

        if (chaser)
        {
            // Flee toward main tank in 10y increments, leash ignored
            Unit* mainTank = AI_VALUE(Unit*, "main tank");
            if (mainTank && mainTank->IsAlive())
            {
                auto const [fx, fy] = DefileAwareStep(mainTank->GetPositionX(), mainTank->GetPositionY(), defiles, diff);
                float fz = slotPos.GetPositionZ();
                bot->UpdateAllowedPositionZ(fx, fy, fz);
                return MoveTo(bot->GetMapId(), fx, fy, fz, false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                              true, false);
            }
            // No MT — fall through to leash logic
        }
    }

    // Designated hunter: anchored at platform center with 30y leash. Drops
    // Frost Trap to slow spirits. If a spirit is targeting the hunter, flee
    // toward main tank in 10y steps (leash ignored, trap not dropped).
    if (isDesignatedHunter)
    {
        static constexpr float HUNTER_CENTER_X = 503.62f;
        static constexpr float HUNTER_CENTER_Y = -2124.73f;
        static constexpr float HUNTER_LEASH = 4.0f;
        static constexpr float HUNTER_DROP_TOLERANCE = 5.0f;

        float const dxh = bot->GetPositionX() - HUNTER_CENTER_X;
        float const dyh = bot->GetPositionY() - HUNTER_CENTER_Y;
        float const distToCenter = std::hypot(dxh, dyh);

        if (distToCenter > HUNTER_LEASH)
        {
            auto const [hx, hy] = DefileAwareStep(HUNTER_CENTER_X, HUNTER_CENTER_Y, defiles, diff);
            float hz = slotPos.GetPositionZ();
            bot->UpdateAllowedPositionZ(hx, hy, hz);
            return MoveTo(bot->GetMapId(), hx, hy, hz, false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }

        if (distToCenter <= HUNTER_DROP_TOLERANCE && !bot->HasSpellCooldown(SPELL_FROST_TRAP1))
        {
            if (botAI->CastSpell("frost trap", bot))
                return true;
        }

        return false;
    }

    // Healers may stay up to 20y from the slot so they can heal the assist tank
    float const arriveTol = PlayerbotAI::IsHeal(bot) ? 20.0f : ARRIVE_TOLERANCE;

    float const tx = slotPos.GetPositionX();
    float const ty = slotPos.GetPositionY();
    float const distToSlot = std::hypot(bot->GetPositionX() - tx, bot->GetPositionY() - ty);
    if (distToSlot <= arriveTol)
        return false;

    auto const [sx, sy] = DefileAwareStep(tx, ty, defiles, diff);
    float sz = slotPos.GetPositionZ();
    bot->UpdateAllowedPositionZ(sx, sy, sz);

    return MoveTo(bot->GetMapId(), sx, sy, sz, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IccLichKingAddsAction::HandleIceSphereMechanics()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static constexpr uint8 SPHERE_ICON = RtiTargetValue::circleIndex;

    GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    Unit* currentMark = botAI->GetUnit(group->GetTargetIcon(SPHERE_ICON));
    bool const sphereMarked = currentMark && currentMark->IsAlive() &&
                              IsIceSphere(currentMark->GetEntry());

    if (!sphereMarked)
    {
        Unit* nearestSphere = nullptr;
        float nearestDist = std::numeric_limits<float>::max();

        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || !IsIceSphere(unit->GetEntry()))
                continue;

            float const dist = bot->GetDistance(unit);
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestSphere = unit;
            }
        }

        if (nearestSphere)
            group->SetTargetIcon(SPHERE_ICON, bot->GetGUID(), nearestSphere->GetGUID());
        else
        {
            if (!group->GetTargetIcon(SPHERE_ICON).IsEmpty())
                group->SetTargetIcon(SPHERE_ICON, bot->GetGUID(), ObjectGuid::Empty);
            return false;
        }

        currentMark = botAI->GetUnit(group->GetTargetIcon(SPHERE_ICON));
    }

    if (currentMark && currentMark->IsAlive())
    {
        bot->SetTarget(currentMark->GetGUID());
        bot->SetFacingToObject(currentMark);
        Attack(currentMark);
        return true;
    }

    return false;
}

bool IccLichKingAddsAction::IsValkyr(Unit* unit)
{
    return IsLkValkyr(unit);
}

bool IccLichKingAddsAction::ApplyCCToValkyr(Unit* valkyr)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            if (!botAI->HasAura("Deep Freeze", valkyr) && botAI->CanCastSpell("Deep Freeze", valkyr))
                return botAI->CastSpell("Deep Freeze", valkyr);
            if (!botAI->HasAura("Frost Nova", valkyr) && botAI->CanCastSpell("Frost Nova", valkyr))
                return botAI->CastSpell("Frost Nova", valkyr);
            if (!botAI->HasAura("Cone of Cold", valkyr) && botAI->CanCastSpell("Cone of Cold", valkyr))
                return botAI->CastSpell("Cone of Cold", valkyr);
            if (!botAI->HasAura("Frostbolt", valkyr) && botAI->CanCastSpell("Frostbolt", valkyr))
                return botAI->CastSpell("Frostbolt", valkyr);
            if (!botAI->HasAura("Slow", valkyr) && botAI->CanCastSpell("Slow", valkyr))
                return botAI->CastSpell("Slow", valkyr);
            break;
        case CLASS_DRUID:
            if (!botAI->HasAura("Bash", valkyr) && botAI->CanCastSpell("Bash", valkyr))
                return botAI->CastSpell("Bash", valkyr);
            if (!botAI->HasAura("Maim", valkyr) && botAI->CanCastSpell("Maim", valkyr))
                return botAI->CastSpell("Maim", valkyr);
            break;
        case CLASS_PALADIN:
            if (!botAI->HasAura("Hammer of Justice", valkyr) && botAI->CanCastSpell("Hammer of Justice", valkyr))
                return botAI->CastSpell("Hammer of Justice", valkyr);
            break;
        case CLASS_WARRIOR:
            if (!botAI->HasAura("Concussion Blow", valkyr) && botAI->CanCastSpell("Concussion Blow", valkyr))
                return botAI->CastSpell("Concussion Blow", valkyr);
            if (!botAI->HasAura("Shockwave", valkyr) && botAI->CanCastSpell("Shockwave", valkyr))
                return botAI->CastSpell("Shockwave", valkyr);
            if (!botAI->HasAura("Intercept", valkyr) && botAI->CanCastSpell("Intercept", valkyr))
                return botAI->CastSpell("Intercept", valkyr);
            if (!botAI->HasAura("Charge", valkyr) && botAI->CanCastSpell("Charge", valkyr))
                return botAI->CastSpell("Charge", valkyr);
            if (!botAI->HasAura("Hamstring", valkyr) && botAI->CanCastSpell("Hamstring", valkyr))
                return botAI->CastSpell("Hamstring", valkyr);
            if (!botAI->HasAura("Piercing Howl", valkyr) && botAI->CanCastSpell("Piercing Howl", valkyr))
                return botAI->CastSpell("Piercing Howl", valkyr);
            break;
        case CLASS_HUNTER:
            if (!botAI->HasAura("Intimidation", valkyr) && botAI->CanCastSpell("Intimidation", valkyr))
                return botAI->CastSpell("Intimidation", valkyr);
            if (!botAI->HasAura("Concussive Shot", valkyr) && botAI->CanCastSpell("Concussive Shot", valkyr))
                return botAI->CastSpell("Concussive Shot", valkyr);
            if (!botAI->HasAura("Wing Clip", valkyr) && botAI->CanCastSpell("Wing Clip", valkyr))
                return botAI->CastSpell("Wing Clip", valkyr);
            if (!botAI->HasAura("Freezing Trap", valkyr) && botAI->CanCastSpell("Freezing Trap", valkyr))
                return botAI->CastSpell("Freezing Trap", valkyr);
            break;
        case CLASS_ROGUE:
            if (!botAI->HasAura("Kidney Shot", valkyr) && botAI->CanCastSpell("Kidney Shot", valkyr))
                return botAI->CastSpell("Kidney Shot", valkyr);
            if (!botAI->HasAura("Gouge", valkyr) && botAI->CanCastSpell("Gouge", valkyr))
                return botAI->CastSpell("Gouge", valkyr);
            if (!botAI->HasAura("Blind", valkyr) && botAI->CanCastSpell("Blind", valkyr))
                return botAI->CastSpell("Blind", valkyr);
            if (!botAI->HasAura("Deadly Throw", valkyr) && botAI->CanCastSpell("Deadly Throw", valkyr))
                return botAI->CastSpell("Deadly Throw", valkyr);
            break;
        case CLASS_SHAMAN:
            if (!botAI->HasAura("Thunderstorm", valkyr) && botAI->CanCastSpell("Thunderstorm", valkyr))
                return botAI->CastSpell("Thunderstorm", valkyr);
            if (!botAI->HasAura("Frost Shock", valkyr) && botAI->CanCastSpell("Frost Shock", valkyr))
                return botAI->CastSpell("Frost Shock", valkyr);
            if (!botAI->HasAura("Earthbind Totem", valkyr) && botAI->CanCastSpell("Earthbind Totem", valkyr))
                return botAI->CastSpell("Earthbind Totem", valkyr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (!botAI->HasAura("Hungering Cold", valkyr) && botAI->CanCastSpell("Hungering Cold", valkyr))
                return botAI->CastSpell("Hungering Cold", valkyr);
            if (!botAI->HasAura("Gnaw", valkyr) && botAI->CanCastSpell("Gnaw", valkyr))
                return botAI->CastSpell("Gnaw", valkyr);
            if (!botAI->HasAura("Chains of Ice", valkyr) && botAI->CanCastSpell("Chains of Ice", valkyr))
                return botAI->CastSpell("Chains of Ice", valkyr);
            if (!botAI->HasAura("Desecration", valkyr) && botAI->CanCastSpell("Desecration", valkyr))
                return botAI->CastSpell("Desecration", valkyr);
            break;
        case CLASS_PRIEST:
            if (!botAI->HasAura("Psychic Horror", valkyr) && botAI->CanCastSpell("Psychic Horror", valkyr))
                return botAI->CastSpell("Psychic Horror", valkyr);
            if (!botAI->HasAura("Mind Flay", valkyr) && botAI->CanCastSpell("Mind Flay", valkyr))
                return botAI->CastSpell("Mind Flay", valkyr);
            break;
        case CLASS_WARLOCK:
            if (!botAI->HasAura("Shadowfury", valkyr) && botAI->CanCastSpell("Shadowfury", valkyr))
                return botAI->CastSpell("Shadowfury", valkyr);
            if (!botAI->HasAura("Death Coil", valkyr) && botAI->CanCastSpell("Death Coil", valkyr))
                return botAI->CastSpell("Death Coil", valkyr);
            if (!botAI->HasAura("Curse of Exhaustion", valkyr) && botAI->CanCastSpell("Curse of Exhaustion", valkyr))
                return botAI->CastSpell("Curse of Exhaustion", valkyr);
            break;
        default:
            break;
    }

    return false;
}
