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
#include <limits>

// Festergut
bool IccFestergutGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());

    if (PlayerbotAI::IsTank(bot))
    {
        Aura* aura = botAI->GetAura("gastric bloat", bot, false, true);
        bool const hasGastricBloat = aura && aura->GetStackAmount() >= 6;

        if (hasGastricBloat)
        {
            if (bot->GetVictim() == boss)
                bot->AttackStop();

            if (bot->GetExactDist2d(ICC_FESTERGUT_TANK_POSITION) > 5.0f)
                return MoveTo(bot->GetMapId(), ICC_FESTERGUT_TANK_POSITION.GetPositionX(),
                              ICC_FESTERGUT_TANK_POSITION.GetPositionY(), ICC_FESTERGUT_TANK_POSITION.GetPositionZ(),
                              false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            return true;
        }

        Unit* currentTarget = boss->GetVictim();
        if (currentTarget && currentTarget != bot)
        {
            Aura* victimAura = botAI->GetAura("gastric bloat", currentTarget, false, true);
            if (victimAura && victimAura->GetStackAmount() >= 6)
                IccCastClassTaunt(bot, botAI,boss);
        }

        if (bot->GetExactDist2d(ICC_FESTERGUT_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_FESTERGUT_TANK_POSITION.GetPositionX(),
                          ICC_FESTERGUT_TANK_POSITION.GetPositionY(), ICC_FESTERGUT_TANK_POSITION.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_NORMAL);

        return false;
    }

    // Check for spores in the group
    if (HasSporesInGroup())
        return false;

    // No spore, no goo dodge - melee stack on main tank.
    if (PlayerbotAI::IsMelee(bot))
    {
        Unit* mainTank = AI_VALUE(Unit*, "main tank");
        if (mainTank && bot->GetExactDist2d(mainTank) > 3.0f)
            return MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(),
                          mainTank->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL);
        return false;
    }

    // Position non-tank ranged and healers
    return PositionNonTankMembers();
}

bool IccFestergutGroupPositionAction::HasSporesInGroup()
{
    return IccAnyGroupMemberHasAura(bot, SPELL_GAS_SPORE);
}

bool IccFestergutGroupPositionAction::PositionNonTankMembers()
{
    // Only position ranged and healers without spores
    if (!(PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot)))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int32 positionIndex = CalculatePositionIndex(group);
    if (positionIndex == -1)
        return false;

    // Position calculation parameters
    constexpr float tankToBossAngle = 4.58f;
    constexpr float minBossDistance = 15.0f;
    constexpr float spreadDistance = 10.0f;
    constexpr int32 columnsPerRow = 6;

    // Calculate grid position
    int32 row = positionIndex / columnsPerRow;
    int32 col = positionIndex % columnsPerRow;

    // Calculate base position
    float xOffset = (col - 2) * spreadDistance;                // Center around tank position
    float yOffset = minBossDistance + (row * spreadDistance);  // Each row further back

    // Add zigzag offset for odd rows
    if (row % 2 == 1)
        xOffset += spreadDistance / 2.0f;

    // Rotate position based on tank-to-boss angle
    float finalX =
        ICC_FESTERGUT_TANK_POSITION.GetPositionX() + (cos(tankToBossAngle) * yOffset - sin(tankToBossAngle) * xOffset);
    float finalY =
        ICC_FESTERGUT_TANK_POSITION.GetPositionY() + (sin(tankToBossAngle) * yOffset + cos(tankToBossAngle) * xOffset);
    float finalZ = ICC_FESTERGUT_TANK_POSITION.GetPositionZ();

    // Update Z coordinate
    bot->UpdateAllowedPositionZ(finalX, finalY, finalZ);

    // Move if not in position
    if (bot->GetExactDist2d(finalX, finalY) > 3.0f)
        return MoveTo(bot->GetMapId(), finalX, finalY, finalZ, false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);

    return false;
}

int32 IccFestergutGroupPositionAction::CalculatePositionIndex(Group* group)
{
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> rangedDpsGuids;
    std::vector<ObjectGuid> hunterGuids;

    // Collect all eligible members with their GUIDs
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;

        ObjectGuid memberGuid = member->GetGUID();

        if (PlayerbotAI::IsHeal(member))
            healerGuids.push_back(memberGuid);
        else if (PlayerbotAI::IsRanged(member))
        {
            if (member->getClass() == CLASS_HUNTER)
                hunterGuids.push_back(memberGuid);
            else
                rangedDpsGuids.push_back(memberGuid);
        }
    }

    // Sort GUIDs for consistent ordering across all bots
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(rangedDpsGuids.begin(), rangedDpsGuids.end());
    std::sort(hunterGuids.begin(), hunterGuids.end());

    ObjectGuid botGuid = bot->GetGUID();

    // Find which group this bot belongs to
    auto healerIt = std::find(healerGuids.begin(), healerGuids.end(), botGuid);
    auto rangedIt = std::find(rangedDpsGuids.begin(), rangedDpsGuids.end(), botGuid);
    auto hunterIt = std::find(hunterGuids.begin(), hunterGuids.end(), botGuid);

    // Calculate global position index considering group constraints
    int32 const healerRows = 2;
    int32 const columnsPerRow = 6;

    // Healers: rows 0-1 (first two rows)
    if (healerIt != healerGuids.end())
    {
        int32 healerIndex = static_cast<int32>(std::distance(healerGuids.begin(), healerIt));

        // Ensure healers only occupy first two rows
        if (healerIndex < healerRows * columnsPerRow)
            return healerIndex;

        // If too many healers, overflow to later rows but keep them early
        return healerIndex;  // Will be in row = index / 6, col = index % 6
    }

    // Non-hunter ranged DPS: can be any row (no strict restriction)
    if (rangedIt != rangedDpsGuids.end())
    {
        int32 rangedIndex = static_cast<int32>(std::distance(rangedDpsGuids.begin(), rangedIt));

        // Start after all healers, then fill remaining spots
        return static_cast<int32>(healerGuids.size()) + rangedIndex;
    }

    // Hunters: never in 1st row (row 0)
    if (hunterIt != hunterGuids.end())
    {
        int32 hunterIndex = static_cast<int32>(std::distance(hunterGuids.begin(), hunterIt));

        // Calculate how many non-healer positions are before this hunter position
        int32 baseOffset = static_cast<int32>(healerGuids.size()) + static_cast<int32>(rangedDpsGuids.size());

        // Each row of hunters starts at positions that are multiples of columnsPerRow
        // To avoid row 0, skip first column slots reserved for healers/non-hunters
        return baseOffset + hunterIndex;
    }

    return -1;
}

bool IccFestergutSporeAction::Execute(Event /*event*/)
{
    constexpr float positionTolerance = 4.0f;

    bool hasSpore = bot->HasAura(SPELL_GAS_SPORE);

    Position spreadRangedPos = CalculateSpreadPosition();
    SporeInfo sporeInfo = FindSporedPlayers();
    Position targetPos = DetermineTargetPosition(hasSpore, sporeInfo, spreadRangedPos);

    if (bot->GetExactDist2d(targetPos) > positionTolerance)
    {
        botAI->Reset();
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                      true, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // In position — let combat rotation run (multiplier blocks movement so bot stays put)
    return false;
}

Position IccFestergutSporeAction::CalculateSpreadPosition()
{
    constexpr float spreadRadius = 2.0f;
    constexpr float gooNearSporeRadius = 12.0f;
    constexpr uint32 impactLifetimeMs = 8000;
    constexpr uint32 cycleIdleResetMs = 2000;

    // Group-wide sticky slot decision. Spore + malleable goo can overlap: if
    // an active goo lands near the current spread spot we flip to the other
    // spot, and stay there for the rest of this spore cycle so bots aren't
    // pulled back into the danger zone when the goo expires. The action only
    // runs while the spore trigger is active, so a gap >2s between calls
    // means the cycle ended - reset to the primary slot for the next cycle.
    // State is keyed by instance ID so concurrent ICC raids don't share slots.
    IcecrownHelpers::SpreadSlotState& state = IcecrownHelpers::IccState(bot->GetMap()->GetInstanceId()).fgSlotState;

    uint32 now = getMSTime();
    if (state.lastCallMs == 0 || now - state.lastCallMs > cycleIdleResetMs)
        state.currentSlot = 1;
    state.lastCallMs = now;

    Position currentSpot = (state.currentSlot == 2) ? ICC_FESTERGUT_RANGED_SPORE_2
                                                    : ICC_FESTERGUT_RANGED_SPORE;

    for (Position const& goo : IcecrownHelpers::ActiveGooPositions(bot->GetMap()->GetInstanceId(), impactLifetimeMs))
    {
        float dx = goo.GetPositionX() - currentSpot.GetPositionX();
        float dy = goo.GetPositionY() - currentSpot.GetPositionY();
        if (dx * dx + dy * dy < gooNearSporeRadius * gooNearSporeRadius)
        {
            state.currentSlot = (state.currentSlot == 1) ? 2 : 1;
            currentSpot = (state.currentSlot == 2) ? ICC_FESTERGUT_RANGED_SPORE_2
                                                   : ICC_FESTERGUT_RANGED_SPORE;
            break;
        }
    }

    // Unique angle based on bot's GUID
    float angle = (bot->GetGUID().GetCounter() % 16) * (M_PI / 8);

    Position spreadRangedPos = currentSpot;
    spreadRangedPos.Relocate(spreadRangedPos.GetPositionX() + cos(angle) * spreadRadius,
                             spreadRangedPos.GetPositionY() + sin(angle) * spreadRadius,
                             spreadRangedPos.GetPositionZ(), spreadRangedPos.GetOrientation());

    return spreadRangedPos;
}

IccFestergutSporeAction::SporeInfo IccFestergutSporeAction::FindSporedPlayers()
{
    SporeInfo info;
    GuidVector const members = AI_VALUE(GuidVector, "group members");

    for (auto const& memberGuid : members)
    {
        Unit* unit = botAI->GetUnit(memberGuid);
        if (!unit)
            continue;

        if (unit->HasAura(SPELL_GAS_SPORE))
        {
            info.sporedPlayers.push_back(unit);

            if (!info.hasLowestGuid || unit->GetGUID() < info.lowestGuid)
            {
                info.lowestGuid = unit->GetGUID();
                info.hasLowestGuid = true;
            }
        }
    }

    return info;
}

bool IccFestergutSporeAction::GooNear(Position const& pos)
{
    constexpr float gooDangerRadius = 12.0f;

    for (Position const& goo : IcecrownHelpers::ActiveGooPositions(bot->GetMap()->GetInstanceId(), 8000))
    {
        float dx = pos.GetPositionX() - goo.GetPositionX();
        float dy = pos.GetPositionY() - goo.GetPositionY();
        if (dx * dx + dy * dy < gooDangerRadius * gooDangerRadius)
            return true;
    }
    return false;
}

Position IccFestergutSporeAction::DetermineTargetPosition(bool hasSpore, SporeInfo const& sporeInfo,
                                                          Position const& spreadRangedPos)
{
    // No spores at all
    if (sporeInfo.sporedPlayers.empty())
        return PlayerbotAI::IsMelee(bot) ? ICC_FESTERGUT_MELEE_SPORE : spreadRangedPos;

    bool mainTankHasSpore = CheckMainTankSpore();

    // Goo overlap override — checked before hasSpore so non-spored bots also redirect.
    bool gooAtMelee = GooNear(ICC_FESTERGUT_MELEE_SPORE);
    bool gooAtRanged = GooNear(ICC_FESTERGUT_RANGED_SPORE) || GooNear(ICC_FESTERGUT_RANGED_SPORE_2);

    if (gooAtMelee && !gooAtRanged)
    {
        // Goo at melee: tank + melee-spore bot hold, other melee flee to ranged slot 1.
        bool isMeleeSporeBot = (hasSpore && bot->GetGUID() == sporeInfo.lowestGuid && !PlayerbotAI::IsTank(bot) && !mainTankHasSpore);
        if (PlayerbotAI::IsMainTank(bot) || isMeleeSporeBot)
            return ICC_FESTERGUT_MELEE_SPORE;
        if (PlayerbotAI::IsMelee(bot))
            return ICC_FESTERGUT_RANGED_SPORE;
        return spreadRangedPos;
    }

    if (gooAtRanged && !gooAtMelee)
    {
        // Goo at ranged: all ranged collapse to melee spot.
        if (!PlayerbotAI::IsMelee(bot))
            return ICC_FESTERGUT_MELEE_SPORE;
        return ICC_FESTERGUT_MELEE_SPORE;
    }

    // Normal spore logic (no overlap or both spots hit).
    if (!hasSpore)
        return PlayerbotAI::IsMelee(bot) ? ICC_FESTERGUT_MELEE_SPORE : spreadRangedPos;

    if (PlayerbotAI::IsMainTank(bot))
        return ICC_FESTERGUT_MELEE_SPORE;

    if (bot->GetGUID() == sporeInfo.lowestGuid && !PlayerbotAI::IsTank(bot) && !mainTankHasSpore)
        return ICC_FESTERGUT_MELEE_SPORE;

    return spreadRangedPos;
}

bool IccFestergutSporeAction::CheckMainTankSpore()
{
    GuidVector const members = AI_VALUE(GuidVector, "group members");

    for (auto const& memberGuid : members)
    {
        Unit* unit = botAI->GetUnit(memberGuid);
        if (!unit)
            continue;

        if (PlayerbotAI::IsMainTank(unit->ToPlayer()) && unit->HasAura(SPELL_GAS_SPORE))
            return true;
    }

    return false;
}

bool IccFestergutAvoidMalleableGooAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return false;

    // Tanks hold aggro at the fixed tank spot - never dodge.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Festergut heroic - Putricide throws Malleable Goo from the balcony at
    // random non-tank players. The impact is a triggered cast with no cast bar
    // and no DynamicObject, so the IccPutricideListenerScript stamps the target's
    // position into IcecrownHelpers::malleableGooImpacts at OnSpellCast time.
    // Any bot within 12yd of an active impact must flee; danger persists 8s.
    // Once a bot dodges, we return true (blocking group-position) until the
    // impact expires so the bot doesn't immediately re-enter the danger zone.
    constexpr float gooDangerRadius = 12.0f;

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    ObjectGuid botGuid = bot->GetGUID();

    std::vector<Position> goos = IcecrownHelpers::ActiveGooPositions(bot->GetMap()->GetInstanceId(), 8000);
    bool botInDanger = false;
    for (Position const& goo : goos)
    {
        float dx = botX - goo.GetPositionX();
        float dy = botY - goo.GetPositionY();
        if (dx * dx + dy * dy < gooDangerRadius * gooDangerRadius)
            botInDanger = true;
    }

    if (!botInDanger)
    {
        // Already safe. Return false so DPS/heal rotations can still fire -
        // the multiplier blocks repositioning actions during the wait window
        // via festergutGooWaitUntil so the bot stays put without idling.
        return false;
    }

    // Keep 10yd between fleeing bots. During spore phase melee stack at the
    // tank spot, so we ignore melee allies (the tank/melee pile would reject
    // every nearby candidate). When no spore is active, melee also spread.
    constexpr float botSpacing = 10.0f;
    bool const sporeActive = IccAnyGroupMemberHasAura(bot, SPELL_GAS_SPORE);

    std::vector<Position> alliesToSpace;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || member->GetGUID() == botGuid)
                continue;
            if (sporeActive && PlayerbotAI::IsMelee(member))
                continue;
            alliesToSpace.push_back(member->GetPosition());
        }
    }

    constexpr int angleSteps = 24;
    float const radii[] = {13.0f, 16.0f, 20.0f};
    float bestScore = -1.0f;
    float bestX = botX;
    float bestY = botY;
    bool found = false;

    // Per-bot preferred flee angle - stable across ticks, distinct per GUID -
    // so stacked bots fan out into different sectors instead of converging.
    float preferredAngle = (botGuid.GetCounter() % angleSteps) * (2.0f * float(M_PI) / angleSteps);
    constexpr float angleBias = 3.0f;

    for (float r : radii)
    {
        for (int i = 0; i < angleSteps; ++i)
        {
            float a = (2.0f * float(M_PI) * i) / angleSteps;
            float cx = botX + std::cos(a) * r;
            float cy = botY + std::sin(a) * r;

            float minGooDistSq = std::numeric_limits<float>::max();
            bool safe = true;
            for (Position const& g : goos)
            {
                float gdx = cx - g.GetPositionX();
                float gdy = cy - g.GetPositionY();
                float d2 = gdx * gdx + gdy * gdy;
                if (d2 < gooDangerRadius * gooDangerRadius)
                {
                    safe = false;
                    break;
                }
                if (d2 < minGooDistSq)
                    minGooDistSq = d2;
            }
            if (!safe)
                continue;

            bool tooCloseToAlly = false;
            for (Position const& a2 : alliesToSpace)
            {
                float adx = cx - a2.GetPositionX();
                float ady = cy - a2.GetPositionY();
                if (adx * adx + ady * ady < botSpacing * botSpacing)
                {
                    tooCloseToAlly = true;
                    break;
                }
            }
            if (tooCloseToAlly)
                continue;

            if (!bot->IsWithinLOS(cx, cy, botZ))
                continue;

            float travel = std::sqrt((cx - botX) * (cx - botX) + (cy - botY) * (cy - botY));
            float score = std::sqrt(minGooDistSq) - travel * 0.1f + std::cos(a - preferredAngle) * angleBias;

            if (score > bestScore)
            {
                bestScore = score;
                bestX = cx;
                bestY = cy;
                found = true;
            }
        }
        if (found)
            break;
    }

    if (found)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, botZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}
