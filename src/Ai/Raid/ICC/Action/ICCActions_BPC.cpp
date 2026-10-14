/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

static float const BPC_FLOOR_Z = 361.18222f;

bool IccBpcKelesethTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss)
        return false;

    static float const Z_TELEPORT_THRESHOLD = 6.0f;

    // Teleport Z-axis back to center-level if the bot is significantly off in Z
    float centerZ = ICC_BPC_CENTER_POSITION.GetPositionZ();
    if (std::abs(bot->GetPositionZ() - centerZ) > Z_TELEPORT_THRESHOLD)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), centerZ, bot->GetOrientation());
        return false;
    }

    if (!PlayerbotAI::IsAssistTank(bot))
    {
        // Non-assist-tank: pull toward center if too far
        static float const CENTER_MOVE_THRESHOLD = 30.0f;
        static float const MOVE_INCREMENT = 15.0f;
        float distToCenter = bot->GetExactDist2d(ICC_BPC_CENTER_POSITION);
        if (distToCenter > CENTER_MOVE_THRESHOLD)
        {
            float dirX = ICC_BPC_CENTER_POSITION.GetPositionX() - bot->GetPositionX();
            float dirY = ICC_BPC_CENTER_POSITION.GetPositionY() - bot->GetPositionY();
            float length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length > 0.001f)
            {
                dirX /= length;
                dirY /= length;
                float needed = distToCenter - CENTER_MOVE_THRESHOLD;
                float step = std::min(MOVE_INCREMENT, needed);
                float moveX = bot->GetPositionX() + dirX * step;
                float moveY = bot->GetPositionY() + dirY * step;
                MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
                return false;
            }
        }
    }

    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    bool const isBossVictim = boss->GetVictim() == bot;

    if (!isBossVictim)
    {
        IccCastClassTaunt(bot, botAI, boss);
        bot->SetTarget(boss->GetGUID());
        bot->SetFacingToObject(boss);
    }
    else
    {
        // Collect stray Dark Nuclei via taunt, move toward if out of range
        static float const TAUNT_RANGE = 30.0f;
        std::list<Creature*> nuclei;
        bot->GetCreatureListWithEntryInGrid(nuclei, NPC_DARK_NUCLEUS, 100.0f);

        Unit* strayNucleus = nullptr;
        for (Creature* nucleus : nuclei)
        {
            if (nucleus && nucleus->IsAlive() && nucleus->GetVictim() != bot)
            {
                strayNucleus = nucleus;
                break;
            }
        }

        if (strayNucleus)
        {
            float dist = bot->GetExactDist2d(strayNucleus);
            if (dist <= TAUNT_RANGE)
                IccCastClassTaunt(bot, botAI, strayNucleus);
            else
            {
                float dirX = strayNucleus->GetPositionX() - bot->GetPositionX();
                float dirY = strayNucleus->GetPositionY() - bot->GetPositionY();
                float length = std::sqrt(dirX * dirX + dirY * dirY);
                if (length > 0.001f)
                {
                    dirX /= length;
                    dirY /= length;
                    static float const NUCLEUS_MOVE_STEP = 10.0f;
                    static float const NUCLEUS_APPROACH_BUFFER = 5.0f;
                    float step = std::min(NUCLEUS_MOVE_STEP, dist - TAUNT_RANGE + NUCLEUS_APPROACH_BUFFER);
                    float moveX = bot->GetPositionX() + dirX * step;
                    float moveY = bot->GetPositionY() + dirY * step;
                    MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(),
                           false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
                }
                return false;
            }
        }
    }

    bot->SetTarget(boss->GetGUID());
    bot->SetFacingToObject(boss);
    Attack(boss);

    return false;
}

bool IccBpcMainTankAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsMainTank(bot))
    {
        if (PlayerbotAI::IsTank(bot))
            MarkEmpoweredPrince();
        return false;
    }

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");

    bool const isVictimOfValanar = valanar && valanar->GetVictim() == bot;
    bool const isVictimOfTaldaram = taldaram && taldaram->GetVictim() == bot;

    // Move to MT position if targeted by both princes and not already close
    static float const MT_POSITION_THRESHOLD = 15.0f;
    if (isVictimOfValanar && isVictimOfTaldaram && bot->GetExactDist2d(ICC_BPC_MT_POSITION) > MT_POSITION_THRESHOLD)
    {
        float dirX = ICC_BPC_MT_POSITION.GetPositionX() - bot->GetPositionX();
        float dirY = ICC_BPC_MT_POSITION.GetPositionY() - bot->GetPositionY();
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.001f)
        {
            dirX /= length;
            dirY /= length;
            float moveDist = std::min(3.0f, length);
            float moveX = bot->GetPositionX() + dirX * moveDist;
            float moveY = bot->GetPositionY() + dirY * moveDist;
            MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                   MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Taunt princes not targeting us
    if (valanar && !isVictimOfValanar)
    {
        IccCastClassTaunt(bot, botAI, valanar);
        bot->SetTarget(valanar->GetGUID());
        bot->SetFacingToObject(valanar);
        Attack(valanar);
    }

    if (taldaram && !isVictimOfTaldaram)
    {
        IccCastClassTaunt(bot, botAI, taldaram);
        bot->SetTarget(taldaram->GetGUID());
        bot->SetFacingToObject(taldaram);
        Attack(taldaram);
    }

    // Only the princes are tanked here (Dark Nuclei are handled by the Keleseth tank).
    // Never taunt other hostile NPCs: threat/fixate bypasses taunt immunity and would
    // lock a Ball of Flame / Kinetic Bomb onto the tank, breaking that mechanic.

    // Target marking for all tanks, called after main tank priority actions
    if (PlayerbotAI::IsTank(bot))
        MarkEmpoweredPrince();

    return false;
}

bool IccBpcMainTankAction::MarkEmpoweredPrince()
{
    static constexpr uint8 SKULL_RAID_ICON = RtiTargetValue::skullIndex;

    // Find empowered prince (Invocation of Blood)
    Unit* empoweredPrince = nullptr;
    GuidVector const& targets = AI_VALUE(GuidVector, "possible targets");

    for (auto const& targetGuid : targets)
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (botAI->HasAura("Invocation of Blood", unit))
        {
            uint32 const entry = unit->GetEntry();
            if (entry == NPC_PRINCE_KELESETH || entry == NPC_PRINCE_VALANAR || entry == NPC_PRINCE_TALDARAM)
            {
                empoweredPrince = unit;
                break;
            }
        }
    }

    // Handle marking if we found an empowered prince
    if (empoweredPrince && empoweredPrince->IsAlive())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            ObjectGuid const currentSkullGuid = group->GetTargetIcon(SKULL_RAID_ICON);
            Unit* markedUnit = botAI->GetUnit(currentSkullGuid);

            // Clear dead marks or marks that are not on empowered prince. Null out
            // markedUnit here too - otherwise the check below still sees the stale
            // guid/unit from before the clear and skips re-marking for a full tick.
            if (markedUnit && (!markedUnit->IsAlive() || markedUnit != empoweredPrince))
            {
                group->SetTargetIcon(SKULL_RAID_ICON, bot->GetGUID(), ObjectGuid::Empty);
                markedUnit = nullptr;
            }

            // Mark alive empowered prince if needed
            if (!markedUnit)
                group->SetTargetIcon(SKULL_RAID_ICON, bot->GetGUID(), empoweredPrince->GetGUID());
        }
    }

    return false;
}

bool IccBpcEmpoweredVortexAction::Execute(Event /*event*/)
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    // Check if boss is casting empowered vortex
    bool const isCastingVortex = valanar->HasUnitState(UNIT_STATE_CASTING) &&
                           (valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX1) ||
                            valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX2) ||
                            valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX3) ||
                            valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX4));

    // Use complex positioning system for empowered vortex
    if (isCastingVortex)
        return HandleEmpoweredVortexSpread();

    // Use simple ranged spreading for non-vortex situations
    return MaintainRangedSpacing();
}

bool IccBpcEmpoweredVortexAction::MaintainRangedSpacing()
{
    static float const IDEAL_RADIUS = 25.0f;
    static float const RADIUS_TOLERANCE = 3.0f;
    static float const MOVE_INCREMENT = 3.0f;

    bool const isRanged = PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot);
    if (!isRanged)
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    // Collect ranged/healer bots sorted by GUID for consistent slot assignment
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedBots;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (PlayerbotAI::IsTank(member))
            continue;
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI)
            continue;
        if (PlayerbotAI::IsRanged(member) || PlayerbotAI::IsHeal(member))
            rangedBots.push_back(member);
    }

    if (rangedBots.empty())
        return false;

    std::sort(rangedBots.begin(), rangedBots.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    // Find this bot's slot index
    int slotIndex = -1;
    for (size_t i = 0; i < rangedBots.size(); ++i)
    {
        if (rangedBots[i] == bot)
        {
            slotIndex = static_cast<int>(i);
            break;
        }
    }

    if (slotIndex < 0)
        return false;

    // Calculate angular position around Valanar
    float angleStep = 2.0f * static_cast<float>(M_PI) / static_cast<float>(rangedBots.size());
    float targetAngle = angleStep * static_cast<float>(slotIndex);

    // Collect shock vortex positions to avoid
    static float const VORTEX_AVOID_DIST = 13.0f;
    std::list<Creature*> vortexList;
    bot->GetCreatureListWithEntryInGrid(vortexList, NPC_SHOCK_VORTEX, 100.0f);

    // Find safe angle — try assigned slot first, then nudge away from vortices
    static float const ANGLE_NUDGE = static_cast<float>(M_PI) / 18.0f;
    static int const MAX_NUDGE_STEPS = 18;
    float bestAngle = targetAngle;

    auto isAngleSafe = [&](float angle) -> bool
    {
        float testX = valanar->GetPositionX() + IDEAL_RADIUS * std::cos(angle);
        float testY = valanar->GetPositionY() + IDEAL_RADIUS * std::sin(angle);
        for (Creature* vortex : vortexList)
        {
            if (!vortex || !vortex->IsAlive())
                continue;
            float vdx = testX - vortex->GetPositionX();
            float vdy = testY - vortex->GetPositionY();
            if (std::sqrt(vdx * vdx + vdy * vdy) < VORTEX_AVOID_DIST)
                return false;
        }
        return true;
    };

    if (!isAngleSafe(bestAngle))
    {
        for (int i = 1; i <= MAX_NUDGE_STEPS; ++i)
        {
            float plusAngle = targetAngle + ANGLE_NUDGE * static_cast<float>(i);
            if (isAngleSafe(plusAngle))
            {
                bestAngle = plusAngle;
                break;
            }
            float minusAngle = targetAngle - ANGLE_NUDGE * static_cast<float>(i);
            if (isAngleSafe(minusAngle))
            {
                bestAngle = minusAngle;
                break;
            }
        }
    }

    float targetX = valanar->GetPositionX() + IDEAL_RADIUS * std::cos(bestAngle);
    float targetY = valanar->GetPositionY() + IDEAL_RADIUS * std::sin(bestAngle);
    float targetZ = bot->GetPositionZ();

    float distToSlot = bot->GetExactDist2d(targetX, targetY);
    if (distToSlot <= RADIUS_TOLERANCE)
        return false;

    // Move toward assigned slot
    float dx = targetX - bot->GetPositionX();
    float dy = targetY - bot->GetPositionY();
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        return false;

    dx /= len;
    dy /= len;
    float step = std::min(MOVE_INCREMENT, distToSlot);
    float moveX = bot->GetPositionX() + dx * step;
    float moveY = bot->GetPositionY() + dy * step;

    if (bot->IsWithinLOS(moveX, moveY, targetZ))
    {
        MoveTo(bot->GetMapId(), moveX, moveY, targetZ, false, false, false, true,
               MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool IccBpcEmpoweredVortexAction::HandleEmpoweredVortexSpread()
{
    auto& spreadLockTimers = IcecrownHelpers::IccState(bot->GetInstanceId()).bpcSpreadLockTimers;
    static uint32 const SPREAD_LOCK_DURATION_MS = 250;
    static float const MOVE_INCREMENT = 4.0f;
    static float const SLOT_TOLERANCE = 2.0f;
    static float const MIN_SPACING = 13.0f;

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    // If locked in position, don't move — allow combat/healing
    uint32 now = getMSTime();

    // Prune expired entries to prevent unbounded growth
    for (auto it = spreadLockTimers.begin(); it != spreadLockTimers.end(); )
    {
        if (getMSTimeDiff(it->second, now) >= SPREAD_LOCK_DURATION_MS)
            it = spreadLockTimers.erase(it);
        else
            ++it;
    }

    auto it = spreadLockTimers.find(bot->GetGUID());
    if (it != spreadLockTimers.end())
        return false;

    // Collect ALL non-tank bots for spreading (melee must scatter too)
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> spreadBots;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (PlayerbotAI::IsTank(member))
            continue;
        spreadBots.push_back(member);
    }

    if (spreadBots.empty())
        return false;

    std::sort(spreadBots.begin(), spreadBots.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    int slotIndex = -1;
    for (size_t i = 0; i < spreadBots.size(); ++i)
    {
        if (spreadBots[i] == bot)
        {
            slotIndex = static_cast<int>(i);
            break;
        }
    }

    if (slotIndex < 0)
        return false;

    // Calculate radius so angular spacing >= MIN_SPACING
    // Arc length between slots = 2*pi*R / N, need >= MIN_SPACING
    float const botCount = static_cast<float>(spreadBots.size());
    float idealRadius = (botCount * MIN_SPACING) / (2.0f * static_cast<float>(M_PI));
    // Clamp radius to room bounds (room ~40y radius from center)
    if (idealRadius < 15.0f)
        idealRadius = 15.0f;
    if (idealRadius > 38.0f)
        idealRadius = 38.0f;

    float angleStep = 2.0f * static_cast<float>(M_PI) / botCount;
    float targetAngle = angleStep * static_cast<float>(slotIndex);

    // Collect shock vortex positions to avoid
    static float const VORTEX_AVOID_DIST = 13.0f;
    std::list<Creature*> vortexList;
    bot->GetCreatureListWithEntryInGrid(vortexList, NPC_SHOCK_VORTEX, 100.0f);

    // Find safe angle — try assigned slot first, then nudge away from vortices
    static float const ANGLE_NUDGE = static_cast<float>(M_PI) / 18.0f;
    static int const MAX_NUDGE_STEPS = 18;
    float bestAngle = targetAngle;

    auto isAngleSafe = [&](float angle) -> bool
    {
        float testX = valanar->GetPositionX() + idealRadius * std::cos(angle);
        float testY = valanar->GetPositionY() + idealRadius * std::sin(angle);
        for (Creature* vortex : vortexList)
        {
            if (!vortex || !vortex->IsAlive())
                continue;
            float vdx = testX - vortex->GetPositionX();
            float vdy = testY - vortex->GetPositionY();
            if (std::sqrt(vdx * vdx + vdy * vdy) < VORTEX_AVOID_DIST)
                return false;
        }
        return true;
    };

    if (!isAngleSafe(bestAngle))
    {
        for (int i = 1; i <= MAX_NUDGE_STEPS; ++i)
        {
            float plusAngle = targetAngle + ANGLE_NUDGE * static_cast<float>(i);
            if (isAngleSafe(plusAngle))
            {
                bestAngle = plusAngle;
                break;
            }
            float minusAngle = targetAngle - ANGLE_NUDGE * static_cast<float>(i);
            if (isAngleSafe(minusAngle))
            {
                bestAngle = minusAngle;
                break;
            }
        }
    }

    float targetX = valanar->GetPositionX() + idealRadius * std::cos(bestAngle);
    float targetY = valanar->GetPositionY() + idealRadius * std::sin(bestAngle);
    float targetZ = bot->GetPositionZ();

    float distToSlot = bot->GetExactDist2d(targetX, targetY);

    // Close enough to slot — lock position
    if (distToSlot <= SLOT_TOLERANCE)
    {
        spreadLockTimers[bot->GetGUID()] = now;
        return false;
    }

    // Move toward assigned slot
    float dx = targetX - bot->GetPositionX();
    float dy = targetY - bot->GetPositionY();
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f)
        return false;

    dx /= len;
    dy /= len;
    float step = std::min(MOVE_INCREMENT, distToSlot);
    float moveX = bot->GetPositionX() + dx * step;
    float moveY = bot->GetPositionY() + dy * step;

    if (bot->IsWithinLOS(moveX, moveY, targetZ))
    {
        MoveTo(bot->GetMapId(), moveX, moveY, targetZ, false, false, false, true,
               MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool IccBpcKineticBombAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    static float const MAX_HEIGHT_DIFF = 30.0f;
    static float const SAFE_HEIGHT_Z = 371.16473f;
    static float const TELEPORT_DOWN_Z = 367.16473f;

    // Fix bot stuck above arena
    if (bot->GetPositionZ() > SAFE_HEIGHT_Z)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), TELEPORT_DOWN_Z,
                        bot->GetOrientation());
    }

    // Assign bombs 1-to-1 to nearest available ranged DPS
    Unit* assignedBomb = FindNearestBomb();
    if (!assignedBomb)
        return false;

    // Already attacking a valid bomb that's still low enough, keep going
    if (Unit* currentTarget = AI_VALUE(Unit*, "current target"))
    {
        if (currentTarget == assignedBomb && currentTarget->IsAlive())
        {
            float const heightDiff = currentTarget->GetPositionZ() - BPC_FLOOR_Z;
            if (heightDiff < MAX_HEIGHT_DIFF)
                return false;
        }
    }

    bot->SetTarget(assignedBomb->GetGUID());
    bot->SetFacingToObject(assignedBomb);
    Attack(assignedBomb);
    return false;
}

Unit* IccBpcKineticBombAction::FindNearestBomb()
{
    static constexpr std::array<uint32, 4> BOMB_ENTRIES = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2, NPC_KINETIC_BOMB3,
                                                           NPC_KINETIC_BOMB4};
    static float const MAX_HEIGHT_DIFF = 35.0f;

    GuidVector const targets = AI_VALUE(GuidVector, "possible targets no los");
    if (targets.empty())
        return nullptr;

    // Gather all valid bombs, sorted by Z ascending (lowest = most urgent)
    std::vector<Unit*> kineticBombs;
    for (auto const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (std::find(BOMB_ENTRIES.begin(), BOMB_ENTRIES.end(), unit->GetEntry()) == BOMB_ENTRIES.end())
            continue;

        if (unit->GetPositionZ() - BPC_FLOOR_Z > MAX_HEIGHT_DIFF)
            continue;

        kineticBombs.push_back(unit);
    }

    if (kineticBombs.empty())
        return nullptr;

    std::sort(kineticBombs.begin(), kineticBombs.end(),
              [](Unit* unitA, Unit* unitB) { return unitA->GetPositionZ() < unitB->GetPositionZ(); });

    // Gather alive ranged DPS bots only (no real players)
    std::vector<Player*> rangedDps;
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && PlayerbotAI::IsRangedDps(member))
                rangedDps.push_back(member);
        }
    }
    else
    {
        rangedDps.push_back(bot);
    }

    // Greedy 1-to-1 assignment: hunters first, druids second, then any ranged DPS
    static float const MAX_ASSIGN_RANGE = 80.0f;
    std::set<Player*> assigned;
    for (Unit* bomb : kineticBombs)
    {
        Player* nearest = nullptr;
        float nearestDist = std::numeric_limits<float>::max();

        // Priority classes: hunter > druid > any
        static constexpr std::array<uint8, 3> classPriority = {CLASS_HUNTER, CLASS_DRUID, 0};
        for (uint8 priorityClass : classPriority)
        {
            nearest = nullptr;
            nearestDist = std::numeric_limits<float>::max();

            for (Player* dps : rangedDps)
            {
                if (assigned.count(dps))
                    continue;

                if (priorityClass != 0 && dps->getClass() != priorityClass)
                    continue;

                float dist = dps->GetDistance(bomb);
                if (dist < nearestDist && dist < MAX_ASSIGN_RANGE)
                {
                    nearestDist = dist;
                    nearest = dps;
                }
            }

            if (nearest)
                break;
        }

        if (nearest)
        {
            assigned.insert(nearest);
            if (nearest == bot)
                return bomb;
        }
    }

    return nullptr;
}

bool IccBpcBallOfFlameAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince taldaram");
    if (!boss)
        return false;

    Unit* ballOfFlameUnit = bot->FindNearestCreature(NPC_BALL_OF_FLAME, 100.0f);
    Unit* infernoFlameUnit = bot->FindNearestCreature(NPC_BALL_OF_INFERNO_FLAME, 100.0f);

    bool const hasBallOfFlame = ballOfFlameUnit && (ballOfFlameUnit->GetVictim() == bot);
    bool const hasInfernoFlame = infernoFlameUnit && (infernoFlameUnit->GetVictim() == bot);

    float infernoDist = infernoFlameUnit ? infernoFlameUnit->GetDistance2d(boss) : 0.0f;
    // Hunters excluded — they can DPS from range without needing to soak
    if (infernoFlameUnit && infernoDist > 2.0f && infernoDist <= 10.0f && !hasInfernoFlame &&
        bot->getClass() != CLASS_HUNTER)
    {
        if (!PlayerbotAI::IsTank(bot) && infernoFlameUnit->GetVictim() != bot)
        {
            float flameX = infernoFlameUnit->GetPositionX();
            float flameY = infernoFlameUnit->GetPositionY();
            float botX = bot->GetPositionX();
            float botY = bot->GetPositionY();

            // Calculate direction vector
            float dx = flameX - botX;
            float dy = flameY - botY;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Normalize and scale to 5 units (or remaining distance if less than 5)
            float step = std::min(5.0f, distance);
            if (distance > 0.001f)
            {
                dx = dx / distance * step;
                dy = dy / distance * step;
            }

            // Calculate intermediate position
            float newX = botX + dx;
            float newY = botY + dy;

            MoveTo(infernoFlameUnit->GetMapId(), newX, newY, bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // If victim of ball of flame, keep at least 15f from other party members
    if (hasBallOfFlame || hasInfernoFlame)
    {
        static float const SAFE_DIST = 15.0f;
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto const& memberGuid : members)
        {
            Unit* member = botAI->GetUnit(memberGuid);
            if (!member || !member->IsAlive() || member == bot)
                continue;
            float dist = bot->GetExactDist2d(member);
            if (dist < SAFE_DIST)
            {
                // Move away from this member
                float dx = bot->GetPositionX() - member->GetPositionX();
                float dy = bot->GetPositionY() - member->GetPositionY();
                float len = std::sqrt(dx * dx + dy * dy);
                if (len < 0.001f)
                    continue;
                dx /= len;
                dy /= len;
                float moveX = bot->GetPositionX() + dx * (SAFE_DIST - dist + 1.0f);
                float moveY = bot->GetPositionY() + dy * (SAFE_DIST - dist + 1.0f);
                float moveZ = bot->GetPositionZ();
                if (bot->IsWithinLOS(moveX, moveY, moveZ))
                {
                    MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                           MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}
