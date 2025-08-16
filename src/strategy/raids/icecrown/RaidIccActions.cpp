#include "RaidIccActions.h"
#include "strategy/values/NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "RaidIccStrategy.h"
#include "Playerbots.h"
#include "Timer.h"
#include "Vehicle.h"
#include "RtiValue.h"
#include "GenericSpellActions.h"
#include "GenericActions.h"
#include <fstream>
#include "RaidIccTriggers.h"
#include "Multiplier.h"

// Lord Marrowgwar
bool IccLmTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    if (!botAI->IsTank(bot))
        return false;

    const bool isBossInBoneStorm = botAI->GetAura("Bone Storm", boss) != nullptr;

    if (isBossInBoneStorm)
        return false;

    if (botAI->HasAggro(boss) && botAI->IsMainTank(bot) && boss->GetVictim() == bot)
    {
        const float maxDistanceThreshold = 3.0f;
        const float distance = bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());

        if (distance > maxDistanceThreshold)
            return MoveTowardPosition(ICC_LM_TANK_POSITION, maxDistanceThreshold);
    }

    if (botAI->IsAssistTank(bot))
    {
        const float maxDistanceThreshold = 3.0f;
        const float distance = bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());

        if (distance > maxDistanceThreshold)
            return MoveTowardPosition(ICC_LM_TANK_POSITION, maxDistanceThreshold);

        if (distance < maxDistanceThreshold)
        {
            bot->SetFacingToObject(boss);
            return true;
        }
    }

    return false;
}

bool IccLmTankPositionAction::MoveTowardPosition(const Position& position, float incrementSize)
{
    // Calculate direction vector
    const float dirX = position.GetPositionX() - bot->GetPositionX();
    const float dirY = position.GetPositionY() - bot->GetPositionY();
    const float length = std::sqrt(dirX * dirX + dirY * dirY);

    // Normalize direction vector
    const float normalizedDirX = dirX / length;
    const float normalizedDirY = dirY / length;

    // Calculate new position with increment
    const float moveX = bot->GetPositionX() + normalizedDirX * incrementSize;
    const float moveY = bot->GetPositionY() + normalizedDirY * incrementSize;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccSpikeAction::Execute(Event event)
{
    // If we're impaled, we can't do anything
    if (botAI->GetAura("Impaled", bot))
        return false;

    // Find the boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    const bool isBossInBoneStorm = botAI->GetAura("Bone Storm", boss) != nullptr;
    const bool shouldMoveToSafePosition = boss->isInFront(bot) && !botAI->IsTank(bot) && !isBossInBoneStorm;

    if (shouldMoveToSafePosition)
    {
        const Position safePosition{-390.6757f, 2230.5283f, 0.0f};  // Z value to be overridden by actual bot Z
        const float distance = bot->GetExactDist2d(safePosition.GetPositionX(), safePosition.GetPositionY());
        const float maxDistanceThreshold = 3.0f;

        if (distance > maxDistanceThreshold)
            return MoveTowardPosition(safePosition, maxDistanceThreshold);

        return false;
    }

    if (!botAI->IsTank(bot))
        return false;

    return HandleSpikeTargeting(boss);
}

bool IccSpikeAction::HandleSpikeTargeting(Unit* boss)
{
    static const std::array<uint32, 3> spikeEntries = {NPC_SPIKE1, NPC_SPIKE2, NPC_SPIKE3};
    const GuidVector spikes = AI_VALUE(GuidVector, "possible targets no los");

    Unit* priorityTarget = nullptr;
    bool anySpikesExist = false;

    // First check for alive spikes
    for (const auto entry : spikeEntries)
    {
        for (const auto& guid : spikes)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry)
                {
                    anySpikesExist = true;  // At least one spike exists

                    if (unit->IsAlive())
                    {  // Only consider alive ones for targeting
                        priorityTarget = unit;
                        break;
                    }
                }
            }
        }
        if (priorityTarget)
            break;
    }

    // Only fallback to boss if NO spikes exist at all (alive or dead)
    if (!anySpikesExist && boss->IsAlive())
        priorityTarget = boss;

    // Update raid target icon if needed
    if (priorityTarget)
        UpdateRaidTargetIcon(priorityTarget);

    return false;
}

bool IccSpikeAction::MoveTowardPosition(const Position& position, float incrementSize)
{
    // Calculate direction vector
    const float dirX = position.GetPositionX() - bot->GetPositionX();
    const float dirY = position.GetPositionY() - bot->GetPositionY();
    const float length = std::sqrt(dirX * dirX + dirY * dirY);

    // Normalize direction vector
    const float normalizedDirX = dirX / length;
    const float normalizedDirY = dirY / length;

    // Calculate new position with increment
    const float moveX = bot->GetPositionX() + normalizedDirX * incrementSize;
    const float moveY = bot->GetPositionY() + normalizedDirY * incrementSize;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

void IccSpikeAction::UpdateRaidTargetIcon(Unit* target)
{
    static constexpr uint8_t SKULL_ICON_INDEX = 7;

    if (Group* group = bot->GetGroup())
    {
        const ObjectGuid currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
        Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

        const bool needsUpdate = !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != target;

        if (needsUpdate)
            group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), target->GetGUID());
    }
}

// Lady Deathwhisper
bool IccDarkReckoningAction::Execute(Event event)
{
    constexpr float SAFE_DISTANCE_THRESHOLD = 2.0f;

    // Check if the bot needs to move to the safe position
    if (bot->HasAura(SPELL_DARK_RECKONING) &&
        bot->GetExactDist2d(ICC_DARK_RECKONING_SAFE_POSITION) > SAFE_DISTANCE_THRESHOLD)
    {
        // Move to the safe position with the same parameters as before
        return MoveTo(bot->GetMapId(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionX(),
                      ICC_DARK_RECKONING_SAFE_POSITION.GetPositionY(),
                      ICC_DARK_RECKONING_SAFE_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool IccRangedPositionLadyDeathwhisperAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    const float currentDistance = bot->GetDistance2d(boss);
    const float minDistance = 7.0f;
    const float maxDistance = 30.0f;

    if (currentDistance < minDistance || currentDistance > maxDistance)
        return false;

    if (!botAI->IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    return MaintainRangedSpacing();
}

bool IccRangedPositionLadyDeathwhisperAction::MaintainRangedSpacing()
{
    const float safeSpacingRadius = 3.0f;
    const float moveIncrement = 2.0f;
    const float maxMoveDistance = 5.0f;  // Limit maximum movement distance
    const bool isRanged = botAI->IsRanged(bot) || botAI->IsHeal(bot);

    if (!isRanged)
        return false;

    // Ranged: spread from other members
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    // Calculate a combined vector representing all nearby members' positions
    float totalX = 0.0f;
    float totalY = 0.0f;
    int nearbyCount = 0;

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }

        const float distance = bot->GetExactDist2d(member);
        if (distance < safeSpacingRadius)
        {
            // Calculate vector from member to bot
            float dx = bot->GetPositionX() - member->GetPositionX();
            float dy = bot->GetPositionY() - member->GetPositionY();

            // Weight by inverse distance (closer members have more influence)
            float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
            totalX += dx * weight;
            totalY += dy * weight;
            nearbyCount++;
        }
    }

    // If we have nearby members, move away in the combined direction
    if (nearbyCount > 0)
    {
        // Normalize the combined vector
        float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
        if (magnitude > 0.001f)  // Avoid division by zero
        {
            totalX /= magnitude;
            totalY /= magnitude;

            // Calculate move distance based on nearest member
            float moveDistance = std::min(moveIncrement, maxMoveDistance);

            // Create target position in the combined direction
            float targetX = bot->GetPositionX() + totalX * moveDistance;
            float targetY = bot->GetPositionY() + totalY * moveDistance;
            float targetZ = bot->GetPositionZ();  // Maintain current Z

            // Check if the target position is valid and move there
            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            else
            {
                // If los check fails, try shorter distance
                targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;  // Everyone is properly spaced
}

bool IccAddsLadyDeathwhisperAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    if (botAI->HasAura("Dominate Mind", bot, false, false) && !bot->HasAura(SPELL_CYCLONE))
        bot->AddAura(SPELL_CYCLONE, bot);
    else if (bot->HasAura(SPELL_CYCLONE) && !botAI->HasAura("Dominate Mind", bot, false, false))
        bot->RemoveAura(SPELL_CYCLONE);

    const uint32 shadeEntryId = NPC_SHADE;

    if (botAI->IsTank(bot) && boss && boss->HealthBelowPct(95) && boss->GetVictim() == bot)
    {
        // Check if the bot is not the victim of a shade
        if (IsTargetedByShade(shadeEntryId))
            return false;

        const float maxDistanceToTankPosition = 20.0f;
        const float moveIncrement = 3.0f;

        const float distance = bot->GetExactDist2d(ICC_LDW_TANK_POSTION.GetPositionX(), ICC_LDW_TANK_POSTION.GetPositionY());

        if (distance > maxDistanceToTankPosition)
        {
            return MoveTowardPosition(ICC_LDW_TANK_POSTION, moveIncrement);
        }
    }

    if (!botAI->IsTank(bot))
        return false;

    return HandleAddTargeting(boss);
}

bool IccAddsLadyDeathwhisperAction::IsTargetedByShade(uint32 shadeEntry)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == shadeEntry && unit->GetVictim() == bot)
            return true;
    }
    return false;
}

bool IccAddsLadyDeathwhisperAction::MoveTowardPosition(const Position& position, float incrementSize)
{
    // Calculate direction vector
    const float dirX = position.GetPositionX() - bot->GetPositionX();
    const float dirY = position.GetPositionY() - bot->GetPositionY();
    const float length = std::sqrt(dirX * dirX + dirY * dirY);

    // Normalize direction vector
    const float normalizedDirX = dirX / length;
    const float normalizedDirY = dirY / length;

    // Calculate new position with increment
    const float moveX = bot->GetPositionX() + normalizedDirX * incrementSize;
    const float moveY = bot->GetPositionY() + normalizedDirY * incrementSize;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccAddsLadyDeathwhisperAction::HandleAddTargeting(Unit* boss)
{
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    Unit* priorityTarget = nullptr;
    bool hasValidAdds = false;

    // First check for alive adds
    for (const auto& entry : addEntriesLady)
    {
        for (const auto& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            {
                priorityTarget = unit;
                hasValidAdds = true;
                break;
            }
        }
        if (priorityTarget)
            break;
    }

    // Only fallback to boss if NO adds exist
    if (!hasValidAdds && boss->IsAlive())
        priorityTarget = boss;

    // Update skull icon if needed
    if (priorityTarget)
        UpdateRaidTargetIcon(priorityTarget);

    return false;
}

void IccAddsLadyDeathwhisperAction::UpdateRaidTargetIcon(Unit* target)
{
    static constexpr uint8_t SKULL_ICON_INDEX = 7;

    if (Group* group = bot->GetGroup())
    {
        const ObjectGuid currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
        Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

        const bool needsUpdate = !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != target;

        if (needsUpdate)
            group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), target->GetGUID());
    }
}

bool IccShadeLadyDeathwhisperAction::Execute(Event event)
{
    static constexpr uint32 VENGEFUL_SHADE_ID = NPC_SHADE;
    static constexpr float SAFE_DISTANCE = 12.0f;

    // Get the nearest hostile NPCs
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npcGuid : npcs)
    {
        Unit* shade = botAI->GetUnit(npcGuid);

        // Skip if not a vengeful shade
        if (!shade || shade->GetEntry() != VENGEFUL_SHADE_ID)
            continue;

        // Only run away if the shade is targeting us
        // Check by GUID comparison to ensure we're accurately identifying the specific shade
        // This is especially important in 25HC where multiple shades can spawn
        if (!shade->GetVictim() || shade->GetVictim()->GetGUID() != bot->GetGUID())
            continue;

        const float currentDistance = bot->GetDistance2d(shade);

        // Move away from the Vengeful Shade if the bot is too close
        if (currentDistance < SAFE_DISTANCE)
        {
            // Calculate direction away from shade
            float dx = bot->GetPositionX() - shade->GetPositionX();
            float dy = bot->GetPositionY() - shade->GetPositionY();
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < 0.001f)
                continue;

            dx /= dist;
            dy /= dist;

            float moveDistance = SAFE_DISTANCE - currentDistance;
            float targetX = bot->GetPositionX() + dx * moveDistance;
            float targetY = bot->GetPositionY() + dy * moveDistance;
            float targetZ = bot->GetPositionZ();

            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                botAI->Reset();
                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

    return false;
}

bool IccRottingFrostGiantTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("death plague", bot, false, false);
    if (aura)
        bot->RemoveAura(aura->GetId());

/* TODO: code works for handling plague, but atm script is bugged and one bot can have 2 plagues at the same time or when cured, which should not happen, and it is immpossible to handle plague atm the legit way.
    const bool hasCure = botAI->GetAura("recently infected", bot) != nullptr;

    // Tank behavior - unchanged
    if (botAI->IsTank(bot) && botAI->HasAggro(boss) && !isInfected)
        if (bot->GetExactDist2d(ICC_ROTTING_FROST_GIANT_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionX(),
                          ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionY(),
                          ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL);

    if (botAI->IsTank(bot))
        return false;

    // Handle infected bot behavior - move near a non-infected, non-cured bot
    if (isInfected)
    {
        const GuidVector members = AI_VALUE(GuidVector, "group members");

        // Count how many bots are targeting each potential target
        std::map<ObjectGuid, int> targetCounts;

        // First, identify all infected bots and their current targets (approximate)
        for (const auto& memberGuid : members)
        {
            Unit* member = botAI->GetUnit(memberGuid);
            if (!member || !member->IsAlive() || member == bot)
                continue;

            const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;

            if (memberIsInfected)
            {
                // Find the nearest non-infected bot to this infected bot (as a guess of its target)
                float minDist = 5.0f;  // Only count if they're close enough to likely be targeting
                Unit* likelyTarget = nullptr;

                for (const auto& targetGuid : members)
                {
                    Unit* potentialTarget = botAI->GetUnit(targetGuid);
                    if (!potentialTarget || !potentialTarget->IsAlive() || potentialTarget == member)
                        continue;

                    const bool targetIsInfected = botAI->GetAura("death plague", potentialTarget) != nullptr;
                    const bool targetHasCure = botAI->GetAura("recently infected", potentialTarget) != nullptr;

                    if (!targetIsInfected && !targetHasCure)
                    {
                        float dist = member->GetExactDist2d(potentialTarget);
                        if (dist < minDist)
                        {
                            minDist = dist;
                            likelyTarget = potentialTarget;
                        }
                    }
                }

                if (likelyTarget)
                {
                    targetCounts[likelyTarget->GetGUID()]++;
                }
            }
        }

        // Find viable targets and score them based on various factors
        std::vector<std::pair<Unit*, float>> viableTargets;

        // First try to find ranged, non-infected, non-cured bots
        for (const auto& memberGuid : members)
        {
            Unit* member = botAI->GetUnit(memberGuid);
            if (!member || !member->IsAlive() || member == bot)
                continue;

            const bool memberHasCure = botAI->GetAura("recently infected", member) != nullptr;
            const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;

            if (!memberIsInfected && !memberHasCure)
            {
                // Base score is distance (lower is better)
                float score = bot->GetExactDist2d(member);

                // Prefer ranged targets
                if (botAI->IsRanged(bot))
                {
                    score *= 0.7f;  // Bonus for ranged targets
                }

                // Apply penalty based on how many other infected bots are targeting this one
                int targetingCount = targetCounts[member->GetGUID()];
                score *= (1.0f + targetingCount * 0.5f);  // Increase score (worse) for heavily targeted bots

                viableTargets.push_back(std::make_pair(member, score));
            }
        }

        // Sort targets by score (lowest/best first)
        std::sort(viableTargets.begin(), viableTargets.end(),
                  [](const std::pair<Unit*, float>& a, const std::pair<Unit*, float>& b)
                  { return a.second < b.second; });

        // Choose the best target
        Unit* targetBot = nullptr;
        if (!viableTargets.empty())
        {
            targetBot = viableTargets[0].first;
        }

        // Move to target bot if found
        if (targetBot)
        {
            // If we're already close enough (1 yard), no need to move
            if (bot->GetExactDist2d(targetBot) > 1.0f)
            {
                // Calculate a unique angle based on bot's GUID to ensure different approach angles
                // This helps spread infected bots around the target
                uint32 guidLow = bot->GetGUID().GetCounter();
                float angleOffset = float(guidLow % 628) / 100.0f;  // Random angle between 0 and 2π

                // Calculate position 1 yard away from target at our unique angle
                float angle = targetBot->GetOrientation() + angleOffset;
                float targetX = targetBot->GetPositionX() + cos(angle) * 1.0f;
                float targetY = targetBot->GetPositionY() + sin(angle) * 1.0f;
                float targetZ = targetBot->GetPositionZ();

                return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                              MovementPriority::MOVEMENT_COMBAT);
            }
            return true;  // Already in position
        }
        // No suitable target found, continue with normal behavior
    }

    // For ranged bots, only spread from non-infected bots
    if (botAI->IsRanged(bot))
    {
        const float safeSpacingRadius = 11.0f;
        const float moveIncrement = 2.0f;
        const float maxMoveDistance = 15.0f;

        const GuidVector members = AI_VALUE(GuidVector, "group members");

        // Calculate a combined vector representing all nearby NON-INFECTED members' positions
        float totalX = 0.0f;
        float totalY = 0.0f;
        int nearbyCount = 0;

        for (const auto& memberGuid : members)
        {
            Unit* member = botAI->GetUnit(memberGuid);
            if (!member || !member->IsAlive() || member == bot)
                continue;

            // Only spread from non-infected bots (can stay near infected or cured bots)
            const bool memberIsInfected = botAI->GetAura("death plague", member) != nullptr;
            if (memberIsInfected)
                continue;

            const float distance = bot->GetExactDist2d(member);
            if (distance < safeSpacingRadius)
            {
                // Calculate vector from member to bot
                float dx = bot->GetPositionX() - member->GetPositionX();
                float dy = bot->GetPositionY() - member->GetPositionY();

                // Weight by inverse distance (closer members have more influence)
                float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
                totalX += dx * weight;
                totalY += dy * weight;
                nearbyCount++;
            }
        }

        // If we have nearby non-infected members, move away in the combined direction
        if (nearbyCount > 0)
        {
            // Normalize the combined vector
            float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
            if (magnitude > 0.001f)  // Avoid division by zero
            {
                totalX /= magnitude;
                totalY /= magnitude;

                // Calculate move distance based on nearest member
                float moveDistance = std::min(moveIncrement, maxMoveDistance);

                // Create target position in the combined direction
                float targetX = bot->GetPositionX() + totalX * moveDistance;
                float targetY = bot->GetPositionY() + totalY * moveDistance;
                float targetZ = bot->GetPositionZ();  // Maintain current Z

                // Check if the target position is valid and move there
                if (bot->IsWithinLOS(targetX, targetY, targetZ))
                {
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                                  MovementPriority::MOVEMENT_NORMAL);
                }
                else
                {
                    // If los check fails, try shorter distance
                    targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                    targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                                  MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }
*/
    return false;  // No movement needed
}

//Gunship
bool IccCannonFireAction::Execute(Event event)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();

    if (!vehicleBase || !vehicle)
        return false;

    Unit* target = FindValidCannonTarget();
    if (!target)
        return false;

    // Try to cast Incinerating Blast if we have enough energy
    const float energyThreshold = 90.0f;
    if (vehicleBase->GetPower(POWER_ENERGY) >= energyThreshold)
    {
        const uint32 blastSpellId = AI_VALUE2(uint32, "vehicle spell id", "incinerating blast");
        if (TryCastCannonSpell(blastSpellId, target, vehicleBase))
            return true;
    }

    // Otherwise just use regular Cannon Blast
    const uint32 cannonSpellId = AI_VALUE2(uint32, "vehicle spell id", "cannon blast");
    return TryCastCannonSpell(cannonSpellId, target, vehicleBase);
}

Unit* IccCannonFireAction::FindValidCannonTarget()
{
    const GuidVector attackers = AI_VALUE(GuidVector, "possible targets no los");

    for (const auto& attackerGuid : attackers)
    {
        Unit* unit = botAI->GetUnit(attackerGuid);
        if (!unit)
            continue;

        for (const uint32 entry : availableTargetsGS)
        {
            if (unit->GetEntry() == entry)
                return unit;
        }
    }

    return nullptr;
}

bool IccCannonFireAction::TryCastCannonSpell(uint32 spellId, Unit* target, Unit* vehicleBase)
{
    static constexpr uint32 cooldownMs = 1000;

    if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
    {
        vehicleBase->AddSpellCooldown(spellId, 0, cooldownMs);
        return true;
    }

    return false;
}

bool IccGunshipEnterCannonAction::Execute(Event event)
{
    // Do not switch vehicles if already in one
    if (bot->GetVehicle())
        return false;

    Unit* bestVehicle = FindBestAvailableCannon();
    if (!bestVehicle)
        return false;

    return EnterVehicle(bestVehicle, true);
}

Unit* IccGunshipEnterCannonAction::FindBestAvailableCannon()
{
    const uint32 validCannonEntries[] = {NPC_CANNONA, NPC_CANNONH};
    Unit* bestVehicle = nullptr;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (const auto& npcGuid : npcs)
    {
        Unit* vehicleBase = botAI->GetUnit(npcGuid);
        if (!IsValidCannon(vehicleBase, validCannonEntries))
            continue;

        // Choose the closest valid cannon
        if (!bestVehicle || bot->GetExactDist(vehicleBase) < bot->GetExactDist(bestVehicle))
            bestVehicle = vehicleBase;
    }

    return bestVehicle;
}

bool IccGunshipEnterCannonAction::IsValidCannon(Unit* vehicle, const uint32 validEntries[])
{
    if (!vehicle)
        return false;

    // Must be selectable
    if (vehicle->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    // Must be friendly
    if (!vehicle->IsFriendlyTo(bot))
        return false;

    // Must have available seats
    if (!vehicle->GetVehicleKit() || !vehicle->GetVehicleKit()->GetAvailableSeatCount())
        return false;

    // Must be one of the cannon entries
    const uint32 entry = vehicle->GetEntry();
    bool isValidEntry = false;
    for (size_t i = 0; i < 2; ++i)
    {  // 2 is the size of validEntries
        if (entry == validEntries[i])
        {
            isValidEntry = true;
            break;
        }
    }

    if (!isValidEntry)
        return false;

    // Must not have these auras (frozen or disabled)
    if (vehicle->HasAura(69704) || vehicle->HasAura(69705))
        return false;

    return true;
}

bool IccGunshipEnterCannonAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    const float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    // Prepare for entering vehicle
    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();

    // Enter the vehicle
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // Dismount because bots can enter vehicle while mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return true;
}

bool IccGunshipTeleportAllyAction::Execute(Event event)
{
    static constexpr float MAX_WAITING_DISTANCE = 45.0f;
    static constexpr float MAX_ATTACK_DISTANCE = 15.0f;
    static constexpr uint8_t SKULL_ICON_INDEX = 7;

    // Find the Battle-Mage boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "kor'kron battle-mage");

    // Check if we need to remove skull icon when boss is dead
    CleanupSkullIcon(SKULL_ICON_INDEX);

    // If no boss found or boss is dead or not casting, check waiting position
    if (!boss || !boss->IsAlive() || !boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // If we're too far from waiting position, go there
        if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_ALLY2) > MAX_WAITING_DISTANCE)
            return TeleportTo(ICC_GUNSHIP_TELEPORT_ALLY2);
    }
    else if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BELOW_ZERO) &&
             boss->IsAlive())
    {
        // Mark the boss with skull icon
        UpdateBossSkullIcon(boss, SKULL_ICON_INDEX);

        // Teleport non-tank bots to attack position if not already there
        if (!botAI->IsAssistTank(bot) && bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_ALLY) > MAX_ATTACK_DISTANCE)
            return TeleportTo(ICC_GUNSHIP_TELEPORT_ALLY);
    }

    return false;
}

bool IccGunshipTeleportAllyAction::TeleportTo(const Position& position)
{
    return bot->TeleportTo(bot->GetMapId(), position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                           bot->GetOrientation());
}

void IccGunshipTeleportAllyAction::CleanupSkullIcon(uint8_t SKULL_ICON_INDEX)
{
    if (Group* group = bot->GetGroup())
    {
        const ObjectGuid currentSkullTarget = group->GetTargetIcon(SKULL_ICON_INDEX);

        if (!currentSkullTarget.IsEmpty())
        {
            Unit* skullTarget = ObjectAccessor::GetUnit(*bot, currentSkullTarget);

            if (!skullTarget || !skullTarget->IsAlive())
            {
                // Target is dead or doesn't exist, remove icon
                group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), ObjectGuid::Empty);
            }
        }
    }
}

void IccGunshipTeleportAllyAction::UpdateBossSkullIcon(Unit* boss, uint8_t SKULL_ICON_INDEX)
{
    if (Group* group = bot->GetGroup())
    {
        if (group->GetTargetIcon(SKULL_ICON_INDEX) != boss->GetGUID())
            group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), boss->GetGUID());
    }
}

bool IccGunshipTeleportHordeAction::Execute(Event event)
{
    static constexpr float MAX_WAITING_DISTANCE = 45.0f;
    static constexpr float MAX_ATTACK_DISTANCE = 15.0f;
    static constexpr uint8_t SKULL_ICON_INDEX = 7;

    // Find the Sorcerer boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "skybreaker sorcerer");

    // Check if we need to remove skull icon when boss is dead
    CleanupSkullIcon(SKULL_ICON_INDEX);

    // If no boss found or boss is dead or not casting, check waiting position
    if (!boss || !boss->IsAlive() || !boss->HasUnitState(UNIT_STATE_CASTING))
    {
        // If we're too far from waiting position, go there
        if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_HORDE2) > MAX_WAITING_DISTANCE)
            return TeleportTo(ICC_GUNSHIP_TELEPORT_HORDE2);
    }
    else if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BELOW_ZERO) &&
             boss->IsAlive())
    {
        // Mark the boss with skull icon
        UpdateBossSkullIcon(boss, SKULL_ICON_INDEX);

        // Teleport non-tank bots to attack position if not already there
        if (!botAI->IsAssistTank(bot) && bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_HORDE) > MAX_ATTACK_DISTANCE)
            return TeleportTo(ICC_GUNSHIP_TELEPORT_HORDE);
    }

    return false;
}

bool IccGunshipTeleportHordeAction::TeleportTo(const Position& position)
{
    return bot->TeleportTo(bot->GetMapId(), position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                           bot->GetOrientation());
}

void IccGunshipTeleportHordeAction::CleanupSkullIcon(uint8_t SKULL_ICON_INDEX)
{
    if (Group* group = bot->GetGroup())
    {
        const ObjectGuid currentSkullTarget = group->GetTargetIcon(SKULL_ICON_INDEX);

        if (!currentSkullTarget.IsEmpty())
        {
            Unit* skullTarget = ObjectAccessor::GetUnit(*bot, currentSkullTarget);

            if (!skullTarget || !skullTarget->IsAlive())
            {
                // Target is dead or doesn't exist, remove icon
                group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), ObjectGuid::Empty);
            }
        }
    }
}

void IccGunshipTeleportHordeAction::UpdateBossSkullIcon(Unit* boss, uint8_t SKULL_ICON_INDEX)
{
    if (Group* group = bot->GetGroup())
    {
        if (group->GetTargetIcon(SKULL_ICON_INDEX) != boss->GetGUID())
            group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), boss->GetGUID());
    }
}

//DBS
bool IccDbsTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    Unit* beast = AI_VALUE2(Unit*, "find target", "blood beast");

    // Handle tank positioning
    if (botAI->IsTank(bot) && !beast)
    {
        if (bot->GetExactDist2d(ICC_DBS_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_DBS_TANK_POSITION.GetPositionX(), ICC_DBS_TANK_POSITION.GetPositionY(),
                          ICC_DBS_TANK_POSITION.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL);



        // Early return if this tank has Rune of Blood
        if (botAI->GetAura("Rune of Blood", bot))
            return true;
    }

    if (!botAI->IsTank(bot))
    {
        if (CrowdControlBloodBeasts())
            return true;
    }

    // Handle ranged and healer positioning
    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
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
    const std::array<uint32_t, 4> bloodBeastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3,
                                                       NPC_BLOOD_BEAST4};
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    bool appliedCC = false;

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if this is a blood beast
        const bool isBloodBeast =
            std::find(bloodBeastEntries.begin(), bloodBeastEntries.end(), unit->GetEntry()) != bloodBeastEntries.end();

        if (!isBloodBeast)
            continue;

        // Apply class-specific CC
        switch (bot->getClass())
        {
            case CLASS_MAGE:
                if (!botAI->HasAura("Frost Nova", unit))
                {
                    botAI->CastSpell("Frost Nova", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_DRUID:
                if (!botAI->HasAura("Entangling Roots", unit))
                {
                    botAI->CastSpell("Entangling Roots", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_PALADIN:
                if (!botAI->HasAura("Hammer of Justice", unit))
                {
                    botAI->CastSpell("Hammer of Justice", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_WARRIOR:
                if (!botAI->HasAura("Hamstring", unit))
                {
                    botAI->CastSpell("Hamstring", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_HUNTER:
                if (!botAI->HasAura("Concussive Shot", unit))
                {
                    botAI->CastSpell("Concussive Shot", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_ROGUE:
                if (!botAI->HasAura("Kidney Shot", unit))
                {
                    botAI->CastSpell("Kidney Shot", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_SHAMAN:
                if (!botAI->HasAura("Frost Shock", unit))
                {
                    botAI->CastSpell("Frost Shock", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_DEATH_KNIGHT:
                if (!botAI->HasAura("Chains of Ice", unit))
                {
                    botAI->CastSpell("Chains of Ice", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_PRIEST:
                if (!botAI->HasAura("Psychic Scream", unit))
                {
                    botAI->CastSpell("Psychic Scream", unit);
                    appliedCC = true;
                }
                break;
            case CLASS_WARLOCK:
                if (!botAI->HasAura("Fear", unit))
                {
                    botAI->CastSpell("Fear", unit);
                    appliedCC = true;
                }
                break;
            default:
                break;
        }
    }

    return false;
}

bool IccDbsTankPositionAction::EvadeBloodBeasts()
{
    const float evasionDistance = 12.0f;
    const std::array<uint32_t, 4> bloodBeastEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3, NPC_BLOOD_BEAST4};

    // Get the nearest hostile NPCs
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        // Check if this is a blood beast
        const bool isBloodBeast = std::find(bloodBeastEntries.begin(), bloodBeastEntries.end(), unit->GetEntry()) != bloodBeastEntries.end();

        // Only evade if it's a blood beast targeting us
        if (isBloodBeast && unit->GetVictim() == bot)
        {
            float currentDistance = bot->GetDistance2d(unit);

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

    // Find this bot's position among ranged/healers in the group
    int rangedIndex = -1;
    int currentIndex = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if ((botAI->IsRanged(member) || botAI->IsHeal(member)) && !botAI->IsTank(member))
        {
            if (member == bot)
            {
                rangedIndex = currentIndex;
                break;
            }
            currentIndex++;
        }
    }

    if (rangedIndex == -1)
        return false;

    // Fixed positions calculation
    constexpr float tankToBossAngle = 3.14f;
    constexpr float minBossDistance = 11.0f;
    constexpr float spreadDistance = 10.0f;
    constexpr int columnsPerRow = 5;

    // Calculate position in a fixed grid (3 rows x 5 columns)
    int row = rangedIndex / columnsPerRow;
    int col = rangedIndex % columnsPerRow;

    // Calculate base position
    float xOffset = (col - 2) * spreadDistance;                // Center around tank position
    float yOffset = minBossDistance + (row * spreadDistance);  // Each row further back

    // Add zigzag offset for odd rows
    if (row % 2 == 1)
        xOffset += spreadDistance / 2;

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

bool IccAddsDbsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    // This action is only for melee
    if (!botAI->IsMelee(bot))
        return false;

    Unit* priorityTarget = FindPriorityTarget(boss);

    // Update raid target icons if needed
    UpdateSkullMarker(priorityTarget);

    return false;
}

Unit* IccAddsDbsAction::FindPriorityTarget(Unit* boss)
{
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    // Blood beast entry IDs
    constexpr std::array<uint32_t, 4> addEntries = {NPC_BLOOD_BEAST1, NPC_BLOOD_BEAST2, NPC_BLOOD_BEAST3, NPC_BLOOD_BEAST4};

    // First check for alive adds
    for (uint32_t entry : addEntries)
    {
        for (const ObjectGuid& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
                return unit;
        }
    }

    // Only fallback to boss if it's alive
    return boss->IsAlive() ? const_cast<Unit*>(boss) : nullptr;
}

void IccAddsDbsAction::UpdateSkullMarker(Unit* priorityTarget)
{
    if (!priorityTarget)
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    constexpr uint8_t skullIconId = 7;

    // Get current skull target
    ObjectGuid currentSkull = group->GetTargetIcon(skullIconId);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

    // Determine if skull marker needs updating
    bool needsUpdate = !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != priorityTarget;

    // Update if needed
    if (needsUpdate)
        group->SetTargetIcon(skullIconId, bot->GetGUID(), priorityTarget->GetGUID());
}

// Festergut
bool IccFestergutGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());

    // Handle tank positioning
    if ((botAI->HasAggro(boss) && botAI->IsMainTank(bot)) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_FESTERGUT_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_FESTERGUT_TANK_POSITION.GetPositionX(),
                          ICC_FESTERGUT_TANK_POSITION.GetPositionY(), ICC_FESTERGUT_TANK_POSITION.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    // Check for spores in the group
    if (HasSporesInGroup())
        return false;

    // Position non-tank ranged and healers
    return PositionNonTankMembers();
}

bool IccFestergutGroupPositionAction::HasSporesInGroup()
{
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* unit = botAI->GetUnit(memberGuid);
        if (unit && unit->HasAura(SPELL_GAS_SPORE))
            return true;
    }

    return false;
}

bool IccFestergutGroupPositionAction::PositionNonTankMembers()
{
    // Only position ranged and healers without spores
    if (!(botAI->IsRanged(bot) || botAI->IsHeal(bot)))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int positionIndex = CalculatePositionIndex(group);
    if (positionIndex == -1)
        return false;

    // Position calculation parameters
    constexpr float tankToBossAngle = 4.58f;
    constexpr float minBossDistance = 15.0f;
    constexpr float spreadDistance = 10.0f;
    constexpr int columnsPerRow = 6;

    // Calculate grid position
    int row = positionIndex / columnsPerRow;
    int col = positionIndex % columnsPerRow;

    // Calculate base position
    float xOffset = (col - 2) * spreadDistance;                // Center around tank position
    float yOffset = minBossDistance + (row * spreadDistance);  // Each row further back

    // Add zigzag offset for odd rows
    if (row % 2 == 1)
        xOffset += spreadDistance / 2;

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

int IccFestergutGroupPositionAction::CalculatePositionIndex(Group* group)
{
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> rangedDpsGuids;
    std::vector<ObjectGuid> hunterGuids;

    // Collect all eligible members with their GUIDs
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || botAI->IsTank(member))
            continue;

        ObjectGuid memberGuid = member->GetGUID();

        if (botAI->IsHeal(member))
        {
            healerGuids.push_back(memberGuid);
        }
        else if (botAI->IsRanged(member))
        {
            if (member->getClass() == CLASS_HUNTER)
            {
                hunterGuids.push_back(memberGuid);
            }
            else
            {
                rangedDpsGuids.push_back(memberGuid);
            }
        }
    }

    // Sort GUIDs for consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(rangedDpsGuids.begin(), rangedDpsGuids.end());
    std::sort(hunterGuids.begin(), hunterGuids.end());

    ObjectGuid botGuid = bot->GetGUID();

    // Find bot's position among healers
    auto healerIt = std::find(healerGuids.begin(), healerGuids.end(), botGuid);
    if (healerIt != healerGuids.end())
    {
        int healerIndex = std::distance(healerGuids.begin(), healerIt);
        int totalHealers = healerGuids.size();

        // Healers in first two rows, distributed evenly
        int healersPerRow = (totalHealers + 1) / 2;  // Round up for first row

        if (healerIndex < healersPerRow)
        {
            // First row of healers (positions 0-5)
            return healerIndex;
        }
        else
        {
            // Second row of healers (positions 6-11)
            return healerIndex - healersPerRow + 6;
        }
    }

    // Find bot's position among non-hunter ranged DPS
    auto rangedIt = std::find(rangedDpsGuids.begin(), rangedDpsGuids.end(), botGuid);
    if (rangedIt != rangedDpsGuids.end())
    {
        int rangedIndex = std::distance(rangedDpsGuids.begin(), rangedIt);
        int totalHealers = healerGuids.size();

        // Non-hunter ranged DPS fill remaining spots in healer rows first
        int healerSpotsUsed = totalHealers;
        int remainingSpotsInHealerRows = 12 - healerSpotsUsed;  // 2 rows of 6 spots each

        if (rangedIndex < remainingSpotsInHealerRows)
        {
            // Fill remaining spots in healer rows
            if (healerSpotsUsed < 6)
            {
                // Fill remaining spots in first row
                return healerSpotsUsed + rangedIndex;
            }
            else
            {
                // Fill remaining spots in second row
                int spotsInFirstRow = 6;
                int spotsInSecondRow = healerSpotsUsed - 6;
                int remainingInSecondRow = 6 - spotsInSecondRow;

                if (rangedIndex < remainingInSecondRow)
                {
                    return 6 + spotsInSecondRow + rangedIndex;
                }
                else
                {
                    // Move to third row
                    return 12 + (rangedIndex - remainingInSecondRow);
                }
            }
        }
        else
        {
            // Start new rows for remaining ranged DPS
            return 12 + (rangedIndex - remainingSpotsInHealerRows);
        }
    }

    // Find bot's position among hunters
    auto hunterIt = std::find(hunterGuids.begin(), hunterGuids.end(), botGuid);
    if (hunterIt != hunterGuids.end())
    {
        int hunterIndex = std::distance(hunterGuids.begin(), hunterIt);
        int totalHealers = healerGuids.size();
        int totalRangedDps = rangedDpsGuids.size();

        // Hunters start after healers and non-hunter ranged DPS
        // But ensure they're at least in the second row (position 6+)
        int basePosition = totalHealers + totalRangedDps + hunterIndex;

        // If position would be in first row (0-5), move to second row minimum
        if (basePosition < 6)
            basePosition = 6 + hunterIndex;

        return basePosition;
    }

    return -1;
}

bool IccFestergutSporeAction::Execute(Event event)
{
    constexpr float POSITION_TOLERANCE = 4.0f;
    constexpr float SPREAD_RADIUS = 2.0f;

    // Check if bot has spore
    bool hasSpore = bot->HasAura(SPELL_GAS_SPORE);  // gas spore

    // Stop attacking if spored
    if (hasSpore)
        botAI->Reset();

    // Calculate unique spread position for ranged
    Position spreadRangedPos = CalculateSpreadPosition();

    // Find spored players
    SporeInfo sporeInfo = FindSporedPlayers();

    // Determine target position
    Position targetPos = DetermineTargetPosition(hasSpore, sporeInfo, spreadRangedPos);

    // Move to position if not already there
    if (bot->GetExactDist2d(targetPos) > POSITION_TOLERANCE)
    {
        botAI->Reset();
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                      true, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return hasSpore;
}

Position IccFestergutSporeAction::CalculateSpreadPosition()
{
    constexpr float SPREAD_RADIUS = 2.0f;

    // Unique angle based on bot's GUID
    float angle = (bot->GetGUID().GetCounter() % 16) * (M_PI / 8);

    Position spreadRangedPos = ICC_FESTERGUT_RANGED_SPORE;
    spreadRangedPos.m_positionX += cos(angle) * SPREAD_RADIUS;
    spreadRangedPos.m_positionY += sin(angle) * SPREAD_RADIUS;

    return spreadRangedPos;
}

IccFestergutSporeAction::SporeInfo IccFestergutSporeAction::FindSporedPlayers()
{
    SporeInfo info;
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
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

Position IccFestergutSporeAction::DetermineTargetPosition(bool hasSpore, const SporeInfo& sporeInfo, const Position& spreadRangedPos)
{
    // No spores at all
    if (sporeInfo.sporedPlayers.empty())
        return botAI->IsMelee(bot) ? ICC_FESTERGUT_MELEE_SPORE : spreadRangedPos;

    // Bot has no spore, go to standard position
    if (!hasSpore)
        return botAI->IsMelee(bot) ? ICC_FESTERGUT_MELEE_SPORE : spreadRangedPos;

    // Check if main tank has spore
    bool mainTankHasSpore = CheckMainTankSpore();

    // Determine position based on spore logic
    if (botAI->IsMainTank(bot))
        return ICC_FESTERGUT_MELEE_SPORE;

    if (bot->GetGUID() == sporeInfo.lowestGuid && !botAI->IsTank(bot) && !mainTankHasSpore)
        return ICC_FESTERGUT_MELEE_SPORE;

    return spreadRangedPos;
}

bool IccFestergutSporeAction::CheckMainTankSpore()
{
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    for (const auto& memberGuid : members)
    {
        Unit* unit = botAI->GetUnit(memberGuid);
        if (!unit)
            continue;

        if (botAI->IsMainTank(unit->ToPlayer()) && unit->HasAura(SPELL_GAS_SPORE))
            return true;
    }

    return false;
}

// Rotface
bool IccRotfaceTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");
    bool victimOfSmallOoze = smallOoze && smallOoze->GetVictim() == bot;
    // Mark Rotface with skull
    MarkBossWithSkull(boss);

    // Main tank positioning and melee positioning
    if ((botAI->IsMainTank(bot) || botAI->IsMelee(bot)) && !botAI->IsAssistTank(bot) && !victimOfSmallOoze)
        return PositionMainTankAndMelee(boss);

    // Assist tank positioning for big ooze
    if (botAI->IsAssistTank(bot))
        return HandleAssistTankPositioning(boss);

    return false;
}

void IccRotfaceTankPositionAction::MarkBossWithSkull(Unit* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    constexpr uint8_t skullIconId = 7;
    ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);
    if (skullGuid != boss->GetGUID())
        group->SetTargetIcon(skullIconId, bot->GetGUID(), boss->GetGUID());
}

bool IccRotfaceTankPositionAction::PositionMainTankAndMelee(Unit* boss)
{
    bool isBossCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->GetCurrentSpell(SPELL_SLIME_SPRAY))
        bool isBossCasting = true;

    if (bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION) > 7.0f && botAI->HasAggro(boss) && botAI->IsMainTank(bot))
        MoveTo(bot->GetMapId(), ICC_ROTFACE_CENTER_POSITION.GetPositionX(),
               ICC_ROTFACE_CENTER_POSITION.GetPositionY(), ICC_ROTFACE_CENTER_POSITION.GetPositionZ(),
               false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

    if (boss && isBossCasting && !botAI->IsTank(bot))
    {
        float x = boss->GetPositionX();
        float y = boss->GetPositionY();
        float z = boss->GetPositionZ();

        // If not already close to the boss's position, move there
        if (bot->GetExactDist2d(x, y) > 0.5f)
        {
            MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                   false);
        }
        // Otherwise, already at the correct position
        return false;
    }

    if (!isBossCasting && (bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION) < 2.0f || bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION) > 7.0f) && !botAI->IsTank(bot))
    {
       MoveTo(bot->GetMapId(), ICC_ROTFACE_CENTER_POSITION.GetPositionX(), ICC_ROTFACE_CENTER_POSITION.GetPositionY(),
              bot->GetPositionZ(),  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);                 
    }

    return false;
}

bool IccRotfaceTankPositionAction::HandleAssistTankPositioning(Unit* boss)
{
    // Handle big ooze positioning
    return HandleBigOozePositioning(boss);
}


bool IccRotfaceTankPositionAction::HandleBigOozePositioning(Unit* boss)
{
    // Find all big oozes
    GuidVector bigOozes = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> activeBigOozes;

    for (const auto& guid : bigOozes)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_BIG_OOZE && unit->IsVisible())
            activeBigOozes.push_back(unit);
    }

    if (activeBigOozes.empty())
        return false;

    // Iterate through all big oozes and handle them
    for (Unit* bigOoze : activeBigOozes)
    {
        // Taunt if not targeting us
        if (bigOoze->GetVictim() != bot && bigOoze->IsAlive() && bigOoze->IsVisible())
        {
            if (botAI->CastSpell("taunt", bigOoze))
                return true;
            bot->SetTarget(bigOoze->GetGUID());
            bot->SetFacingToObject(bigOoze);
            return Attack(bigOoze);
        }

        // Calculate distances
        float oozeDistance = bot->GetExactDist2d(bigOoze);

        // Stop moving if ooze is far enough
        if (oozeDistance > 12.0f)
        {
            bot->SetTarget(bigOoze->GetGUID());
            bot->SetFacingToObject(bigOoze);
            return true;
        }

        // If we have the ooze's aggro, kite it in a larger circular pattern between 20f and 30f from the center
        if (bigOoze->GetVictim() == bot)
        {
            const float minRadius = 24.0f;
            const float maxRadius = 34.0f;
            const float safeDistanceFromOoze = 13.0f;
            const float puddleSafeDistance = 30.0f;
            const Position centerPosition = ICC_ROTFACE_CENTER_POSITION;

            float currentDistance = bot->GetExactDist2d(centerPosition);

            // If too close or too far, adjust position
            if (currentDistance < minRadius || currentDistance > maxRadius)
            {
                // Calculate direction vector from bot to center
                float dirX = bot->GetPositionX() - centerPosition.GetPositionX();
                float dirY = bot->GetPositionY() - centerPosition.GetPositionY();
                float length = std::sqrt(dirX * dirX + dirY * dirY);

                // Normalize direction vector
                dirX /= length;
                dirY /= length;

                // Adjust position to stay within the desired radius
                float targetX = centerPosition.GetPositionX() + dirX * maxRadius;
                float targetY = centerPosition.GetPositionY() + dirY * maxRadius;

                // Ensure the position is at least 10f away from the ooze
                if (bigOoze->GetExactDist2d(targetX, targetY) >= safeDistanceFromOoze)
                {
                    return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, true,
                                  MovementPriority::MOVEMENT_COMBAT);
                }
            }

            // If within the desired radius, continue kiting in a circular pattern
            float currentAngle = atan2(bot->GetPositionY() - centerPosition.GetPositionY(),
                                       bot->GetPositionX() - centerPosition.GetPositionX());

            // Adjust rotation direction to find a safe position
            for (int i = 0; i < 16; ++i)  // Try 16 directions around the circle
            {
                float angleOffset = (i % 2 == 0 ? 1 : -1) * (M_PI / 16.0f) * (i / static_cast<float>(2));
                float newAngle = currentAngle + angleOffset;

                // Calculate new position along the circle
                float newX = centerPosition.GetPositionX() + maxRadius * cos(newAngle);
                float newY = centerPosition.GetPositionY() + maxRadius * sin(newAngle);

                // Ensure the position is at least 10f away from the ooze
                if (bigOoze->GetExactDist2d(newX, newY) >= safeDistanceFromOoze)
                {
                    // Check if the position is at least 30f away from any puddle
                    GuidVector puddles = AI_VALUE(GuidVector, "nearest hostile npcs");
                    bool isSafeFromPuddles = true;

                    for (const auto& puddleGuid : puddles)
                    {
                        Unit* puddle = botAI->GetUnit(puddleGuid);
                        if (puddle && botAI->GetAura("Ooze Flood", puddle))
                        {
                            float puddleDistance = std::sqrt(std::pow(newX - puddle->GetPositionX(), 2) +
                                                             std::pow(newY - puddle->GetPositionY(), 2));
                            if (puddleDistance < puddleSafeDistance)
                            {
                                isSafeFromPuddles = false;
                                break;
                            }
                        }
                    }

                    if (isSafeFromPuddles)
                    {
                        return MoveTo(bot->GetMapId(), newX, newY, bot->GetPositionZ(), false, false, false, true,
                                      MovementPriority::MOVEMENT_COMBAT);
                    }
                }
            }
        }
    }

    return false;
}


bool IccRotfaceGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool floodPresent = false;

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !botAI->HasAura("Ooze Flood", unit))
            continue;

        float puddleDistance = bot->GetExactDist2d(unit);

        if (puddleDistance < 30.0f)
            floodPresent = true;
    }

    Unit* bigOoze = AI_VALUE2(Unit*, "find target", "big ooze");
    bool hasOozeFlood = botAI->HasAura("Ooze Flood", bot);
    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");
    bool hasMutatedInfection = botAI->HasAura("Mutated Infection", bot);

    // Handle puddle avoidance
    if (!botAI->IsTank(bot) && HandlePuddleAvoidance(boss))
        return true;

    // Handle little ooze or mutated infection
    if (HandleOozeTargeting())
        return true;

    // Position ranged and healers
    if (/*!floodPresent && */ !((smallOoze && smallOoze->GetVictim() == bot) || hasMutatedInfection) && !hasOozeFlood && PositionRangedAndHealers(boss, smallOoze))
        return true;

    //if (!hasOozeFlood && bigOoze && bigOoze->IsAlive() && MoveAwayFromBigOoze(bigOoze))
        //return true;

    return false;
}

bool IccRotfaceGroupPositionAction::HandlePuddleAvoidance(Unit* boss)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !botAI->HasAura("Ooze Flood", unit))
            continue;

        float puddleDistance = bot->GetExactDist2d(unit);
        float bossDistance = bot->GetExactDist2d(ICC_ROTFACE_CENTER_POSITION);

        if (bossDistance < 15.0f)  // Reduced center distance threshold
            return false;

        if (puddleDistance < 30.0f)
            return MoveAwayFromPuddle(boss, unit, puddleDistance);
    }

    return false;
}

bool IccRotfaceGroupPositionAction::MoveAwayFromPuddle(Unit* boss, Unit* puddle, float puddleDistance)
{
    if (!boss || !puddle)
        return false;

    // Calculate angle and move direction
    float dx = puddle->GetPositionX() - bot->GetPositionX();
    float dy = puddle->GetPositionY() - bot->GetPositionY();
    float angle = atan2(dy, dx);

    // Try to find a valid position in 8 directions
    const float increment = 5.0f;
    const float minPuddleDistance = 30.0f;
    const float minCenterDistance = 15.0f;  // Reduced center distance threshold
    const float maxCenterDistance = 25.0f;  // New maximum center distance threshold
    const int directions = 8;               // Number of directions to check
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = 0.0f;

    for (int i = 0; i < directions; ++i)
    {
        float testAngle = angle + (i * M_PI / 4);  // 8 directions (45-degree increments)
        for (float distance = increment; distance <= 35.0f; distance += increment)
        {
            float moveX = bot->GetPositionX() - distance * cos(testAngle);
            float moveY = bot->GetPositionY() - distance * sin(testAngle);
            float moveZ = bot->GetPositionZ();

            // Check distances and line of sight
            float newPuddleDistance =
                sqrt(pow(moveX - puddle->GetPositionX(), 2) + pow(moveY - puddle->GetPositionY(), 2));
            float newCenterDistance = sqrt(pow(moveX - ICC_ROTFACE_CENTER_POSITION.GetPositionX(), 2) +
                                           pow(moveY - ICC_ROTFACE_CENTER_POSITION.GetPositionY(), 2));

            if (newPuddleDistance >= minPuddleDistance && newCenterDistance >= minCenterDistance &&
                newCenterDistance <= maxCenterDistance && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // Calculate safety score (favor positions farther from puddle and center)
                float safetyScore = newPuddleDistance + newCenterDistance;
                if (safetyScore > maxSafetyScore)
                {
                    maxSafetyScore = safetyScore;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }

        // If we are already in a valid position, stop moving
        if (maxSafetyScore > 0.0f && bot->GetExactDist2d(bestX, bestY) <= increment)
            return false;
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool IccRotfaceGroupPositionAction::HandleOozeTargeting()
{
    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");
    bool hasMutatedInfection = botAI->HasAura("Mutated Infection", bot);

    if ((smallOoze && smallOoze->GetVictim() == bot) || hasMutatedInfection)
        return HandleOozeMemberPositioning();

    return false;
}

bool IccRotfaceGroupPositionAction::HandleOozeMemberPositioning()
{
    Unit* bigOoze = AI_VALUE2(Unit*, "find target", "big ooze");

    // First case: No big ooze exists or is not alive, move to designated position
    if (!bigOoze || !bigOoze->IsAlive() || !bigOoze->IsVisible())
    {
        if (bot->GetExactDist2d(ICC_ROTFACE_BIG_OOZE_POSITION) > 3.0f)
        {
            return MoveTo(bot->GetMapId(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionX(),
                          ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionY(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionZ(),
                          false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    // Second case: Big ooze exists and is alive, move to it for merging
    else if (bot->GetExactDist2d(bigOoze) > 2.0f && bigOoze->IsAlive() && bigOoze->IsVisible())
    {
        // Move to big ooze for merge in increments of 5
        float dx = bigOoze->GetPositionX() - bot->GetPositionX();
        float dy = bigOoze->GetPositionY() - bot->GetPositionY();
        float dz = bigOoze->GetPositionZ() - bot->GetPositionZ();
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 5.0f)
        {
            dx /= dist;
            dy /= dist;
            float moveX = bot->GetPositionX() + dx * 5.0f;
            float moveY = bot->GetPositionY() + dy * 5.0f;
            float moveZ = bot->GetPositionZ() + (dz / dist) * 5.0f;
            return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT);
        }
        return MoveTo(bot->GetMapId(), bigOoze->GetPositionX(), bigOoze->GetPositionY(), bigOoze->GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_COMBAT);

    }

    return false;  // Stay at position
}

bool IccRotfaceGroupPositionAction::PositionRangedAndHealers(Unit* boss,Unit *smallOoze)
{
    // Only for ranged and healers
    if (!(botAI->IsRanged(bot) || botAI->IsHeal(bot)))
        return false;

    Difficulty diff = bot->GetRaidDifficulty();
    bool isBossCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->GetCurrentSpell(SPELL_SLIME_SPRAY))
        bool isBossCasting = true;

    bool isHeroic = (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC);

   // Move to the exact same position as the boss during slime spray
    if (boss && isBossCasting && !isHeroic)
    {
        float x = boss->GetPositionX();
        float y = boss->GetPositionY();
        float z = boss->GetPositionZ();

        // If not already close to the boss's position, move there
        if (bot->GetExactDist2d(x, y) > 0.5f)
        {
            MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_FORCED, true,
                          false);
        }
        // Otherwise, already at the correct position
        return false;
    }

    if (!isHeroic && !isBossCasting && boss && !(bot->getClass() == CLASS_HUNTER) &&
        (bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) < 2.0f ||
         bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) > 5.0f))
    {
        MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(),
                      bot->GetPositionZ(),  false, false, false, true, MovementPriority::MOVEMENT_COMBAT);                 
    }

    if (!isHeroic)
        return false;

    return FindAndMoveFromClosestMember(boss, smallOoze);
}

bool IccRotfaceGroupPositionAction::FindAndMoveFromClosestMember(Unit* boss, Unit* smallOoze)
{

    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* puddle = nullptr;

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !botAI->HasAura("Ooze Flood", unit))
            continue;

        puddle = unit;
        break;
    }

    const float safeSpacingRadius = 10.0f;
    const float moveIncrement = 2.0f;
    const float maxMoveDistance = 12.0f;     // Limit maximum movement distance
    const float puddleSafeDistance = 30.0f;  // Minimum distance to stay away from puddle
    const float minCenterDistance = 20.0f;   // Minimum distance from center position
    const bool isRanged = botAI->IsRanged(bot) || botAI->IsHeal(bot);

    // Ranged: spread from other members
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    // Calculate a combined vector representing all nearby members' positions
    float totalX = 0.0f;
    float totalY = 0.0f;
    int nearbyCount = 0;

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || member == bot || (smallOoze && smallOoze->GetVictim() == member) ||
            (member->GetTypeId() == TYPEID_PLAYER && botAI->IsAssistTank(static_cast<Player*>(member))))
            continue;

        const float distance = bot->GetExactDist2d(member);
        if (distance < safeSpacingRadius)
        {
            // Calculate vector from member to bot
            float dx = bot->GetPositionX() - member->GetPositionX();
            float dy = bot->GetPositionY() - member->GetPositionY();

            // Weight by inverse distance (closer members have more influence)
            float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
            totalX += dx * weight;
            totalY += dy * weight;
            nearbyCount++;
        }
    }

    // If we have nearby members, move away in the combined direction
    if (nearbyCount > 0)
    {
        // Normalize the combined vector
        float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
        if (magnitude > 0.001f)  // Avoid division by zero
        {
            totalX /= magnitude;
            totalY /= magnitude;

            // Calculate move distance based on nearest member
            float moveDistance = std::min(moveIncrement, maxMoveDistance);

            // Create target position in the combined direction
            float targetX = bot->GetPositionX() + totalX * moveDistance;
            float targetY = bot->GetPositionY() + totalY * moveDistance;
            float targetZ = bot->GetPositionZ();  // Maintain current Z

            // Ensure the target position is at least 30 yards away from the puddle
            if (puddle)
            {
                float puddleDistance = std::sqrt(std::pow(targetX - puddle->GetPositionX(), 2) +
                                                 std::pow(targetY - puddle->GetPositionY(), 2));
                if (puddleDistance < puddleSafeDistance)
                {
                    // Adjust the target position to move further away from the puddle
                    float dx = targetX - puddle->GetPositionX();
                    float dy = targetY - puddle->GetPositionY();
                    float adjustmentFactor = (puddleSafeDistance - puddleDistance) / puddleDistance;
                    targetX += dx * adjustmentFactor;
                    targetY += dy * adjustmentFactor;
                }
            }

            // Ensure the target position is at least 20 yards away from the center position
            const float centerX = ICC_ROTFACE_CENTER_POSITION.GetPositionX();
            const float centerY = ICC_ROTFACE_CENTER_POSITION.GetPositionY();
            float centerDistance = std::sqrt(std::pow(targetX - centerX, 2) + std::pow(targetY - centerY, 2));
            if (centerDistance < minCenterDistance)
            {
                // Adjust the target position to move further away from the center
                float dx = targetX - centerX;
                float dy = targetY - centerY;
                float adjustmentFactor = (minCenterDistance - centerDistance) / centerDistance;
                targetX += dx * adjustmentFactor;
                targetY += dy * adjustmentFactor;
            }

            // Check if the target position is valid and move there
            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            else
            {
                // If los check fails, try shorter distance
                targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                Position targetPos(targetX, targetY, targetZ);
                MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;  // Everyone is properly spaced
}

bool IccRotfaceMoveAwayFromExplosionAction::Execute(Event event)
{
    // Skip if main tank or ooze flood
    if (botAI->IsMainTank(bot))
        return false;

    botAI->Reset();

    return MoveToRandomSafeLocation();
}

bool IccRotfaceMoveAwayFromExplosionAction::MoveToRandomSafeLocation()
{
    // Generate random angle
    float angle = frand(0, 2 * M_PI);

    // Calculate initial move position
    float moveX = bot->GetPositionX() + 5.0f * cos(angle);
    float moveY = bot->GetPositionY() + 5.0f * sin(angle);
    float moveZ = bot->GetPositionZ();

    // Ensure the position is at least 30 yards away from any puddle
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        Unit* puddle = botAI->GetUnit(npc);
        if (!puddle || !botAI->HasAura("Ooze Flood", puddle))
            continue;

        float puddleDistance =
            std::sqrt(std::pow(moveX - puddle->GetPositionX(), 2) + std::pow(moveY - puddle->GetPositionY(), 2));
        if (puddleDistance < 30.0f)
        {
            // Adjust the position to move further away from the puddle
            float dx = moveX - puddle->GetPositionX();
            float dy = moveY - puddle->GetPositionY();
            float adjustmentFactor = (30.0f - puddleDistance) / puddleDistance;
            moveX += dx * adjustmentFactor;
            moveY += dy * adjustmentFactor;
        }
    }

    // Ensure the position is at least 30 yards away from the center position
    const Position centerPosition = ICC_ROTFACE_CENTER_POSITION;
    float centerDistance = std::sqrt(std::pow(moveX - centerPosition.GetPositionX(), 2) +
                                     std::pow(moveY - centerPosition.GetPositionY(), 2));
    if (centerDistance < 40.0f)
    {
        // Adjust the position to move further away from the center
        float dx = moveX - centerPosition.GetPositionX();
        float dy = moveY - centerPosition.GetPositionY();
        float adjustmentFactor = (40.0f - centerDistance) / centerDistance;
        moveX += dx * adjustmentFactor;
        moveY += dy * adjustmentFactor;
    }

    // Check line of sight
    if (!bot->IsWithinLOS(moveX, moveY, moveZ))
        return false;

    // Move in increments of 5.0f towards the calculated position
    float currentX = bot->GetPositionX();
    float currentY = bot->GetPositionY();
    float currentZ = bot->GetPositionZ();

    float directionX = moveX - currentX;
    float directionY = moveY - currentY;
    float distance = std::sqrt(directionX * directionX + directionY * directionY);

    if (distance > 5.0f)
    {
        directionX /= distance;
        directionY /= distance;

        moveX = currentX + directionX * 5.0f;
        moveY = currentY + directionY * 5.0f;
    }

    // Move to the position
    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED);
}

// Proffesor Putricide
bool IccPutricideGrowingOozePuddleAction::Execute(Event event)
{
    Unit* closestPuddle = FindClosestThreateningPuddle();
    if (!closestPuddle)
        return false;

    Position movePosition = CalculateSafeMovePosition(closestPuddle);
    return MoveTo(bot->GetMapId(), movePosition.GetPositionX(), movePosition.GetPositionY(),
                  movePosition.GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}

Unit* IccPutricideGrowingOozePuddleAction::FindClosestThreateningPuddle()
{
    static const float BASE_RADIUS = 2.0f;
    static const float STACK_MULTIPLIER = 0.6f;
    static const float MIN_DISTANCE = 0.1f;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    if (npcs.empty())
        return nullptr;

    Unit* closestPuddle = nullptr;
    float closestDistance = FLT_MAX;
    float closestSafeDistance = BASE_RADIUS;

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || unit->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;

        float currentDistance = std::max(MIN_DISTANCE, bot->GetExactDist(unit));
        float safeDistance = BASE_RADIUS;

        if (Aura* grow = unit->GetAura(SPELL_GROW_AURA))
            safeDistance += (grow->GetStackAmount() * STACK_MULTIPLIER);

        if (currentDistance < safeDistance && currentDistance < closestDistance)
        {
            closestDistance = currentDistance;
            closestSafeDistance = safeDistance;
            closestPuddle = unit;
        }
    }

    return closestPuddle;
}

Position IccPutricideGrowingOozePuddleAction::CalculateSafeMovePosition(Unit* closestPuddle)
{
    static const float BASE_RADIUS = 2.0f;
    static const float STACK_MULTIPLIER = 0.6f;
    static const float BUFFER_DISTANCE = 2.0f;
    static const float MIN_DISTANCE = 0.1f;
    static const int NUM_ANGLES_TO_TEST = 8;

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();

    // Calculate current distance and safe distance
    float currentDistance = std::max(MIN_DISTANCE, bot->GetExactDist(closestPuddle));
    float safeDistance = BASE_RADIUS;
    if (Aura* grow = closestPuddle->GetAura(SPELL_GROW_AURA))
        safeDistance += (grow->GetStackAmount() * STACK_MULTIPLIER);

    // Calculate direction vector
    float dx = botX - closestPuddle->GetPositionX();
    float dy = botY - closestPuddle->GetPositionY();
    float dist = std::max(MIN_DISTANCE, sqrt(dx * dx + dy * dy));

    if (dist < MIN_DISTANCE * 2)
    {
        float randomAngle = float(rand()) / float(RAND_MAX) * 2 * M_PI;
        dx = cos(randomAngle);
        dy = sin(randomAngle);
    }
    else
    {
        dx /= dist;
        dy /= dist;
    }

    float moveDistance = safeDistance - currentDistance + BUFFER_DISTANCE;

    // Try different angles to find safe position
    for (int i = 0; i < NUM_ANGLES_TO_TEST; i++)
    {
        float angle = (2 * M_PI * i) / NUM_ANGLES_TO_TEST;
        float rotatedDx = dx * cos(angle) - dy * sin(angle);
        float rotatedDy = dx * sin(angle) + dy * cos(angle);

        float testX = botX + rotatedDx * moveDistance;
        float testY = botY + rotatedDy * moveDistance;

        if (!IsPositionTooCloseToOtherPuddles(testX, testY, closestPuddle) && bot->IsWithinLOS(testX, testY, botZ))
        {
            // If main tank, add 6f to calculated position in the direction away from the puddle
            if (botAI->IsTank(bot))
            {
                float awayDx = testX - closestPuddle->GetPositionX();
                float awayDy = testY - closestPuddle->GetPositionY();
                float awayDist = std::sqrt(awayDx * awayDx + awayDy * awayDy);
                if (awayDist > 0.001f)
                {
                    awayDx /= awayDist;
                    awayDy /= awayDist;
                    testX += awayDx * 6.0f;
                    testY += awayDy * 6.0f;
                }
            }
            return Position(testX, testY, botZ);
        }
    }

    // Fallback position if no safe position found
    float fallbackX = botX + dx * moveDistance;
    float fallbackY = botY + dy * moveDistance;
    if (botAI->IsTank(bot))
    {
        float awayDx = fallbackX - closestPuddle->GetPositionX();
        float awayDy = fallbackY - closestPuddle->GetPositionY();
        float awayDist = std::sqrt(awayDx * awayDx + awayDy * awayDy);
        if (awayDist > 0.001f)
        {
            awayDx /= awayDist;
            awayDy /= awayDist;
            fallbackX += awayDx * 6.0f;
            fallbackY += awayDy * 6.0f;
        }
    }
    return Position(fallbackX, fallbackY, botZ);
}

bool IccPutricideGrowingOozePuddleAction::IsPositionTooCloseToOtherPuddles(float x, float y, Unit* ignorePuddle)
{
    static const float BASE_RADIUS = 2.0f;
    static const float STACK_MULTIPLIER = 0.6f;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || unit == ignorePuddle || unit->GetEntry() != NPC_GROWING_OOZE_PUDDLE)
            continue;

        float safeDistance = BASE_RADIUS;
        if (Aura* grow = unit->GetAura(SPELL_GROW_AURA))
            safeDistance += (grow->GetStackAmount() * STACK_MULTIPLIER);

        float dist = sqrt(pow(x - unit->GetPositionX(), 2) + pow(y - unit->GetPositionY(), 2));
        if (dist < safeDistance)
            return true;
    }

    return false;
}

bool IccPutricideVolatileOozeAction::Execute(Event event)
{
    static const float STACK_DISTANCE = 7.0f;

    Unit* ooze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!ooze)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    // Main tank handling
    if (botAI->IsMainTank(bot) && bot->GetExactDist2d(ICC_PUTRICIDE_TANK_POSITION) > 20.0f &&
        !boss->HealthBelowPct(36) && boss->GetVictim() == bot)
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_POSITION.GetPositionX(), ICC_PUTRICIDE_TANK_POSITION.GetPositionY(),
               ICC_PUTRICIDE_TANK_POSITION.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);

    // Skip if we have forbidden auras
    if (botAI->HasAura("Gaseous Bloat", bot) || botAI->HasAura("Unbound Plague", bot))
        return false;

    // Find all alive oozes
    std::vector<Unit*> aliveOozes;
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == ooze->GetEntry())
            aliveOozes.push_back(unit);
    }

    // If more than one ooze is alive, kill all but one
    if (aliveOozes.size() > 1)
    {
        for (size_t i = 0; i < aliveOozes.size() - 1; ++i)
        {
            bot->Kill(bot, aliveOozes[i]);
        }
    }

    // Mark ooze with skull
    MarkOozeWithSkull(ooze);

    // Melee handling (non-tanks)
    if (botAI->IsMelee(bot) && !botAI->IsMainTank(bot))
    {
        bot->SetTarget(ooze->GetGUID());
        bot->SetFacingToObject(ooze);
        if (bot->IsWithinMeleeRange(ooze))
            return Attack(ooze);
    }

    // Ranged/healer handling
    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
        Unit* stackTarget = FindAuraTarget();
        if (stackTarget && bot->GetDistance2d(stackTarget) > STACK_DISTANCE)
        {
            return MoveTo(bot->GetMapId(), stackTarget->GetPositionX(), stackTarget->GetPositionY(),
                          stackTarget->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }

        if (ooze && !botAI->IsHeal(bot) && stackTarget && bot->GetDistance2d(stackTarget) <= STACK_DISTANCE)
        {
            bot->SetTarget(ooze->GetGUID());
            bot->SetFacingToObject(ooze);
            if (bot->IsWithinRange(ooze, 25.0f))
                return Attack(ooze);
        }
    }

    return false;
}

void IccPutricideVolatileOozeAction::MarkOozeWithSkull(Unit* ooze)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    constexpr uint8_t skullIconId = 7;
    ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);
    Unit* markedUnit = botAI->GetUnit(skullGuid);

    // Clear dead marks or marks that are not on ooze
    if (markedUnit && (!markedUnit->IsAlive() || (ooze && markedUnit != ooze)))
        group->SetTargetIcon(skullIconId, bot->GetGUID(), ObjectGuid::Empty);

    // Mark alive ooze if needed
    if (ooze && ooze->IsAlive() && (!skullGuid || !markedUnit))
        group->SetTargetIcon(skullIconId, bot->GetGUID(), ooze->GetGUID());
}

Unit* IccPutricideVolatileOozeAction::FindAuraTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (botAI->HasAura("Volatile Ooze Adhesive", member))
            return member;
    }

    return nullptr;
}


bool IccPutricideGasCloudAction::Execute(Event event)
{
    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!gasCloud)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    // Tank positioning logic
    if (botAI->IsTank(bot) && bot->GetExactDist2d(ICC_PUTRICIDE_TANK_POSITION) > 20.0f && !boss->HealthBelowPct(36) &&
        boss->GetVictim() == bot)
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_POSITION.GetPositionX(),
                      ICC_PUTRICIDE_TANK_POSITION.GetPositionY(), ICC_PUTRICIDE_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_COMBAT, true, false);

    if (botAI->IsMainTank(bot))
        return false;

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);
    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");

    // Find all alive gasCloud
    std::vector<Unit*> aliveGasCloud;
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == gasCloud->GetEntry())
            aliveGasCloud.push_back(unit);
    }

    // If more than one GasCloud is alive, kill all but one
    if (aliveGasCloud.size() > 1)
    {
        for (size_t i = 0; i < aliveGasCloud.size() - 1; ++i)
        {
            bot->Kill(bot, aliveGasCloud[i]);
        }
    }

    // Skip if we have no aura but ooze exists
    if (!hasGaseousBloat && volatileOoze)
        return false;

    if (hasGaseousBloat)
        return HandleGaseousBloatMovement(gasCloud);

    return HandleGroupAuraSituation(gasCloud);
}

bool IccPutricideGasCloudAction::HandleGaseousBloatMovement(Unit* gasCloud)
{
    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);

    if (!hasGaseousBloat)
        return false;

    if (hasGaseousBloat && !bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS,
                     bot);  // to make it a bit easier when abo fails to slow or bots take forever to kill oozes

    static const int NUM_ANGLES = 32;  // Increased from 16 for better corner escape
    static const float MIN_SAFE_DISTANCE = 35.0f;
    static const float EMERGENCY_DISTANCE = 15.0f;
    static const float GAS_BOMB_SAFE_DIST = 6.0f;
    static const float MOVEMENT_INCREMENT = 5.0f;  // Fixed movement increment

    Position botPos = bot->GetPosition();
    Position cloudPos = gasCloud->GetPosition();
    float cloudDist = gasCloud->GetExactDist2d(botPos);

    if (cloudDist >= MIN_SAFE_DISTANCE)
        return false;

    // Gather all choking gas bombs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> gasBombs;
    for (const auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_CHOKING_GAS_BOMB)
            gasBombs.push_back(unit);
    }

    // Calculate direction away from cloud
    float dx = botPos.GetPositionX() - cloudPos.GetPositionX();
    float dy = botPos.GetPositionY() - cloudPos.GetPositionY();
    float dist = std::max(0.1f, sqrt(dx * dx + dy * dy));

    if (dist <= 0)
        return false;

    dx /= dist;
    dy /= dist;

    // Try to find safe movement position with strict corner avoidance
    Position bestPos;
    bool foundPath = false;
    float bestScore = 0.0f;

    for (int i = 0; i < NUM_ANGLES; i++)
    {
        float angle = (2 * M_PI * i) / NUM_ANGLES;
        float rotatedDx = dx * cos(angle) - dy * sin(angle);
        float rotatedDy = dx * sin(angle) + dy * cos(angle);

        // Only test positions at fixed increments of 5.0f
        for (float testDist = MOVEMENT_INCREMENT; testDist <= 20.0f; testDist += MOVEMENT_INCREMENT)
        {
            float testX = botPos.GetPositionX() + rotatedDx * testDist;
            float testY = botPos.GetPositionY() + rotatedDy * testDist;
            float testZ = botPos.GetPositionZ();
            Position testPos(testX, testY, testZ);

            float newCloudDist = cloudPos.GetExactDist2d(testPos);

            // Check gas bomb distance
            float minGasBombDist = FLT_MAX;
            for (Unit* bomb : gasBombs)
            {
                float bombDist = sqrt(pow(testX - bomb->GetPositionX(), 2) + pow(testY - bomb->GetPositionY(), 2));
                if (bombDist < minGasBombDist)
                    minGasBombDist = bombDist;
            }

            if (newCloudDist > cloudDist && minGasBombDist >= GAS_BOMB_SAFE_DIST &&
                bot->IsWithinLOS(testX, testY, testZ))
            {
                // Strict corner avoidance - test movement freedom thoroughly
                int freeDirections = 0;
                static const int CHECK_DIRS = 16;      // More directions for better detection
                static const float CHECK_DIST = 8.0f;  // Longer distance to detect walls/corners early

                for (int j = 0; j < CHECK_DIRS; j++)
                {
                    float checkAngle = (2 * M_PI * j) / CHECK_DIRS;
                    float checkX = testX + cos(checkAngle) * CHECK_DIST;
                    float checkY = testY + sin(checkAngle) * CHECK_DIST;
                    if (bot->IsWithinLOS(checkX, checkY, testZ))
                        freeDirections++;
                }

                float freedomScore = (float)freeDirections / CHECK_DIRS;

                // STRICT: Reject any position that looks like a corner or restricted area
                // Require at least 80% of directions to be free (13 out of 16)
                if (freedomScore < 0.8f)
                    continue;  // Skip this position entirely

                // Also check for "dead ends" - positions that might lead to corners
                // Test if we can move further in the same direction
                bool canContinueMoving = true;
                float continueX = testX + rotatedDx * MOVEMENT_INCREMENT;
                float continueY = testY + rotatedDy * MOVEMENT_INCREMENT;
                if (!bot->IsWithinLOS(continueX, continueY, testZ))
                    canContinueMoving = false;

                // Bonus for positions that allow continued movement in the same direction
                float continuityBonus = canContinueMoving ? 5.0f : 0.0f;

                float combinedScore = newCloudDist + (freedomScore * 15.0f) + minGasBombDist + continuityBonus;

                if (!foundPath || combinedScore > bestScore)
                {
                    bestPos = testPos;
                    bestScore = combinedScore;
                    foundPath = true;
                }
            }
        }
    }

    if (foundPath)
    {
        botAI->Reset();
        return MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }
    // Emergency movement - only when very close to cloud
    if (cloudDist < EMERGENCY_DISTANCE)
    {
        Position emergencyPos = CalculateEmergencyPosition(botPos, dx, dy);

        // Even for emergency, avoid corners
        float minEmergencyGasBombDist = FLT_MAX;
        for (Unit* bomb : gasBombs)
        {
            float bombDist = sqrt(pow(emergencyPos.GetPositionX() - bomb->GetPositionX(), 2) +
                                  pow(emergencyPos.GetPositionY() - bomb->GetPositionY(), 2));
            if (bombDist < minEmergencyGasBombDist)
                minEmergencyGasBombDist = bombDist;
        }

        if (minEmergencyGasBombDist >= GAS_BOMB_SAFE_DIST &&
            bot->IsWithinLOS(emergencyPos.GetPositionX(), emergencyPos.GetPositionY(), emergencyPos.GetPositionZ()))
        {
            botAI->Reset();
            return MoveTo(bot->GetMapId(), emergencyPos.GetPositionX(), emergencyPos.GetPositionY(),
                          emergencyPos.GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool IccPutricideGasCloudAction::FindSafeMovementPosition(const Position& botPos, const Position& cloudPos, float dx,
                                                          float dy, int numAngles, Position& resultPos)
{
    float bestScore = 0.0f;
    bool foundPath = false;
    resultPos = botPos;
    static const float MOVEMENT_INCREMENT = 5.0f;  // Fixed movement increment

    for (int i = 0; i < numAngles; i++)
    {
        float angle = (2 * M_PI * i) / numAngles;
        float rotatedDx = dx * cos(angle) - dy * sin(angle);
        float rotatedDy = dx * sin(angle) + dy * cos(angle);

        for (float testDist = MOVEMENT_INCREMENT; testDist <= 20.0f; testDist += MOVEMENT_INCREMENT)
        {
            Position testPos(botPos.GetPositionX() + rotatedDx * testDist, botPos.GetPositionY() + rotatedDy * testDist,
                             botPos.GetPositionZ());

            float newCloudDist = cloudPos.GetExactDist2d(testPos);
            if (newCloudDist > cloudPos.GetExactDist2d(botPos) &&
                bot->IsWithinLOS(testPos.GetPositionX(), testPos.GetPositionY(), testPos.GetPositionZ()))
            {
                // Strict corner prevention - test movement freedom thoroughly
                int freeDirections = 0;
                static const int CHECK_DIRECTIONS = 16;
                static const float CHECK_DISTANCE = 8.0f;

                for (int j = 0; j < CHECK_DIRECTIONS; j++)
                {
                    float checkAngle = (2 * M_PI * j) / CHECK_DIRECTIONS;
                    float checkX = testPos.GetPositionX() + cos(checkAngle) * CHECK_DISTANCE;
                    float checkY = testPos.GetPositionY() + sin(checkAngle) * CHECK_DISTANCE;
                    if (bot->IsWithinLOS(checkX, checkY, testPos.GetPositionZ()))
                        freeDirections++;
                }

                float freedomScore = (float)freeDirections / CHECK_DIRECTIONS;

                // REJECT positions that could lead to corners - require high freedom
                if (freedomScore < 0.8f)  // 80% of directions must be free
                    continue;

                // Check if we can continue moving in the same direction (avoid dead ends)
                float continueX = testPos.GetPositionX() + rotatedDx * MOVEMENT_INCREMENT;
                float continueY = testPos.GetPositionY() + rotatedDy * MOVEMENT_INCREMENT;
                bool canContinue = bot->IsWithinLOS(continueX, continueY, testPos.GetPositionZ());

                float continuityBonus = canContinue ? 3.0f : 0.0f;
                float combinedScore = newCloudDist + (freedomScore * 12.0f) + continuityBonus;

                if (!foundPath || combinedScore > bestScore)
                {
                    resultPos = testPos;
                    bestScore = combinedScore;
                    foundPath = true;
                }
            }
        }
    }

    return foundPath;
}

Position IccPutricideGasCloudAction::CalculateEmergencyPosition(const Position& botPos, float dx, float dy)
{
    // For emergency, still try to avoid corners but prioritize getting away from immediate danger
    Position bestPos =
        Position(botPos.GetPositionX() + dx * 15.0f, botPos.GetPositionY() + dy * 15.0f, botPos.GetPositionZ());
    float bestFreedom = 0.0f;
    static const float MOVEMENT_INCREMENT = 5.0f;  // Fixed movement increment

    // Try fewer directions for emergency but still avoid corners
    for (int i = 0; i < 8; i++)
    {
        float angle = (2 * M_PI * i) / 8;
        float rotatedDx = dx * cos(angle) - dy * sin(angle);
        float rotatedDy = dx * sin(angle) + dy * cos(angle);

        Position testPos(botPos.GetPositionX() + rotatedDx * 15.0f, botPos.GetPositionY() + rotatedDy * 15.0f,
                         botPos.GetPositionZ());

        if (bot->IsWithinLOS(testPos.GetPositionX(), testPos.GetPositionY(), testPos.GetPositionZ()))
        {
            // Quick freedom check for emergency
            int freeDirections = 0;
            static const int EMERGENCY_CHECK_DIRS = 8;
            static const float EMERGENCY_CHECK_DIST = 6.0f;

            for (int j = 0; j < EMERGENCY_CHECK_DIRS; j++)
            {
                float checkAngle = (2 * M_PI * j) / EMERGENCY_CHECK_DIRS;
                float checkX = testPos.GetPositionX() + cos(checkAngle) * EMERGENCY_CHECK_DIST;
                float checkY = testPos.GetPositionY() + sin(checkAngle) * EMERGENCY_CHECK_DIST;
                if (bot->IsWithinLOS(checkX, checkY, testPos.GetPositionZ()))
                    freeDirections++;
            }

            float freedom = (float)freeDirections / EMERGENCY_CHECK_DIRS;

            // For emergency, accept positions with at least 60% freedom
            if (freedom >= 0.6f && freedom > bestFreedom)
            {
                bestPos = testPos;
                bestFreedom = freedom;
            }
        }
    }

    return bestPos;
}

bool IccPutricideGasCloudAction::HandleGroupAuraSituation(Unit* gasCloud)
{
    Group* group = bot->GetGroup();
    if (!group || botAI->IsHeal(bot))
        return false;

    // Mark gas cloud with skull if no volatile ooze is present or alive
    Unit* volatileOoze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if ((!volatileOoze || !volatileOoze->IsAlive()) && gasCloud && gasCloud->IsAlive())
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            constexpr uint8_t skullIconId = 7;
            ObjectGuid currentSkull = group->GetTargetIcon(skullIconId);
            Unit* markedUnit = botAI->GetUnit(currentSkull);
            if (!markedUnit || !markedUnit->IsAlive() || markedUnit != gasCloud)
                group->SetTargetIcon(skullIconId, bot->GetGUID(), gasCloud->GetGUID());
        }
    }

    float currentDist = gasCloud ? bot->GetDistance(gasCloud) : 0.0f;
    const float MIN_SAFE_DISTANCE = 15.0f;

    // Always maintain minimum distance when group doesn't have bloat
    if (!GroupHasGaseousBloat(group) && gasCloud && currentDist < MIN_SAFE_DISTANCE)
    {
        // Move away from gas cloud
        float angle = bot->GetAngle(gasCloud);
        float x = bot->GetPositionX() + cos(angle) * -MIN_SAFE_DISTANCE;
        float y = bot->GetPositionY() + sin(angle) * -MIN_SAFE_DISTANCE;
        return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    if (GroupHasGaseousBloat(group) && gasCloud)
    {
        bot->SetTarget(gasCloud->GetGUID());
        bot->SetFacingToObject(gasCloud);

        // Attack logic for group with Gaseous Bloat
        if (botAI->IsRanged(bot))
        {
            // For ranged attackers, maintain optimal distance (15-25 yards)
            if (currentDist > 25.0f)
            {
                // Move closer if too far
                return MoveTo(bot->GetMapId(), gasCloud->GetPositionX(), gasCloud->GetPositionY(),
                              gasCloud->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
            else if (currentDist < MIN_SAFE_DISTANCE)
            {
                // Move away if too close (but stay closer than normal safe distance since we need to attack)
                float angle = bot->GetAngle(gasCloud);
                float x = bot->GetPositionX() + cos(angle) * -12.0f;
                float y = bot->GetPositionY() + sin(angle) * -12.0f;
                return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT);
            }
            else
            {
                // Attack if at good range
                return Attack(gasCloud);
            }
        }
        else if (botAI->IsMelee(bot) && !botAI->IsTank(bot))
        {
            // For melee attackers, move to attack range (0-5 yards)
            if (currentDist > 5.0f)
            {
                return MoveTo(bot->GetMapId(), gasCloud->GetPositionX(), gasCloud->GetPositionY(),
                              gasCloud->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
            else
            {
                return Attack(gasCloud);
            }
        }
    }

    return false;
}

bool IccPutricideGasCloudAction::GroupHasGaseousBloat(Group* group)
{
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && botAI->HasAura("Gaseous Bloat", member))
            return true;
    }
    return false;
}

bool IccPutricideAvoidMalleableGooAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    // Tank handling for choking gas bomb
    if (HandleTankPositioning(boss))
        return true;

    // Skip if volatile ooze or gas cloud exists
    if (AI_VALUE2(Unit*, "find target", "volatile ooze") || AI_VALUE2(Unit*, "find target", "gas cloud"))
        return false;

    // Handle unbound plague movement
    if (HandleUnboundPlague(boss))
        return true;

    // Handle ranged/melee positioning
    return HandleBossPositioning(boss);
}

bool IccPutricideAvoidMalleableGooAction::HandleTankPositioning(Unit* boss)
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* bomb = bot->FindNearestCreature(NPC_CHOKING_GAS_BOMB, 100.0f);
    if (!bomb)
        return false;

    const float safeDistance = 15.0f;
    float currentDistance = bot->GetDistance2d(bomb);

    if (currentDistance < safeDistance)
        return MoveAway(bomb, safeDistance - currentDistance);

    return false;
}

bool IccPutricideAvoidMalleableGooAction::HandleUnboundPlague(Unit* boss)
{
    if (boss && boss->HealthBelowPct(35))
        return false;

    if (!botAI->HasAura("Unbound Plague", bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const float UNBOUND_PLAGUE_DISTANCE = 20.0f;
    float closestDistance = UNBOUND_PLAGUE_DISTANCE;
    Unit* closestPlayer = nullptr;

    // Find closest player
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float dist = bot->GetDistance2d(member);
        if (dist < closestDistance)
        {
            closestDistance = dist;
            closestPlayer = member;
        }
    }

    if (!closestPlayer || closestDistance >= UNBOUND_PLAGUE_DISTANCE)
    {
        bot->Kill(bot, bot);
        return true;
    }

    // Calculate move away position
    float dx = bot->GetPositionX() - closestPlayer->GetPositionX();
    float dy = bot->GetPositionY() - closestPlayer->GetPositionY();
    float dist = sqrt(dx * dx + dy * dy);

    if (dist <= 0)
        return false;

    dx /= dist;
    dy /= dist;
    float moveDistance = UNBOUND_PLAGUE_DISTANCE - closestDistance + 2.0f;

    float moveX = bot->GetPositionX() + dx * moveDistance;
    float moveY = bot->GetPositionY() + dy * moveDistance;

    if (bot->IsWithinLOS(moveX, moveY, bot->GetPositionZ()))
    {
        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}


bool IccPutricideAvoidMalleableGooAction::HandleBossPositioning(Unit* boss)
{
    if (botAI->IsTank(bot))
        return false;

    // If boss is close to putricide_bad_position, all non-tank bots should be 1f in front of boss
    const float BAD_POS_THRESHOLD = 10.0f;
    const float IN_FRONT_DISTANCE = 1.0f;
    float bossToBadPos = boss->GetExactDist2d(ICC_PUTRICIDE_BAD_POSITION.GetPositionX(), ICC_PUTRICIDE_BAD_POSITION.GetPositionY());

    if (bossToBadPos <= BAD_POS_THRESHOLD)
    {
        // Move to 1f in front of boss
        float bossOrientation = boss->GetOrientation();
        float targetX = boss->GetPositionX() + cos(bossOrientation) * IN_FRONT_DISTANCE;
        float targetY = boss->GetPositionY() + sin(bossOrientation) * IN_FRONT_DISTANCE;
        float targetZ = boss->GetPositionZ();

        // Only move if not already close enough
        if (bot->GetExactDist2d(targetX, targetY) > 0.5f)
        {
            bot->SetFacingToObject(boss);
            return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, botAI->IsRanged(bot),
                          MovementPriority::MOVEMENT_COMBAT);
        }
        return false;
    }

    float distToBoss = bot->GetExactDist2d(boss);
    bool isRanged = botAI->IsRanged(bot);

    // Calculate desired position in front of boss
    float desiredDistance =
        isRanged ? ((bot->getClass() == CLASS_HUNTER) ? 14.0f : 6.0f) : (distToBoss < 2.0f ? 3.0f : 5.0f);

    // Check if we need to move
    if ((std::abs(distToBoss - desiredDistance) > 0.5f || !boss->isInFront(bot)) &&
        (!isRanged || (isRanged && !botAI->IsTank(bot))))
    {
        Position targetPos = CalculateBossPosition(boss, desiredDistance);

        // Check for obstacles
        if (HasObstacleBetween(bot->GetPosition(), targetPos))
        {
            // Use arc movement to navigate around obstacles
            Position arcPoint = CalculateArcPoint(bot->GetPosition(), targetPos, boss->GetPosition());

            if (bot->GetExactDist2d(arcPoint) > 1.0f)
            {
                bot->SetFacingToObject(boss);
                return MoveTo(bot->GetMapId(), arcPoint.GetPositionX(), arcPoint.GetPositionY(),
                              arcPoint.GetPositionZ(), false, false, false, isRanged,
                              MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else
        {
            // No obstacles, move in increments directly toward target
            Position adjustedTarget = CalculateIncrementalMove(bot->GetPosition(), targetPos, 2.0f);
            bot->SetFacingToObject(boss);
            return MoveTo(bot->GetMapId(), adjustedTarget.GetPositionX(), adjustedTarget.GetPositionY(),
                          adjustedTarget.GetPositionZ(), false, false, false, isRanged,
                          MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

Position IccPutricideAvoidMalleableGooAction::CalculateBossPosition(Unit* boss, float distance)
{
    float bossOrientation = boss->GetOrientation();
    return Position(boss->GetPositionX() + cos(bossOrientation) * distance,
                    boss->GetPositionY() + sin(bossOrientation) * distance, boss->GetPositionZ());
}

bool IccPutricideAvoidMalleableGooAction::HasObstacleBetween(const Position& from, const Position& to)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_GROWING_OOZE_PUDDLE || unit->GetEntry() == NPC_CHOKING_GAS_BOMB)
        {
            if (IsOnPath(from, to, unit->GetPosition(), 3.0f))
                return true;
        }
    }
    return false;
}

bool IccPutricideAvoidMalleableGooAction::IsOnPath(const Position& from, const Position& to, const Position& point,
                                                   float threshold)
{
    float pathX = to.GetPositionX() - from.GetPositionX();
    float pathY = to.GetPositionY() - from.GetPositionY();
    float pathLen = std::sqrt(pathX * pathX + pathY * pathY);

    if (pathLen < 0.1f)
        return false;

    float normX = pathX / pathLen;
    float normY = pathY / pathLen;

    float toPointX = point.GetPositionX() - from.GetPositionX();
    float toPointY = point.GetPositionY() - from.GetPositionY();
    float proj = toPointX * normX + toPointY * normY;

    if (proj < 0 || proj > pathLen)
        return false;

    float closestX = from.GetPositionX() + normX * proj;
    float closestY = from.GetPositionY() + normY * proj;
    float distToPath = std::sqrt((point.GetPositionX() - closestX) * (point.GetPositionX() - closestX) +
                                 (point.GetPositionY() - closestY) * (point.GetPositionY() - closestY));

    return distToPath < threshold;
}

Position IccPutricideAvoidMalleableGooAction::CalculateArcPoint(const Position& current, const Position& target, const Position& center)
{
    // Calculate vectors from center to current position and target
    float currentX = current.GetPositionX() - center.GetPositionX();
    float currentY = current.GetPositionY() - center.GetPositionY();
    float targetX = target.GetPositionX() - center.GetPositionX();
    float targetY = target.GetPositionY() - center.GetPositionY();

    // Calculate distances
    float currentDist = std::sqrt(currentX * currentX + currentY * currentY);
    float targetDist = std::sqrt(targetX * targetX + targetY * targetY);

    // Normalize vectors
    currentX /= currentDist;
    currentY /= currentDist;
    targetX /= targetDist;
    targetY /= targetDist;

    // Calculate dot product to find the angle between vectors
    float dotProduct = currentX * targetX + currentY * targetY;
    dotProduct = std::max(-1.0f, std::min(1.0f, dotProduct));  // Clamp to [-1, 1]
    float angle = std::acos(dotProduct);

    // Determine rotation direction (clockwise or counterclockwise)
    float crossProduct = currentX * targetY - currentY * targetX;
    float stepAngle = angle * 0.25f;  // Move 25% along the arc

    if (crossProduct < 0)
        stepAngle = -stepAngle;  // Clockwise

    // Calculate rotation matrix components
    float cos_a = std::cos(stepAngle);
    float sin_a = std::sin(stepAngle);

    // Rotate current vector
    float rotatedX = currentX * cos_a - currentY * sin_a;
    float rotatedY = currentX * sin_a + currentY * cos_a;

    // Scale to match the target distance for smoother approach
    float desiredDist = currentDist * 0.9f + targetDist * 0.1f;

    // Calculate the new position
    return Position(center.GetPositionX() + rotatedX * desiredDist, center.GetPositionY() + rotatedY * desiredDist,
                    current.GetPositionZ());
}

Position IccPutricideAvoidMalleableGooAction::CalculateIncrementalMove(const Position& current, const Position& target,
                                                                       float maxDistance)
{
    float dx = target.GetPositionX() - current.GetPositionX();
    float dy = target.GetPositionY() - current.GetPositionY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= maxDistance)
        return target;

    dx /= distance;
    dy /= distance;

    return Position(current.GetPositionX() + dx * maxDistance, current.GetPositionY() + dy * maxDistance,
                    target.GetPositionZ());
}

// BPC
bool IccBpcKelesethTankAction::Execute(Event event)
{
    if (!botAI->IsAssistTank(bot))
        return false;

    // Handle boss positioning
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss)
        return false;

    bool isBossVictim = false;
    if (boss && boss->GetVictim() == bot)
        isBossVictim = true;

    // If not actively tanking, attack the boss
    if (boss->GetVictim() != bot)
    {
        bot->SetTarget(boss->GetGUID());
        bot->SetFacingToObject(boss);
        Attack(boss);
    }

    // If tanking boss, check for Dark Nucleus logic - collect any nucleus not targeting us
    if (boss->GetVictim() == bot)
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        for (const auto& targetGuid : targets)
        {
            Unit* nucleus = botAI->GetUnit(targetGuid);
            if (nucleus && nucleus->IsAlive() && nucleus->GetEntry() == NPC_DARK_NUCLEUS)
            {
                // Attack nucleus that are NOT targeting us (to collect them)
                if (nucleus->GetVictim() != bot)
                {
                    bot->SetTarget(nucleus->GetGUID());
                    bot->SetFacingToObject(nucleus);
                    Attack(nucleus);
                    // Return early to focus on nucleus collection first
                    return false;
                }
            }
        }
    }

    // Positioning logic - only execute if no nucleus needs collecting
    if (botAI->HasAura("Invocation of Blood", boss) && bot->GetExactDist2d(ICC_BPC_MT_POSITION) > 15.0f && isBossVictim)
    {
        // Calculate direction vector
        float dirX = ICC_BPC_MT_POSITION.GetPositionX() - bot->GetPositionX();
        float dirY = ICC_BPC_MT_POSITION.GetPositionY() - bot->GetPositionY();

        // Calculate distance and normalize
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.001f)
        {
            dirX /= length;
            dirY /= length;

            // Calculate movement distance (max 3 yards at a time)
            float moveDist = std::min(3.0f, length);

            // Calculate target position
            float moveX = bot->GetPositionX() + dirX * moveDist;
            float moveY = bot->GetPositionY() + dirY * moveDist;
            float moveZ = bot->GetPositionZ();

            MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Always attack boss when tanking (if no nucleus was prioritized)
    if (boss->GetVictim() == bot)
    {
        bot->SetTarget(boss->GetGUID());
        bot->SetFacingToObject(boss);
        Attack(boss);
    }

    return false;
}

bool IccBpcMainTankAction::Execute(Event event)
{
    // Main tank specific behavior (higher priority)
    if (botAI->IsMainTank(bot))
    {
        // Get target princes
        auto* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
        auto* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");

        // Check if we're the target of both princes
        bool isVictimOfValanar = false;
        if (valanar && valanar->GetVictim() == bot)
            isVictimOfValanar = true;

        bool isVictimOfTaldaram = false;
        if (taldaram && taldaram->GetVictim() == bot)
            isVictimOfTaldaram = true;

        // Move to MT position if targeted by both princes and not already close
        if (isVictimOfValanar && isVictimOfTaldaram && bot->GetExactDist2d(ICC_BPC_MT_POSITION) > 15.0f)
        {
            // Calculate direction vector
            float dirX = ICC_BPC_MT_POSITION.GetPositionX() - bot->GetPositionX();
            float dirY = ICC_BPC_MT_POSITION.GetPositionY() - bot->GetPositionY();

            // Calculate distance and normalize
            float length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length > 0.001f)
            {
                dirX /= length;
                dirY /= length;

                // Calculate movement distance (max 3 yards at a time)
                float moveDist = std::min(3.0f, length);

                // Calculate target position
                float moveX = bot->GetPositionX() + dirX * moveDist;
                float moveY = bot->GetPositionY() + dirY * moveDist;
                float moveZ = bot->GetPositionZ();

                MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                       MovementPriority::MOVEMENT_COMBAT);
            }
        }

        // Attack prince that's not targeting us
        if (valanar && !isVictimOfValanar)
        {
            bot->SetTarget(valanar->GetGUID());
            bot->SetFacingToObject(valanar);
            Attack(valanar);
        }

        if (taldaram && !isVictimOfTaldaram)
        {
            bot->SetTarget(taldaram->GetGUID());
            bot->SetFacingToObject(taldaram);
            Attack(taldaram);
        }
    }

    // Target marking for all tanks, called after main tank priority actions
    if (botAI->IsTank(bot))
        MarkEmpoweredPrince();

    return false;
}

void IccBpcMainTankAction::MarkEmpoweredPrince()
{
    static constexpr uint8_t SKULL_RAID_ICON = 7;

    // Find empowered prince (Invocation of Blood)
    Unit* empoweredPrince = nullptr;
    const GuidVector& targets = AI_VALUE(GuidVector, "possible targets");

    for (const auto& targetGuid : targets)
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (!unit || !unit->IsAlive())
            continue;

        if (botAI->HasAura("Invocation of Blood", unit))
        {
            const uint32 entry = unit->GetEntry();
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
            const ObjectGuid currentSkullGuid = group->GetTargetIcon(SKULL_RAID_ICON);
            Unit* markedUnit = botAI->GetUnit(currentSkullGuid);

            // Clear dead marks or marks that are not on empowered prince
            if (markedUnit && (!markedUnit->IsAlive() || markedUnit != empoweredPrince))
            {
                group->SetTargetIcon(SKULL_RAID_ICON, bot->GetGUID(), ObjectGuid::Empty);
            }

            // Mark alive empowered prince if needed
            if (!currentSkullGuid || !markedUnit)
            {
                group->SetTargetIcon(SKULL_RAID_ICON, bot->GetGUID(), empoweredPrince->GetGUID());
            }
        }
    }
}

bool IccBpcEmpoweredVortexAction::Execute(Event event)
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    // Check if boss is casting empowered vortex
    bool isCastingVortex = false;
    if (valanar && valanar->HasUnitState(UNIT_STATE_CASTING) &&
       (valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX1) ||
        valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX2) ||
        valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX3) ||
        valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX4)))
        isCastingVortex = true;

    if (isCastingVortex)
    {
        // Use complex positioning system for empowered vortex
        return HandleEmpoweredVortexSpread();
    }
    else
    {
        // Use simple ranged spreading for non-vortex situations
        return MaintainRangedSpacing();
    }
}

bool IccBpcEmpoweredVortexAction::MaintainRangedSpacing()
{
    const float safeSpacingRadius = 7.0f;
    const float moveIncrement = 2.0f;
    const float maxMoveDistance = 5.0f;
    const bool isRanged = botAI->IsRanged(bot) || botAI->IsHeal(bot);

    if (!isRanged)
        return false;

    // Get group members
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    // Calculate a combined vector representing all nearby members' positions
    float totalX = 0.0f;
    float totalY = 0.0f;
    int nearbyCount = 0;

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }

        const float distance = bot->GetExactDist2d(member);
        if (distance < safeSpacingRadius)
        {
            // Calculate vector from member to bot
            float dx = bot->GetPositionX() - member->GetPositionX();
            float dy = bot->GetPositionY() - member->GetPositionY();

            // Weight by inverse distance (closer members have more influence)
            float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
            totalX += dx * weight;
            totalY += dy * weight;
            nearbyCount++;
        }
    }

    // If we have nearby members, move away in the combined direction
    if (nearbyCount > 0)
    {
        // Normalize the combined vector
        float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
        if (magnitude > 0.001f)  // Avoid division by zero
        {
            totalX /= magnitude;
            totalY /= magnitude;

            // Calculate move distance
            float moveDistance = std::min(moveIncrement, maxMoveDistance);

            // Create target position in the combined direction
            float targetX = bot->GetPositionX() + totalX * moveDistance;
            float targetY = bot->GetPositionY() + totalY * moveDistance;
            float targetZ = bot->GetPositionZ();

            // Check if the target position is valid and move there
            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                       MovementPriority::MOVEMENT_NORMAL);
            }
            else
            {
                // If LOS check fails, try shorter distance
                targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                       MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;  // Everyone is properly spaced
}

bool IccBpcEmpoweredVortexAction::HandleEmpoweredVortexSpread()
{
    const float safeSpacingRadius = 13.0f;
    const float moveIncrement = 2.0f;
    const float maxMoveDistance = 5.0f;
    const bool isTank = botAI->IsTank(bot);

    if (isTank)
        return false;

    // Get group members
    const GuidVector members = AI_VALUE(GuidVector, "group members");

    // Calculate a combined vector representing all nearby members' positions
    float totalX = 0.0f;
    float totalY = 0.0f;
    int nearbyCount = 0;

    for (const auto& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }

        const float distance = bot->GetExactDist2d(member);
        if (distance < safeSpacingRadius)
        {
            // Calculate vector from member to bot
            float dx = bot->GetPositionX() - member->GetPositionX();
            float dy = bot->GetPositionY() - member->GetPositionY();

            // Weight by inverse distance (closer members have more influence)
            float weight = (safeSpacingRadius - distance) / safeSpacingRadius;
            totalX += dx * weight;
            totalY += dy * weight;
            nearbyCount++;
        }
    }

    // If we have nearby members, move away in the combined direction
    if (nearbyCount > 0)
    {
        // Normalize the combined vector
        float magnitude = std::sqrt(totalX * totalX + totalY * totalY);
        if (magnitude > 0.001f)  // Avoid division by zero
        {
            totalX /= magnitude;
            totalY /= magnitude;

            // Calculate move distance
            float moveDistance = std::min(moveIncrement, maxMoveDistance);

            // Create target position in the combined direction
            float targetX = bot->GetPositionX() + totalX * moveDistance;
            float targetY = bot->GetPositionY() + totalY * moveDistance;
            float targetZ = bot->GetPositionZ();

            // Check if the target position is valid and move there
            if (bot->IsWithinLOS(targetX, targetY, targetZ))
            {
                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                       MovementPriority::MOVEMENT_NORMAL);
            }
            else
            {
                // If LOS check fails, try shorter distance
                targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                       MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }

    return false;  // Everyone is properly spaced
}

bool IccBpcKineticBombAction::Execute(Event event)
{
    // Early exit if not ranged DPS
    if (!botAI->IsRangedDps(bot))
        return false;

    // Static constants
    static constexpr float MAX_HEIGHT_DIFF = 20.0f;
    static constexpr float SAFE_HEIGHT = 371.16473f;
    static constexpr float TELEPORT_HEIGHT = 366.16473f;
    static constexpr std::array<uint32_t, 4> KINETIC_BOMB_ENTRIES = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2,
                                                                     NPC_KINETIC_BOMB3, NPC_KINETIC_BOMB4};

    // Handle edge case where bot is too high
    if (bot->GetPositionZ() > SAFE_HEIGHT)
    {
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), TELEPORT_HEIGHT,
                        bot->GetOrientation());
    }

    // Check current target if valid
    if (Unit* currentTarget = AI_VALUE(Unit*, "current target"))
    {
        if (currentTarget->IsAlive() && std::find(KINETIC_BOMB_ENTRIES.begin(), KINETIC_BOMB_ENTRIES.end(),
                                                  currentTarget->GetEntry()) != KINETIC_BOMB_ENTRIES.end())
        {
            const float heightDiff = currentTarget->GetPositionZ() - 361.18222f;
            if (heightDiff < MAX_HEIGHT_DIFF)
                return false;  // Continue current attack
        }
    }

    // Find the best kinetic bomb to attack
    if (Unit* bestBomb = FindOptimalKineticBomb())
    {
        bot->SetTarget(bestBomb->GetGUID());
        bot->SetFacingToObject(bestBomb);
        Attack(bestBomb);
    }

    return false;
}

Unit* IccBpcKineticBombAction::FindOptimalKineticBomb()
{
    static constexpr float MAX_HEIGHT_DIFF = 20.0f;
    static constexpr std::array<uint32_t, 4> KINETIC_BOMB_ENTRIES = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2,
                                                                     NPC_KINETIC_BOMB3, NPC_KINETIC_BOMB4};

    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    if (targets.empty())
        return nullptr;

    const float botZ = 361.18222f;
    Group* group = bot->GetGroup();

    // Gather all valid kinetic bombs
    std::vector<Unit*> kineticBombs;
    for (const auto& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        if (std::find(KINETIC_BOMB_ENTRIES.begin(), KINETIC_BOMB_ENTRIES.end(), unit->GetEntry()) ==
            KINETIC_BOMB_ENTRIES.end())
            continue;
        kineticBombs.push_back(unit);
    }

    if (kineticBombs.empty())
        return nullptr;

    // Sort bombs by Z ascending (lowest first), then by heightDiff ascending (closest to ground)
    std::sort(kineticBombs.begin(), kineticBombs.end(),
              [botZ](Unit* a, Unit* b)
              {
                  if (a->GetPositionZ() != b->GetPositionZ())
                      return a->GetPositionZ() < b->GetPositionZ();
                  return std::abs(a->GetPositionZ() - botZ) < std::abs(b->GetPositionZ() - botZ);
              });

    // Assign each ranged DPS to a unique bomb (lowest Z first)
    std::vector<Player*> rangedDps;
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->IsAlive() && botAI->IsRangedDps(member))
                rangedDps.push_back(member);
        }
        // Sort by GUID for deterministic assignment
        std::sort(rangedDps.begin(), rangedDps.end(), [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });
    }
    else
    {
        rangedDps.push_back(bot);
    }

    // Find this bot's index among ranged DPS
    auto it = std::find(rangedDps.begin(), rangedDps.end(), bot);
    if (it == rangedDps.end())
        return nullptr;
    size_t botIndex = std::distance(rangedDps.begin(), it);

    // Assign bombs in order, skip bombs already handled by other ranged DPS
    size_t bombCount = kineticBombs.size();
    for (size_t i = 0, assigned = 0; i < bombCount; ++i)
    {
        Unit* bomb = kineticBombs[i];
        // Check if bomb is already handled by another ranged DPS closer than this bot
        if (IsBombAlreadyHandled(bomb, group))
            continue;
        if (assigned == botIndex)
            return bomb;
        ++assigned;
    }

    // Fallback: pick the lowest bomb not already handled
    for (Unit* bomb : kineticBombs)
    {
        if (!IsBombAlreadyHandled(bomb, group))
            return bomb;
    }

    return nullptr;
}

bool IccBpcKineticBombAction::IsBombAlreadyHandled(Unit* bomb, Group* group)
{
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive() || !botAI->IsRangedDps(member))
            continue;

        if (member->GetTarget() == bomb->GetGUID() && member->GetDistance(bomb) < bot->GetDistance(bomb))
            return true;
    }

    return false;
}


bool IccBpcBallOfFlameAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince taldaram");
    if (!boss)
        return false;

    Unit* flame1 = bot->FindNearestCreature(NPC_BALL_OF_FLAME, 100.0f);
    Unit* flame2 = bot->FindNearestCreature(NPC_BALL_OF_INFERNO_FLAME, 100.0f);

    bool ballOfFlame = flame1 && (flame1->GetVictim() == bot);
    bool infernoFlame = flame2 && (flame2->GetVictim() == bot);

    if (flame2 && (flame2->GetDistance2d(boss) > 2.0f) && !(flame2->GetDistance2d(boss) > 10.0f) && !infernoFlame &&
        bot->getClass() != CLASS_HUNTER)
        {
            if (!botAI->IsTank(bot) && !(flame2->GetVictim() == bot))
            {
                float targetX = flame2->GetPositionX();
                float targetY = flame2->GetPositionY();
                float currentX = bot->GetPositionX();
                float currentY = bot->GetPositionY();

                // Calculate direction vector
                float dx = targetX - currentX;
                float dy = targetY - currentY;
                float distance = sqrt(dx * dx + dy * dy);

                // Normalize and scale to 5 units (or remaining distance if less than 5)
                float step = std::min(5.0f, distance);
                if (distance > 0.1)
                {
                    dx = dx / distance * step;
                    dy = dy / distance * step;
                }

                // Calculate intermediate position
                float newX = currentX + dx;
                float newY = currentY + dy;

                MoveTo(flame2->GetMapId(), newX, newY, bot->GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT);
            }
        }

    // If victim of ball of flame, keep at least 15f from other party members
    if (ballOfFlame || infernoFlame)
    {
        const float SAFE_DIST = 15.0f;
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (const auto& memberGuid : members)
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
                if (len < 0.01f)
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

// Blood Queen Lana'thel
bool IccBqlGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Aura* frenzyAura = botAI->GetAura("Frenzied Bloodthirst", bot);
    Aura* shadowAura = botAI->GetAura("Swarming Shadows", bot);
    bool isTank = botAI->IsTank(bot);
    // Handle tank positioning
    if (isTank && HandleTankPosition(boss, frenzyAura, shadowAura))
        return true;

    // Handle swarming shadows movement
    if (shadowAura && HandleShadowsMovement())
        return true;

    // Handle group positioning
    if (!frenzyAura && !shadowAura && HandleGroupPosition(boss, frenzyAura, shadowAura))
        return true;

    return false;
}

bool IccBqlGroupPositionAction::HandleTankPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura)
{
    if (frenzyAura || shadowAura)
        return false;

    // Main tank positioning
    if (botAI->IsMainTank(bot) && botAI->HasAggro(boss))
    {
        if (bot->GetExactDist2d(ICC_BQL_TANK_POSITION) > 3.0f)
        {
            MoveTo(bot->GetMapId(), ICC_BQL_TANK_POSITION.GetPositionX(), ICC_BQL_TANK_POSITION.GetPositionY(),
                   ICC_BQL_TANK_POSITION.GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Assist tank positioning
    if (botAI->IsAssistTank(bot) && !botAI->GetAura("Blood Mirror", bot))
    {
        if (Unit* mainTank = AI_VALUE(Unit*, "main tank"))
        {
            MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(), mainTank->GetPositionZ(),
                   false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}

bool IccBqlGroupPositionAction::HandleShadowsMovement()
{
    const float SAFE_SHADOW_DIST = 4.0f;
    const float ARC_STEP = 0.05f;
    const float CURVE_SPACING = 15.0f;
    const int MAX_CURVES = 3;
    const float maxClosestDist = botAI->IsMelee(bot) ? 25.0f : 20.0f;
    const Position& center = ICC_BQL_CENTER_POSITION;
    const float OUTER_CURVE_PREFERENCE = 200.0f;   // Strong preference for outer curves
    const float CURVE_SWITCH_PENALTY = 50.0f;      // Penalty for switching curves
    const float DISTANCE_PENALTY_FACTOR = 100.0f;  // Penalty per yard moved from current position
    const float MAX_CURVE_JUMP_DIST = 5.0f;        // Maximum distance for jumping between curves

    // Track current curve to avoid unnecessary switching
    static std::map<ObjectGuid, int> botCurrentCurve;
    int currentCurve = botCurrentCurve.count(bot->GetGUID()) ? botCurrentCurve[bot->GetGUID()] : 0;

    // Find closest wall path
    Position lwall[4] = {ICC_BQL_LWALL1_POSITION, AdjustControlPoint(ICC_BQL_LWALL2_POSITION, center, 1.30f),
                         AdjustControlPoint(ICC_BQL_LWALL3_POSITION, center, 1.30f), ICC_BQL_LRWALL4_POSITION};
    Position rwall[4] = {ICC_BQL_RWALL1_POSITION, AdjustControlPoint(ICC_BQL_RWALL2_POSITION, center, 1.30f),
                         AdjustControlPoint(ICC_BQL_RWALL3_POSITION, center, 1.30f), ICC_BQL_LRWALL4_POSITION};
    Position* basePath = (bot->GetExactDist2d(lwall[0]) < bot->GetExactDist2d(rwall[0])) ? lwall : rwall;

    // Find all swarming shadows
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* shadows[100]{};  // Reasonable max estimate
    int shadowCount = 0;
    for (int i = 0; i < npcs.size() && shadowCount < 100; i++)
    {
        Unit* unit = botAI->GetUnit(npcs[i]);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SWARMING_SHADOWS)
            shadows[shadowCount++] = unit;
    }

    // Helper lambda to check if a position is inside a shadow
    auto IsPositionInShadow = [&](const Position& pos) -> bool
    {
        for (int i = 0; i < shadowCount; ++i)
        {
            if (pos.GetExactDist2d(shadows[i]) < SAFE_SHADOW_DIST)
                return true;
        }
        return false;
    };

    // If bot is at the 4th position (end of the wall), move towards 3rd position or center to avoid getting stuck
    float distToL4 = bot->GetExactDist2d(lwall[3]);
    float distToR4 = bot->GetExactDist2d(rwall[3]);
    const float STUCK_DIST = 2.0f;  // within 2 yards is considered stuck at the end

    if (distToL4 < STUCK_DIST || distToR4 < STUCK_DIST)
    {
        // Move towards 3rd position of the same wall, or towards center if blocked
        Position target;
        if (distToL4 < distToR4)
        {
            target = lwall[2];
        }
        else
        {
            target = rwall[2];
        }

        float tx = target.GetPositionX();
        float ty = target.GetPositionY();
        float tz = target.GetPositionZ();
        bot->UpdateAllowedPositionZ(tx, ty, tz);
        if (!bot->IsWithinLOS(tx, ty, tz) || IsPositionInShadow(Position(tx, ty, tz)))
        {
            tx = center.GetPositionX();
            ty = center.GetPositionY();
            tz = center.GetPositionZ();
        }

        if (bot->GetExactDist2d(tx, ty) > 1.0f)
        {
            MoveTo(bot->GetMapId(), tx, ty, tz, false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                   true, false);
        }
        return false;
    }

    CurveInfo bestCurve;
    bestCurve.foundSafe = false;
    bestCurve.score = FLT_MAX;
    bool foundCurve = false;

    // Keep track of information about all curves for possible fallback
    CurveInfo curveInfos[MAX_CURVES];
    for (int i = 0; i < MAX_CURVES; i++)
    {
        curveInfos[i].foundSafe = false;
        curveInfos[i].score = FLT_MAX;
    }

    // Evaluate all curves starting from outermost (lowest index)
    for (int curveIdx = 0; curveIdx < MAX_CURVES; curveIdx++)
    {
        float curveShrink = float(curveIdx) * CURVE_SPACING;
        float shrinkFactor = 1.30f - (curveShrink / 30.0f);
        if (shrinkFactor < 1.0f)
            shrinkFactor = 1.0f;

        Position path[4] = {basePath[0], AdjustControlPoint(basePath[1], center, shrinkFactor / 1.30f),
                            AdjustControlPoint(basePath[2], center, shrinkFactor / 1.30f), basePath[3]};

        // Find closest point on curve
        float minDist = 9999.0f;
        float t_closest = 0.0f;
        Position closestPoint = path[0];

        for (float t = 0.0f; t <= 1.0f; t += ARC_STEP)
        {
            Position pt = CalculateBezierPoint(t, path);
            float dist = bot->GetExactDist2d(pt);
            if (dist < minDist)
            {
                minDist = dist;
                t_closest = t;
                closestPoint = pt;
            }
        }

        // Check if the closest point is safe
        bool closestIsSafe = !IsPositionInShadow(closestPoint);

        // Find closest safe point by searching in both directions from closest point
        Position safeMoveTarget = closestPoint;
        float safeMoveTargetDist = FLT_MAX;
        bool foundSafe = closestIsSafe;

        // Only search for safe spots if the closest point isn't already safe
        if (!closestIsSafe)
        {
            // Find the nearest safe point along the curve, not by direct distance
            // but by distance along the curve from the closest point

            // Search forward on curve from closest point
            float forwardT = -1.0f;
            Position forwardPt;
            for (float t = t_closest + ARC_STEP; t <= 1.0f; t += ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                if (!IsPositionInShadow(pt))
                {
                    forwardT = t;
                    forwardPt = pt;
                    break;
                }
            }

            // Search backward on curve from closest point
            float backwardT = -1.0f;
            Position backwardPt;
            for (float t = t_closest - ARC_STEP; t >= 0.0f; t -= ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                if (!IsPositionInShadow(pt))
                {
                    backwardT = t;
                    backwardPt = pt;
                    break;
                }
            }

            // Choose the closest safe point based on curve distance, not direct distance
            if (forwardT >= 0 && backwardT >= 0)
            {
                // Both directions have safe points, choose the closer one by curve distance
                if (std::abs(forwardT - t_closest) < std::abs(backwardT - t_closest))
                {
                    safeMoveTarget = forwardPt;
                    foundSafe = true;
                }
                else
                {
                    safeMoveTarget = backwardPt;
                    foundSafe = true;
                }
            }
            else if (forwardT >= 0)
            {
                safeMoveTarget = forwardPt;
                foundSafe = true;
            }
            else if (backwardT >= 0)
            {
                safeMoveTarget = backwardPt;
                foundSafe = true;
            }
        }

        // Score this curve
        float distancePenalty = 0.0f;
        float score = 0.0f;

        if (foundSafe)
        {
            // If we found a safe point, penalize based on travel distance along the curve to reach it
            float stepsToCurve = minDist / 2.0f;  // Approximate steps to reach the curve
            float safeDist = bot->GetExactDist2d(safeMoveTarget);

            // Add distance penalty based on how far we need to move along the curve
            distancePenalty = safeDist * (1.0f / DISTANCE_PENALTY_FACTOR);
            score = safeDist + distancePenalty;
        }
        else
        {
            // No safe point found, assign a high score
            distancePenalty = minDist * (1.0f / DISTANCE_PENALTY_FACTOR);
            score = minDist + distancePenalty + 1000.0f;  // Penalty for unsafe position
        }

        // Apply strong penalty for curves that are too far
        if (minDist > maxClosestDist)
            score += 500.0f;

        // Apply penalty for unsafe curves
        if (!foundSafe)
            score += 1000.0f;

        // Apply curve index preference (strongly prefer outer curves)
        score += curveIdx * OUTER_CURVE_PREFERENCE;

        // Apply curve switching penalty
        if (curveIdx != currentCurve && currentCurve != 0)
            score += CURVE_SWITCH_PENALTY;

        // MORE IMPORTANT: Apply additional curve switching penalty if the bot is far away
        // from the target curve (prevent jumping between curves when far away)
        if (curveIdx != currentCurve && minDist > MAX_CURVE_JUMP_DIST)
            score += 2000.0f;  // Strong penalty to prevent jumping between curves

        // Store this curve's info
        curveInfos[curveIdx].moveTarget = foundSafe ? safeMoveTarget : closestPoint;
        curveInfos[curveIdx].foundSafe = foundSafe;
        curveInfos[curveIdx].minDist = minDist;
        curveInfos[curveIdx].curveIdx = curveIdx;
        curveInfos[curveIdx].score = score;
        curveInfos[curveIdx].closestPoint = closestPoint;
        curveInfos[curveIdx].t_closest = t_closest;

        // Only update if this curve is better than our current best
        if (!foundCurve || score < bestCurve.score)
        {
            bestCurve = curveInfos[curveIdx];
            foundCurve = true;
        }
    }

    // Fallback: If we're trying to switch to a far curve and we're not near any curve,
    // find and use the closest curve instead of making a direct beeline
    if (foundCurve && bestCurve.minDist > MAX_CURVE_JUMP_DIST && bestCurve.curveIdx != currentCurve)
    {
        // Look for the closest curve first
        float closestDist = FLT_MAX;
        int closestCurveIdx = -1;

        for (int i = 0; i < MAX_CURVES; i++)
        {
            if (curveInfos[i].minDist < closestDist)
            {
                closestDist = curveInfos[i].minDist;
                closestCurveIdx = i;
            }
        }

        // If we found a closer curve, use that instead
        if (closestCurveIdx >= 0 && closestCurveIdx != bestCurve.curveIdx)
        {
            bestCurve = curveInfos[closestCurveIdx];
        }
    }

    // Remember the selected curve for next time
    if (foundCurve)
    {
        botCurrentCurve[bot->GetGUID()] = bestCurve.curveIdx;
    }

    // Create a move plan to guide the bot along the curve if necessary
    if (foundCurve && bot->GetExactDist2d(bestCurve.moveTarget) > 1.0f)
    {
        // Final check: ensure we're not moving into a shadow
        if (!IsPositionInShadow(bestCurve.moveTarget))
        {
            // Get the curve
            float curveShrink = float(bestCurve.curveIdx) * CURVE_SPACING;
            float shrinkFactor = 1.30f - (curveShrink / 30.0f);
            if (shrinkFactor < 1.0f)
                shrinkFactor = 1.0f;

            Position path[4] = {basePath[0], AdjustControlPoint(basePath[1], center, shrinkFactor / 1.30f),
                                AdjustControlPoint(basePath[2], center, shrinkFactor / 1.30f), basePath[3]};

            // CRITICAL CHANGE: First check if we need to move to the curve
            float distToClosestPoint = bot->GetExactDist2d(bestCurve.closestPoint);

            // If we're not on the curve yet, first move to the closest point on the curve
            if (distToClosestPoint > 2.0f)
            {
                botAI->Reset();
                return MoveTo(bot->GetMapId(), bestCurve.closestPoint.GetPositionX(),
                              bestCurve.closestPoint.GetPositionY(), bestCurve.closestPoint.GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
            }

            // Now we know we're on or very close to the curve, so we'll follow it properly

            // Find target point on curve (t_target parameter)
            float t_target = 0.0f;
            float targetMinDist = 9999.0f;

            for (float t = 0.0f; t <= 1.0f; t += ARC_STEP)
            {
                Position pt = CalculateBezierPoint(t, path);
                float dist = bestCurve.moveTarget.GetExactDist2d(pt);
                if (dist < targetMinDist)
                {
                    targetMinDist = dist;
                    t_target = t;
                }
            }

            // Find an intermediate point along the curve between closest and target
            float t_step = (t_target > bestCurve.t_closest) ? ARC_STEP : -ARC_STEP;
            float t_intermediate = bestCurve.t_closest + t_step;
            Position intermediateTarget;
            bool foundValidIntermediate = false;

            // Limit the distance we move along the curve in one step
            const float MAX_CURVE_MOVEMENT = 7.0f;  // Max yards to move along curve
            float curveDistanceMoved = 0.0f;
            Position lastPos = bestCurve.closestPoint;

            while ((t_step > 0 && t_intermediate <= t_target) || (t_step < 0 && t_intermediate >= t_target))
            {
                Position pt = CalculateBezierPoint(t_intermediate, path);

                // Check if this point is safe
                if (!IsPositionInShadow(pt))
                {
                    // Calculate distance moved along curve so far
                    curveDistanceMoved += lastPos.GetExactDist2d(pt);
                    lastPos = pt;

                    // If we've moved the maximum allowed distance, use this position
                    if (curveDistanceMoved >= MAX_CURVE_MOVEMENT)
                    {
                        intermediateTarget = pt;
                        foundValidIntermediate = true;
                        break;
                    }

                    // Otherwise, continue moving along the curve
                    intermediateTarget = pt;
                    foundValidIntermediate = true;
                }
                else
                {
                    // We've hit a shadow, stop here
                    break;
                }

                t_intermediate += t_step;
            }

            // If we found a valid intermediate point, use it
            if (foundValidIntermediate)
            {
                botAI->Reset();
                MoveTo(bot->GetMapId(), intermediateTarget.GetPositionX(), intermediateTarget.GetPositionY(),
                       intermediateTarget.GetPositionZ(), false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
            }

            botAI->Reset();
            // Fallback to direct movement to the target point on the curve
            MoveTo(bot->GetMapId(), bestCurve.moveTarget.GetPositionX(), bestCurve.moveTarget.GetPositionY(),
                   bestCurve.moveTarget.GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

Position IccBqlGroupPositionAction::AdjustControlPoint(const Position& wall, const Position& center, float factor)
{
    float dx = wall.GetPositionX() - center.GetPositionX();
    float dy = wall.GetPositionY() - center.GetPositionY();
    float dz = wall.GetPositionZ() - center.GetPositionZ();
    return Position(center.GetPositionX() + dx * factor, center.GetPositionY() + dy * factor,
                    center.GetPositionZ() + dz * factor);
}

Position IccBqlGroupPositionAction::CalculateBezierPoint(float t, const Position path[4])
{
    float omt = 1 - t;
    float omt2 = omt * omt;
    float omt3 = omt2 * omt;
    float t2 = t * t;
    float t3 = t2 * t;

    float x = omt3 * path[0].GetPositionX() + 3 * omt2 * t * path[1].GetPositionX() +
              3 * omt * t2 * path[2].GetPositionX() + t3 * path[3].GetPositionX();

    float y = omt3 * path[0].GetPositionY() + 3 * omt2 * t * path[1].GetPositionY() +
              3 * omt * t2 * path[2].GetPositionY() + t3 * path[3].GetPositionY();

    float z = omt3 * path[0].GetPositionZ() + 3 * omt2 * t * path[1].GetPositionZ() +
              3 * omt * t2 * path[2].GetPositionZ() + t3 * path[3].GetPositionZ();

    return Position(x, y, z);
}

bool IccBqlGroupPositionAction::HandleGroupPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura)
{
    if (frenzyAura || shadowAura)
        return false;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    bool isRanged = botAI->IsRanged(bot);
    bool isMelee = botAI->IsMelee(bot);

    if (isRanged && bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) > 35.0f)
        MoveTo(boss, 5.0f, MovementPriority::MOVEMENT_FORCED);

    if ((boss->GetExactDist2d(ICC_BQL_TANK_POSITION.GetPositionX(), ICC_BQL_TANK_POSITION.GetPositionY()) > 10.0f) &&
        isRanged && !((boss->GetPositionZ() - ICC_BQL_CENTER_POSITION.GetPositionZ()) > 5.0f) &&
        (bot->GetExactDist2d(ICC_BQL_CENTER_POSITION.GetPositionX(), ICC_BQL_CENTER_POSITION.GetPositionY()) > 10.0f))
        MoveTo(bot->GetMapId(), ICC_BQL_CENTER_POSITION.GetPositionX(), ICC_BQL_CENTER_POSITION.GetPositionY(),
               ICC_BQL_CENTER_POSITION.GetPositionZ(), false, false, false, true,
               MovementPriority::MOVEMENT_COMBAT, true, false);

    // --- Ranged bots wall assignment logic ---
    if (isRanged)
    {
        // Gather all ranged and healers, sort by GUID for deterministic assignment
        std::vector<Player*> rangedBots;
        std::vector<Player*> healers;
        for (const auto& guid : members)
        {
            Unit* member = botAI->GetUnit(guid);
            if (!member || !member->IsAlive())
                continue;
            Player* player = member->ToPlayer();
            if (!player)
                continue;
            if (botAI->IsRanged(player))
                rangedBots.push_back(player);
            if (botAI->IsHeal(player))
                healers.push_back(player);
        }
        // Remove duplicates (healer can be ranged)
        std::sort(rangedBots.begin(), rangedBots.end(),
                  [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });
        std::sort(healers.begin(), healers.end(), [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

        // Assign at least one healer to each side, then balance the rest
        std::vector<Player*> leftSide, rightSide;
        Position leftPos = ICC_BQL_LWALL2_POSITION;
        Position rightPos = ICC_BQL_RWALL2_POSITION;

        // Assign healers first
        if (!healers.empty())
        {
            leftSide.push_back(healers[0]);
            if (healers.size() > 1)
                rightSide.push_back(healers[1]);
        }
        // If only one healer, assign to left, right will be filled by ranged DPS

        // Remove assigned healers from rangedBots
        for (Player* h : leftSide)
            rangedBots.erase(std::remove(rangedBots.begin(), rangedBots.end(), h), rangedBots.end());
        for (Player* h : rightSide)
            rangedBots.erase(std::remove(rangedBots.begin(), rangedBots.end(), h), rangedBots.end());

        // Distribute remaining ranged evenly
        size_t totalRanged = leftSide.size() + rightSide.size() + rangedBots.size();
        size_t leftCount = leftSide.size();
        size_t rightCount = rightSide.size();
        for (Player* p : rangedBots)
        {
            if (leftCount <= rightCount)
            {
                leftSide.push_back(p);
                leftCount++;
            }
            else
            {
                rightSide.push_back(p);
                rightCount++;
            }
        }

        // Determine which side this bot is assigned to
        bool isLeft = std::find(leftSide.begin(), leftSide.end(), bot) != leftSide.end();
        bool isRight = std::find(rightSide.begin(), rightSide.end(), bot) != rightSide.end();

        // Move to assigned wall position if not already close
        const float MAX_WALL_DIST = 30.0f;
        const float MOVE_INCREMENT = 2.0f;
        const float MAX_MOVE_DISTANCE = 7.0f;
        const float SAFE_SPACING_RADIUS = 7.0f;
        const float MIN_CENTER_DISTANCE = 10.0f;

        Position targetWall = isLeft ? leftPos : (isRight ? rightPos : Position());
        if (isLeft || isRight)
        {
            float distToWall = bot->GetExactDist2d(targetWall.GetPositionX(), targetWall.GetPositionY());
            if (distToWall > MAX_WALL_DIST)
            {
                // Move in increments toward wall
                float dx = targetWall.GetPositionX() - bot->GetPositionX();
                float dy = targetWall.GetPositionY() - bot->GetPositionY();
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f)
                {
                    dx /= len;
                    dy /= len;
                    float moveDist = std::min(MOVE_INCREMENT, distToWall);
                    float targetX = bot->GetPositionX() + dx * moveDist;
                    float targetY = bot->GetPositionY() + dy * moveDist;
                    float targetZ = bot->GetPositionZ();
                    if (!bot->IsWithinLOS(targetX, targetY, targetZ))
                    {
                        targetX = bot->GetPositionX() + dx * (moveDist * 0.5f);
                        targetY = bot->GetPositionY() + dy * (moveDist * 0.5f);
                    }
                    botAI->Reset();
                    MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                           MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
            // Spread from other assigned members on the same side and from swarming shadows
            float totalX = 0.0f, totalY = 0.0f;
            int nearbyCount = 0;
            const std::vector<Player*>& mySide = isLeft ? leftSide : rightSide;
            for (Player* member : mySide)
            {
                if (!member || !member->IsAlive() || member == bot)
                    continue;
                float distance = bot->GetExactDist2d(member);
                if (distance < SAFE_SPACING_RADIUS)
                {
                    float dx = bot->GetPositionX() - member->GetPositionX();
                    float dy = bot->GetPositionY() - member->GetPositionY();
                    float weight = (SAFE_SPACING_RADIUS - distance) / SAFE_SPACING_RADIUS;
                    totalX += dx * weight;
                    totalY += dy * weight;
                    nearbyCount++;
                }
            }
            // Also spread from swarming shadows
            GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (const auto& npcGuid : npcs)
            {
                Unit* unit = botAI->GetUnit(npcGuid);
                if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SWARMING_SHADOWS)
                {
                    float distance = bot->GetExactDist2d(unit);
                    if (distance < SAFE_SPACING_RADIUS)
                    {
                        float dx = bot->GetPositionX() - unit->GetPositionX();
                        float dy = bot->GetPositionY() - unit->GetPositionY();
                        float weight = (SAFE_SPACING_RADIUS - distance) / SAFE_SPACING_RADIUS;
                        totalX += dx * weight;
                        totalY += dy * weight;
                        nearbyCount++;
                    }
                }
            }
            if (nearbyCount > 0)
            {
                float magnitude = sqrt(totalX * totalX + totalY * totalY);
                if (magnitude > 0.001f)
                {
                    totalX /= magnitude;
                    totalY /= magnitude;
                    float moveDistance = std::min(MOVE_INCREMENT, MAX_MOVE_DISTANCE);
                    float targetX = bot->GetPositionX() + totalX * moveDistance;
                    float targetY = bot->GetPositionY() + totalY * moveDistance;
                    float targetZ = bot->GetPositionZ();
                    if (!bot->IsWithinLOS(targetX, targetY, targetZ))
                    {
                        targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                        targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                    }
                    botAI->Reset();
                    MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                           MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
            // Maintain minimum distance from center position (if too close to center, move out)
            float centerX = ICC_BQL_CENTER_POSITION.GetPositionX();
            float centerY = ICC_BQL_CENTER_POSITION.GetPositionY();
            float centerDist =
                std::sqrt(std::pow(bot->GetPositionX() - centerX, 2) + std::pow(bot->GetPositionY() - centerY, 2));
            if (centerDist < MIN_CENTER_DISTANCE && !((boss->GetPositionZ() - bot->GetPositionZ()) > 5.0f))
            {
                float dx = bot->GetPositionX() - centerX;
                float dy = bot->GetPositionY() - centerY;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist > 0.001f)
                {
                    dx /= dist;
                    dy /= dist;
                    float moveDistance = std::min(MIN_CENTER_DISTANCE - centerDist + 1.0f, MAX_MOVE_DISTANCE);
                    float targetX = bot->GetPositionX() + dx * moveDistance;
                    float targetY = bot->GetPositionY() + dy * moveDistance;
                    float targetZ = bot->GetPositionZ();
                    if (!bot->IsWithinLOS(targetX, targetY, targetZ))
                    {
                        targetX = bot->GetPositionX() + dx * (moveDistance * 0.5f);
                        targetY = bot->GetPositionY() + dy * (moveDistance * 0.5f);
                    }
                    botAI->Reset();
                    MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                           MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
        }
    }

    if (isMelee && ((boss->GetPositionZ() - ICC_BQL_CENTER_POSITION.GetPositionZ()) > 5.0f))
    {
        const float SAFE_SPACING_RADIUS = 7.0f;
        const float MOVE_INCREMENT = 2.0f;
        const float MAX_MOVE_DISTANCE = 7.0f;

        float totalX = 0.0f;
        float totalY = 0.0f;
        int nearbyCount = 0;

        // Find all swarming shadows
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        std::vector<Unit*> swarmingShadows;
        for (int i = 0; i < npcs.size(); ++i)
        {
            Unit* unit = botAI->GetUnit(npcs[i]);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SWARMING_SHADOWS)
                swarmingShadows.push_back(unit);
        }

        for (int i = 0; i < members.size(); i++)
        {
            Unit* member = botAI->GetUnit(members[i]);
            if (!member || !member->IsAlive() || member == bot || botAI->GetAura("Frenzied Bloodthirst", member) ||
                botAI->GetAura("Uncontrollable Frenzy", member))
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < SAFE_SPACING_RADIUS)
            {
                float dx = bot->GetPositionX() - member->GetPositionX();
                float dy = bot->GetPositionY() - member->GetPositionY();
                float weight = (SAFE_SPACING_RADIUS - distance) / SAFE_SPACING_RADIUS;
                totalX += dx * weight;
                totalY += dy * weight;
                nearbyCount++;
            }
        }

        // Also spread from swarming shadows
        for (Unit* shadow : swarmingShadows)
        {
            float distance = bot->GetExactDist2d(shadow);
            if (distance < SAFE_SPACING_RADIUS)
            {
                float dx = bot->GetPositionX() - shadow->GetPositionX();
                float dy = bot->GetPositionY() - shadow->GetPositionY();
                float weight = (SAFE_SPACING_RADIUS - distance) / SAFE_SPACING_RADIUS;
                totalX += dx * weight;
                totalY += dy * weight;
                nearbyCount++;
            }
        }

        if (nearbyCount > 0)
        {
            float magnitude = sqrt(totalX * totalX + totalY * totalY);
            if (magnitude > 0.001f)
            {
                totalX /= magnitude;
                totalY /= magnitude;
                float moveDistance = MOVE_INCREMENT < MAX_MOVE_DISTANCE ? MOVE_INCREMENT : MAX_MOVE_DISTANCE;
                float targetX = bot->GetPositionX() + totalX * moveDistance;
                float targetY = bot->GetPositionY() + totalY * moveDistance;
                float targetZ = bot->GetPositionZ();

                if (!bot->IsWithinLOS(targetX, targetY, targetZ))
                {
                    targetX = bot->GetPositionX() + totalX * (moveDistance * 0.5f);
                    targetY = bot->GetPositionY() + totalY * (moveDistance * 0.5f);
                }

                MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true,
                       MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        return true;
    }


    return false;
}

bool IccBqlPactOfDarkfallenAction::Execute(Event event)
{
    // Check if bot has Pact of the Darkfallen
    if (!botAI->GetAura("Pact of the Darkfallen", bot))
        return false;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find other players with Pact of the Darkfallen
    Player* tankWithAura = nullptr;
    std::vector<Player*> playersWithAura;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot)
            continue;
        if (botAI->GetAura("Pact of the Darkfallen", member))
        {
            playersWithAura.push_back(member);
            if (botAI->IsTank(member))
                tankWithAura = member;
        }
    }

    if (playersWithAura.empty())
        return false;

    // Determine target position
    Position targetPos;
    if (tankWithAura)
    {
        // If there's a tank with aura, everyone moves to the tank (including the tank itself for center positioning)
        if (botAI->IsTank(bot))
        {
            // If current bot is the tank, stay put or move slightly for better positioning
            targetPos.Relocate(bot);
        }
        else
        {
            // Non-tank bots move to the tank
            targetPos.Relocate(tankWithAura);
        }
    }
    else if (playersWithAura.size() >= 2)
    {
        // Calculate center position of all players with aura (including bot)
        CalculateCenterPosition(targetPos, playersWithAura);
    }
    else if (playersWithAura.size() == 1)
    {
        // Move to the single other player with aura
        targetPos.Relocate(playersWithAura[0]);
    }
    else
    {
        // No valid movement case found
        return true;
    }

    // Move to target position if needed
    return MoveToTargetPosition(targetPos, playersWithAura.size() + 1);  // +1 to include the bot itself
}

void IccBqlPactOfDarkfallenAction::CalculateCenterPosition(Position& targetPos,
                                                           const std::vector<Player*>& playersWithAura)
{
    float sumX = bot->GetPositionX();
    float sumY = bot->GetPositionY();
    float sumZ = bot->GetPositionZ();

    // Add positions of all other players with aura
    for (Player* player : playersWithAura)
    {
        sumX += player->GetPositionX();
        sumY += player->GetPositionY();
        sumZ += player->GetPositionZ();
    }

    // Calculate average position (center)
    int totalPlayers = playersWithAura.size() + 1;  // +1 for the bot itself
    targetPos.Relocate(sumX / totalPlayers, sumY / totalPlayers, sumZ / totalPlayers);
}

bool IccBqlPactOfDarkfallenAction::MoveToTargetPosition(const Position& targetPos, int auraCount)
{
    const float POSITION_TOLERANCE = 0.1f;
    float distance = bot->GetDistance(targetPos);
    if (distance <= POSITION_TOLERANCE)
        return true;

    // Calculate movement increment
    float dx = targetPos.GetPositionX() - bot->GetPositionX();
    float dy = targetPos.GetPositionY() - bot->GetPositionY();
    float dz = targetPos.GetPositionZ() - bot->GetPositionZ();
    float len = sqrt(dx * dx + dy * dy);

    float moveX, moveY, moveZ;
    if (len > 5.0f && auraCount <= 2)
    {
        dx /= len;
        dy /= len;
        moveX = bot->GetPositionX() + dx * 5.0f;
        moveY = bot->GetPositionY() + dy * 5.0f;
        moveZ = bot->GetPositionZ() + (dz / distance) * 5.0f;
    }
    else
    {
        moveX = targetPos.GetPositionX();
        moveY = targetPos.GetPositionY();
        moveZ = targetPos.GetPositionZ();
    }

    botAI->Reset();
    MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    return false;
}

bool IccBqlVampiricBiteAction::Execute(Event event)
{
    // Only act when bot has Frenzied Bloodthirst
    if (!botAI->GetAura("Frenzied Bloodthirst", bot))
        return false;

    const float BITE_RANGE = 2.0f;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find best target
    Player* target = FindBestBiteTarget(group);
    if (!target)
        return false;

    // Handle movement or casting
    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    if (bot->GetExactDist2d(target) > BITE_RANGE)
    {
        return MoveTowardsTarget(target);
    }
    else if (bot->IsWithinLOS(x, y, z))
    {
        return CastVampiricBite(target);
    }

    return false;
}

Player* IccBqlVampiricBiteAction::FindBestBiteTarget(Group* group)
{
    std::set<ObjectGuid> currentlyTargetedPlayers;
    std::vector<std::pair<Player*, float>> dpsTargets;
    std::vector<std::pair<Player*, float>> healTargets;

    // Get currently targeted players
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (botAI->GetAura("Frenzied Bloodthirst", member) && member->GetTarget())
        {
            currentlyTargetedPlayers.insert(member->GetTarget());
        }
    }

    // Evaluate potential targets
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (IsInvalidTarget(member) || currentlyTargetedPlayers.count(member->GetGUID()))
            continue;

        float distance = bot->GetDistance(member);
        if (botAI->IsDps(member))
            dpsTargets.push_back({member, distance});
        else if (botAI->IsHeal(member))
            healTargets.push_back({member, distance});
    }

    // Sort by distance
    auto sortByDistance = [](const auto& a, const auto& b) { return a.second < b.second; };
    std::sort(dpsTargets.begin(), dpsTargets.end(), sortByDistance);
    std::sort(healTargets.begin(), healTargets.end(), sortByDistance);

    // Return closest valid target
    if (!dpsTargets.empty())
        return dpsTargets[0].first;
    if (!healTargets.empty())
        return healTargets[0].first;
    return nullptr;
}

bool IccBqlVampiricBiteAction::IsInvalidTarget(Player* player)
{
    return botAI->GetAura("Frenzied Bloodthirst", player) || botAI->GetAura("Essence of the Blood Queen", player) ||
           botAI->GetAura("Uncontrollable Frenzy", player) || botAI->GetAura("Swarming Shadows", player) ||
           botAI->IsTank(player);
}

bool IccBqlVampiricBiteAction::MoveTowardsTarget(Player* target)
{
    if (IsInvalidTarget(target) || !target->IsAlive())
        return false;

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    if (!bot->IsWithinLOS(x, y, z))
        return false;

    float dx = x - bot->GetPositionX();
    float dy = y - bot->GetPositionY();
    float dz = z - bot->GetPositionZ();
    float len = sqrt(dx * dx + dy * dy);

    float moveX, moveY, moveZ;
    if (len > 5.0f)
    {
        dx /= len;
        dy /= len;
        moveX = bot->GetPositionX() + dx * 5.0f;
        moveY = bot->GetPositionY() + dy * 5.0f;
        moveZ = bot->GetPositionZ() + (dz / len) * 5.0f;
    }
    else
    {
        moveX = x;
        moveY = y;
        moveZ = z;
    }

    MoveTo(target->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
           MovementPriority::MOVEMENT_FORCED);

    return false;
}

bool IccBqlVampiricBiteAction::CastVampiricBite(Player* target)
{
    if (IsInvalidTarget(target) || !target->IsAlive())
        return false;

    return botAI->CanCastSpell("Vampiric Bite", target) && botAI->CastSpell("Vampiric Bite", target);
}

// Sister Svalna
bool IccValkyreSpearAction::Execute(Event event)
{
    // Find the nearest spear
    Creature* spear = bot->FindNearestCreature(NPC_SPEAR, 100.0f);
    if (!spear)
        return false;

    // Move to the spear if not in range
    if (!spear->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
        return MoveTo(spear, INTERACTION_DISTANCE);

    // Remove shapeshift forms
    botAI->RemoveShapeshift();

    // Stop movement and click the spear
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    spear->HandleSpellClick(bot);

    // Dismount if mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return false;
}

bool IccSisterSvalnaAction::Execute(Event event)
{
    Unit* svalna = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!svalna || !svalna->HasAura(SPELL_AETHER_SHIELD)) // Check for Aether Shield aura
        return false;

    // Check if bot has the spear item
    if (!botAI->HasItemInInventory(ITEM_SPEAR))
        return false;

    // Get all items from inventory
    std::vector<Item*> items = botAI->GetInventoryItems();
    for (Item* item : items)
    {
        if (item->GetEntry() == ITEM_SPEAR) // Spear ID
        {
            botAI->ImbueItem(item, svalna); // Use spear on Svalna
            return false;
        }
    }

    return false;
}

// VDW
bool IccValithriaGroupAction::Execute(Event event)
{
    // Helper lambda to find nearest creature of given entries
    auto findNearestCreature = [this](std::initializer_list<uint32> entries, float range) -> Creature*
    {
        for (uint32 entry : entries)
        {
            if (Creature* creature = bot->FindNearestCreature(entry, range))
            {
                return creature;
            }
        }
        return nullptr;
    };

    // Find portals and enemies
    Creature* portal = findNearestCreature({NPC_DREAM_PORTAL, NPC_DREAM_PORTAL_PRE_EFFECT, NPC_NIGHTMARE_PORTAL, NPC_NIGHTMARE_PORTAL_PRE_EFFECT}, 100.0f);

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* zombie = bot->FindNearestCreature(NPC_BLISTERING_ZOMBIE, 100.0f);
    Creature* manaVoid = bot->FindNearestCreature(NPC_MANA_VOID, 100.0f);

    // Find column of frost units
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> columnOfFrost;
    for (ObjectGuid guid : npcs)
    {
        if (Unit* unit = botAI->GetUnit(guid))
        {
            if (unit->IsAlive() && unit->GetEntry() == NPC_COLUMN_OF_FROST)
            {
                columnOfFrost.push_back(unit);
            }
        }
    }

    // Tank behavior
    if (botAI->IsTank(bot))
    {
        for (const auto& targetGuid : AI_VALUE(GuidVector, "possible targets"))
        {
            if (Unit* unit = botAI->GetUnit(targetGuid))
            {
                if (unit->IsAlive() &&
                    (unit->GetEntry() == NPC_GLUTTONOUS_ABOMINATION || unit->GetEntry() == NPC_ROT_WORM))
                {
                    // Skip if unit is already attacking any tank
                    if (Unit* victim = unit->GetVictim())
                    {
                        if (victim->GetTypeId() == TYPEID_PLAYER && botAI->IsTank(static_cast<Player*>(victim)))
                        {
                            continue;
                        }
                    }

                    // Only attack if not already targeting us
                    if (unit->GetVictim() != bot)
                    {
                        bot->SetTarget(unit->GetGUID());
                        bot->SetFacingToObject(unit);
                        Attack(unit);
                    }
                }
            }
        }
    }

    // Healer movement logic
    if (botAI->IsHeal(bot) && bot->GetExactDist2d(ICC_VDW_HEAL_POSITION) > 30.0f && !portal)
        return MoveTo(bot->GetMapId(), ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY(),
                      ICC_VDW_HEAL_POSITION.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_NORMAL);

    // Avoidance behaviors
    if (manaVoid && bot->GetExactDist2d(manaVoid) < 10.0f &&
        !(botAI->GetAura("Twisted Nightmares", bot) || botAI->GetAura("Emerald Vigor", bot)))
    {
        botAI->Reset();
        FleePosition(manaVoid->GetPosition(), 11.0f, 250U);
    }

    for (Unit* column : columnOfFrost)
    {
        if (column && bot->GetExactDist2d(column) < 7.0f)
        {
            botAI->Reset();
            FleePosition(column->GetPosition(), 8.0f, 250U);
        }
    }

    if (worm && worm->IsAlive() && worm->GetVictim() == bot && !botAI->IsTank(bot))
    {
        botAI->Reset();
        FleePosition(worm->GetPosition(), 10.0f, 250U);
    }

    if (zombie && zombie->IsAlive() && zombie->GetVictim() == bot && !botAI->IsTank(bot) &&
        bot->GetExactDist2d(zombie) < 20.0f)
    {
        botAI->Reset();
        FleePosition(zombie->GetPosition(), 21.0f, 250U);
    }

    // Crowd control logic
    if (zombie && !botAI->IsMainTank(bot) && !botAI->IsHeal(bot) && zombie->GetVictim() != bot)
    {
        switch (bot->getClass())
        {
            case CLASS_MAGE:
                if (!botAI->HasAura("Frost Nova", zombie))
                    botAI->CastSpell("Frost Nova", zombie);
                break;
            case CLASS_DRUID:
                if (!botAI->HasAura("Entangling Roots", zombie))
                    botAI->CastSpell("Entangling Roots", zombie);
                break;
            case CLASS_PALADIN:
                if (!botAI->HasAura("Hammer of Justice", zombie))
                    botAI->CastSpell("Hammer of Justice", zombie);
                break;
            case CLASS_WARRIOR:
                if (!botAI->HasAura("Hamstring", zombie))
                    botAI->CastSpell("Hamstring", zombie);
                break;
            case CLASS_HUNTER:
                if (!botAI->HasAura("Concussive Shot", zombie))
                    botAI->CastSpell("Concussive Shot", zombie);
                break;
            case CLASS_ROGUE:
                if (!botAI->HasAura("Kidney Shot", zombie))
                    botAI->CastSpell("Kidney Shot", zombie);
                break;
            case CLASS_SHAMAN:
                if (!botAI->HasAura("Frost Shock", zombie))
                    botAI->CastSpell("Frost Shock", zombie);
                break;
            case CLASS_DEATH_KNIGHT:
                if (!botAI->HasAura("Chains of Ice", zombie))
                    botAI->CastSpell("Chains of Ice", zombie);
                break;
            default:
                break;
        }
    }

    // Group assignment and movement logic
    Difficulty diff = bot->GetRaidDifficulty();
    Group* group = bot->GetGroup();

    if (group && (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return Handle25ManGroupLogic();
    else
        return Handle10ManGroupLogic();
}

bool IccValithriaGroupAction::MoveTowardsPosition(const Position& pos, float increment)
{
    float dx = pos.GetPositionX() - bot->GetPositionX();
    float dy = pos.GetPositionY() - bot->GetPositionY();
    float dz = pos.GetPositionZ() - bot->GetPositionZ();
    float dist = std::hypot(dx, dy);

    float moveX, moveY, moveZ;
    if (dist > increment)
    {
        dx /= dist;
        dy /= dist;
        moveX = bot->GetPositionX() + dx * increment;
        moveY = bot->GetPositionY() + dy * increment;
        moveZ = bot->GetPositionZ() + (dz / dist) * increment;
    }
    else
    {
        moveX = pos.GetPositionX();
        moveY = pos.GetPositionY();
        moveZ = pos.GetPositionZ();
    }

    MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true, MovementPriority::MOVEMENT_COMBAT,
           true, false);

    return false;
}

bool IccValithriaGroupAction::Handle25ManGroupLogic()
{
    const Position group1Pos = ICC_VDW_GROUP1_POSITION;
    const Position group2Pos = ICC_VDW_GROUP2_POSITION;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect group members
    std::vector<Player*> tanks, dps;
    std::vector<std::pair<ObjectGuid, Player*>> nonHeals;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        if (Player* member = itr->GetSource())
        {
            if (member->IsAlive() && !botAI->IsHeal(member))
            {
                if (botAI->IsTank(member))
                {
                    tanks.push_back(member);
                }
                else
                {
                    dps.push_back(member);
                }
                nonHeals.emplace_back(member->GetGUID(), member);
            }
        }
    }

    // Sort by GUID for consistent ordering
    std::sort(nonHeals.begin(), nonHeals.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    // Assign to groups
    std::vector<Player*> group1, group2;
    if (!tanks.empty())
        group1.push_back(tanks[0]);

    if (tanks.size() > 1)
        group2.push_back(tanks[1]);
    else if (tanks.size() == 1 && !dps.empty())
        group2.push_back(dps[0]);

    // Assign remaining DPS
    std::set<ObjectGuid> assigned;
    for (Player* p : group1)
        assigned.insert(p->GetGUID());
    for (Player* p : group2)
        assigned.insert(p->GetGUID());

    for (Player* p : dps)
    {
        if (assigned.find(p->GetGUID()) == assigned.end())
        {
            (group1.size() <= group2.size() ? group1 : group2).push_back(p);
        }
    }

    // Check which group the bot is in
    bool inGroup1 = std::any_of(group1.begin(), group1.end(), [this](Player* p) { return p == bot; });
    bool inGroup2 = std::any_of(group2.begin(), group2.end(), [this](Player* p) { return p == bot; });

    // Marking logic for tanks and DPS
    if (botAI->IsTank(bot) || botAI->IsDps(bot))
        HandleMarkingLogic(inGroup1, inGroup2, group1Pos, group2Pos);

    
    // Movement logic for non-healers
    if (!botAI->IsHeal(bot))
    {
        if (inGroup1)
        {
            float distance = bot->GetDistance(group1Pos);
            if (distance > 25.0f)
            {
                // If far away, move directly to position
                MoveTowardsPosition(group1Pos, 5.0f);
            }
        }
        else if (inGroup2)
        {
            float distance = bot->GetDistance(group2Pos);
            if (distance > 25.0f)
            {
                MoveTowardsPosition(group2Pos, 5.0f);
            }
        }
    }
    
    return false;
}

bool IccValithriaGroupAction::HandleMarkingLogic(bool inGroup1, bool inGroup2, const Position& group1Pos,
                                                 const Position& group2Pos)
{
    static constexpr uint8_t SKULL_ICON_INDEX = 7;
    static constexpr uint8_t CROSS_ICON_INDEX = 6;
    static const std::array<uint32, 6> addPriority = {NPC_BLAZING_SKELETON,       NPC_SUPPRESSER,
                                                      NPC_RISEN_ARCHMAGE,         NPC_BLISTERING_ZOMBIE,
                                                      NPC_GLUTTONOUS_ABOMINATION, NPC_ROT_WORM};

    const Position* groupPos = nullptr;
    uint8_t iconIndex = 0;
    std::string rtiValue;

    if (inGroup1)
    {
        iconIndex = SKULL_ICON_INDEX;
        groupPos = &group1Pos;
        rtiValue = "skull";
    }
    else if (inGroup2)
    {
        iconIndex = CROSS_ICON_INDEX;
        groupPos = &group2Pos;
        rtiValue = "cross";
    }
    else
        return false;

    context->GetValue<std::string>("rti")->Set(rtiValue);

    // Find priority target
    const GuidVector adds = AI_VALUE(GuidVector, "possible targets");
    Unit* priorityTarget = nullptr;

    for (uint32 entry : addPriority)
    {
        for (const auto& guid : adds)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->IsAlive() && unit->GetEntry() == entry &&
                    unit->GetExactDist2d(groupPos->GetPositionX(), groupPos->GetPositionY()) <= 40.0f)
                {
                    priorityTarget = unit;
                    break;
                }
            }
        }
        if (priorityTarget)
            break;
    }

    // Update target icon if needed
    if (priorityTarget && bot->GetGroup())
    {
        Group* group = bot->GetGroup();
        ObjectGuid currentIcon = group->GetTargetIcon(iconIndex);
        Unit* currentIconUnit = botAI->GetUnit(currentIcon);

        // Check if the target already has any raid icon
        bool hasOtherIcon = false;
        for (uint8 i = 0; i < 8; ++i)
        {
            if (i == iconIndex)
                continue;  // Skip our own icon index
            if (group->GetTargetIcon(i) == priorityTarget->GetGUID())
            {
                hasOtherIcon = true;
                break;
            }
        }

        if (!hasOtherIcon && (!currentIconUnit || !currentIconUnit->IsAlive() || currentIconUnit != priorityTarget))
        {
            group->SetTargetIcon(iconIndex, bot->GetGUID(), priorityTarget->GetGUID());
        }
    }

    return false;
}

bool IccValithriaGroupAction::Handle10ManGroupLogic()
{
    static constexpr uint8_t DEFAULT_ICON_INDEX = 7;
    static const std::array<uint32, 6> addPriority = {NPC_BLAZING_SKELETON,       NPC_SUPPRESSER,
                                                      NPC_RISEN_ARCHMAGE,         NPC_BLISTERING_ZOMBIE,
                                                      NPC_GLUTTONOUS_ABOMINATION, NPC_ROT_WORM};

    // Marking logic
    Group* group = bot->GetGroup();
    if (group)
    {
        const GuidVector adds = AI_VALUE(GuidVector, "possible targets");
        Unit* priorityTarget = nullptr;

        for (uint32 entry : addPriority)
        {
            for (const auto& guid : adds)
            {
                if (Unit* unit = botAI->GetUnit(guid))
                {
                    if (unit->IsAlive() && unit->GetEntry() == entry &&
                        unit->GetExactDist2d(ICC_VDW_HEAL_POSITION.GetPositionX(),
                                             ICC_VDW_HEAL_POSITION.GetPositionY()) <= 50.0f)
                    {
                        priorityTarget = unit;
                        break;
                    }
                }
            }
            if (priorityTarget)
                break;
        }

        if (priorityTarget)
        {
            ObjectGuid currentIcon = group->GetTargetIcon(DEFAULT_ICON_INDEX);
            Unit* currentIconUnit = botAI->GetUnit(currentIcon);

            if (!currentIconUnit || !currentIconUnit->IsAlive() || currentIconUnit != priorityTarget)
            {
                group->SetTargetIcon(DEFAULT_ICON_INDEX, bot->GetGUID(), priorityTarget->GetGUID());
            }
        }
    }

    // Movement logic
    if (bot->GetExactDist2d(ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY()) > 25.0f)
        MoveTowardsPosition(ICC_VDW_HEAL_POSITION, 5.0f);

    return false;
}

bool IccValithriaPortalAction::Execute(Event event)
{
    // Only healers should take portals, and not if already inside
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE))
        return false;

    // Gather all portals (pre-effect and real) using nearest npcs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    std::vector<Creature*> preEffectPortals;
    std::vector<Creature*> realPortals;
    for (const auto& guid : npcs)
    {
        Creature* c = dynamic_cast<Creature*>(botAI->GetUnit(guid));
        if (!c)
            continue;
        uint32 entry = c->GetEntry();
        if (entry == NPC_DREAM_PORTAL_PRE_EFFECT || entry == NPC_NIGHTMARE_PORTAL_PRE_EFFECT)
            preEffectPortals.push_back(c);
        else if (entry == NPC_DREAM_PORTAL || entry == NPC_NIGHTMARE_PORTAL)
            realPortals.push_back(c);
    }

    if (preEffectPortals.empty() && realPortals.empty())
        return false;

    // Remove duplicates (in case of overlap)
    auto sortByGuid = [](Creature* a, Creature* b) { return a->GetGUID() < b->GetGUID(); };
    std::sort(preEffectPortals.begin(), preEffectPortals.end(), sortByGuid);
    preEffectPortals.erase(std::unique(preEffectPortals.begin(), preEffectPortals.end()), preEffectPortals.end());
    std::sort(realPortals.begin(), realPortals.end(), sortByGuid);
    realPortals.erase(std::unique(realPortals.begin(), realPortals.end()), realPortals.end());

    // Gather all healers in group, sort by GUID for deterministic assignment
    Group* group = bot->GetGroup();
    std::vector<Player*> healers;
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->IsAlive() && botAI->IsHeal(member))
                healers.push_back(member);
        }
        std::sort(healers.begin(), healers.end(), [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });
    }
    else
        healers.push_back(bot);

    // Find this bot's index among healers
    auto it = std::find(healers.begin(), healers.end(), bot);
    if (it == healers.end())
        return false;
    size_t healerIndex = std::distance(healers.begin(), it);

    // Assign each healer to a pre-effect portal by index (wrap if more healers than portals)
    Creature* assignedPreEffect = nullptr;
    if (!preEffectPortals.empty())
        assignedPreEffect = preEffectPortals[healerIndex % preEffectPortals.size()];

    // Move to assigned pre-effect portal, stand at portal
    if (assignedPreEffect)
    {
        float portalX = assignedPreEffect->GetPositionX();
        float portalY = assignedPreEffect->GetPositionY();
        float portalZ = assignedPreEffect->GetPositionZ();
        float dist = bot->GetDistance2d(portalX, portalY);

        if (dist > 0.5f)
        {
            // Move directly to the pre-effect portal position
            MoveTo(assignedPreEffect->GetMapId(), portalX, portalY, portalZ, false, false, false, true,
                   MovementPriority::MOVEMENT_NORMAL);
        }
        // Remove shapeshift forms
        botAI->RemoveShapeshift();

        // Try to click the real portal if it is close enough
        Creature* nearestRealPortal = nullptr;
        float minDist = 9999.0f;
        for (Creature* portal : realPortals)
        {
            float d = bot->GetDistance2d(portal);
            if (d < 3.0f && d < minDist)
            {
                nearestRealPortal = portal;
                minDist = d;
            }
        }

        if (nearestRealPortal)
        {
            botAI->RemoveShapeshift();
            bot->GetMotionMaster()->Clear();
            bot->StopMoving();
            bot->SetFacingToObject(nearestRealPortal);
            nearestRealPortal->HandleSpellClick(bot);
            return true;
        }

        // If no real portal is close, wait at the position
        return false;
    }

    // If no pre-effect portals, try to find a real portal within 3f
    Creature* nearestRealPortal = nullptr;
    float minDist = 9999.0f;
    for (Creature* portal : realPortals)
    {
        float d = bot->GetDistance2d(portal);
        if (d < 3.0f && d < minDist)
        {
            nearestRealPortal = portal;
            minDist = d;
        }
    }

    if (nearestRealPortal && minDist > 2.0f)
        MoveTo(bot->GetMapId(), nearestRealPortal->GetPositionX(), nearestRealPortal->GetPositionY(),
               nearestRealPortal->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);

    if (nearestRealPortal)
    {
        botAI->RemoveShapeshift();
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        bot->SetFacingToObject(nearestRealPortal);
        nearestRealPortal->HandleSpellClick(bot);
        return true;
    }

    return false;
}

bool IccValithriaHealAction::Execute(Event event)
{
    // Early validation checks
    if (!botAI->IsHeal(bot) || bot->GetHealthPct() < 50.0f)
        return false;

    // Handle movement speed when not in dream state
    if (!bot->HasAura(SPELL_DREAM_STATE))
    {
        constexpr float NORMAL_SPEED = 1.0f;
        bot->SetSpeed(MOVE_RUN, NORMAL_SPEED, true);
        bot->SetSpeed(MOVE_WALK, NORMAL_SPEED, true);
        bot->SetSpeed(MOVE_FLIGHT, NORMAL_SPEED, true);
    }

    // Enforce Z-position limit
    constexpr float MAX_Z_POSITION = 367.961f;
    constexpr float TARGET_Z_POSITION = 365.0f;
    if (bot->GetPositionZ() > MAX_Z_POSITION)
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), TARGET_Z_POSITION,
                        bot->GetOrientation());

    // Find Valithria within range
    Creature* valithria = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!valithria)
        return false;

    // Execute class-specific healing logic
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            // Druid healing spell constants
            constexpr uint32 SPELL_REJUVENATION = 48441;
            constexpr uint32 SPELL_REGROWTH = 48443;
            constexpr uint32 SPELL_LIFEBLOOM = 48451;
            constexpr uint32 SPELL_WILD_GROWTH = 53251;
            constexpr uint8 LIFEBLOOM_MAX_STACKS = 3;

            // Apply Rejuvenation if missing
            if (!valithria->HasAura(SPELL_REJUVENATION, bot->GetGUID()))
                return botAI->CastSpell(SPELL_REJUVENATION, valithria);

            // Apply Regrowth if missing
            if (!valithria->HasAura(SPELL_REGROWTH, bot->GetGUID()))
                return botAI->CastSpell(SPELL_REGROWTH, valithria);

            // Stack Lifebloom to maximum stacks
            Aura* lifebloom = valithria->GetAura(SPELL_LIFEBLOOM, bot->GetGUID());
            if (!lifebloom || lifebloom->GetStackAmount() < LIFEBLOOM_MAX_STACKS)
                return botAI->CastSpell(SPELL_LIFEBLOOM, valithria);

            // All HoTs active with full stacks - cast Wild Growth
            return botAI->CastSpell(SPELL_WILD_GROWTH, valithria);
        }
        case CLASS_SHAMAN:
        {
            constexpr uint32 SPELL_RIPTIDE = 61301;
            constexpr uint32 SPELL_HEALING_WAVE = 49273;

            // Cast Healing Wave if Riptide is active, otherwise apply Riptide
            return valithria->HasAura(SPELL_RIPTIDE, bot->GetGUID()) ? botAI->CastSpell(SPELL_HEALING_WAVE, valithria)
                                                                     : botAI->CastSpell(SPELL_RIPTIDE, valithria);
        }
        case CLASS_PRIEST:
        {
            constexpr uint32 SPELL_RENEW = 48068;
            constexpr uint32 SPELL_GREATER_HEAL = 48063;

            // Cast Greater Heal if Renew is active, otherwise apply Renew
            return valithria->HasAura(SPELL_RENEW, bot->GetGUID()) ? botAI->CastSpell(SPELL_GREATER_HEAL, valithria)
                                                            : botAI->CastSpell(SPELL_RENEW, valithria);
        }
        case CLASS_PALADIN:
        {
            constexpr uint32 SPELL_BEACON_OF_LIGHT = 53563;
            constexpr uint32 SPELL_HOLY_LIGHT = 48782;

            // Cast Holy Light if Beacon is active, otherwise apply Beacon of Light
            return valithria->HasAura(SPELL_BEACON_OF_LIGHT, bot->GetGUID())
                       ? botAI->CastSpell(SPELL_HOLY_LIGHT, valithria)
                       : botAI->CastSpell(SPELL_BEACON_OF_LIGHT, valithria);
        }
        default:
            return false;
    }

    return false;
}

bool IccValithriaDreamCloudAction::Execute(Event event)
{
    // Only execute if we're in dream state
    if (!bot->HasAura(SPELL_DREAM_STATE))
        return false;

    // Set speed to match players in dream state
    bot->SetSpeed(MOVE_RUN, 2.0f, true);
    bot->SetSpeed(MOVE_WALK, 2.0f, true);
    bot->SetSpeed(MOVE_FLIGHT, 2.0f, true);

    // Gather all group members with dream state
    const GuidVector members = AI_VALUE(GuidVector, "group members");
    std::vector<Unit*> dreamBots;
    for (const auto& guid : members)
    {
        Unit* member = botAI->GetUnit(guid);
        if (member && member->IsAlive() && member->HasAura(SPELL_DREAM_STATE))
            dreamBots.push_back(member);
    }

    if (dreamBots.empty())
        return false;

    // Sort dreamBots by GUID (lowest first)
    std::sort(dreamBots.begin(), dreamBots.end(), [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    // Find this bot's index in the sorted list
    auto it = std::find(dreamBots.begin(), dreamBots.end(), bot);
    if (it == dreamBots.end())
        return false;
    size_t myIndex = std::distance(dreamBots.begin(), it);

    // Check if all dream bots are stacked within 3f of the current leader (lowest guid)
    constexpr float STACK_RADIUS = 2.0f;
    Unit* leader = dreamBots.front();
    bool allStacked = true;
    for (Unit* member : dreamBots)
    {
        // Only require stacking for bots, not real players
        Player* player = member->ToPlayer();
        if (player && !player->GetSession())  // is a bot
        {
            if (member->GetDistance(leader) > STACK_RADIUS)
            {
                allStacked = false;
                break;
            }
        }
    }

    // If not all stacked, everyone moves to the leader's position (clouds' position)
    constexpr float PORTALSTART_TOLERANCE = 1.0f;
    if (!allStacked)
    {
        if (bot != leader)
        {
            if (bot->GetDistance(leader) > PORTALSTART_TOLERANCE)
            {
                bot->TeleportTo(bot->GetMapId(), leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ(),
                                bot->GetOrientation());
            }
        }
    }

    
    // All stacked: leader (lowest guid) moves to next cloud, others follow and stack at leader's new position
    // Find all dream and nightmare clouds
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Creature*> dreamClouds;
    std::vector<Creature*> nightmareClouds;

    for (int i = 0; i < npcs.size(); ++i)
    {
        Unit* unit = botAI->GetUnit(npcs[i]);
        if (unit && unit->IsAlive())
        {
            if (Creature* creature = unit->ToCreature())
            {
                if (creature->GetEntry() == NPC_DREAM_CLOUD)
                    dreamClouds.push_back(creature);
                else if (creature->GetEntry() == NPC_NIGHTMARE_CLOUD)
                    nightmareClouds.push_back(creature);
            }
        }
    }

    // Sort clouds by distance
    std::sort(dreamClouds.begin(), dreamClouds.end(),
              [this](Creature* a, Creature* b) { return bot->GetExactDist2d(a) < bot->GetExactDist2d(b); });

    std::sort(nightmareClouds.begin(), nightmareClouds.end(),
              [this](Creature* a, Creature* b) { return bot->GetExactDist2d(a) < bot->GetExactDist2d(b); });

    // Only the leader moves to the next cloud
    if (bot == leader)
    {
        // Use GUID to determine which cloud type to prefer
        bool preferDream = (bot->GetGUID().GetCounter() % 2 == 0);

        // Check if we're close to any cloud
        bool atDreamCloud = false;
        bool atNightmareCloud = false;

        for (Creature* cloud : dreamClouds)
        {
            if (bot->GetExactDist2d(cloud) <= 2.0f)
            {
                atDreamCloud = true;
                break;
            }
        }

        for (Creature* cloud : nightmareClouds)
        {
            if (bot->GetExactDist2d(cloud) <= 2.0f)
            {
                atNightmareCloud = true;
                break;
            }
        }

        // If we have emerald vigor, prioritize dream clouds
        if (bot->HasAura(SPELL_EMERALD_VIGOR))
        {
            // If at dream cloud, move to 2nd closest dream cloud or closest nightmare cloud
            if (atDreamCloud)
            {
                Creature* targetCloud = nullptr;
                // Try 2nd closest dream cloud first
                if (dreamClouds.size() >= 2 && bot->GetExactDist2d(dreamClouds[1]) > 2.0f)
                    targetCloud = dreamClouds[1];
                // Otherwise move to closest nightmare cloud
                else if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                    targetCloud = nightmareClouds[0];

                if (targetCloud)
                    MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                           targetCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            // If at nightmare cloud, move to closest dream cloud or 2nd closest nightmare cloud
            else if (atNightmareCloud)
            {
                Creature* targetCloud = nullptr;
                // Try closest dream cloud first
                if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                    targetCloud = dreamClouds[0];
                // Otherwise move to 2nd closest nightmare cloud
                else if (nightmareClouds.size() >= 2 && bot->GetExactDist2d(nightmareClouds[1]) > 2.0f)
                    targetCloud = nightmareClouds[1];

                if (targetCloud)
                    MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                           targetCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            // If not at any cloud, move to closest dream cloud or nightmare cloud
            else
            {
                if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                    MoveTo(dreamClouds[0]->GetMapId(), dreamClouds[0]->GetPositionX(), dreamClouds[0]->GetPositionY(),
                           dreamClouds[0]->GetPositionZ(), false, false, false, true,
                           MovementPriority::MOVEMENT_NORMAL);
                else if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                    MoveTo(nightmareClouds[0]->GetMapId(), nightmareClouds[0]->GetPositionX(),
                           nightmareClouds[0]->GetPositionY(), nightmareClouds[0]->GetPositionZ(), false, false, false,
                           true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
        // Otherwise use GUID-based preference
        else
        {
            // If prefer dream clouds based on GUID
            if (preferDream)
            {
                // If at dream cloud, move to 2nd closest dream cloud or closest nightmare cloud
                if (atDreamCloud)
                {
                    Creature* targetCloud = nullptr;
                    // Try 2nd closest dream cloud first
                    if (dreamClouds.size() >= 2 && bot->GetExactDist2d(dreamClouds[1]) > 2.0f)
                        targetCloud = dreamClouds[1];
                    // Otherwise move to closest nightmare cloud
                    else if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                        targetCloud = nightmareClouds[0];

                    if (targetCloud)
                        MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                               targetCloud->GetPositionZ(), false, false, false, true,
                               MovementPriority::MOVEMENT_NORMAL);
                }
                // If at nightmare cloud, move to closest dream cloud or 2nd closest nightmare cloud
                else if (atNightmareCloud)
                {
                    Creature* targetCloud = nullptr;
                    // Try closest dream cloud first
                    if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                        targetCloud = dreamClouds[0];
                    // Otherwise move to 2nd closest nightmare cloud
                    else if (nightmareClouds.size() >= 2 && bot->GetExactDist2d(nightmareClouds[1]) > 2.0f)
                        targetCloud = nightmareClouds[1];

                    if (targetCloud)
                        MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                               targetCloud->GetPositionZ(), false, false, false, true,
                               MovementPriority::MOVEMENT_NORMAL);
                }
                // If not at any cloud, move to closest dream cloud or nightmare cloud based on preference
                else
                {
                    if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                        MoveTo(dreamClouds[0]->GetMapId(), dreamClouds[0]->GetPositionX(),
                               dreamClouds[0]->GetPositionY(), dreamClouds[0]->GetPositionZ(), false, false, false,
                               true, MovementPriority::MOVEMENT_NORMAL);
                    else if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                        MoveTo(nightmareClouds[0]->GetMapId(), nightmareClouds[0]->GetPositionX(),
                               nightmareClouds[0]->GetPositionY(), nightmareClouds[0]->GetPositionZ(), false, false,
                               false, true, MovementPriority::MOVEMENT_NORMAL);
                }
            }
            // If prefer nightmare clouds based on GUID
            else
            {
                // If at nightmare cloud, move to 2nd closest nightmare cloud or closest dream cloud
                if (atNightmareCloud)
                {
                    Creature* targetCloud = nullptr;
                    // Try 2nd closest nightmare cloud first
                    if (nightmareClouds.size() >= 2 && bot->GetExactDist2d(nightmareClouds[1]) > 2.0f)
                        targetCloud = nightmareClouds[1];
                    // Otherwise move to closest dream cloud
                    else if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                        targetCloud = dreamClouds[0];

                    if (targetCloud)
                        MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                               targetCloud->GetPositionZ(), false, false, false, true,
                               MovementPriority::MOVEMENT_NORMAL);
                }
                // If at dream cloud, move to closest nightmare cloud or 2nd closest dream cloud
                else if (atDreamCloud)
                {
                    Creature* targetCloud = nullptr;
                    // Try closest nightmare cloud first
                    if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                        targetCloud = nightmareClouds[0];
                    // Otherwise move to 2nd closest dream cloud
                    else if (dreamClouds.size() >= 2 && bot->GetExactDist2d(dreamClouds[1]) > 2.0f)
                        targetCloud = dreamClouds[1];

                    if (targetCloud)
                        MoveTo(targetCloud->GetMapId(), targetCloud->GetPositionX(), targetCloud->GetPositionY(),
                               targetCloud->GetPositionZ(), false, false, false, true,
                               MovementPriority::MOVEMENT_NORMAL);
                }
                // If not at any cloud, move to closest nightmare cloud or dream cloud based on preference
                else
                {
                    if (!nightmareClouds.empty() && bot->GetExactDist2d(nightmareClouds[0]) > 2.0f)
                        MoveTo(nightmareClouds[0]->GetMapId(), nightmareClouds[0]->GetPositionX(),
                               nightmareClouds[0]->GetPositionY(), nightmareClouds[0]->GetPositionZ(), false, false,
                               false, true, MovementPriority::MOVEMENT_NORMAL);
                    else if (!dreamClouds.empty() && bot->GetExactDist2d(dreamClouds[0]) > 2.0f)
                        MoveTo(dreamClouds[0]->GetMapId(), dreamClouds[0]->GetPositionX(),
                               dreamClouds[0]->GetPositionY(), dreamClouds[0]->GetPositionZ(), false, false, false,
                               true, MovementPriority::MOVEMENT_NORMAL);
                }
            }
        }
    }
    else
    {
        // Non-leader bots follow and stack at leader's position
        if (bot->GetDistance(leader) > PORTALSTART_TOLERANCE)
        {
            botAI->Reset();
            bot->TeleportTo(bot->GetMapId(), leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ(),
                            bot->GetOrientation());
        }
    }

    return false;
}

// Sindragosa
bool IccSindragosaGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);

    if (aura && aura->GetStackAmount() >= 6 && botAI->IsMainTank(bot))
        return false;

    if (botAI->IsTank(bot) && boss->GetVictim() == bot)
        return HandleTankPositioning(boss);

    if (boss && boss->GetVictim() != bot)
        return HandleNonTankPositioning();

    return false;
}

bool IccSindragosaGroupPositionAction::HandleTankPositioning(Unit* boss)
{
    float distBossToCenter = boss->GetExactDist2d(ICC_SINDRAGOSA_CENTER_POSITION);
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    float targetOrientation = M_PI / 2;  // We want boss to face east
    float currentOrientation = boss->GetOrientation();

    // Normalize both orientations to 0-2π range
    currentOrientation = fmod(currentOrientation + 2 * M_PI, 2 * M_PI);
    targetOrientation = fmod(targetOrientation + 2 * M_PI, 2 * M_PI);

    float orientationDiff = currentOrientation - targetOrientation;

    // Normalize the difference to be between -PI and PI
    while (orientationDiff > M_PI)
        orientationDiff -= 2 * M_PI;
    while (orientationDiff < -M_PI)
        orientationDiff += 2 * M_PI;

    // Stage 1: Move boss to center if too far
    if (boss && boss->GetVictim() == bot && distBossToCenter > 16.0f && distToTankPos <= 20.0f)
    {
        // Calculate direction vector from boss to center
        float dirX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() - boss->GetPositionX();
        float dirY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() - boss->GetPositionY();

        // Move 10 yards beyond center in the same direction
        float moveX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() + (dirX / distBossToCenter) * 4.0f;
        float moveY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() + (dirY / distBossToCenter) * 4.0f;

        return MoveTo(bot->GetMapId(), moveX, moveY, boss->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Stage 2: Move to tank position if too far
    if (boss && boss->GetVictim() == bot && distToTankPos > 10.0f)
    {
        Position botPos = bot->GetPosition();
        Position tankPos = ICC_SINDRAGOSA_TANK_POSITION;

        float dx = tankPos.GetPositionX() - botPos.GetPositionX();
        float dy = tankPos.GetPositionY() - botPos.GetPositionY();

        float distance = std::sqrt(dx * dx + dy * dy);
        float step = 1.0f;

        // Normalize and scale direction vector
        float scale = step / distance;

        float targetX = botPos.GetPositionX() + dx * scale;
        float targetY = botPos.GetPositionY() + dy * scale;

        return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Stage 3: Adjust orientation when in position
    if (boss && boss->GetVictim() == bot && std::abs(orientationDiff) > 0.15f)
    {
        // Move in an arc (circle) north or south around the boss until the orientation matches
        float currentX = bot->GetPositionX();
        float currentY = bot->GetPositionY();
        float centerX = boss->GetPositionX();
        float centerY = boss->GetPositionY();
        float radius = std::max(2.0f, bot->GetExactDist2d(centerX, centerY));  // keep at least 2 yards from boss

        // Calculate current angle from boss to bot
        float angle = atan2(currentY - centerY, currentX - centerX);

        // Determine direction: negative diff = move counterclockwise (north), positive = clockwise (south)
        float arcStep = 0.125f;  // radians per move, adjust for smoothness
        if (orientationDiff < 0)
            angle += arcStep;  // move north (counterclockwise)
        else
            angle -= arcStep;  // move south (clockwise)

        // Calculate new position on the arc
        float moveX = centerX + radius * cos(angle);
        float moveY = centerY + radius * sin(angle);

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Stage 4: Adjust Y-axis position if too far from tank position
    float yDiff = std::abs(bot->GetPositionY() - ICC_SINDRAGOSA_TANK_POSITION.GetPositionY());
    if (boss && boss->GetVictim() == bot && yDiff > 2.0f)
    {
        Position botPos = bot->GetPosition();
        Position tankPos = ICC_SINDRAGOSA_TANK_POSITION;

        // Only adjust Y position, keep X and Z the same
        float newY = botPos.GetPositionY() + (tankPos.GetPositionY() > botPos.GetPositionY() ? 1.0f : -1.0f);

        return MoveTo(bot->GetMapId(), botPos.GetPositionX(), newY, botPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IccSindragosaGroupPositionAction::HandleNonTankPositioning()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect all alive raid members
    std::vector<Player*> raidMembers;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
        raidMembers.push_back(member);
    }

    // Count members without aura 1111
    size_t membersWithoutAura = 0;
    for (Player* member : raidMembers)
    {
        if (!botAI->GetAura("mystic buffet", member))
            membersWithoutAura++;
    }

    // Calculate percentage without aura
    size_t totalMembers = raidMembers.size();
    if (totalMembers == 0)
        return false;

    double percentageWithoutAura = static_cast<double>(membersWithoutAura) / totalMembers;
    bool raidClear = (percentageWithoutAura >= 0.6);  // 60% or more don't have aura 1111

    if (raidClear && botAI->IsTank(bot))
    {
        static const std::array<uint32, 4> tombEntries = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};
        const GuidVector tombGuids = AI_VALUE(GuidVector, "possible targets no los");

        Unit* nearestTomb = nullptr;
        float minDist = 150.0f;

        for (const auto entry : tombEntries)
        {
            for (const auto& guid : tombGuids)
            {
                if (Unit* unit = botAI->GetUnit(guid))
                {
                    if (unit->GetEntry() == entry && unit->IsAlive())
                    {
                        float dist = bot->GetDistance(unit);
                        if (dist < minDist)
                        {
                            minDist = dist;
                            nearestTomb = unit;
                        }
                    }
                }
            }
        }

        static constexpr uint8_t SKULL_ICON_INDEX = 7;

        Group* group = bot->GetGroup();
        if (!group)
            return false;  // Cannot assign icon without group

        Unit* targetToMark = nearestTomb;

        // Fallback: mark boss if no tomb is found
        if (!targetToMark)
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
            if (boss && boss->IsAlive())
                targetToMark = boss;
        }

        if (targetToMark)
        {
            const ObjectGuid currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
            Unit* currentSkullUnit = botAI->GetUnit(currentSkull);

            const bool needsUpdate =
                !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != targetToMark;

            if (needsUpdate)
                group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), targetToMark->GetGUID());
        }
    }

    context->GetValue<std::string>("rti")->Set("skull");
    if (botAI->IsRanged(bot))
    {
        const float TOLERANCE = 9.0f;
        const float MAX_STEP = 5.0f;

        float distToTarget = bot->GetExactDist2d(ICC_SINDRAGOSA_RANGED_POSITION);

        // Only move if outside tolerance
        if (distToTarget > TOLERANCE)
            return MoveIncrementallyToPosition(ICC_SINDRAGOSA_RANGED_POSITION, MAX_STEP);

        return false;
    }
    else
    {
        const float TOLERANCE = 10.0f;
        const float MAX_STEP = 5.0f;

        float distToTarget = bot->GetExactDist2d(ICC_SINDRAGOSA_MELEE_POSITION);

        // Only move if outside tolerance
        if (distToTarget > TOLERANCE)
            return MoveIncrementallyToPosition(ICC_SINDRAGOSA_MELEE_POSITION, MAX_STEP);

        return false;
    }
}

bool IccSindragosaGroupPositionAction::MoveIncrementallyToPosition(const Position& targetPos, float maxStep)
{
    // Calculate direction vector to target
    float dirX = targetPos.GetPositionX() - bot->GetPositionX();
    float dirY = targetPos.GetPositionY() - bot->GetPositionY();

    // Normalize direction vector
    float length = sqrt(dirX * dirX + dirY * dirY);
    dirX /= length;
    dirY /= length;

    // Calculate intermediate point
    float distToTarget = bot->GetExactDist2d(targetPos);
    float stepSize = std::min(maxStep, distToTarget);
    float moveX = bot->GetPositionX() + dirX * stepSize;
    float moveY = bot->GetPositionY() + dirY * stepSize;

    return MoveTo(bot->GetMapId(), moveX, moveY, targetPos.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccSindragosaTankSwapPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTank(bot))
        return false;

    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    
    // Move to tank position
    if (distToTankPos > 3.0f)  // Tighter tolerance for tank swap
    {
        return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_TANK_POSITION.GetPositionX(),
                     ICC_SINDRAGOSA_TANK_POSITION.GetPositionY(),
                     ICC_SINDRAGOSA_TANK_POSITION.GetPositionZ(),
                     false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IccSindragosaFrostBeaconAction::Execute(Event event)
{
    const Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    HandleSupportActions();

    if (bot->HasAura(FROST_BEACON_AURA_ID))
    {
        return HandleBeaconedPlayer(boss);
    }

    return HandleNonBeaconedPlayer(boss);
}

void IccSindragosaFrostBeaconAction::HandleSupportActions()
{
    Group* group = bot->GetGroup();

    // Tank support - Paladin Hand of Freedom
    if (group && bot->getClass() == CLASS_PALADIN)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsTank(member))
            {
                continue;
            }

            if (botAI->GetAura("Frost Breath", member) && !member->HasAura(HAND_OF_FREEDOM_SPELL_ID))
            {
                botAI->CastSpell(HAND_OF_FREEDOM_SPELL_ID, member);
                break;
            }
        }
    }

    // Healer support - Apply HoTs to beaconed players
    if (botAI->IsHeal(bot) && !bot->HasAura(FROST_BEACON_AURA_ID))
    {
        const auto members = AI_VALUE(GuidVector, "group members");
        for (const auto& memberGuid : members)
        {
            Unit* member = botAI->GetUnit(memberGuid);
            if (!member || !member->IsAlive() || !member->HasAura(FROST_BEACON_AURA_ID))
            {
                continue;
            }

            // Apply class-specific HoT spells
            uint32 spellId = 0;
            switch (bot->getClass())
            {
                case CLASS_PRIEST:
                    spellId = 48068;
                    break;  // Renew
                case CLASS_SHAMAN:
                    spellId = 61301;
                    break;  // Riptide
                case CLASS_DRUID:
                    spellId = 48441;
                    break;  // Rejuvenation
                default:
                    continue;
            }

            if (!member->HasAura(spellId))
            {
                botAI->CastSpell(spellId, member);
            }
        }
    }
}

bool IccSindragosaFrostBeaconAction::HandleBeaconedPlayer(const Unit* boss)
{
    // Phase 3 positioning (below 35% health, not flying)
    if (boss->HealthBelowPct(35) && !IsBossFlying(boss))
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);
        botAI->Reset();
        return MoveToPositionIfNeeded(ICC_SINDRAGOSA_THOMBMB2_POSITION, POSITION_TOLERANCE);
    }

    // Regular beacon positioning using tomb spots
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    // Collect and sort beaconed players by GUID for deterministic assignment
    std::vector<Player*> beaconedPlayers;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && member->HasAura(FROST_BEACON_AURA_ID))
        {
            beaconedPlayers.push_back(member);
        }
    }

    std::sort(beaconedPlayers.begin(), beaconedPlayers.end(),
              [](const Player* a, const Player* b) { return a->GetGUID() < b->GetGUID(); });

    // Find this bot's index
    const auto it = std::find(beaconedPlayers.begin(), beaconedPlayers.end(), bot);
    if (it == beaconedPlayers.end())
    {
        return false;
    }

    const size_t myIndex = std::distance(beaconedPlayers.begin(), it);
    const size_t beaconCount = beaconedPlayers.size();

    // Calculate tomb spot based on beacon count
    size_t spot = 0;
    switch (beaconCount)
    {
        case 2:
            spot = (myIndex == 0) ? 0 : 2;
            break;
        case 5:
            spot = (myIndex < 2) ? 0 : ((myIndex == 2) ? 1 : 2);
            break;
        case 6:
            spot = myIndex / 2;
            break;
        default:
            spot = myIndex % 3;
            break;
    }

    // Get tomb position and move if needed
    static constexpr std::array<const Position*, 3> tombPositions = {
        &ICC_SINDRAGOSA_THOMB1_POSITION, &ICC_SINDRAGOSA_THOMB2_POSITION, &ICC_SINDRAGOSA_THOMB3_POSITION};

    const Position& tombPosition = *tombPositions[std::min(spot, tombPositions.size() - 1)];
    return MoveToPositionIfNeeded(tombPosition, TOMB_POSITION_TOLERANCE);
}

bool IccSindragosaFrostBeaconAction::HandleNonBeaconedPlayer(const Unit* boss)
{
    // Collect beaconed players
    std::vector<Unit*> beaconedPlayers;
    const auto members = AI_VALUE(GuidVector, "group members");
    for (const auto& memberGuid : members)
    {
        Unit* player = botAI->GetUnit(memberGuid);
        if (player && player->GetGUID() != bot->GetGUID() && player->HasAura(FROST_BEACON_AURA_ID))
        {
            beaconedPlayers.push_back(player);
        }
    }

    if (beaconedPlayers.empty())
    {
        return false;
    }

    // Air phase positioning
    if (IsBossFlying(boss))
    {
        if (!bot->HasAura(FROST_BEACON_AURA_ID))
        {
            const Difficulty diff = bot->GetRaidDifficulty();
            bool is25Man = false;
            if (diff && (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC))
                is25Man = true;

            const Position& safePosition = is25Man ? ICC_SINDRAGOSA_FBOMB_POSITION : ICC_SINDRAGOSA_FBOMB10_POSITION;

            const float dist = bot->GetExactDist2d(safePosition.GetPositionX(), safePosition.GetPositionY());
            if (dist > MOVE_TOLERANCE)
            {
                return MoveToPosition(safePosition);
            }
        }
        return botAI->IsHeal(bot);  // Continue for healers, wait for others
    }

    // Ground phase - position based on role and avoid beaconed players
    const bool isRanged = botAI->IsRanged(bot) || (bot->GetExactDist2d(ICC_SINDRAGOSA_RANGED_POSITION.GetPositionX(),ICC_SINDRAGOSA_RANGED_POSITION.GetPositionY()) <
                          bot->GetExactDist2d(ICC_SINDRAGOSA_MELEE_POSITION.GetPositionX(),ICC_SINDRAGOSA_MELEE_POSITION.GetPositionY()));

    const Position& targetPosition = isRanged ? ICC_SINDRAGOSA_RANGED_POSITION : ICC_SINDRAGOSA_MELEE_POSITION; 

    const float deltaX = std::abs(targetPosition.GetPositionX() - bot->GetPositionX());
    const float deltaY = std::abs(targetPosition.GetPositionY() - bot->GetPositionY());
    if (boss && boss->GetVictim() != bot)
    {
        if ((deltaX > MOVE_TOLERANCE) || (deltaY > MOVE_TOLERANCE))
        {
            if (bot->HasUnitState(UNIT_STATE_CASTING))
            {
                botAI->Reset();
            }
            return MoveToPosition(targetPosition);
        }
    }
    return false;
}

bool IccSindragosaFrostBeaconAction::MoveToPositionIfNeeded(const Position& position, float tolerance)
{
    const float distance = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distance > tolerance)
    {
        return MoveToPosition(position);
    }
    return distance <= tolerance;
}

bool IccSindragosaFrostBeaconAction::MoveToPosition(const Position& position)
{
    float posX = position.GetPositionX();
    float posY = position.GetPositionY();
    float posZ = position.GetPositionZ();

    bot->UpdateAllowedPositionZ(posX, posY, posZ);

    return MoveTo(bot->GetMapId(), posX, posY, posZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                  true, false);
}

bool IccSindragosaFrostBeaconAction::IsBossFlying(const Unit* boss)
{
    return boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(),
                                ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f;
}

bool IccSindragosaBlisteringColdAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only non-tanks should move out
    if (botAI->IsMainTank(bot))
        return false;

   float dist = bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());

    if (dist >= 33.0f)
        return false;

    Position const& targetPos = ICC_SINDRAGOSA_BLISTERING_COLD_POSITION;

    // Only move if we're too close to the boss (< 30 yards)
    if (dist < 33.0f)
    {

        float const STEP_SIZE = 15.0f;
        float distToTarget = bot->GetDistance2d(targetPos.GetPositionX(), targetPos.GetPositionY());
        
        if (distToTarget > 0.1f)  // Avoid division by zero
        {
            if (!bot->HasAura(SPELL_NITRO_BOOSTS))
                bot->AddAura(SPELL_NITRO_BOOSTS, bot);
            // Calculate direction vector
            float dirX = targetPos.GetPositionX() - bot->GetPositionX();
            float dirY = targetPos.GetPositionY() - bot->GetPositionY();
            
            // Normalize direction vector
            float length = sqrt(dirX * dirX + dirY * dirY);
            dirX /= length;
            dirY /= length;
            
            // Move STEP_SIZE yards in that direction
            float moveX = bot->GetPositionX() + dirX * STEP_SIZE;
            float moveY = bot->GetPositionY() + dirY * STEP_SIZE;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    return false;
}

bool IccSindragosaUnchainedMagicAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Unchained Magic", bot, false, true);
    if (!aura)
        return false;

    Aura* aura1 = botAI->GetAura("Instability", bot, false, true);

    Difficulty diff = bot->GetRaidDifficulty();
    if (aura && (diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_NORMAL))
    {
        if (aura1 && aura1->GetStackAmount() >= 6)
            return true;  // Stop casting spells
    }

    return false;
}

bool IccSindragosaChilledToTheBoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Chilled to the Bone", bot, false, true);
    if (!aura)
        return false;

    if (aura) // Chilled to the Bone
    {
        if (aura->GetStackAmount() >= 6)
        {
            botAI->Reset();
            bot->AttackStop();
            return true;
        }
    }

    return false;  
}

bool IccSindragosaMysticBuffetAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || !bot || !bot->IsAlive())
        return false;

    // Check if we have Mystic Buffet
    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (!aura)
        return false;

    if (boss->GetVictim() == bot)
        return false;

    // Skip if we have Frost Beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static const std::array<uint32, 4> tombEntries = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};
    const GuidVector tombGuids = AI_VALUE(GuidVector, "possible targets no los");

    Unit* nearestTomb = nullptr;
    float minDist = 150.0f;

    for (const auto entry : tombEntries)
    {
        for (const auto& guid : tombGuids)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry && unit->IsAlive())
                {
                    float dist = bot->GetDistance(unit);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        nearestTomb = unit;
                    }
                }
            }
        }
    }

    // Check if anyone in group has Frost Beacon (SPELL_FROST_BEACON)
    bool anyoneHasFrostBeacon = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
        {
            anyoneHasFrostBeacon = true;
            break;
        }
    }

    bool tombPresent = nearestTomb != nullptr;
    bool atLOS2 = bot->GetExactDist2d(ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(),
                                      ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY()) <= 2.0f;

    // Move to LOS2 position if: tomb is present and no one has Frost Beacon
    bool shouldMoveLOS2 = tombPresent && !anyoneHasFrostBeacon;

    if (shouldMoveLOS2)
    {
        // If already at LOS2 and have 3+ stacks, stay still
        if (atLOS2 && aura && !botAI->IsHeal(bot))
        {
            return true;
        }
                

        botAI->Reset();
        // Move to LOS2 position
        return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(),
                      ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY(), ICC_SINDRAGOSA_LOS2_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    return false;
}

bool IccSindragosaFrostBombAction::Execute(Event event)
{
    if (!bot || !bot->IsAlive() || bot->HasAura(SPELL_ICE_TOMB))  // Skip if dead or in Ice Tomb
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Find frost bomb marker and tombs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    const uint32 tombEntries[] = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};  // tomb id's
    Unit* marker = nullptr;
    std::vector<Unit*> tombs;
    std::vector<ObjectGuid> tombGuids;

    // Manually search for units with frost bomb aura (SPELL_FROST_BOMB_VISUAL) using NearestHostileNpcsValue logic
    std::list<Unit*> units;
    float range = 200.0f;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, units, u_check);
    Cell::VisitObjects(bot, searcher, range);

    for (Unit* unit : units)
    {
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->HasAura(SPELL_FROST_BOMB_VISUAL))  // Frost bomb visual
            marker = unit;

        // Check if unit is a tomb
        for (uint32 entry : tombEntries)
        {
            if (unit->GetEntry() == entry)
            {
                tombs.push_back(unit);
                tombGuids.push_back(unit->GetGUID());
                break;
            }
        }
    }

    if (!marker || tombs.empty())
    {
        bot->AttackStop();
        return true;
    }

    // Get persistent group assignment - use a static map to store assignments
    static std::map<ObjectGuid, int> persistentGroupAssignments;
    static std::vector<ObjectGuid> allGroupGuids;  // All guids that have ever been in the raid

    // Gather all group members (alive and dead, including those with ice tomb)
    std::vector<ObjectGuid> currentGuids;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member)
            currentGuids.push_back(member->GetGUID());
    }

    // Add any new GUIDs to our persistent list
    for (const ObjectGuid& guid : currentGuids)
    {
        if (std::find(allGroupGuids.begin(), allGroupGuids.end(), guid) == allGroupGuids.end())
        {
            allGroupGuids.push_back(guid);
        }
    }

    // Sort the complete guid list for consistency
    std::sort(allGroupGuids.begin(), allGroupGuids.end());

    Difficulty diff = bot->GetRaidDifficulty();
    // Determine group count (2 for 10m, 3 for 25m)
    int groupCount = (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC) ? 3 : 2;

    // Assign group indices to GUIDs that don't have assignments yet
    for (size_t i = 0; i < allGroupGuids.size(); ++i)
    {
        const ObjectGuid& guid = allGroupGuids[i];
        if (persistentGroupAssignments.find(guid) == persistentGroupAssignments.end())
        {
            // Assign to group based on their position in the sorted list
            persistentGroupAssignments[guid] = int(i) % groupCount;
        }
    }

    // Get this bot's group assignment
    auto it = persistentGroupAssignments.find(bot->GetGUID());
    if (it == persistentGroupAssignments.end())
        return false;

    int myGroupIndex = it->second;

    // Build group positions based on available tombs
    std::vector<Position> groupPositions;
    for (int i = 0; i < groupCount; ++i)
    {
        if (i < int(tombs.size()))
        {
            groupPositions.push_back(tombs[i]->GetPosition());
        }
        else
        {
            groupPositions.push_back(marker->GetPosition());
        }
    }

    // PRIORITY 1: Check if there are any tombs near our current position (within 8 yards)
    std::vector<Unit*> nearbyTombs;
    for (Unit* tomb : tombs)
    {
        if (tomb->GetExactDist2d(bot) <= 8.0f)
        {
            nearbyTombs.push_back(tomb);
        }
    }

    // PRIORITY 2: If no tombs nearby, find tombs near our assigned group position
    std::vector<Unit*> groupPositionTombs;
    if (nearbyTombs.empty())
    {
        for (Unit* tomb : tombs)
        {
            if (tomb->GetExactDist2d(groupPositions[myGroupIndex]) <= 8.0f)
            {
                groupPositionTombs.push_back(tomb);
            }
        }
    }

    // Select which tombs to use based on priority
    std::vector<Unit*> myTombs;
    std::vector<ObjectGuid> myTombGuids;

    if (!nearbyTombs.empty())
    {
        // Use tombs near current position (highest priority)
        myTombs = nearbyTombs;
        for (Unit* tomb : nearbyTombs)
        {
            myTombGuids.push_back(tomb->GetGUID());
        }
    }
    else if (!groupPositionTombs.empty())
    {
        // Use tombs near group position (medium priority)
        myTombs = groupPositionTombs;
        for (Unit* tomb : groupPositionTombs)
        {
            myTombGuids.push_back(tomb->GetGUID());
        }
    }
    else
    {
        // Fallback: use closest available tomb (lowest priority)
        Unit* closestTomb = nullptr;
        float closestDist = 999.0f;
        for (Unit* tomb : tombs)
        {
            float dist = tomb->GetExactDist2d(bot);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestTomb = tomb;
            }
        }
        if (closestTomb)
        {
            myTombs.push_back(closestTomb);
            myTombGuids.push_back(closestTomb->GetGUID());
        }
    }

    if (myTombs.empty())
        return false;

    // Pick the tomb with highest HP in our selection
    size_t bestIdx = 0;
    float bestHp = 0.0f;
    for (size_t i = 0; i < myTombs.size(); ++i)
    {
        float hp = myTombs[i]->GetHealthPct();
        if (i == 0 || hp > bestHp)
        {
            bestHp = hp;
            bestIdx = i;
        }
    }
    Unit* losTomb = myTombs[bestIdx];
    ObjectGuid losTombGuid = myTombGuids[bestIdx];

    // Calculate position for LOS (stand at least 6.5f behind the tomb from the bomb)
    float angle = marker->GetAngle(losTomb);
    float posX = losTomb->GetPositionX() + cos(angle) * 6.5f;
    float posY = losTomb->GetPositionY() + sin(angle) * 6.5f;
    float posZ = losTomb->GetPositionZ();

    // Always move to exact LOS position for safety
    float distToLosPos = bot->GetDistance2d(posX, posY);
    if (distToLosPos > 0.01f)
    {
        botAI->Reset();
        bot->AttackStop();
        return MoveTo(bot->GetMapId(), posX, posY, posZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Check if we are in LOS of the bomb (must be very close to calculated position)
    bool inLOS = (distToLosPos <= 0.01f);

    // RTI marker constants
    static constexpr uint8_t SKULL_ICON_INDEX = 7;
    static constexpr uint8_t CROSS_ICON_INDEX = 6;
    static constexpr uint8_t STAR_ICON_INDEX = 0;

    // If in LOS, handle RTI marking for group's tombs
    if (inLOS)
    {
        // Determine RTI marker for this group
        uint8_t iconIndex = 0;
        std::string rtiValue;
        if (myGroupIndex == 0)
        {
            iconIndex = SKULL_ICON_INDEX;
            rtiValue = "skull";
        }
        else if (myGroupIndex == 1)
        {
            iconIndex = CROSS_ICON_INDEX;
            rtiValue = "cross";
        }
        else if (myGroupIndex == 2)
        {
            iconIndex = STAR_ICON_INDEX;
            rtiValue = "star";
        }
        else
            return false;

        context->GetValue<std::string>("rti")->Set(rtiValue);

        // Find a tomb in our group with 45% or more HP to mark
        Unit* tombToMark = nullptr;
        for (size_t i = 0; i < myTombs.size(); ++i)
        {
            Unit* tomb = myTombs[i];
            if (tomb->IsAlive() && tomb->HealthAbovePct(45))
            {
                tombToMark = tomb;
                break;
            }
        }

        if (tombToMark)
        {
            // Check if this tomb is already marked with our group's icon
            ObjectGuid currentIcon = group->GetTargetIcon(iconIndex);
            Unit* currentIconUnit = botAI->GetUnit(currentIcon);
            if (!currentIconUnit || !currentIconUnit->IsAlive() || currentIconUnit != tombToMark)
            {
                // Mark the tomb with our group's target icon
                group->SetTargetIcon(iconIndex, bot->GetGUID(), tombToMark->GetGUID());
            }
        }
        else
        {
            // No tombs above 45% HP, remove marker if one exists
            ObjectGuid currentIcon = group->GetTargetIcon(iconIndex);
            if (!currentIcon.IsEmpty())
            {
                // Clear the marker for our group's icon
                group->SetTargetIcon(iconIndex, bot->GetGUID(), ObjectGuid::Empty);
            }
            Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
            bot->AttackStop();
            return true;
        }
    }

    return false;
}

// The Lich King
bool IccLichKingShadowTrapAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss || !botAI->IsTank(bot))
        return false;

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //-------CHEAT-------
        if (!bot->HasAura(SPELL_EXPERIENCED))
            bot->AddAura(SPELL_EXPERIENCED, bot);

        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        if (!bot->HasAura(SPELL_NO_THREAT) && !botAI->IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (!bot->HasAura(SPELL_PAIN_SUPPRESION))
            bot->AddAura(SPELL_PAIN_SUPPRESION, bot);
        //-------CHEAT-------
    }

    // Define ICC_LICH_POSITION and circle parameters
    const float X = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionX();
    const float Y = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionY();
    const float Z = ICC_LICH_KING_ASSISTHC_POSITION.GetPositionZ();
    const float CIRCLE_RADIUS = 20.0f;
    const float SAFE_DISTANCE = 12.0f;
    const int TEST_POSITIONS = 16;
    const float ANGLE_STEP = 2 * M_PI / TEST_POSITIONS;

    // Find all nearby shadow traps
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<ObjectGuid> trapGuids;
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_SHADOW_TRAP)
            continue;
        if (bot->GetDistance(unit) < SAFE_DISTANCE + 5.0f)
        {
            trapGuids.push_back(npc);
        }
    }

    if (trapGuids.empty())
        return false;

    // Check if current position is already safe
    bool currentPositionSafe = true;
    for (auto& trapGuid : trapGuids)
    {
        Unit* trap = botAI->GetUnit(trapGuid);
        if (!trap)
            continue;
        if (bot->GetDistance(trap) < SAFE_DISTANCE)
        {
            currentPositionSafe = false;
            break;
        }
    }

    // If current position is safe, no need to move
    if (currentPositionSafe)
        return false;

    // Calculate current angle relative to ICC_LICH_POSITION
    float currentX = bot->GetPositionX() - X;
    float currentY = bot->GetPositionY() - Y;
    float currentAngle = atan2(currentY, currentX);

    // Test clockwise positions first, then opposite position
    std::vector<float> testAngles;
    // Add clockwise positions
    for (int i = 1; i <= TEST_POSITIONS; ++i)
    {
        testAngles.push_back(currentAngle - (ANGLE_STEP * i));
    }
    // Add opposite position as fallback
    testAngles.push_back(currentAngle + M_PI);

    // Test all positions
    for (float testAngle : testAngles)
    {
        // Calculate position on circle
        float testX = X + cos(testAngle) * CIRCLE_RADIUS;
        float testY = Y + sin(testAngle) * CIRCLE_RADIUS;
        float testZ = Z;

        // Update Z coordinate for terrain
        bot->UpdateAllowedPositionZ(testX, testY, testZ);

        // Check line of sight
        if (!bot->IsWithinLOS(testX, testY, testZ))
            continue;

        // Check if this position is safe from all traps
        bool isSafe = true;
        for (auto& trapGuid : trapGuids)
        {
            Unit* trap = botAI->GetUnit(trapGuid);
            if (!trap)
                continue;
            float distToTrap = sqrt(pow(testX - trap->GetPositionX(), 2) + pow(testY - trap->GetPositionY(), 2));
            if (distToTrap < SAFE_DISTANCE)
            {
                isSafe = false;
                break;
            }
        }

        // Found a safe spot - move there
        if (isSafe)
        {
            // Remove botAI->Reset() as it might interfere with movement
            MoveTo(bot->GetMapId(), testX, testY, testZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                   true, false);
        }
    }

    // No safe position found
    return false;
}

bool IccLichKingNecroticPlagueAction::Execute(Event event)
{
    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);
    // Only execute if we have the plague
    if (!hasPlague)
        return false;

    // Find closest shambling horror
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestHorror = nullptr;
    float minHorrorDist = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_SHAMBLING_HORROR1 || entry == NPC_SHAMBLING_HORROR2 ||
            entry == NPC_SHAMBLING_HORROR3 || entry == NPC_SHAMBLING_HORROR4)
        {
            float distance = bot->GetDistance(unit);
            if (distance < minHorrorDist)
            {
                minHorrorDist = distance;
                closestHorror = unit;
            }
        }
    }

    // If we found a shambling horror, handle movement
    if (closestHorror)
    {
        // If we're close enough, stop and return success
        if (minHorrorDist <= 2.0f)
        {
            bot->StopMoving();
            return true;
        }

        // We need to move to the horror
        botAI->Reset();
        MoveTo(closestHorror, 2.0f, MovementPriority::MOVEMENT_FORCED);

        return false;  // Still moving, not finished yet
    }

    // No shambling horror found, but we have plague - this shouldn't happen normally
    return false;
}

bool IccLichKingWinterAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    Unit* iceSphere = AI_VALUE2(Unit*, "find target", "ice sphere");

    bool isVictim = false;
    if (iceSphere && iceSphere->GetVictim() == bot && !botAI->IsTank(bot))
        isVictim = true;

    // First priority: Get out of Defile if we're in one
    if (!IsPositionSafeFromDefile(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 3.0f))
    {
        // Find nearest safe position (use tank position as fallback)
        const Position* safePos = botAI->IsTank(bot) ? GetMainTankPosition() : GetMainTankRangedPosition();
        TryMoveToPosition(safePos->GetPositionX(), safePos->GetPositionY(), 840.857f, true);
        return true;
    }

    float currentDistance = bot->GetDistance2d(boss);

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //------CHEAT-------
        if (!bot->HasAura(SPELL_EXPERIENCED))
            bot->AddAura(SPELL_EXPERIENCED, bot);

        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        if (!bot->HasAura(SPELL_NO_THREAT) && !botAI->IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (!bot->HasAura(SPELL_PAIN_SUPPRESION))
            bot->AddAura(SPELL_PAIN_SUPPRESION, bot);
        //------CHEAT-------
    }

    if (currentDistance < 35.0f && !bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

    // Handle group target management
    if (Group* group = bot->GetGroup())
    {
        const ObjectGuid currentSkullTarget = group->GetTargetIcon(7);
        if (!currentSkullTarget.IsEmpty())
        {
            Unit* skullTarget = ObjectAccessor::GetUnit(*bot, currentSkullTarget);
            group->SetTargetIcon(7, bot->GetGUID(), ObjectGuid::Empty);
        }
    }

    if (isVictim)
        MoveFromGroup(6.0f);

    if (!isVictim)
    {
        HandlePositionCorrection();

        // Handle tank positioning and add management FIRST
        HandleTankPositioning();  // New method that handles both main and assist tanks

        // Then handle other roles
        HandleMeleePositioning();
        HandleRangedPositioning();
    }

    return false;
}

void IccLichKingWinterAction::HandlePositionCorrection()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    // Fix underground bug
    if (abs(bot->GetPositionZ() - 840.857f) > 1.0f)
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), 840.857f, bot->GetOrientation());

    // Reset targeting for specific conditions
    if (currentTarget && boss && currentTarget == boss)
        botAI->Reset();

    if (botAI->IsTank(bot) && currentTarget &&
        ((currentTarget->GetEntry() == NPC_ICE_SPHERE1 || currentTarget->GetEntry() == NPC_ICE_SPHERE2 ||
          currentTarget->GetEntry() == NPC_ICE_SPHERE3 || currentTarget->GetEntry() == NPC_ICE_SPHERE4)))
        botAI->Reset();
}

const Position* IccLichKingWinterAction::GetMainTankPosition()
{
    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
    {
        // FIXED: When no main tank, use the bot with lowest GUID to determine position
        // This ensures ALL bots make the same decision collectively

        Unit* referenceBot = nullptr;
        ObjectGuid lowestGuid;

        // Find the bot with lowest GUID in the group
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && member->IsInWorld())
                {
                    if (lowestGuid.IsEmpty() || member->GetGUID() < lowestGuid)
                    {
                        lowestGuid = member->GetGUID();
                        referenceBot = member;
                    }
                }
            }
        }

        // If no group or reference bot found, fall back to current bot
        if (!referenceBot)
            referenceBot = bot;

        // Use the reference bot's position to determine closest tank position
        float dist1 =
            referenceBot->GetDistance2d(ICC_LK_FROST1_POSITION.GetPositionX(), ICC_LK_FROST1_POSITION.GetPositionY());
        float dist2 =
            referenceBot->GetDistance2d(ICC_LK_FROST2_POSITION.GetPositionX(), ICC_LK_FROST2_POSITION.GetPositionY());
        float dist3 =
            referenceBot->GetDistance2d(ICC_LK_FROST3_POSITION.GetPositionX(), ICC_LK_FROST3_POSITION.GetPositionY());

        if (dist2 < dist1 && dist2 < dist3)
            return &ICC_LK_FROST2_POSITION;
        else if (dist3 < dist1 && dist3 < dist2)
            return &ICC_LK_FROST3_POSITION;
        else
            return &ICC_LK_FROST1_POSITION;
    }

    // Calculate which position the main tank is closest to
    float dist1 = mainTank->GetDistance2d(ICC_LK_FROST1_POSITION.GetPositionX(), ICC_LK_FROST1_POSITION.GetPositionY());
    float dist2 = mainTank->GetDistance2d(ICC_LK_FROST2_POSITION.GetPositionX(), ICC_LK_FROST2_POSITION.GetPositionY());
    float dist3 = mainTank->GetDistance2d(ICC_LK_FROST3_POSITION.GetPositionX(), ICC_LK_FROST3_POSITION.GetPositionY());

    if (dist2 < dist1 && dist2 < dist3)
        return &ICC_LK_FROST2_POSITION;
    else if (dist3 < dist1 && dist3 < dist2)
        return &ICC_LK_FROST3_POSITION;
    else
        return &ICC_LK_FROST1_POSITION;
}

const Position* IccLichKingWinterAction::GetMainTankRangedPosition()
{
    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
    {
        // FIXED: When no main tank, use the bot with lowest GUID to determine position
        // This ensures ALL bots make the same decision collectively

        Unit* referenceBot = nullptr;
        ObjectGuid lowestGuid;

        // Find the bot with lowest GUID in the group
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && member->IsInWorld())
                {
                    if (lowestGuid.IsEmpty() || member->GetGUID() < lowestGuid)
                    {
                        lowestGuid = member->GetGUID();
                        referenceBot = member;
                    }
                }
            }
        }

        // If no group or reference bot found, fall back to current bot
        if (!referenceBot)
            referenceBot = bot;

        // Use the reference bot's position to determine closest ranged position
        float dist1 =
            referenceBot->GetDistance2d(ICC_LK_FROSTR1_POSITION.GetPositionX(), ICC_LK_FROSTR1_POSITION.GetPositionY());
        float dist2 =
            referenceBot->GetDistance2d(ICC_LK_FROSTR2_POSITION.GetPositionX(), ICC_LK_FROSTR2_POSITION.GetPositionY());
        float dist3 =
            referenceBot->GetDistance2d(ICC_LK_FROSTR3_POSITION.GetPositionX(), ICC_LK_FROSTR3_POSITION.GetPositionY());

        if (dist2 < dist1 && dist2 < dist3)
            return &ICC_LK_FROSTR2_POSITION;
        else if (dist3 < dist1 && dist3 < dist2)
            return &ICC_LK_FROSTR3_POSITION;
        else
            return &ICC_LK_FROSTR1_POSITION;
    }

    // Map main tank's melee position to corresponding ranged position
    const Position* tankMeleePos = GetMainTankPosition();

    if (tankMeleePos == &ICC_LK_FROST1_POSITION)
        return &ICC_LK_FROSTR1_POSITION;
    else if (tankMeleePos == &ICC_LK_FROST2_POSITION)
        return &ICC_LK_FROSTR2_POSITION;
    else
        return &ICC_LK_FROSTR3_POSITION;
}

bool IccLichKingWinterAction::IsPositionSafeFromDefile(float x, float y, float z, float minSafeDistance)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return true;  // No boss, assume safe

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    const float BASE_RADIUS = 6.0f;
    const float SAFETY_MARGIN = 3.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)
        {
            // Calculate current defile radius including growth
            float currentRadius = BASE_RADIUS;
            Aura* growAura = nullptr;

            // Find growth aura (you'll need to define DEFILE_AURAS array)
            for (size_t i = 0; i < DEFILE_AURA_COUNT; i++)
            {
                growAura = unit->GetAura(DEFILE_AURAS[i]);
                if (growAura)
                    break;
            }

            if (growAura)
            {
                uint8 stacks = growAura->GetStackAmount();
                float growthMultiplier = (bot->GetRaidDifficulty() == RAID_DIFFICULTY_10MAN_HEROIC ||
                                          bot->GetRaidDifficulty() == RAID_DIFFICULTY_10MAN_NORMAL)
                                             ? 1.4f
                                             : 0.95f;
                currentRadius = BASE_RADIUS + (stacks * growthMultiplier);
            }

            float dx = x - unit->GetPositionX();
            float dy = y - unit->GetPositionY();
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < (currentRadius + SAFETY_MARGIN + minSafeDistance))
                return false;
        }
    }
    return true;
}

bool IccLichKingWinterAction::TryMoveToPosition(float targetX, float targetY, float targetZ, bool isForced)
{
    float currentX = bot->GetPositionX();
    float currentY = bot->GetPositionY();
    float currentZ = bot->GetPositionZ();

    float dx = targetX - currentX;
    float dy = targetY - currentY;
    float dz = targetZ - currentZ;
    float distance = sqrtf(dx * dx + dy * dy + dz * dz);

    if (distance < 0.1f)
        return true;  // Already at the position

    dx /= distance;
    dy /= distance;

    // First check if direct path is safe
    if (bot->IsWithinLOS(targetX, targetY, targetZ) && IsPositionSafeFromDefile(targetX, targetY, targetZ, 3.0f))
    {
        if (isForced)
            botAI->Reset();

        MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED,
               true, false);
        return true;
    }

    // If direct path isn't safe, try to find a safe path around defiles
    const int MAX_ATTEMPTS = 8;
    const float ANGLE_STEP = M_PI / 4.0f;
    float attemptDistance = std::min(10.0f, distance);

    for (int i = 0; i < MAX_ATTEMPTS; i++)
    {
        float angle = i * ANGLE_STEP;
        float offsetX = attemptDistance * cos(angle);
        float offsetY = attemptDistance * sin(angle);

        // Try positions clockwise and counter-clockwise
        for (int direction = -1; direction <= 1; direction += 2)
        {
            if (i == 0 && direction == 1)
                continue;  // Skip duplicate first attempt

            float testX = currentX + dx * attemptDistance + offsetX * direction;
            float testY = currentY + dy * attemptDistance + offsetY * direction;
            float testZ = targetZ;

            if (bot->IsWithinLOS(testX, testY, testZ) && IsPositionSafeFromDefile(testX, testY, testZ, 3.0f))
            {
                if (isForced)
                    botAI->Reset();

                MoveTo(bot->GetMapId(), testX, testY, testZ, false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED, true, false);
                return false;  // Not at final position yet
            }
        }
    }

    // If no safe path found, just move directly (better than standing in defile)
    if (isForced)
        botAI->Reset();

    MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED,
           true, false);
    return false;
}

// Helper function to check if a unit is a valid collectible add
bool IccLichKingWinterAction::IsValidCollectibleAdd(Unit* unit)
{
    if (!unit || !unit->IsAlive())
        return false;

    uint32 entry = unit->GetEntry();

    // Only spirits, shambling horrors, and ghouls are valid collectible adds
    return (entry == NPC_SHAMBLING_HORROR1 || entry == NPC_SHAMBLING_HORROR2 || entry == NPC_SHAMBLING_HORROR3 ||
            entry == NPC_SHAMBLING_HORROR4 || entry == NPC_RAGING_SPIRIT1 || entry == NPC_RAGING_SPIRIT2 ||
            entry == NPC_RAGING_SPIRIT3 || entry == NPC_RAGING_SPIRIT4 || entry == NPC_DRUDGE_GHOUL1 ||
            entry == NPC_DRUDGE_GHOUL2 || entry == NPC_DRUDGE_GHOUL3 || entry == NPC_DRUDGE_GHOUL4);
}

// FIXED HandleTankPositioning method
void IccLichKingWinterAction::HandleTankPositioning()
{
    if (!botAI->IsTank(bot))
        return;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return;

    // Get the target position based on main tank's choice
    const Position* targetPos = GetMainTankPosition();

    // First check if current position is safe
    if (!IsPositionSafeFromDefile(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 3.0f))
    {
        // If in defile, prioritize getting out
        TryMoveToPosition(targetPos->GetPositionX(), targetPos->GetPositionY(), 840.857f, true);
        return;
    }

    float distToTarget = bot->GetDistance2d(targetPos->GetPositionX(), targetPos->GetPositionY());

    // MAIN TANK: Always stay at tank position
    if (botAI->IsMainTank(bot))
    {
        // Main tank should always maintain position at tank spot
        if (distToTarget > 2.0f)
        {
            float targetX = targetPos->GetPositionX();
            float targetY = targetPos->GetPositionY();
            float targetZ = 840.857f;

            TryMoveToPosition(targetX, targetY, targetZ, true);
            return;  // Don't do add management until in position
        }

        // Once in position, handle add management from tank position
        HandleMainTankAddManagement(boss, targetPos);
    }
    // ASSIST TANK: More flexible positioning based on add collection
    else if (botAI->IsAssistTank(bot))
    {
        // First ensure we're reasonably close to tank area
        if (distToTarget > 15.0f)
        {
            float targetX = targetPos->GetPositionX() + 3.0f;  // Slight offset from main tank
            float targetY = targetPos->GetPositionY() + 2.0f;
            float targetZ = 840.857f;

            TryMoveToPosition(targetX, targetY, targetZ, true);
            return;
        }

        // Handle assist tank add collection and positioning
        HandleAssistTankAddManagement(boss, targetPos);
    }
}

// Updated HandleMeleePositioning method - only for non-tanks
void IccLichKingWinterAction::HandleMeleePositioning()
{
    // Skip if this is a tank - they have their own positioning logic
    if (botAI->IsTank(bot))
        return;
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    // Handle melee positioning behind target (for DPS only)
    if (currentTarget && !botAI->IsRanged(bot) && currentTarget->isInFront(bot) && currentTarget->IsAlive() &&
        currentTarget->GetEntry() != NPC_THE_LICH_KING && currentTarget->GetEntry() != NPC_ICE_SPHERE1 &&
        currentTarget->GetEntry() != NPC_ICE_SPHERE2 && currentTarget->GetEntry() != NPC_ICE_SPHERE3 && currentTarget->GetEntry() != NPC_ICE_SPHERE4)
    {
        // Calculate desired position (4.0f behind the target)
        float orientation = currentTarget->GetOrientation() + M_PI + M_PI / 8;
        float x = currentTarget->GetPositionX();
        float y = currentTarget->GetPositionY();
        float z = bot->GetPositionZ();
        float targetX = x + cos(orientation) * 4.0f;
        float targetY = y + sin(orientation) * 4.0f;
        Position botPos = bot->GetPosition();
        float dx = targetX - botPos.GetPositionX();
        float dy = targetY - botPos.GetPositionY();
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= 1.0f)
            return;

        // Move in increments of 2 yards toward the target position
        float moveDistance = std::min(2.0f, distance);
        float normalizedDx = dx / distance;
        float normalizedDy = dy / distance;

        float newX = botPos.GetPositionX() + normalizedDx * moveDistance;
        float newY = botPos.GetPositionY() + normalizedDy * moveDistance;

        TryMoveToPosition(newX, newY, z, false);
    }
    // Handle non-ranged DPS positioning - USE MAIN TANK'S POSITION
    if (!botAI->IsRanged(bot))
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
        const Position* targetPos = GetMainTankPosition();
        float distToTarget = bot->GetDistance2d(targetPos->GetPositionX(), targetPos->GetPositionY());
        if (distToTarget > 8.0f)
        {
            float targetX = targetPos->GetPositionX() - 5.0f;
            float targetY = targetPos->GetPositionY() + 5.0f;
            float targetZ = 840.857f;
            if (boss && !boss->HealthAbovePct(50))
            {
                targetX = targetPos->GetPositionX();
                targetY = targetPos->GetPositionY();
                targetZ = 840.857f;
            }

            // Move in increments of 2 yards toward the tank position
            Position botPos = bot->GetPosition();
            float dx = targetX - botPos.GetPositionX();
            float dy = targetY - botPos.GetPositionY();
            float distance = sqrt(dx * dx + dy * dy);

            if (distance > 2.0f)
            {
                float normalizedDx = dx / distance;
                float normalizedDy = dy / distance;

                float newX = botPos.GetPositionX() + normalizedDx * 2.0f;
                float newY = botPos.GetPositionY() + normalizedDy * 2.0f;

                TryMoveToPosition(newX, newY, targetZ);
            }
            else
            {
                TryMoveToPosition(targetX, targetY, targetZ);
            }
        }
    }
}

// Updated HandleRangedPositioning method
void IccLichKingWinterAction::HandleRangedPositioning()
{
    if (!botAI->IsRanged(bot))
        return;

    // Get the ranged position based on main tank's choice
    const Position* targetPos = GetMainTankRangedPosition();

    // First check if current position is safe
    if (!IsPositionSafeFromDefile(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 3.0f))
    {
        // If in defile, prioritize getting out
        TryMoveToPosition(targetPos->GetPositionX(), targetPos->GetPositionY(), 840.857f, true);
        return;
    }

    float distToTarget = bot->GetDistance2d(targetPos->GetPositionX(), targetPos->GetPositionY());

    if (distToTarget > 2.0f)
    {
        float targetX = targetPos->GetPositionX();
        float targetY = targetPos->GetPositionY();
        float targetZ = 840.857f;

        TryMoveToPosition(targetX, targetY, targetZ);
    }

    // Handle sphere targeting for ranged DPS
    if (botAI->IsRangedDps(bot))
    {
        bool hasHunter = false;
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && member->IsInWorld() && member->getClass() == CLASS_HUNTER)
                {
                    hasHunter = true;
                    break;
                }
            }
        }

        if (bot->getClass() == CLASS_HUNTER || !hasHunter)
        {
            Unit* currentTarget = bot->GetVictim();
            if (currentTarget && currentTarget->IsAlive())
            {
                uint32 entry = currentTarget->GetEntry();
                if (entry == NPC_ICE_SPHERE1 || entry == NPC_ICE_SPHERE2 || entry == NPC_ICE_SPHERE3 || entry == NPC_ICE_SPHERE4)
                {
                    bot->SetFacingToObject(currentTarget);
                    Attack(currentTarget);
                    return;
                }
            }

            Unit* closestSphere = nullptr;
            float closestDist = 100.0f;
            GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto& npc : npcs)
            {
                Unit* unit = botAI->GetUnit(npc);
                if (!unit || !unit->IsAlive())
                    continue;
                uint32 entry = unit->GetEntry();
                if (entry == NPC_ICE_SPHERE1 || entry == NPC_ICE_SPHERE2 || entry == NPC_ICE_SPHERE3 || entry == NPC_ICE_SPHERE4)
                {
                    float dist = bot->GetDistance(unit);
                    if (!closestSphere || dist < closestDist)
                    {
                        closestSphere = unit;
                        closestDist = dist;
                    }
                }
            }
            if (closestSphere)
            {
                bot->SetTarget(closestSphere->GetGUID());
                bot->SetFacingToObject(closestSphere);
                Attack(closestSphere);
            }
        }
    }
}

void IccLichKingWinterAction::HandleMainTankAddManagement(Unit* boss, const Position* tankPos)
{
    if (!botAI->IsMainTank(bot))
        return;

    // First, ensure we're at the correct tank position
    float distToTankPos = bot->GetDistance2d(tankPos->GetPositionX(), tankPos->GetPositionY());
    if (distToTankPos > 3.0f)
    {
        TryMoveToPosition(tankPos->GetPositionX(), tankPos->GetPositionY(), 840.857f, true);
        return;  // Wait until we're in position
    }

    // Get all valid adds in the encounter area
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    std::vector<Unit*> validAdds;
    Unit* currentTarget = bot->GetVictim();

    // Collect all valid adds
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!IsValidCollectibleAdd(unit))
            continue;

        validAdds.push_back(unit);
    }

    // If we have no adds, clear target if needed
    if (validAdds.empty())
    {
        if (currentTarget && !IsValidCollectibleAdd(currentTarget))
        {
            bot->SetTarget(ObjectGuid::Empty);
        }
        return;
    }

    // Strategy for add management:
    // 1. First priority: Adds attacking non-tanks
    // 2. Second priority: Adds not attacking us
    // 3. Third priority: All other valid adds
    Unit* priorityAdd = nullptr;
    Unit* secondaryAdd = nullptr;
    Unit* otherAdd = nullptr;

    for (Unit* add : validAdds)
    {
        Unit* addVictim = add->GetVictim();

        // Highest priority: Adds attacking non-tanks
        if (addVictim && addVictim->IsPlayer() && !botAI->IsTank(addVictim->ToPlayer()))
        {
            if (!priorityAdd || bot->GetDistance(add) < bot->GetDistance(priorityAdd))
            {
                priorityAdd = add;
            }
            continue;
        }

        // Medium priority: Adds not attacking us
        if (addVictim != bot)
        {
            if (!secondaryAdd || bot->GetDistance(add) < bot->GetDistance(secondaryAdd))
            {
                secondaryAdd = add;
            }
            continue;
        }

        // Lowest priority: All other valid adds
        if (!otherAdd || bot->GetDistance(add) < bot->GetDistance(otherAdd))
        {
            otherAdd = add;
        }
    }

    // Select the highest priority add available
    Unit* targetAdd = priorityAdd ? priorityAdd : (secondaryAdd ? secondaryAdd : otherAdd);

    if (targetAdd)
    {
        float addDist = bot->GetDistance(targetAdd);

        // If add is close enough (within melee range), attack it
        if (addDist < 10.0f)
        {
            // If we're not already attacking this add, switch to it
            if (currentTarget != targetAdd)
            {
                bot->SetTarget(targetAdd->GetGUID());
                bot->SetFacingToObject(targetAdd);
                Attack(targetAdd);
            }
        }
        else
        {
            // Add is too far - move toward it while staying near tank position
            float moveX = targetAdd->GetPositionX();
            float moveY = targetAdd->GetPositionY();

            // Don't move too far from tank position (max 15 yards)
            float maxPullDistance = 15.0f;
            float pullRatio = std::min(1.0f, maxPullDistance / addDist);

            float adjustedX = tankPos->GetPositionX() + (moveX - tankPos->GetPositionX()) * pullRatio;
            float adjustedY = tankPos->GetPositionY() + (moveY - tankPos->GetPositionY()) * pullRatio;

            TryMoveToPosition(adjustedX, adjustedY, 840.857f, false);

            // Still try to attack while moving
            bot->SetTarget(targetAdd->GetGUID());
            bot->SetFacingToObject(targetAdd);
            Attack(targetAdd);
        }
    }
    else if (currentTarget && !IsValidCollectibleAdd(currentTarget))
    {
        // Clear invalid target
        bot->SetTarget(ObjectGuid::Empty);
    }
}

void IccLichKingWinterAction::HandleAssistTankAddManagement(Unit* boss, const Position* tankPos)
{
    if (!botAI->IsAssistTank(bot))
        return;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
        return;

    // Look for priority adds that need to be collected
    Unit* targetAdd = nullptr;
    float closestDist = FLT_MAX;
    bool foundPriorityAdd = false;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    // Priority 1: Adds attacking non-tanks (players/healers)
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!IsValidCollectibleAdd(unit))
            continue;

        Unit* addVictim = unit->GetVictim();
        if (addVictim && addVictim->IsPlayer() && !botAI->IsTank(addVictim->ToPlayer()))
        {
            float addDist = bot->GetDistance(unit);
            if (addDist < closestDist)
            {
                targetAdd = unit;
                closestDist = addDist;
                foundPriorityAdd = true;
            }
        }
    }

    // Priority 2: Adds not attacking main tank (if no priority adds found)
    if (!foundPriorityAdd)
    {
        closestDist = FLT_MAX;
        for (auto i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (!IsValidCollectibleAdd(unit))
                continue;

            // Only target adds that are NOT attacking the main tank
            if (unit->GetVictim() != mainTank)
            {
                float addDist = bot->GetDistance(unit);
                if (addDist < closestDist)
                {
                    targetAdd = unit;
                    closestDist = addDist;
                }
            }
        }
    }

    if (targetAdd)
    {
        // Calculate position between add and main tank
        float pullX = (targetAdd->GetPositionX() + tankPos->GetPositionX()) / 2;
        float pullY = (targetAdd->GetPositionY() + tankPos->GetPositionY()) / 2;

        // Move to intercept position if add is far from main tank
        if (targetAdd->GetDistance2d(tankPos->GetPositionX(), tankPos->GetPositionY()) > 10.0f)
        {
            if (bot->GetDistance2d(pullX, pullY) > 3.0f)
            {
                TryMoveToPosition(pullX, pullY, 840.857f, false);
            }
        }
        // Otherwise move toward the add
        else if (closestDist > 5.0f)
        {
            TryMoveToPosition(targetAdd->GetPositionX(), targetAdd->GetPositionY(), 840.857f, false);
        }

        // Attack the add
        bot->SetTarget(targetAdd->GetGUID());
        bot->SetFacingToObject(targetAdd);
        Attack(targetAdd);
    }
    else
    {
        // No adds to collect, position near main tank
        float distToTankPos = bot->GetDistance2d(tankPos->GetPositionX(), tankPos->GetPositionY());
        if (distToTankPos > 2.0f)
        {
            float X = mainTank->GetPositionX();
            float Y = mainTank->GetPositionY();
            TryMoveToPosition(X, Y, 840.857f, false);
        }

        // Check current target validity
        Unit* currentTarget = bot->GetVictim();
        if (currentTarget && !IsValidCollectibleAdd(currentTarget))
        {
            bot->SetTarget(ObjectGuid::Empty);
        }
    }
}

bool IccLichKingAddsAction::Execute(Event event)
{
    if (bot->HasAura(SPELL_HARVEST_SOUL_VALKYR))  // Don't process actions if bot is picked up by Val'kyr
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //------CHEAT-------
        if (!bot->HasAura(SPELL_EXPERIENCED))
            bot->AddAura(SPELL_EXPERIENCED, bot);

        if (!bot->HasAura(SPELL_AGEIS_OF_DALARAN))
            bot->AddAura(SPELL_AGEIS_OF_DALARAN, bot);

        if (boss && boss->HealthBelowPct(60) && boss->HealthAbovePct(40) && !bot->HasAura(SPELL_EMPOWERED_BLOOD))
            bot->AddAura(SPELL_EMPOWERED_BLOOD, bot);

        if (!bot->HasAura(SPELL_NO_THREAT) && !botAI->IsTank(bot))
            bot->AddAura(SPELL_NO_THREAT, bot);

        if (!bot->HasAura(SPELL_PAIN_SUPPRESION))
            bot->AddAura(SPELL_PAIN_SUPPRESION, bot);
        //------CHEAT-------
    }

    Unit* spiritWarden = AI_VALUE2(Unit*, "find target", "spirit warden");
    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);
    Unit* terenasMenethilHC = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);

    Group* group = bot->GetGroup();
    if (group && boss && boss->HealthAbovePct(71))
    {
        constexpr uint8_t skullIconId = 7;
        ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);
        if (skullGuid != boss->GetGUID())
            group->SetTargetIcon(skullIconId, bot->GetGUID(), boss->GetGUID());
    }

    //-----------Valkyr bot suicide if group fails to kill Valkyr in time------------- comment out if you dont want it
    if (bot->HasAura(30440))  // Random aura tracking whether bot has fallen off edge / been thrown by Val'kyr
    {
        if (bot->GetPositionZ() > 779.0f)
            return JumpTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), 740.01f);
        else
        {
            bot->Kill(bot, bot);  // If bot has jumped past the kill Z (780), **Now it is fixed and bots will actually die instead of beeing frozen**
            return true;
        }
    }

    bool hasWinterAura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
                 boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4)))
        hasWinterAura = true;

    bool hasWinter2Aura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
                 boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8)))
        hasWinter2Aura = true;

    if (boss && boss->GetHealthPct() < 70 && boss->GetHealthPct() > 40 && !hasWinterAura &&
        !hasWinter2Aura)  // If boss is in p2, check if bot has been thrown off platform
    {
        float dx = bot->GetPositionX() - 503.0f;
        float dy = bot->GetPositionY() - (-2124.0f);
        float distance = sqrt(dx * dx + dy * dy);  // Calculate distance from the center of the platform

        if (distance > 52.0f && distance < 70.0f &&
            bot->GetPositionZ() > 844)  // If bot has fallen off edge, distance is over 52
        {
            bot->AddAura(30440, bot);  // Apply random 30 sec aura to track that we've initiated a jump
            return JumpTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                          740.01f);  // Start jumping to the abyss
        }
    }
    //-----------Valkyr bot suicide if group fails to kill Valkyr in time------------- comment out if you dont want it
    
    // Handle teleportation fixes
    HandleTeleportationFixes(diff, terenasMenethilHC);

    // Handle heroic mode spirit bomb avoidance for main tank
    if (HandleSpiritBombAvoidance(diff, terenasMenethilHC))
        return true;

    // Handle non-main tank positioning in heroic mode
    HandleHeroicNonTankPositioning(diff, terenasMenethilHC);

    // Handle spirit marking and targeting in heroic mode
    HandleSpiritMarkingAndTargeting(diff, terenasMenethilHC);

    if (terenasMenethilHC)
        return false;

    // Handle quake mechanics
    if (HandleQuakeMechanics(boss))
        return true;

    // Handle shambling horror interactions
    HandleShamblingHorrors(boss, hasPlague);

    // Handle assist tank add management
    if (HandleAssistTankAddManagement(boss, diff))
        return true;

    // Handle melee positioning
    HandleMeleePositioning(boss, hasPlague, diff);

    // Handle main tank targeting in heroic
    HandleMainTankTargeting(boss, diff);

    // Handle non-tank positioning in heroic
    HandleNonTankHeroicPositioning(boss, diff, hasPlague);

    // Handle ranged positioning
    HandleRangedPositioning(boss, hasPlague, diff);

    // Handle defile mechanics
    HandleDefileMechanics(boss, diff);

    // Handle Val'kyr mechanics
    HandleValkyrMechanics(diff);

    // Handle vile spirit mechanics
    HandleVileSpiritMechanics();

    return false;
}

void IccLichKingAddsAction::HandleTeleportationFixes(Difficulty diff, Unit* terenasMenethilHC)
{
    // temp soultion for bots when they get teleport far away into another dimension (they are unable to attack spirit
    // warden) in heroic they will be in safe spot while player is surviving vile spirits
    if (!(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC) &&
        abs(bot->GetPositionY() - -2095.7915f) > 200.0f)
    {
        bot->TeleportTo(bot->GetMapId(), ICC_LICH_KING_ADDS_POSITION.GetPositionX(),
                        ICC_LICH_KING_ADDS_POSITION.GetPositionY(), ICC_LICH_KING_ADDS_POSITION.GetPositionZ(),
                        bot->GetOrientation());
    }

    // temp solution for bots going underground due to buggy ice platfroms and adds that go underground
    if (abs(bot->GetPositionZ() - 840.857f) > 1.0f && !botAI->GetAura("Harvest Soul", bot, false, false) &&
        !botAI->GetAura("Harvest Souls", bot, false, false))
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), 840.857f, bot->GetOrientation());

    if (abs(bot->GetPositionZ() - 1049.865f) > 5.0f && botAI->GetAura("Harvest Soul", bot, false, false) &&
        terenasMenethilHC)
        bot->TeleportTo(bot->GetMapId(), terenasMenethilHC->GetPositionX(), terenasMenethilHC->GetPositionY(), 1049.865f,
                        bot->GetOrientation());
}

bool IccLichKingAddsAction::HandleSpiritBombAvoidance(Difficulty diff, Unit* terenasMenethilHC)
{
    if (!botAI->IsMainTank(bot) || !terenasMenethilHC || !diff ||
        !(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return false;

    std::map<ObjectGuid, Unit*> spiritBombs;

    // Gather all spirit bombs using their GUIDs for reliable tracking
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npcGuid : npcs1)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_SPIRIT_BOMB)
        {
            spiritBombs[npcGuid] = unit;
        }
    }

    // Only proceed if there are actually spirit bombs present
    if (spiritBombs.empty())
        return false;

    const float SAFE_DISTANCE = 14.0f;        // Minimum safe horizontal distance
    const float SAFE_HEIGHT_DIFF = 12.0f;     // Safe if Z difference is greater than this
    const float BOMB_DENSITY_RADIUS = 10.0f;  // Radius for counting nearby bombs
    const float BOMB_COUNT_PENALTY = 10.0f;   // Score penalty per bomb in vicinity
    const float MAX_HEIGHT_DIFF = 8.0f;       // Increased from 5.0f for more flexibility

    // First check if current position is already safe
    bool currentPositionSafe = true;
    float minDistanceToAnyBomb = std::numeric_limits<float>::max();

    // Clean up invalid bombs and check current position safety
    auto it = spiritBombs.begin();
    while (it != spiritBombs.end())
    {
        Unit* verifiedBomb = botAI->GetUnit(it->first);
        if (!verifiedBomb || !verifiedBomb->IsAlive() || verifiedBomb->GetEntry() != NPC_SPIRIT_BOMB)
        {
            it = spiritBombs.erase(it);
            continue;
        }

        float dx = bot->GetPositionX() - verifiedBomb->GetPositionX();
        float dy = bot->GetPositionY() - verifiedBomb->GetPositionY();
        float dz = bot->GetPositionZ() - verifiedBomb->GetPositionZ();
        float horizontalDistance = sqrt(dx * dx + dy * dy);
        float verticalDistance = fabs(dz);

        minDistanceToAnyBomb = std::min(minDistanceToAnyBomb, horizontalDistance);

        // Position is dangerous if horizontally close AND not high enough above/below
        if (horizontalDistance < SAFE_DISTANCE && verticalDistance <= SAFE_HEIGHT_DIFF)
        {
            currentPositionSafe = false;
        }
        ++it;
    }

    // If no valid bombs remain after cleanup, exit early
    if (spiritBombs.empty())
    {
        return false;
    }

    // Only move if current position is unsafe
    if (!currentPositionSafe)
    {
        float bestScore = -std::numeric_limits<float>::max();
        float bestX = 0, bestY = 0, bestZ = 0;
        bool foundSafePosition = false;

        // Multi-distance search to avoid getting stuck
        std::vector<float> searchDistances = {6.0f, 10.0f, 15.0f, 20.0f, 25.0f};

        for (float searchDistance : searchDistances)
        {
            // Try 36 different angles for thorough coverage
            for (int i = 0; i < 36; i++)
            {
                float testAngle = i * 2 * M_PI / 36;

                float testX = bot->GetPositionX() + searchDistance * cos(testAngle);
                float testY = bot->GetPositionY() + searchDistance * sin(testAngle);
                float testZ = bot->GetPositionZ();

                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                // More lenient LOS and height checks
                bool validPosition = true;
                float heightDiff = fabs(testZ - bot->GetPositionZ());

                // Skip positions that are too high/low or not in LOS
                if (heightDiff >= MAX_HEIGHT_DIFF)
                {
                    validPosition = false;
                }

                // Only check LOS if height difference is reasonable
                if (validPosition && !bot->IsWithinLOS(testX, testY, testZ))
                {
                    validPosition = false;
                }

                if (!validPosition)
                    continue;

                // Check position safety and bomb density
                bool positionSafe = true;
                float minDistAtPos = std::numeric_limits<float>::max();
                int bombCountInVicinity = 0;

                for (const auto& bombPair : spiritBombs)
                {
                    Unit* bomb = bombPair.second;
                    if (!bomb || !bomb->IsAlive())
                        continue;

                    float dx = testX - bomb->GetPositionX();
                    float dy = testY - bomb->GetPositionY();
                    float dz = testZ - bomb->GetPositionZ();
                    float horizontalDist = sqrt(dx * dx + dy * dy);
                    float verticalDist = fabs(dz);

                    // Track minimum distance to any bomb
                    minDistAtPos = std::min(minDistAtPos, horizontalDist);

                    // Count bombs within density radius
                    if (horizontalDist < BOMB_DENSITY_RADIUS)
                    {
                        bombCountInVicinity++;
                    }

                    // Check safety condition
                    if (horizontalDist < SAFE_DISTANCE && verticalDist <= SAFE_HEIGHT_DIFF)
                    {
                        positionSafe = false;
                        break;
                    }
                }

                // Skip unsafe positions
                if (!positionSafe)
                    continue;

                // Calculate composite score with distance bonus to prefer closer safe positions
                float distanceBonus = std::max(0.0f, 30.0f - searchDistance);  // Prefer closer positions
                float score = minDistAtPos - (bombCountInVicinity * BOMB_COUNT_PENALTY) + distanceBonus;

                // Update best position if this one is better
                if (score > bestScore)
                {
                    bestScore = score;
                    bestX = testX;
                    bestY = testY;
                    bestZ = testZ;
                    foundSafePosition = true;
                }
            }

            // If we found a safe position at this distance, use it (prefer closer positions)
            if (foundSafePosition && searchDistance <= 15.0f)
                break;
        }

        // Move to the best position found
        if (foundSafePosition)
        {
            // Final validation before moving
            if (bot->IsWithinLOS(bestX, bestY, bestZ) && fabs(bestZ - bot->GetPositionZ()) <= MAX_HEIGHT_DIFF)
            {
                MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    return true;
}

void IccLichKingAddsAction::HandleHeroicNonTankPositioning(Difficulty diff, Unit* terenasMenethilHC)
{
    if (!terenasMenethilHC || botAI->IsMainTank(bot) || !diff ||
        !(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    // Only move if significantly far from main tank (increased threshold to reduce jittery movement)
    if (mainTank && bot->GetExactDist2d(mainTank->GetPositionX(), mainTank->GetPositionY()) > 2.0f)
    {
        MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(), mainTank->GetPositionZ(), false,
               false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
}

void IccLichKingAddsAction::HandleSpiritMarkingAndTargeting(Difficulty diff, Unit* terenasMenethilHC)
{
    if (!terenasMenethilHC || botAI->IsMainTank(bot) || !diff ||
        !(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    static constexpr uint8_t STAR_ICON_INDEX = 0;
    static constexpr float MAX_Z_DIFF = 20.0f;

    // Check if current marked target is still valid and threatening
    Unit* currentMarkedTarget = botAI->GetUnit(group->GetTargetIcon(STAR_ICON_INDEX));
    bool needNewMark = !currentMarkedTarget || !currentMarkedTarget->IsAlive();

    // Check if current marked spirit is targeting a group member
    bool currentTargetingGroupMember = false;
    if (currentMarkedTarget && currentMarkedTarget->IsAlive())
    {
        Unit* spiritTarget = currentMarkedTarget->GetVictim();
        if (spiritTarget && spiritTarget->GetTypeId() == TYPEID_PLAYER)
        {
            if (Group* spiritTargetGroup = spiritTarget->ToPlayer()->GetGroup())
            {
                if (spiritTargetGroup->GetGUID() == group->GetGUID())
                {
                    currentTargetingGroupMember = true;
                }
            }
        }
    }

    // Only search for new target if we need to mark OR if we can find a higher priority target
    if (needNewMark || !currentTargetingGroupMember)
    {
        Unit* prioritySpirit = nullptr;  // Spirit targeting group member
        Unit* nearestSpirit = nullptr;   // Fallback: nearest spirit
        float priorityDist = 100.0f;
        float nearestDist = 100.0f;

        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive() || !unit->isTargetableForAttack())
                continue;

            uint32 entry = unit->GetEntry();
            if (entry == NPC_WICKED_SPIRIT1 || entry == NPC_WICKED_SPIRIT2 || entry == NPC_WICKED_SPIRIT3 ||
                entry == NPC_WICKED_SPIRIT4)
            {
                // Check Z-axis difference first
                float zDiff = std::abs(unit->GetPositionZ() - bot->GetPositionZ());
                if (zDiff <= MAX_Z_DIFF)
                {
                    float dist = bot->GetDistance(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());

                    // Check if this spirit is targeting a group member
                    bool targetingGroupMember = false;
                    Unit* spiritTarget = unit->GetVictim();
                    if (spiritTarget && spiritTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (Group* spiritTargetGroup = spiritTarget->ToPlayer()->GetGroup())
                        {
                            if (spiritTargetGroup->GetGUID() == group->GetGUID())
                            {
                                targetingGroupMember = true;
                            }
                        }
                    }

                    // Priority: spirits targeting group members
                    if (targetingGroupMember)
                    {
                        if (!prioritySpirit || dist < priorityDist)
                        {
                            prioritySpirit = unit;
                            priorityDist = dist;
                        }
                    }

                    // Fallback: track nearest spirit regardless of target
                    if (!nearestSpirit || dist < nearestDist)
                    {
                        nearestSpirit = unit;
                        nearestDist = dist;
                    }
                }
            }
        }

        // Mark priority spirit if found, otherwise fall back to nearest
        Unit* spiritToMark = prioritySpirit ? prioritySpirit : nearestSpirit;

        // Only mark if we found a better target or need a new mark
        if (spiritToMark && (needNewMark || (prioritySpirit && !currentTargetingGroupMember)))
        {
            group->SetTargetIcon(STAR_ICON_INDEX, bot->GetGUID(), spiritToMark->GetGUID());
        }
    }

    // Only ranged DPS use star for RTI
    if (botAI->IsRangedDps(bot))
    {
        context->GetValue<std::string>("rti")->Set("star");
        Unit* starTarget = botAI->GetUnit(group->GetTargetIcon(STAR_ICON_INDEX));
        if (starTarget && starTarget->IsAlive())
        {
            bot->SetTarget(starTarget->GetGUID());
            bot->SetFacingToObject(starTarget);
            Attack(starTarget);
            bot->Kill(bot, starTarget); //temp solution since bots struggle to kill spirits in time, they have to follow main tank closely so that they do not get hit by bomb, thus making them have very limited time to react
        }
    }
}

bool IccLichKingAddsAction::HandleQuakeMechanics(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_QUAKE))
        return false;

    float currentDistance = bot->GetExactDist2d(boss);

    // If already at ideal distance (40f), no need to move
    if (currentDistance >= 35.0f && currentDistance <= 45.0f)
        return false;

    if (bot->HasUnitState(UNIT_STATE_CASTING))
        botAI->Reset();

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float targetX, targetY;

    if (!botAI->IsTank(bot))
    {
        // Non-tanks: use offset position as direction guide
        float offsetX = boss->GetPositionX() + 15.0f;
        float offsetY = boss->GetPositionY() + 15.0f;

        // Calculate direction towards offset position
        float dx = offsetX - botX;
        float dy = offsetY - botY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0.0f)
        {
            // Move 10f towards offset position
            float ratio = 10.0f / distance;
            targetX = botX + dx * ratio;
            targetY = botY + dy * ratio;
        }
        else
        {
            targetX = botX;
            targetY = botY;
        }
    }
    else
    {
        // Tanks: use offset position as direction guide
        float offsetX = boss->GetPositionX() - 15.0f;
        float offsetY = boss->GetPositionY() - 15.0f;

        // Calculate direction towards offset position
        float dx = offsetX - botX;
        float dy = offsetY - botY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 0.0f)
        {
            // Move 10f towards offset position
            float ratio = 10.0f / distance;
            targetX = botX + dx * ratio;
            targetY = botY + dy * ratio;
        }
        else
        {
            targetX = botX;
            targetY = botY;
        }
    }

    MoveTo(bot->GetMapId(), targetX, targetY, boss->GetPositionZ(), false, false, false, false,
           MovementPriority::MOVEMENT_COMBAT);

    return false;
}

void IccLichKingAddsAction::HandleShamblingHorrors(Unit* boss, bool hasPlague)
{
    // Find closest shambling horror
    GuidVector npcs2 = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestHorror = nullptr;
    float minHorrorDistance = std::numeric_limits<float>::max();

    for (auto& npc : npcs2)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() &&
            (unit->GetEntry() == NPC_SHAMBLING_HORROR1 || unit->GetEntry() == NPC_SHAMBLING_HORROR2 || unit->GetEntry() == NPC_SHAMBLING_HORROR3 ||
             unit->GetEntry() == NPC_SHAMBLING_HORROR4))  // Shambling horror entries
        {
            float distance = bot->GetDistance(unit);
            if (distance < minHorrorDistance)
            {
                minHorrorDistance = distance;
                closestHorror = unit;
            }
        }
    }

    /*
    if (!closestHorror || hasPlague)
    {
    }
    else if (!hasPlague && closestHorror->isInFront(bot) && closestHorror->IsAlive() && !botAI->IsTank(bot) &&
             bot->GetDistance2d(closestHorror) < 3.0f)
        return FleePosition(closestHorror->GetPosition(), 2.0f, 250U);
    */

    // If bot is hunter and shambling is enraged, use Tranquilizing Shot
    if (bot->getClass() == CLASS_HUNTER && closestHorror && botAI->HasAura("Enrage", closestHorror))
        botAI->CastSpell("Tranquilizing Shot", closestHorror);
}

bool IccLichKingAddsAction::HandleAssistTankAddManagement(Unit* boss, Difficulty diff)
{
    if (!botAI->IsAssistTank(bot) || !boss || boss->HealthBelowPct(71))
        return false;

    // Find all adds and categorize them by targeting status
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    std::vector<ObjectGuid> addsNotTargetingUs;
    std::vector<ObjectGuid> addsTargetingUs;

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->IsAlive() &&
            (unit->GetEntry() == NPC_SHAMBLING_HORROR1 || unit->GetEntry() == NPC_SHAMBLING_HORROR2 ||
             unit->GetEntry() == NPC_SHAMBLING_HORROR3 ||
             unit->GetEntry() == NPC_SHAMBLING_HORROR4 ||  // Shambling entry
             unit->GetEntry() == NPC_RAGING_SPIRIT1 || unit->GetEntry() == NPC_RAGING_SPIRIT2 ||
             unit->GetEntry() == NPC_RAGING_SPIRIT3 || unit->GetEntry() == NPC_RAGING_SPIRIT4 ||  // Spirits entry
             unit->GetEntry() == NPC_DRUDGE_GHOUL1 || unit->GetEntry() == NPC_DRUDGE_GHOUL2 ||
             unit->GetEntry() == NPC_DRUDGE_GHOUL3 || unit->GetEntry() == NPC_DRUDGE_GHOUL4))  // Drudge Ghouls entry
        {
            if (unit->GetVictim() == bot)
            {
                addsTargetingUs.push_back(unit->GetGUID());
            }
            else
            {
                addsNotTargetingUs.push_back(unit->GetGUID());
            }
        }
    }

    // If there are adds not targeting us, we need to collect them all
    if (!addsNotTargetingUs.empty())
    {
        // Find the highest priority target (Shamblings first, then closest)
        Unit* priorityTarget = nullptr;
        Unit* closestAdd = nullptr;
        float closestDist = 999.0f;

        for (const ObjectGuid& addGuid : addsNotTargetingUs)
        {
            Unit* add = botAI->GetUnit(addGuid);
            if (add && add->IsAlive())
            {
                // Shambling takes absolute priority regardless of distance
                if (add->GetEntry() == NPC_SHAMBLING_HORROR1 || add->GetEntry() == NPC_SHAMBLING_HORROR2 ||
                    add->GetEntry() == NPC_SHAMBLING_HORROR3 || add->GetEntry() == NPC_SHAMBLING_HORROR4)
                {
                    priorityTarget = add;
                    break;  // Found shambling, stop looking
                }

                // Track closest add as backup
                float dist = bot->GetExactDist2d(add);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestAdd = add;
                }
            }
        }

        // Choose target: Shambling first, then closest
        Unit* targetToAttack = priorityTarget ? priorityTarget : closestAdd;

        if (targetToAttack)
        {
            // Generate threat on ALL adds not targeting us using ranged abilities
            for (const ObjectGuid& addGuid : addsNotTargetingUs)
            {
                Unit* add = botAI->GetUnit(addGuid);
                if (add && add->IsAlive())
                {
                    float dist = bot->GetExactDist2d(add);

                    // Use ranged threat generation if within range
                    if (dist <= 30.0f)
                    {
                        // Try taunt first if available
                        if (botAI->CastSpell("taunt", add))
                        {
                            continue;
                        }
                        // Fall back to ranged attack
                        else if (botAI->CastSpell("shoot", add) || botAI->CastSpell("throw", add))
                        {
                            continue;
                        }
                        // Last resort - basic attack state update
                        else
                        {
                            bot->AttackerStateUpdate(add);
                        }
                    }
                }
            }

            // Move towards and attack the priority target
            float distToTarget = bot->GetExactDist2d(targetToAttack);

            // If we're too far from our priority target, move closer
            if (distToTarget > 5.0f)
            {
                MoveTo(bot->GetMapId(), targetToAttack->GetPositionX(), targetToAttack->GetPositionY(),
                       targetToAttack->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED,
                       true, false);
            }
            else
            {
                // We're close enough, set target and attack
                bot->SetTarget(targetToAttack->GetGUID());
                bot->SetFacingToObject(targetToAttack);
                Attack(targetToAttack);
            }
        }
    }
    // If all adds are targeting us or there are no adds, maintain position based on difficulty
    else
    {
        // In heroic mode, stay at melee position
        if (diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        {
            Unit* mainTank = AI_VALUE(Unit*, "main tank");

            if (bot->GetExactDist2d(ICC_LICH_KING_ASSISTHC_POSITION.GetPositionX(),
                                    ICC_LICH_KING_ASSISTHC_POSITION.GetPositionY()) > 2.0f)
            {
                MoveTo(bot->GetMapId(), ICC_LICH_KING_ASSISTHC_POSITION.GetPositionX(),
                       ICC_LICH_KING_ASSISTHC_POSITION.GetPositionY(), ICC_LICH_KING_ASSISTHC_POSITION.GetPositionZ(),
                       false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
        // In normal mode, stay at adds position
        else
        {
            if (bot->GetExactDist2d(ICC_LICH_KING_ADDS_POSITION) > 2.0f)
            {
                MoveTo(bot->GetMapId(), ICC_LICH_KING_ADDS_POSITION.GetPositionX(),
                       ICC_LICH_KING_ADDS_POSITION.GetPositionY(), ICC_LICH_KING_ADDS_POSITION.GetPositionZ(), false,
                       false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }

        // If we have adds targeting us, attack them with stable target selection
        if (!addsTargetingUs.empty())
        {
            Unit* currentTarget = bot->GetVictim();
            bool needNewTarget = true;

            // Check if current target is still valid (alive and attacking us)
            if (currentTarget && currentTarget->IsAlive())
            {
                for (const ObjectGuid& addGuid : addsTargetingUs)
                {
                    if (addGuid == currentTarget->GetGUID())
                    {
                        needNewTarget = false;
                        break;
                    }
                }
            }

            // Only pick new target if current one is invalid
            if (needNewTarget)
            {
                currentTarget = nullptr;

                // Priority 1: Shambling Horror
                for (const ObjectGuid& addGuid : addsTargetingUs)
                {
                    Unit* add = botAI->GetUnit(addGuid);
                    if (add && add->IsAlive())
                    {
                        if (add->GetEntry() == NPC_SHAMBLING_HORROR1 || add->GetEntry() == NPC_SHAMBLING_HORROR2 ||
                            add->GetEntry() == NPC_SHAMBLING_HORROR3 || add->GetEntry() == NPC_SHAMBLING_HORROR4)
                        {
                            currentTarget = add;
                            break;
                        }
                    }
                }

                // Priority 2: Any other add if no Shambling Horror
                if (!currentTarget)
                {
                    for (const ObjectGuid& addGuid : addsTargetingUs)
                    {
                        Unit* add = botAI->GetUnit(addGuid);
                        if (add && add->IsAlive())
                        {
                            currentTarget = add;
                            break;
                        }
                    }
                }
            }

            if (currentTarget)
            {
                bot->SetTarget(currentTarget->GetGUID());
                bot->SetFacingToObject(currentTarget);
                Attack(currentTarget);
            }
        }
    }
    return false;
}

void IccLichKingAddsAction::HandleMeleePositioning(Unit* boss, bool hasPlague, Difficulty diff)
{
    if (!boss || !botAI->IsMelee(bot) || botAI->IsAssistTank(bot) || boss->HealthBelowPct(71) || hasPlague)
        return;

    if (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC)
        return;

    float currentDist = bot->GetDistance(ICC_LICH_KING_MELEE_POSITION);

    if (currentDist > 6.0f && !botAI->IsMainTank(bot))
    {
        MoveTo(bot->GetMapId(), ICC_LICH_KING_MELEE_POSITION.GetPositionX(),
               ICC_LICH_KING_MELEE_POSITION.GetPositionY(), ICC_LICH_KING_MELEE_POSITION.GetPositionZ(), false, false,
               false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (currentDist > 6.0f && botAI->IsMainTank(bot) && boss && boss->GetVictim() == bot)
    {
        Position currentPos = bot->GetPosition();
        Position targetPos = ICC_LICH_KING_MELEE_POSITION;

        // Calculate direction vector
        float dx = targetPos.GetPositionX() - currentPos.GetPositionX();
        float dy = targetPos.GetPositionY() - currentPos.GetPositionY();

        // Calculate distance and normalize direction
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0.1)
        {
            dx /= distance;
            dy /= distance;
        }

        // Calculate intermediate position (3f towards target)
        float step = std::min(3.0f, distance - 1.0f);  // Don't overshoot the target
        if (step > 0)
        {
            float intermediateX = currentPos.GetPositionX() + dx * step;
            float intermediateY = currentPos.GetPositionY() + dy * step;

            MoveTo(bot->GetMapId(), intermediateX, intermediateY, bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            // If we're within 1.0f + 3.0f of the target, move directly to it
            MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), bot->GetPositionZ(), false,
                   false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
}

void IccLichKingAddsAction::HandleMainTankTargeting(Unit* boss, Difficulty diff)
{
    if (!botAI->IsMainTank(bot) || !boss)
        return;

    if (!(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return;

    if (boss->HealthBelowPct(71) || boss->GetVictim() == bot)
        return;

    bot->SetTarget(boss->GetGUID());
    bot->SetFacingToObject(boss);
    Attack(boss);
}

void IccLichKingAddsAction::HandleNonTankHeroicPositioning(Unit* boss, Difficulty diff, bool hasPlague)
{
    if (botAI->IsTank(bot) || !boss)
        return;

    if (!(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return;

    if (boss->HealthBelowPct(71) || hasPlague)
        return;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
        return;

    if (bot->GetDistance2d(mainTank->GetPositionX(), mainTank->GetPositionY()) > 20.0f && (bot->getClass() == CLASS_HUNTER))
    {
        botAI->Reset();

        // Calculate direction vector to main tank
        float dx = mainTank->GetPositionX() - bot->GetPositionX();
        float dy = mainTank->GetPositionY() - bot->GetPositionY();

        // Normalize and scale to 2f increments
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0)
        {
            dx = dx / distance * 2.0f;
            dy = dy / distance * 2.0f;

            // Calculate new position (2f closer to main tank)
            float newX = bot->GetPositionX() + dx;
            float newY = bot->GetPositionY() + dy;

            MoveTo(bot->GetMapId(), newX, newY, bot->GetPositionZ(), false, false, false, true,
                   MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    if (bot->GetDistance2d(mainTank->GetPositionX(), mainTank->GetPositionY()) > 1.0f && (bot->getClass() != CLASS_HUNTER))
    {
        botAI->Reset();
        MoveTo(bot->GetMapId(), mainTank->GetPositionX(), mainTank->GetPositionY(), bot->GetPositionZ(), false, false,
               false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

void IccLichKingAddsAction::HandleRangedPositioning(Unit* boss, bool hasPlague, Difficulty diff)
{
    if (!boss || !botAI->IsRanged(bot) || boss->HealthBelowPct(71) || hasPlague)
        return;

    if (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC)
        return;

    float currentDist = bot->GetDistance(ICC_LICH_KING_RANGED_POSITION);
    if (currentDist > 2.0f)
    {
        MoveTo(bot->GetMapId(), ICC_LICH_KING_RANGED_POSITION.GetPositionX(),
               ICC_LICH_KING_RANGED_POSITION.GetPositionY(), ICC_LICH_KING_RANGED_POSITION.GetPositionZ(), false, false,
               false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

void IccLichKingAddsAction::HandleDefileMechanics(Unit* boss, Difficulty diff)
{
    if (!boss)
        return;

    // Constants
    const float BASE_RADIUS = 6.0f;
    const float SAFETY_MARGIN = 3.0f;
    const float MOVE_DISTANCE = 5.0f;
    const float SPREAD_DISTANCE = 12.0f;
    const float FIXED_Z = 840.857f;
    const float MAX_HEIGHT_DIFF = 5.0f;
    const float MIN_PLAYER_SPACING = 5.0f;
    const float MAX_BOSS_DISTANCE = 40.0f;
    const int ANGLE_TESTS = 16;
    const int MAX_ANGLE_OFFSETS = 8;

    // Gather all defile units
    std::vector<Unit*> defiles;
    Unit* closestDefile = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)
        {
            defiles.push_back(unit);
            float dist = bot->GetDistance(unit);
            if (dist < closestDistance)
            {
                closestDistance = dist;
                closestDefile = unit;
            }
        }
    }

    // Only process defile avoidance if defiles exist
    if (!defiles.empty())
    {
        // Check if we need to move away from defiles
        bool needToMove = false;
        float botX = bot->GetPositionX();
        float botY = bot->GetPositionY();

        for (Unit* defile : defiles)
        {
            if (!defile || !defile->IsAlive())
                continue;

            // Calculate current defile radius including growth
            float currentRadius = BASE_RADIUS;
            Aura* growAura = nullptr;

            // Find growth aura
            for (size_t i = 0; i < DEFILE_AURA_COUNT; i++)
            {
                growAura = defile->GetAura(DEFILE_AURAS[i]);
                if (growAura)
                    break;
            }

            if (growAura)
            {
                uint8 stacks = growAura->GetStackAmount();
                float growthMultiplier =
                    (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_10MAN_NORMAL) ? 1.4f : 0.95f;
                currentRadius = BASE_RADIUS + (stacks * growthMultiplier);
            }

            // Check if bot is too close to this defile
            float dx = botX - defile->GetPositionX();
            float dy = botY - defile->GetPositionY();
            float distanceToCenter = sqrt(dx * dx + dy * dy);

            if (distanceToCenter < (currentRadius + SAFETY_MARGIN))
            {
                needToMove = true;
                break;
            }
        }

        // Move away from defiles if needed
        if (needToMove)
        {
            float bestAngle = 0.0f;
            float maxSafetyScore = 0.0f;
            bool foundSafePosition = false;

            // Test multiple angles to find safest escape route
            for (int i = 0; i < ANGLE_TESTS; i++)
            {
                float testAngle = i * M_PI / 8;
                float testX = botX + MOVE_DISTANCE * cos(testAngle);
                float testY = botY + MOVE_DISTANCE * sin(testAngle);
                float testZ = FIXED_Z;

                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                // Skip invalid positions (LOS and height check)
                if (!bot->IsWithinLOS(testX, testY, testZ) || fabs(testZ - bot->GetPositionZ()) >= MAX_HEIGHT_DIFF)
                    continue;

                // Calculate minimum distance to any defile from this position
                float minDefileDistance = std::numeric_limits<float>::max();
                for (Unit* defile : defiles)
                {
                    if (!defile || !defile->IsAlive())
                        continue;

                    float dx = testX - defile->GetPositionX();
                    float dy = testY - defile->GetPositionY();
                    float dist = sqrt(dx * dx + dy * dy);
                    minDefileDistance = std::min(minDefileDistance, dist);
                }

                // Calculate scoring (safety + boss proximity)
                float distanceToBoss = boss->GetDistance2d(testX, testY);
                float safetyScore = minDefileDistance;
                float bossScore = 100.0f - std::min(100.0f, distanceToBoss);
                float totalScore = safetyScore + (bossScore * 0.5f);

                if (totalScore > maxSafetyScore)
                {
                    maxSafetyScore = totalScore;
                    bestAngle = testAngle;
                    foundSafePosition = true;
                }
            }

            // Execute movement if safe position found
            if (foundSafePosition && maxSafetyScore > 0)
            {
                float moveX = botX + MOVE_DISTANCE * cos(bestAngle);
                float moveY = botY + MOVE_DISTANCE * sin(bestAngle);
                float moveZ = FIXED_Z;

                if (bot->HasUnitState(UNIT_STATE_CASTING))
                    botAI->Reset();

                bot->UpdateAllowedPositionZ(moveX, moveY, moveZ);
                MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                       MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    // Handle Defile cast - spread positioning
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(DEFILE_CAST_ID))
    {
        // Count players and determine bot's index
        uint32 playerCount = 0;
        uint32 botIndex = 0;
        uint32 currentIndex = 0;

        float botX = bot->GetPositionX();
        float botY = bot->GetPositionY();

        Map::PlayerList const& players = bot->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player || !player->IsAlive())
                continue;

            if (player == bot)
                botIndex = currentIndex;

            currentIndex++;
            playerCount++;
        }

        // Calculate preferred spread angle based on bot index
        float preferredAngle = (float(botIndex) / float(playerCount)) * 2 * M_PI;
        bool foundSafeSpot = false;
        float bestSpreadAngle = preferredAngle;

        // Try positions starting from preferred angle, expanding outward
        for (int offset = 0; offset <= MAX_ANGLE_OFFSETS && !foundSafeSpot; offset++)
        {
            for (int direction = -1; direction <= 1; direction += 2)
            {
                if (offset == 0 && direction > 0)  // Skip duplicate check of preferred angle
                    continue;

                float testAngle = preferredAngle + (direction * offset * M_PI / 16);
                float testX = botX + SPREAD_DISTANCE * cos(testAngle);
                float testY = botY + SPREAD_DISTANCE * sin(testAngle);
                float testZ = FIXED_Z;

                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                // Validate position basics (LOS and height)
                if (!bot->IsWithinLOS(testX, testY, testZ) || fabs(testZ - bot->GetPositionZ()) >= MAX_HEIGHT_DIFF)
                    continue;

                // Check boss distance
                if (boss->GetDistance2d(testX, testY) > MAX_BOSS_DISTANCE)
                    continue;

                // Check safety from all defiles (only if defiles exist)
                bool safeFromDefiles = true;
                for (Unit* defile : defiles)
                {
                    if (!defile || !defile->IsAlive())
                        continue;

                    // Calculate current defile radius including growth
                    float currentRadius = BASE_RADIUS;
                    Aura* growAura = nullptr;

                    // Find growth aura
                    for (size_t i = 0; i < DEFILE_AURA_COUNT; i++)
                    {
                        growAura = defile->GetAura(DEFILE_AURAS[i]);
                        if (growAura)
                            break;
                    }

                    if (growAura)
                    {
                        uint8 stacks = growAura->GetStackAmount();
                        float growthMultiplier =
                            (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_10MAN_NORMAL) ? 1.4f
                                                                                                           : 0.95f;
                        currentRadius = BASE_RADIUS + (stacks * growthMultiplier);
                    }

                    float dx = testX - defile->GetPositionX();
                    float dy = testY - defile->GetPositionY();
                    float distToDefile = sqrt(dx * dx + dy * dy);

                    if (distToDefile < (currentRadius + SAFETY_MARGIN))
                    {
                        safeFromDefiles = false;
                        break;
                    }
                }

                if (!safeFromDefiles)
                    continue;

                // Check spacing from other players
                bool tooCloseToPlayers = false;
                for (Map::PlayerList::const_iterator playerItr = players.begin(); playerItr != players.end();
                     ++playerItr)
                {
                    Player* player = playerItr->GetSource();
                    if (!player || !player->IsAlive() || player == bot)
                        continue;

                    float dx = testX - player->GetPositionX();
                    float dy = testY - player->GetPositionY();
                    float dist = sqrt(dx * dx + dy * dy);

                    if (dist < MIN_PLAYER_SPACING)
                    {
                        tooCloseToPlayers = true;
                        break;
                    }
                }

                if (tooCloseToPlayers)
                    continue;

                // Found valid position
                bestSpreadAngle = testAngle;
                foundSafeSpot = true;
                break;
            }
        }

        // Execute spread movement if safe spot found
        if (foundSafeSpot)
        {
            float spreadX = botX + SPREAD_DISTANCE * cos(bestSpreadAngle);
            float spreadY = botY + SPREAD_DISTANCE * sin(bestSpreadAngle);
            float spreadZ = FIXED_Z;

            bot->UpdateAllowedPositionZ(spreadX, spreadY, spreadZ);
            MoveTo(bot->GetMapId(), spreadX, spreadY, spreadZ, false, false, false, false,
                   MovementPriority::MOVEMENT_COMBAT);
        }
    }
}

void IccLichKingAddsAction::HandleValkyrMechanics(Difficulty diff)
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> grabbingValkyrs;
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");

    // Find grabbing Val'kyrs
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_VALKYR_SHADOWGUARD1 || unit->GetEntry() == NPC_VALKYR_SHADOWGUARD2 ||
            unit->GetEntry() == NPC_VALKYR_SHADOWGUARD3 || unit->GetEntry() == NPC_VALKYR_SHADOWGUARD4)
        {
            bool isGrabbing = false;

            if (diff && !(diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
            {
                if (unit->HasAura(SPELL_HARVEST_SOUL_VALKYR))
                    isGrabbing = true;
            }
            else
            {
                if (unit->HasAura(SPELL_HARVEST_SOUL_VALKYR) && unit->HealthAbovePct(49))
                    isGrabbing = true;
            }

            if (isGrabbing)
                grabbingValkyrs.push_back(unit);
        }
    }

    Group* group = bot->GetGroup();
    if (!group)
        return;

    // If no grabbing Val'kyrs, mark the Lich King with skull
    if (grabbingValkyrs.empty() || (boss && boss->HealthBelowPct(43)))
    {
        // Find the Lich King
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->IsAlive() && unit->GetEntry() == NPC_THE_LICH_KING && unit->HealthBelowPct(68) && unit->HealthAbovePct(40))
            {
                ObjectGuid currentSkull = group->GetTargetIcon(7);  // Skull icon
                if (currentSkull != unit->GetGUID())
                {
                    group->SetTargetIcon(7, bot->GetGUID(), unit->GetGUID());
                }
                break;
            }
        }
        return;
    }

    if (botAI->IsMainTank(bot))
        return;

    // Filter out dead Val'kyrs to ensure accurate group calculation
    std::vector<Unit*> aliveGrabbingValkyrs;
    for (Unit* valkyr : grabbingValkyrs)
    {
        if (valkyr && valkyr->IsAlive())
            aliveGrabbingValkyrs.push_back(valkyr);
    }

    if (aliveGrabbingValkyrs.empty())
        return;

    HandleValkyrMarking(aliveGrabbingValkyrs, diff);
    HandleValkyrAssignment(aliveGrabbingValkyrs);
}

void IccLichKingAddsAction::HandleValkyrMarking(const std::vector<Unit*>& grabbingValkyrs, Difficulty diff)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    // Sort Val'kyrs by their GUID to ensure consistent ordering
    std::vector<Unit*> sortedValkyrs = grabbingValkyrs;
    std::sort(sortedValkyrs.begin(), sortedValkyrs.end(), [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    static constexpr uint8_t ICON_INDICES[] = {7, 6, 0};  // Skull, Cross, Star
    static constexpr const char* ICON_NAMES[] = {"skull", "cross", "star"};

    // In heroic mode, clean up invalid markers for all possible icons
    if (diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        for (size_t i = 0; i < 3; ++i)
        {
            ObjectGuid currentIcon = group->GetTargetIcon(ICON_INDICES[i]);
            Unit* currentIconUnit = botAI->GetUnit(currentIcon);

            if (currentIconUnit && IsValkyr(currentIconUnit))
            {
                bool shouldRemoveMarker = !currentIconUnit->HasAura(SPELL_HARVEST_SOUL_VALKYR) ||
                                          std::abs(currentIconUnit->GetPositionZ() - bot->GetPositionZ()) > 5.0f;

                if (shouldRemoveMarker)
                    group->SetTargetIcon(ICON_INDICES[i], bot->GetGUID(), ObjectGuid::Empty);
            }
        }
    }

    // Clear unused markers if we have fewer Val'kyrs than icons
    for (size_t i = sortedValkyrs.size(); i < 3; ++i)
    {
        ObjectGuid currentIcon = group->GetTargetIcon(ICON_INDICES[i]);
        if (!currentIcon.IsEmpty())
        {
            group->SetTargetIcon(ICON_INDICES[i], bot->GetGUID(), ObjectGuid::Empty);
        }
    }

    // Mark each alive Val'kyr with appropriate icon
    for (size_t i = 0; i < sortedValkyrs.size() && i < 3; ++i)
    {
        ObjectGuid currentIcon = group->GetTargetIcon(ICON_INDICES[i]);
        Unit* currentIconUnit = botAI->GetUnit(currentIcon);

        if (!currentIconUnit || currentIconUnit != sortedValkyrs[i])
        {
            group->SetTargetIcon(ICON_INDICES[i], bot->GetGUID(), sortedValkyrs[i]->GetGUID());
        }
    }
}

void IccLichKingAddsAction::HandleValkyrAssignment(const std::vector<Unit*>& grabbingValkyrs)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (boss && boss->HealthBelowPct(40))
        return;

    // Double-check that all Val'kyrs in the list are actually alive and valid targets
    std::vector<Unit*> validValkyrs;
    for (Unit* valkyr : grabbingValkyrs)
    {
        if (valkyr && valkyr->IsAlive() && valkyr->HasAura(SPELL_HARVEST_SOUL_VALKYR))
        {
            validValkyrs.push_back(valkyr);
        }
    }

    if (validValkyrs.empty())
        return;

    // Sort valid Val'kyrs for consistent assignment
    std::sort(validValkyrs.begin(), validValkyrs.end(), [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    // Get all non-main-tank members (DPS, healers, and off-tanks)
    std::vector<Player*> assistMembers;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && !botAI->IsMainTank(member))
            assistMembers.push_back(member);
    }

    if (assistMembers.empty())
        return;

    // Sort assist members by GUID for consistent assignment
    std::sort(assistMembers.begin(), assistMembers.end(),
              [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    // Find our position among assist members
    auto it = std::find(assistMembers.begin(), assistMembers.end(), bot);
    if (it == assistMembers.end())
        return;  // We're main tank, shouldn't handle Val'kyrs

    size_t myAssistIndex = std::distance(assistMembers.begin(), it);
    size_t totalAssist = assistMembers.size();
    size_t aliveValkyrs = validValkyrs.size();

    // Calculate balanced group sizes
    std::vector<size_t> groupSizes = CalculateBalancedGroupSizes(totalAssist, aliveValkyrs);

    // Determine which Val'kyr this bot should target
    size_t assignedValkyrIndex = GetAssignedValkyrIndex(myAssistIndex, groupSizes);

    if (assignedValkyrIndex < validValkyrs.size())
    {
        Unit* myValkyr = validValkyrs[assignedValkyrIndex];

        // Set RTI context based on assignment
        std::string rtiValue = GetRTIValueForValkyr(assignedValkyrIndex);
        context->GetValue<std::string>("rti")->Set(rtiValue);

        // Attack and apply CC
        bot->SetTarget(myValkyr->GetGUID());
        bot->SetFacingToObject(myValkyr);
        Difficulty diff = bot->GetRaidDifficulty();

        if (sPlayerbotAIConfig->EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        {
            //---------CHEAT---------
            if (!myValkyr->HasAura(SPELL_HAMMER_OF_JUSTICE))
                bot->AddAura(SPELL_HAMMER_OF_JUSTICE, myValkyr);
            //---------CHEAT---------
        }
        ApplyCCToValkyr(myValkyr);
    }
}

std::vector<size_t> IccLichKingAddsAction::CalculateBalancedGroupSizes(size_t totalAssist, size_t numValkyrs)
{
    std::vector<size_t> groupSizes(numValkyrs, 0);

    if (numValkyrs == 0)
        return groupSizes;

    // Base size for each group
    size_t baseSize = totalAssist / numValkyrs;
    size_t remainder = totalAssist % numValkyrs;

    // Distribute assist members as evenly as possible
    for (size_t i = 0; i < numValkyrs; ++i)
    {
        groupSizes[i] = baseSize;
        if (i < remainder)
            groupSizes[i]++;  // Add extra member to first 'remainder' groups
    }

    return groupSizes;
}

size_t IccLichKingAddsAction::GetAssignedValkyrIndex(size_t assistIndex, const std::vector<size_t>& groupSizes)
{
    size_t currentIndex = 0;

    for (size_t valkyrIndex = 0; valkyrIndex < groupSizes.size(); ++valkyrIndex)
    {
        if (assistIndex < currentIndex + groupSizes[valkyrIndex])
            return valkyrIndex;

        currentIndex += groupSizes[valkyrIndex];
    }

    // Fallback - should not happen with correct logic
    return 0;
}

std::string IccLichKingAddsAction::GetRTIValueForValkyr(size_t valkyrIndex)
{
    switch (valkyrIndex)
    {
        case 0:
            return "skull";
        case 1:
            return "cross";
        case 2:
            return "star";
        default:
            return "skull";  // Fallback
    }
}

void IccLichKingAddsAction::ApplyCCToValkyr(Unit* valkyr)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            if (!botAI->HasAura("Frost Nova", valkyr))
                botAI->CastSpell("Frost Nova", valkyr);
            break;
        case CLASS_DRUID:
            if (!botAI->HasAura("Entangling Roots", valkyr))
                botAI->CastSpell("Entangling Roots", valkyr);
            break;
        case CLASS_PALADIN:
            if (!botAI->HasAura("Hammer of Justice", valkyr))
                botAI->CastSpell("Hammer of Justice", valkyr);
            break;
        case CLASS_WARRIOR:
            if (!botAI->HasAura("Hamstring", valkyr))
                botAI->CastSpell("Hamstring", valkyr);
            break;
        case CLASS_HUNTER:
            if (!botAI->HasAura("Concussive Shot", valkyr))
                botAI->CastSpell("Concussive Shot", valkyr);
            break;
        case CLASS_ROGUE:
            if (!botAI->HasAura("Kidney Shot", valkyr))
                botAI->CastSpell("Kidney Shot", valkyr);
            break;
        case CLASS_SHAMAN:
            if (!botAI->HasAura("Frost Shock", valkyr))
                botAI->CastSpell("Frost Shock", valkyr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (!botAI->HasAura("Chains of Ice", valkyr))
                botAI->CastSpell("Chains of Ice", valkyr);
            break;
        case CLASS_PRIEST:
            if (!botAI->HasAura("Psychic Scream", valkyr))
                botAI->CastSpell("Psychic Scream", valkyr);
            break;
        case CLASS_WARLOCK:
            if (!botAI->HasAura("Fear", valkyr))
                botAI->CastSpell("Fear", valkyr);
            break;
        default:
            break;
    }
}

bool IccLichKingAddsAction::IsValkyr(Unit* unit)
{
    return unit->GetEntry() == NPC_VALKYR_SHADOWGUARD1 || unit->GetEntry() == NPC_VALKYR_SHADOWGUARD2 ||
           unit->GetEntry() == NPC_VALKYR_SHADOWGUARD3 || unit->GetEntry() == NPC_VALKYR_SHADOWGUARD4;
}

void IccLichKingAddsAction::HandleVileSpiritMechanics()
{
    const float radiusVile = 12.0f;

    GuidVector npcs3 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs3)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || (unit->GetEntry() != NPC_VILE_SPIRIT1 && unit->GetEntry() != NPC_VILE_SPIRIT2 && unit->GetEntry() != NPC_VILE_SPIRIT3 &&
                      unit->GetEntry() != NPC_VILE_SPIRIT4))
            continue;

        // Only run away if the spirit is targeting us
        if (unit->GetVictim() && unit->GetVictim()->GetGUID() == bot->GetGUID())
        {
            float currentDistance = bot->GetDistance2d(unit);

            if (currentDistance < radiusVile)
            {
                botAI->Reset();
                MoveAway(unit, radiusVile - currentDistance);
            }
        }
    }
}

