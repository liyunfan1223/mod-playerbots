/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include <map>
#include <string>
#include <vector>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool EredarTwinsMeleeJumpFromBalconyAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Position const& jumpPosition = EREDAR_TWINS_P1_RANGED_POSITION;
    Position const landingPosition = GetEredarTwinsP2MeleePosition(alythess);

    constexpr float arrivalDistance = 2.0f;
    float const distanceToJumpPos = bot->GetExactDist2d(jumpPosition);

    if (distanceToJumpPos > arrivalDistance)
    {
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(
                bot, jumpPosition, arrivalDistance, nullptr, moveX, moveY, backwards))
        {
            return false;
        }

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return JumpTo(
        SWP_MAP_ID, landingPosition.GetPositionX(), landingPosition.GetPositionY(),
        landingPosition.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
}

// Alythess really needs a Paladin tank so has a custom tank-selection method. To try to limit
// confusion, announcements are made about which tank is assigned to Alythess and why.
bool EredarTwinsAnnounceAlythessTankAction::Execute(Event /*event*/)
{
    ResolveEredarTwinsTankAssignment(bot);

    AlythessTankSource const source = GetAlythessTankSource(bot);
    Player* alythessTank = GetAlythessTank(bot);
    if (source == AlythessTankSource::Unresolved || !alythessTank)
        return false;

    std::optional<EredarTwinsTankAssignment>& assignment =
        EredarTwinsState(bot->GetInstanceId()).tankAssignment;
    (assignment ? *assignment : assignment.emplace()).announcementMs = getMSTime();

    std::map<std::string, std::string> placeholders = {{"%bot", alythessTank->GetName()}};
    std::string text;

    switch (source)
    {
        case AlythessTankSource::MainTankPaladin:
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "eredar_twins_alythess_tank_main_tank_paladin",
                "Alythess requires a Paladin tank. %bot is the main tank and a Paladin and is "
                "assigned to tank Alythess.",
                placeholders);
            break;
        }
        case AlythessTankSource::PaladinTank:
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "eredar_twins_alythess_tank_paladin_tank",
                "Alythess requires a Paladin tank. The main tank is not a Paladin. %bot is the "
                "best-geared Paladin tank and is assigned to tank Alythess.",
                placeholders);
            break;
        }
        default:
        {
            text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "eredar_twins_alythess_tank_no_paladin",
                "Alythess requires a Paladin tank. However, no Paladin tank is present. "
                "Therefore, the main tank, %bot, is assigned to tank Alythess.",
                placeholders);
            break;
        }
    }

    return botAI->SayToRaid(text);
}

bool EredarTwinsMisdirectToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == SWP_MAP_ID &&
            member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 3)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    // Ensure that the Alythess tank is always top priority, whether or not it is the main tank.
    Player* const alythessTank = GetAlythessTank(bot);
    Player* const sacrolashTank = GetSacrolashTank(bot, 0);
    Player* const sacrolashSecondTank = GetSacrolashTank(bot, 1);

    Unit* boss = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0)
    {
        boss = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        tank = alythessTank;
    }
    else if (hunterIndex == 1)
    {
        boss = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tank = sacrolashTank;
    }
    else if (hunterIndex == 2)
    {
        boss = AI_VALUE2(Unit*, "find target", "lady sacrolash");
        tank = sacrolashSecondTank;
    }

    if (!boss || !tank || !tank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, tank);
}

bool EredarTwinsPositionSacrolashTanksAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    if (AI_VALUE(Unit*, "current target") != sacrolash)
        return Attack(sacrolash);

    if (sacrolash->GetVictim() != bot || !bot->IsWithinMeleeRange(sacrolash))
        return false;

    constexpr float arrivalDist = 2.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, SACROLASH_TANK_POSITION, arrivalDist, sacrolash, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool EredarTwinsAlythessTankMoveOutOfBlazeAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    if (AI_VALUE(Unit*, "current target") != alythess)
        return Attack(alythess);

    auto const findSafeAlythessTankIndex =
        [&](uint8 startIndex, bool includeStart, uint8& safeIndex)
    {
        size_t const offsetStart = includeStart ? 0 : 1;
        for (size_t offset = offsetStart; offset < ALYTHESS_TANK_POSITIONS.size(); ++offset)
        {
            uint8 const candidateIndex =
                static_cast<uint8>((startIndex + offset) % ALYTHESS_TANK_POSITIONS.size());

            if (IsAlythessTankPositionSafe(
                    botAI, GetAlythessTankPosition(alythess, candidateIndex)))
            {
                safeIndex = candidateIndex;
                return true;
            }
        }

        return false;
    };

    uint8 index = _alythessTankStep;

    if (!IsAlythessTankPositionSafe(botAI, GetAlythessTankPosition(alythess, index)))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
    }

    Position const position = GetAlythessTankPosition(alythess, index);
    constexpr float maxDistance = 1.0f;
    float const distToPosition = bot->GetExactDist2d(position);

    if (alythess->GetVictim() != bot)
        return false;

    if (distToPosition <= maxDistance &&
        ShouldAdvanceAlythessTankPosition(alythess, bot, _lastBlazeGuid))
    {
        uint8 safeIndex = index;
        if (!findSafeAlythessTankIndex(index, false, safeIndex))
            return false;

        index = safeIndex;
        _alythessTankStep = index;
        Position const newPos = GetAlythessTankPosition(alythess, index);

        if (bot->GetExactDist2d(newPos) > maxDistance)
            return StepToPosition(alythess, newPos, maxDistance);
    }
    else if (distToPosition > maxDistance)
        return StepToPosition(alythess, position, maxDistance);

    return false;
}

bool EredarTwinsAlythessTankMoveOutOfBlazeAction::StepToPosition(
    Unit* alythess, Position const& position, float arrivalDist)
{
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, alythess, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool EredarTwinsRangedStackAtBalconyEdgeAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash && sacrolash->GetVictim() != bot && GetEredarTwinsBlazeTarget(bot) != bot)
    {
        Position const& position = EREDAR_TWINS_P1_RANGED_POSITION;
        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    // Jump down during Phase 2 or if the bot pulls aggro on Alythess or Sacrolash
    else if (bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
        Position const& jumpPosition = EREDAR_TWINS_P1_RANGED_POSITION;
        Position const landingPosition = GetEredarTwinsP2RangedPosition(alythess);

        constexpr float arrivalDistance = 2.0f;
        float const distanceToJumpPos = bot->GetExactDist2d(jumpPosition);

        if (distanceToJumpPos > arrivalDistance)
        {
            float moveX;
            float moveY;
            bool backwards;
            if (!GetStepToPosition(
                    bot, jumpPosition, arrivalDistance, nullptr, moveX, moveY, backwards))
            {
                return false;
            }

            return MoveTo(
                SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return JumpTo(
            SWP_MAP_ID, landingPosition.GetPositionX(), landingPosition.GetPositionY(),
            landingPosition.GetPositionZ(), MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool EredarTwinsStackInRoomCenterAction::Execute(Event /*event*/)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return false;

    Position const position = PlayerbotAI::IsRanged(bot) ?
        GetEredarTwinsP2RangedPosition(alythess) :
        GetEredarTwinsP2MeleePosition(alythess);

    constexpr float arrivalDist = 0.5f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, nullptr, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool EredarTwinsDpsPrioritizeSacrolashAction::Execute(Event /*event*/)
{
    // Start the 8s clock for tanks to get aggro first.
    EmplaceIfUnset(EredarTwinsState(bot->GetInstanceId()).dpsHoldStartMs, getMSTime());

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    Unit* twinTarget =
        sacrolash ? sacrolash : AI_VALUE2(Unit*, "find target", "grand warlock alythess");

    if (!twinTarget)
        return false;

    bool const shouldHoldThreat = sacrolash ?
        ShouldHoldSacrolashThreat(bot, twinTarget) : ShouldHoldAlythessThreat(bot, twinTarget);

    if (shouldHoldThreat && bot->GetVictim())
    {
        bot->AttackStop();
        bot->InterruptSpell(CURRENT_MELEE_SPELL);
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid());
        return true;
    }

    return AI_VALUE(Unit*, "current target") != twinTarget && Attack(twinTarget);
}

bool EredarTwinsConflagrationTargetMoveFromGroupAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_ROGUE && botAI->CanCastSpell("vanish", bot) &&
        botAI->CastSpell("vanish", bot))
    {
        return true;
    }

    if (AI_VALUE2(Unit*, "find target", "lady sacrolash"))
    {
        Position const& position = PlayerbotAI::IsRanged(bot) ?
            EREDAR_TWINS_RANGED_CONFLAG_POSITION : EREDAR_TWINS_MELEE_CONFLAG_POSITION;

        if (bot->GetExactDist2d(position) <= 1.0f)
            return false;

        return MoveTo(
            SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, CONFLAGRATION_SAFE_DISTANCE);
    if (!nearestPlayer)
        return false;

    bot->CastStop();
    return MoveAway(
        nearestPlayer, CONFLAGRATION_SAFE_DISTANCE - bot->GetExactDist2d(nearestPlayer));
}

bool EredarTwinsMoveAwayFromSacrolashVictimAction::Execute(Event /*event*/)
{
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return false;

    Unit* victim = sacrolash->GetVictim();
    if (!victim)
        return false;

    if (bot->GetExactDist2d(victim) >= CONFLAGRATION_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveFromGroup(CONFLAGRATION_SAFE_DISTANCE);
}
