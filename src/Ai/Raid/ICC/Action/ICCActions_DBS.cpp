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

bool IccDbsTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    if (PlayerbotAI::IsTank(bot))
    {
        bool const hasRuneOfBlood = botAI->GetAura("Rune of Blood", bot) != nullptr;

        if (hasRuneOfBlood)
        {
            // Stop attacking boss but still taunt loose blood beasts
            if (bot->GetVictim() == boss)
                bot->AttackStop();

            std::array<uint32, 4> const beastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3,
                                                          NPC_BLOOD_BEAST4};
            GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto const& npc : npcs)
            {
                Unit* unit = botAI->GetUnit(npc);
                if (!unit || !unit->IsAlive())
                    continue;

                bool const isBloodBeast =
                    std::find(beastEntries.begin(), beastEntries.end(), unit->GetEntry()) != beastEntries.end();
                if (!isBloodBeast)
                    continue;

                Unit* victim = unit->GetVictim();
                Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
                if (!victimPlayer || !PlayerbotAI::IsTank(victimPlayer))
                {
                    IccCastClassTaunt(bot, botAI,unit);
                    break;
                }
            }

            if (bot->GetExactDist2d(ICC_DBS_TANK_POSITION) > 5.0f)
                return MoveTo(bot->GetMapId(), ICC_DBS_TANK_POSITION.GetPositionX(),
                              ICC_DBS_TANK_POSITION.GetPositionY(), ICC_DBS_TANK_POSITION.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_NORMAL);
            return true;
        }

        // Tank without Rune of Blood: taunt boss if current tank has the debuff
        Unit* currentTarget = boss->GetVictim();
        if (currentTarget && currentTarget != bot && botAI->GetAura("Rune of Blood", currentTarget))
            IccCastClassTaunt(bot, botAI,boss);

        // Taunt any blood beasts not targeting a tank
        std::array<uint32, 4> const bloodBeastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3,
                                                           NPC_BLOOD_BEAST4};
        GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto const& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            bool const isBloodBeast = std::find(bloodBeastEntries.begin(), bloodBeastEntries.end(), unit->GetEntry()) !=
                                      bloodBeastEntries.end();
            if (!isBloodBeast)
                continue;

            Unit* victim = unit->GetVictim();
            Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
            if (!victimPlayer || !PlayerbotAI::IsTank(victimPlayer))
            {
                IccCastClassTaunt(bot, botAI,unit);
                break;
            }
        }

        if (bot->GetExactDist2d(ICC_DBS_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_DBS_TANK_POSITION.GetPositionX(), ICC_DBS_TANK_POSITION.GetPositionY(),
                          ICC_DBS_TANK_POSITION.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL);

        return false;
    }

    if (!PlayerbotAI::IsTank(bot))
    {
        if (CrowdControlBloodBeasts())
            return true;
    }

    // Handle ranged and healer positioning
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot))
    {
        // Handle evasion from blood beasts
        if (EvadeBloodBeasts())
            return true;

        // Position in formation
        return PositionInRangedFormation();
    }

    return false;
}

bool IccDbsTankPositionAction::CrowdControlBloodBeasts()
{
    std::array<uint32, 4> const bloodBeastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3,
                                                       NPC_BLOOD_BEAST4};
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if this is a blood beast
        bool const isBloodBeast =
            std::find(bloodBeastEntries.begin(), bloodBeastEntries.end(), unit->GetEntry()) != bloodBeastEntries.end();

        if (!isBloodBeast)
            continue;

        if (auto cast = IccTryClassCC(bot, botAI, unit))
            return *cast;
    }

    return false;
}

bool IccDbsTankPositionAction::EvadeBloodBeasts()
{
    float const evasionDistance = 12.0f;
    std::array<uint32, 4> const bloodBeastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3,
                                                       NPC_BLOOD_BEAST4};

    // Get the nearest hostile NPCs
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        // Check if this is a blood beast
        bool const isBloodBeast =
            std::find(bloodBeastEntries.begin(), bloodBeastEntries.end(), unit->GetEntry()) != bloodBeastEntries.end();

        // Only evade if it's a blood beast targeting us
        if (isBloodBeast && unit->GetVictim() == bot)
        {
            float const currentDistance = bot->GetDistance2d(unit);

            // Move away if too close
            if (currentDistance < evasionDistance)
                return MoveAway(unit, evasionDistance - currentDistance);
        }
    }

    return false;
}

bool IccDbsTankPositionAction::PositionInRangedFormation()
{
    // Get group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int32 const totalSlots = 15;  // 3 rows x 5 cols
    uint32 const dbsInstanceId = bot->GetInstanceId();

    // Persistent per-bot slot memory shared across all bots.
    // Keyed per-instance to avoid cross-instance pollution.
    auto& botSlotMemory = IcecrownHelpers::IccState(dbsInstanceId).dbsBotSlotMemory;
    ObjectGuid myKey = bot->GetGUID();

    // Single pass: collect natural index (alive ranged/healer non-tank order)
    // and other bots' reserved slots.
    int32 myIndex = -1;
    int32 currentIndex = 0;
    std::vector<int> reservedSlots;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        if (member != bot)
        {
            auto it = botSlotMemory.find(member->GetGUID());
            if (it != botSlotMemory.end() && it->second >= 0 && it->second < totalSlots)
                reservedSlots.push_back(it->second);
        }

        if (!member->IsAlive())
            continue;

        if ((PlayerbotAI::IsRanged(member) || PlayerbotAI::IsHeal(member)) && !PlayerbotAI::IsTank(member))
        {
            if (member == bot)
            {
                myIndex = currentIndex;
            }
            currentIndex++;
        }
    }

    if (myIndex == -1)
        return false;

    auto IsReserved = [&](int s) -> bool
    {
        return std::find(reservedSlots.begin(), reservedSlots.end(), s) != reservedSlots.end();
    };

    int myAssignedSlot = -1;

    // Step 1: keep my remembered slot if still in range and not reserved by someone else.
    auto myMemIt = botSlotMemory.find(myKey);
    if (myMemIt != botSlotMemory.end())
    {
        int prev = myMemIt->second;
        if (prev >= 0 && prev < totalSlots && !IsReserved(prev))
            myAssignedSlot = prev;
    }

    // Step 2: pick first unreserved slot, seeded from my natural index.
    if (myAssignedSlot < 0)
    {
        for (int attempt = 0; attempt < totalSlots; attempt++)
        {
            int s = (myIndex + attempt) % totalSlots;
            if (!IsReserved(s))
            {
                myAssignedSlot = s;
                break;
            }
        }
    }

    // Step 3: overflow (16th+ ranged) - stack with closest non-tank melee bot,
    // lowest GUID on tie. Forget my slot so others can take it.
    if (myAssignedSlot < 0)
    {
        botSlotMemory.erase(myKey);

        Player* anchor = nullptr;
        float anchorDist = 0.0f;
        uint64 anchorGuidRaw = 0;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;
            if (!PlayerbotAI::IsMelee(member) || PlayerbotAI::IsTank(member))
                continue;

            float d = bot->GetExactDist2d(member);
            uint64 g = member->GetGUID().GetRawValue();
            if (!anchor || d < anchorDist || (d == anchorDist && g < anchorGuidRaw))
            {
                anchor = member;
                anchorDist = d;
                anchorGuidRaw = g;
            }
        }

        if (!anchor)
            return false;

        float ax = anchor->GetPositionX();
        float ay = anchor->GetPositionY();
        float az = anchor->GetPositionZ();
        if (bot->GetExactDist2d(ax, ay) > 3.0f)
            return MoveTo(bot->GetMapId(), ax, ay, az, false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
        return false;
    }

    botSlotMemory[myKey] = myAssignedSlot;

    // Fixed positions calculation
    float const tankToBossAngle = 3.14f;
    float const minBossDistance = 11.0f;
    float const spreadDistance = 10.0f;
    int32 const columnsPerRow = 5;

    // Calculate position in a fixed grid (3 rows x 5 columns)
    int32 const row = myAssignedSlot / columnsPerRow;
    int32 const col = myAssignedSlot % columnsPerRow;

    // Calculate base position
    float xOffset = (col - 2) * spreadDistance;                // Center around tank position
    float yOffset = minBossDistance + (row * spreadDistance);  // Each row further back

    // Add zigzag offset for odd rows
    if (row % 2 == 1)
        xOffset += spreadDistance / 2.0f;

    // Rotate position based on tank-to-boss angle
    float finalX =
        ICC_DBS_TANK_POSITION.GetPositionX() + (cos(tankToBossAngle) * yOffset - sin(tankToBossAngle) * xOffset);
    float finalY =
        ICC_DBS_TANK_POSITION.GetPositionY() + (sin(tankToBossAngle) * yOffset + cos(tankToBossAngle) * xOffset);
    float finalZ = ICC_DBS_TANK_POSITION.GetPositionZ();

    // Update Z coordinate
    bot->UpdateAllowedPositionZ(finalX, finalY, finalZ);

    // Move if not in position
    if (bot->GetExactDist2d(finalX, finalY) > 3.0f)
        return MoveTo(bot->GetMapId(), finalX, finalY, finalZ, false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool IccAddsDbsAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    // This action is only for melee
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* priorityTarget = FindPriorityTarget(boss);

    // Update raid target icons if needed
    UpdateSkullMarker(priorityTarget);

    return false;
}

Unit* IccAddsDbsAction::FindPriorityTarget(Unit* boss)
{
    GuidVector const targets = AI_VALUE(GuidVector, "possible targets no los");

    // Blood beast entry IDs
    std::array<uint32, 4> const addEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3, NPC_BLOOD_BEAST4};

    // First check for alive adds
    for (uint32 const entry : addEntries)
    {
        for (ObjectGuid const& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }
    }

    // Only fallback to boss if it's alive
    return boss->IsAlive() ? const_cast<Unit*>(boss) : nullptr;
}

bool IccAddsDbsAction::UpdateSkullMarker(Unit* priorityTarget)
{
    IccEnsureIconOn(bot, botAI, RtiTargetValue::skullIndex, priorityTarget);
    return false;
}
