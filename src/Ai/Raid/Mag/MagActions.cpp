/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagActions.h"
#include "EncounterHelpers.h"
#include "MagHelpers.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <cmath>
#include <limits>
#include <vector>

using namespace MagHelpers;
using namespace EncounterHelpers;

bool MagtheridonResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetInstanceId();

    bool reset = false;
    reset |= magDpsWaitTimer.erase(instanceId) > 0;
    reset |= blastNovaTimer.erase(instanceId) > 0;
    reset |= lastBlastNovaState.erase(instanceId) > 0;
    reset |= ceilingCollapseApplied.erase(instanceId) > 0;
    reset |= botToCubeAssignments.erase(instanceId) > 0;

    if (!AI_VALUE2(bool, "combat", "self target"))
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);

    return reset;
}

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event /*event*/)
{
    Creature* channeler = GetChanneler(bot, SOUTH_CHANNELER_DB_GUID);
    if (!channeler)
        channeler = GetChanneler(bot, WEST_CHANNELER_DB_GUID);
    if (!channeler)
        channeler = GetChanneler(bot, EAST_CHANNELER_DB_GUID);

    if (channeler)
        return AI_VALUE(Unit*, "current target") != channeler && Attack(channeler);

    // After first three channelers are dead, wait for Magtheridon to activate.
    Position const& position = WAITING_FOR_MAGTHERIDON_POSITION;
    if (bot->GetExactDist2d(position) <= 2.0f)
    {
        if (std::fabs(bot->GetOrientation() - position.GetOrientation()) > 0.1f)
            bot->SetFacingTo(position.GetOrientation());

        return true;
    }

    return MoveTo(
        MAG_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonAssistTanksAttackLastTwoChannelersAction::Execute(Event /*event*/)
{
    Creature* channeler = nullptr;
    Position position;
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        channeler = GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID);
        position = NW_CHANNELER_TANK_POSITION;
    }
    else // PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        channeler = GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID);
        position = NE_CHANNELER_TANK_POSITION;
    }

    if (!channeler)
        return false;

    if (AI_VALUE(Unit*, "current target") != channeler)
        return Attack(channeler);

    if (channeler->GetVictim() != bot)
        return false;

    // Movement is intentionally forwards only, so no facing is passed.
    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MagtheridonMisdirectHellfireChannelersToMainTankAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            member->GetMapId() == MAG_MAP_ID && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    Creature* channeler = nullptr;
    if (hunterIndex == 0)
        channeler = GetChanneler(bot, WEST_CHANNELER_DB_GUID);
    else if (hunterIndex == 1)
        channeler = GetChanneler(bot, EAST_CHANNELER_DB_GUID);

    if (!channeler)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (!bot->HasAura(Id(MagSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", channeler) &&
        botAI->CastSpell("steady shot", channeler);
}

bool MagtheridonAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Creature* channeler = nullptr;
    if (Creature* channelerS = GetChanneler(bot, SOUTH_CHANNELER_DB_GUID))
        channeler = channelerS;
    else if (Creature* channelerW = GetChanneler(bot, WEST_CHANNELER_DB_GUID))
        channeler = channelerW;
    else if (Creature* channelerE = GetChanneler(bot, EAST_CHANNELER_DB_GUID))
        channeler = channelerE;
    else if (Creature* channelerNw = GetChanneler(bot, NORTHWEST_CHANNELER_DB_GUID))
        channeler = channelerNw;
    else if (Creature* channelerNe = GetChanneler(bot, NORTHEAST_CHANNELER_DB_GUID))
        channeler = channelerNe;

    if (!channeler)
        return false;

    if (AI_VALUE(Unit*, "current target") != channeler)
        return Attack(channeler);

    return MarkTargetWithSkull(bot, channeler);
}

// Establish a Banish rotation for Burning Abyssals and a Fear rotation with respect to any Burning
// Abyssals that exceed the number of Warlocks in the raid.
bool MagtheridonWarlockCcBurningAbyssalAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Unit*> const abyssals = GetBurningAbyssals(botAI);

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK &&
            member->GetMapId() == MAG_MAP_ID && GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex < 0)
        return false;

    if (warlockIndex < static_cast<int>(abyssals.size()))
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!botAI->HasAura("banish", assignedAbyssal) &&
            botAI->CanCastSpell("banish", assignedAbyssal))
        {
            return botAI->CastSpell("banish", assignedAbyssal);
        }
    }

    for (size_t i = warlocks.size() + warlockIndex; i < abyssals.size(); i += warlocks.size())
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) &&
            !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
        {
            return botAI->CastSpell("fear", excessAbyssal);
        }
    }

    return false;
}

bool MagtheridonMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    if (AI_VALUE(Unit*, "current target") != magtheridon)
        return Attack(magtheridon);

    constexpr float stopForHealHealthPct = 50.0f;
    if (magtheridon->GetVictim() != bot || !bot->IsWithinMeleeRange(magtheridon) ||
        bot->GetHealthPct() < stopForHealHealthPct)
    {
        return false;
    }

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, MAGTHERIDON_TANK_POSITION, arrivalDist, magtheridon, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        MAG_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MagtheridonSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    // This 20y flee is sort of a cheap way to stay out of Magtheridon's Cleave.
    constexpr float safeDistFromBoss = 20.0f;
    constexpr uint32 minInterval = 0;
    if (bot->GetExactDist(magtheridon) < safeDistFromBoss &&
        FleePosition(magtheridon->GetPosition(), safeDistFromBoss, minInterval))
    {
        return true;
    }

    constexpr float safeDistFromPlayer = 6.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer);
}

bool MagtheridonUseManticronCubeAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return false;

    CubeInfo const* cubeInfo = GetAssignedCube();
    if (!cubeInfo)
        return false;

    GameObject* cube = botAI->GetGameObject(cubeInfo->guid);
    if (!cube)
        return false;

    // Release cubes after Blast Nova is interrupted.
    if (HandleCubeRelease(magtheridon))
        return true;

    // If Blast Nova is actively casting (2s cast), always try to click the cube.
    if (IsBlastNovaCasting(magtheridon))
        return HandleCubeInteraction(cube);

    // Otherwise, if Blast Nova is coming soon, move to and wait near the cube.
    return HandleWaitingPhase(*cubeInfo);
}

CubeInfo const* MagtheridonUseManticronCubeAction::GetAssignedCube()
{
    auto mapIt = botToCubeAssignments.find(bot->GetInstanceId());
    if (mapIt == botToCubeAssignments.end())
        return nullptr;

    auto it = mapIt->second.find(bot->GetGUID());
    return it != mapIt->second.end() ? &it->second : nullptr;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (!bot->HasAura(Id(MagSpells::SPELL_SHADOW_GRASP)) ||
        IsBlastNovaCasting(magtheridon))
    {
        return false;
    }

    // Stagger releasing cube so the clickers do not do it on the same tick, which looks stupid.
    constexpr uint32 minReleaseDelayMs = 400;
    constexpr uint32 maxReleaseDelayMs = 1500;
    uint32 const releaseDelay = urand(minReleaseDelayMs, maxReleaseDelayMs);

    botAI->AddTimedEvent(
        [this]
        {
            bot->CastStop();
        },
        releaseDelay);

    botAI->SetNextCheckDelay(releaseDelay + ONE_WORLD_UPDATE_MS);
    return true;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(GameObject* cube)
{
    if (cube->IsAtInteractDistance(*bot, cube->GetInteractionDistance()))
    {
        // If already clicked, don't let go!
        if (bot->HasAura(Id(MagSpells::SPELL_SHADOW_GRASP)))
            return true;

        bot->StopMoving();
        cube->Use(bot);
        return true;
    }

    // If the bot is already moving, don't randomize another move.
    if (!bot->movespline->Finalized())
        return true;

    ObjectGuid const cubeGuid = cube->GetGUID();
    // Stagger the run from the waiting spot so the clickers do not all move on the same tick, which
    // looks stupid.
    constexpr uint32 minRunDelayMs = 200;
    constexpr uint32 maxRunDelayMs = 1000;
    uint32 const runDelay = urand(minRunDelayMs, maxRunDelayMs);

    botAI->AddTimedEvent(
        [this, cubeGuid]
        {
            GameObject* cube = botAI->GetGameObject(cubeGuid);
            if (!cube)
                return;

            float const targetDist = cube->GetInteractionDistance() - 0.5f;
            float const angle = cube->GetAngle(bot);
            float const destX = cube->GetPositionX() + std::cos(angle) * targetDist;
            float const destY = cube->GetPositionY() + std::sin(angle) * targetDist;

            bot->CastStop();
            MoveTo(
                MAG_MAP_ID, destX, destY, cube->GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        },
        runDelay);

    botAI->SetNextCheckDelay(runDelay + ONE_WORLD_UPDATE_MS);
    return true;
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(CubeInfo const& cubeInfo)
{
    auto timerIt = blastNovaTimer.find(bot->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return false;

    if (getMSTimeDiff(timerIt->second, getMSTime()) < BLAST_NOVA_INTERIM_MS)
        return false;

    // If a hazard appears at the waiting position, find a new waiting position at the same distance
    // from the cube.
    constexpr float safeWaitDistance = 10.0f;
    bool const onRing =
        fabs(bot->GetDistance2d(cubeInfo.x, cubeInfo.y) - safeWaitDistance) <= 1.0f;
    bool const inHazard =
        IsPositionInActiveDebris(botAI, bot->GetPositionX(), bot->GetPositionY()) ||
        IsPositionInActiveConflagration(botAI, bot->GetPositionX(), bot->GetPositionY());
    if (onRing && !inHazard)
        return false;

    Position safePos;
    if (!FindSafePositionNearCube(cubeInfo, safeWaitDistance, safePos))
        return false;

    bot->CastStop();
    return MoveTo(
        MAG_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED);
}

bool MagtheridonUseManticronCubeAction::FindSafePositionNearCube(
    CubeInfo const& cubeInfo, float preferredDistance, Position& outPos)
{
    constexpr uint8 numAngles = 16;
    constexpr float angleStep = 2.0f * M_PI / numAngles;

    Position debris;
    bool const hasDebris = GetActiveDebrisPosition(botAI, debris);
    std::vector<GameObject*> const blazes = GetActiveConflagrations(botAI);

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (uint8 i = 0; i < numAngles; ++i)
    {
        float const angle = i * angleStep;
        float const x = cubeInfo.x + std::cos(angle) * preferredDistance;
        float const y = cubeInfo.y + std::sin(angle) * preferredDistance;

        if (hasDebris && debris.GetExactDist2d(x, y) <= DEBRIS_HAZARD_RADIUS)
            continue;

        if (IsPositionInConflagration(blazes, x, y))
            continue;

        float const moveDistance = bot->GetExactDist2d(x, y);
        if (moveDistance < minMoveDistance)
        {
            outPos = Position(x, y, bot->GetPositionZ());
            minMoveDistance = moveDistance;
            foundSafe = true;
        }
    }

    return foundSafe;
}

bool MagtheridonMoveOutOfDebrisAction::Execute(Event /*event*/)
{
    Position safePos;
    if (!FindSafePosition(safePos))
        return false;

    bot->CastStop();
    return MoveTo(
        MAG_MAP_ID, safePos.GetPositionX(), safePos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool MagtheridonMoveOutOfDebrisAction::FindSafePosition(Position& outPos)
{
    constexpr float minSearchRadius = 2.0f;
    constexpr float distanceStep = 1.0f;
    constexpr uint8 numDistSteps = 18;
    constexpr uint8 numAngles = 24;
    constexpr float angleStep = 2.0f * M_PI / numAngles;

    Position debris;
    bool const hasDebris = GetActiveDebrisPosition(botAI, debris);
    std::vector<GameObject*> const blazes = GetActiveConflagrations(botAI);

    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (uint8 i = 0; i <= numDistSteps; ++i)
    {
        float const distance = minSearchRadius + i * distanceStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * angleStep;
            float const x = bot->GetPositionX() + distance * std::cos(angle);
            float const y = bot->GetPositionY() + distance * std::sin(angle);

            if (hasDebris && debris.GetExactDist2d(x, y) <= DEBRIS_HAZARD_RADIUS)
                continue;

            if (IsPositionInConflagration(blazes, x, y))
                continue;

            float const moveDistance = bot->GetExactDist2d(x, y);

            if (moveDistance < minMoveDistance)
            {
                outPos = Position(x, y, bot->GetPositionZ());
                minMoveDistance = moveDistance;
                foundSafe = true;
            }
        }
    }

    return foundSafe;
}

bool MagtheridonUpdateTimersAndAssignmentsAction::Execute(Event /*event*/)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    uint32 const instanceId = magtheridon->GetInstanceId();
    uint32 const now = getMSTime();

    bool const isCasting = IsBlastNovaCasting(magtheridon);
    bool& lastState = lastBlastNovaState.try_emplace(instanceId, false).first->second;
    if (isCasting && !lastState)
        blastNovaTimer[instanceId] = now;

    lastState = isCasting;

    bool updated = false;
    updated |= blastNovaTimer.try_emplace(instanceId, now).second;
    updated |= magDpsWaitTimer.try_emplace(instanceId, now).second;

    if (magtheridon->GetHealthPct() < CEILING_COLLAPSE_HP_PCT &&
        !ceilingCollapseApplied.contains(instanceId))
    {
        blastNovaTimer[instanceId] += CEILING_COLLAPSE_DELAY_MS;
        ceilingCollapseApplied.insert(instanceId);
        updated = true;
    }

    updated |= NeedsCubeReassignment(instanceId) && AssignCubeClickers(instanceId, magtheridon);

    return updated;
}

bool MagtheridonUpdateTimersAndAssignmentsAction::AssignCubeClickers(
    uint32 instanceId, Unit* magtheridon)
{
    std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
    auto& assignment = botToCubeAssignments[instanceId];
    Group* group = bot->GetGroup();

    if (!group || cubes.empty())
    {
        assignment.clear();
        return true;
    }

    // Prune dead or absent players from the existing assignment.
    for (auto it = assignment.begin(); it != assignment.end(); )
    {
        Player* player = ObjectAccessor::FindPlayer(it->first);
        if (!player || !player->IsAlive() || player->GetMapId() != MAG_MAP_ID)
            it = assignment.erase(it);
        else
            ++it;
    }

    // Get Magtheridon's victim outside of the assignment loops to exclude them from assignments.
    Player* victim = magtheridon->GetVictim() ? magtheridon->GetVictim()->ToPlayer() : nullptr;

    // Fill unassigned cubes.
    for (CubeInfo const& cube : cubes)
    {
        bool alreadyAssigned = false;
        for (auto const& pair : assignment)
        {
            if (pair.second.guid == cube.guid)
            {
                alreadyAssigned = true;
                break;
            }
        }
        if (alreadyAssigned)
            continue;

        Player* candidate = nullptr;

        // Pass 1: ranged DPS bots, excluding Warlocks.
        for (GroupReference* ref = group->GetFirstMember(); ref && !candidate; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != MAG_MAP_ID ||
                member->getClass() == CLASS_WARLOCK || !GET_PLAYERBOT_AI(member) ||
                !PlayerbotAI::IsRangedDps(member))
            {
                continue;
            }

            if (assignment.find(member->GetGUID()) != assignment.end())
                continue;

            candidate = member;
        }

        // Pass 2: any bot other than the main tank or whoever Magtheridon is attacking.
        if (!candidate)
        {
            for (GroupReference* ref = group->GetFirstMember();
                 ref && !candidate; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member->GetMapId() != MAG_MAP_ID ||
                    !GET_PLAYERBOT_AI(member) || member == victim ||
                    PlayerbotAI::IsMainTank(member))
                {
                    continue;
                }

                if (assignment.find(member->GetGUID()) != assignment.end())
                    continue;

                candidate = member;
            }
        }

        if (candidate)
            assignment[candidate->GetGUID()] = cube;
    }

    return true;
}

bool MagtheridonUpdateTimersAndAssignmentsAction::NeedsCubeReassignment(uint32 instanceId)
{
    auto mapIt = botToCubeAssignments.find(instanceId);
    if (mapIt == botToCubeAssignments.end() || mapIt->second.empty())
        return true;

    for (auto const& pair : mapIt->second)
    {
        Player* assigned = ObjectAccessor::FindPlayer(pair.first);
        if (!assigned || !assigned->IsAlive() || assigned->GetMapId() != MAG_MAP_ID)
            return true;
    }

    return false;
}
