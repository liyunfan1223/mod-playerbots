/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulActions.h"
#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

using namespace GruulHelpers;
using namespace EncounterHelpers;

// General

bool GruulsLairResetEncounterStatesAction::Execute(Event /*event*/)
{
    bool reset = false;

    Action* action = context->GetAction("gruul the dragonkiller spread ranged");
    if (action &&
        static_cast<GruulTheDragonkillerSpreadRangedAction*>(action)->ResetInitialPosition())
    {
        reset = true;
    }

    if (IsMechanicTrackerBot(bot, GRUUL_MAP_ID) && !AI_VALUE2(bool, "combat", "self target"))
    {
        reset |= ClearTargetIcon(bot, RtiTargetValue::skullIndex);
        reset |= ClearTargetIcon(bot, RtiTargetValue::crossIndex);
    }

    return reset;
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarMeleeTanksPositionBossesAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Position position;
    if (IsMaulgarTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "high king maulgar");
        position = MAULGAR_TANK_POSITION;
    }
    else if (IsOlmTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "olm the summoner");
        position = OLM_TANK_POSITION;
    }
    else if (IsBlindeyeTank(bot))
    {
        target = AI_VALUE2(Unit*, "find target", "blindeye the seer");
        position = BLINDEYE_TANK_POSITION;
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (target->GetVictim() != bot || !bot->IsWithinMeleeRange(target))
        return false;

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, target, moveX, moveY, backwards))
        return false;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool HighKingMaulgarMageTankAttackKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    if (AttackAndCast(krosh))
        return true;

    if (krosh->GetVictim() != bot)
        return false;

    return MoveToDesiredDistance(krosh);
}

bool HighKingMaulgarMageTankAttackKroshAction::AttackAndCast(Unit* krosh)
{
    if (krosh->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)) &&
        botAI->CanCastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh) &&
        botAI->CastSpell(Id(GruulSpells::SPELL_SPELLSTEAL), krosh))
    {
        return true;
    }

    if (AI_VALUE(Unit*, "current target") != krosh)
        return Attack(krosh);

    if (bot->HasAura(Id(GruulSpells::SPELL_SPELL_SHIELD)))
        return false;

    return botAI->CanCastSpell("fire ward", bot) && botAI->CastSpell("fire ward", bot);
}

// The Mage tank moves to a designated position only if Krosh is far enough from that position to
// be tanked from it without standing in Blast Wave, and close enough to still be tanked at all.
bool HighKingMaulgarMageTankAttackKroshAction::MoveToDesiredDistance(Unit* krosh)
{
    Position const& position = KROSH_TANK_POSITION;
    float const distanceKroshToPosition = krosh->GetExactDist2d(position);
    constexpr float minDistance = KROSH_BLAST_WAVE_SAFE_DISTANCE;
    constexpr float maxDistance = 30.0f;

    if (distanceKroshToPosition > minDistance && distanceKroshToPosition < maxDistance &&
        bot->GetExactDist2d(position) > 1.0f)
    {
        return MoveTo(
            GRUUL_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    float const currentDistance = bot->GetExactDist2d(krosh);
    if (currentDistance >= KROSH_BLAST_WAVE_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(krosh, KROSH_BLAST_WAVE_SAFE_DISTANCE - currentDistance);
}

// The moonkin tank has no tank position, but usually Kiggler remains close to where he starts.
bool HighKingMaulgarMoonkinTankAttackKigglerAction::Execute(Event /*event*/)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler)
        return false;

    if (AI_VALUE(Unit*, "current target") != kiggler)
        return Attack(kiggler);

    if (kiggler->GetVictim() != bot)
        return false;

    float const currentDistance = bot->GetExactDist2d(kiggler);
    if (currentDistance >= KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE)
        return false;

    return MoveAway(kiggler, KIGGLER_ARCANE_EXPLOSION_SAFE_DISTANCE - currentDistance);
}

// Priority: (1) Blindeye, (2) Olm, (3) Krosh (ranged only), (4) Kiggler, and (5) Maulgar
bool HighKingMaulgarAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    Unit* krosh = nullptr;
    if (Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer"))
    {
        target = blindeye;
    }
    else if (Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner"))
    {
        target = olm;
    }
    else if (PlayerbotAI::IsRanged(bot) &&
        (krosh = AI_VALUE2(Unit*, "find target", "krosh firehand")))
    {
        target = krosh;
    }
    else if (Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed"))
    {
        target = kiggler;
    }
    else if (Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar"))
    {
        target = maulgar;
    }

    if (!target)
        return false;

    if (target == krosh)
    {
        if (MarkTargetWithCross(bot, target))
            return true;
    }
    else if (MarkTargetWithSkull(bot, target))
    {
        return true;
    }

    return AI_VALUE(Unit*, "current target") != target && Attack(target);
}

bool HighKingMaulgarRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    float const currentDistance = bot->GetExactDist2d(maulgar);
    if (currentDistance >= MAULGAR_WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(maulgar, MAULGAR_WHIRLWIND_SAFE_DISTANCE - currentDistance);
}

bool HighKingMaulgarBackAwayFromKroshAction::Execute(Event /*event*/)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh)
        return false;

    float const currentDistance = bot->GetExactDist2d(krosh);
    if (currentDistance >= KROSH_BLAST_WAVE_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return FleePosition(krosh->GetPosition(), KROSH_BLAST_WAVE_SAFE_DISTANCE);
}

bool HighKingMaulgarBanishFelStalkerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Ordered by GUID so that every bot warlock indexes the same list.
    std::vector<Unit*> const felStalkers = GetNearbyWildFelStalkers(botAI);
    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == GRUUL_MAP_ID &&
            member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
        {
            warlocks.push_back(member);
        }
    }

    auto const it = std::find(warlocks.begin(), warlocks.end(), bot);
    if (it == warlocks.end())
        return false;

    size_t const warlockIndex = static_cast<size_t>(std::distance(warlocks.begin(), it));
    if (warlockIndex >= felStalkers.size())
        return false;

    Unit* assignedFelStalker = felStalkers[warlockIndex];
    if (botAI->HasAura("banish", assignedFelStalker))
        return false;

    return botAI->CanCastSpell("banish", assignedFelStalker) &&
        botAI->CastSpell("banish", assignedFelStalker);
}

// Misdirect order: Blindeye, Olm, Kiggler, Krosh
bool HighKingMaulgarMisdirectOgresToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            member->GetMapId() == GRUUL_MAP_ID && GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 4)
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

    Unit* ogre = nullptr;
    Player* tank = nullptr;
    if (hunterIndex == 0)
    {
        ogre = AI_VALUE2(Unit*, "find target", "blindeye the seer");
        tank = GetGroupAssistTank(bot, 1);
    }
    else if (hunterIndex == 1)
    {
        ogre = AI_VALUE2(Unit*, "find target", "olm the summoner");
        tank = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 2)
    {
        ogre = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
        tank = GetKigglerMoonkinTank(botAI);
    }
    else if (hunterIndex == 3)
    {
        ogre = AI_VALUE2(Unit*, "find target", "krosh firehand");
        tank = GetKroshMageTank(botAI);
    }

    if (!ogre || !tank || !tank->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    if (!bot->HasAura(Id(GruulSpells::SPELL_MISDIRECTION)))
        return false;

    return botAI->CanCastSpell("steady shot", ogre) && botAI->CastSpell("steady shot", ogre);
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    if (AI_VALUE(Unit*, "current target") != gruul)
        return Attack(gruul);

    if (gruul->GetVictim() != bot || !bot->IsWithinMeleeRange(gruul))
        return false;

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, GRUUL_TANK_POSITION, arrivalDist, gruul, moveX, moveY, backwards))
        return false;

    return MoveTo(
        GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Position const& position = GRUUL_TANK_POSITION;

    if (!_hasInitialPosition)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
                !GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRanged(member))
            {
                continue;
            }

            members.push_back(member);
        }

        if (members.empty())
            return false;

        auto it = std::find(members.begin(), members.end(), bot);
        size_t const botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;

        constexpr float minRadius = 25.0f;
        constexpr float maxRadius = 40.0f;
        float const angle = 2.0f * M_PI * botIndex / members.size();
        float const radius = frand(minRadius, maxRadius);
        float const targetX = position.GetPositionX() + radius * std::cos(angle);
        float const targetY = position.GetPositionY() + radius * std::sin(angle);

        _initialPosition = Position(targetX, targetY, position.GetPositionZ());
        _hasInitialPosition = true;
    }

    if (!_hasReachedInitialPosition)
    {
        float const distToTarget = bot->GetExactDist2d(_initialPosition);
        if (distToTarget <= 2.0f)
        {
             _hasReachedInitialPosition = true;
            return false;
        }

        float const moveDist = std::min(3.5f, distToTarget);
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();
        float const moveX =
            botX + ((_initialPosition.GetPositionX() - botX) / distToTarget) * moveDist;
        float const moveY =
            botY + ((_initialPosition.GetPositionY() - botY) / distToTarget) * moveDist;

        return MoveTo(
            GRUUL_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    constexpr float minSpreadDistance = 10.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minSpreadDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minSpreadDistance);
}

// This method attempts to have more preemptive avoidance and post-avoidance awareness for Cave-ins
// as compared to avoid aoe (which recognizes the dynobj only once the bot actually has a damaging
// aura applied to it and immediately forgets the dynobj once the bot is out of danger).
bool GruulTheDragonkillerGetOutOfCaveInAction::Execute(Event /*event*/)
{
    Position pool;
    if (!GetNearestCaveInPosition(botAI, pool))
        return false;

    constexpr uint32 minInterval = 0;
    return FleePosition(pool, CAVE_IN_RADIUS, minInterval);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event /*event*/)
{
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, GRUUL_SHATTER_SAFE_DISTANCE);
    if (!nearestPlayer)
        return false;

    float const distToNearest = bot->GetExactDist2d(nearestPlayer);
    float const moveDist = std::min(3.5f, GRUUL_SHATTER_SAFE_DISTANCE - distToNearest);

    return MoveAway(nearestPlayer, moveDist);
}
