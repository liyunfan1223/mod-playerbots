/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBActions.h"
#include "Playerbots.h"
#include "Timer.h"
#include "UBShared.h"
#include <cmath>

using namespace UnderbogHungarfen;

bool UBRetreatFromFoulSporesAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
    if (!boss)
        return false;

    float const safeDistance = MaxEffectRadius(SPELL_FOUL_SPORES, FOUL_SPORES_RADIUS_FALLBACK) + FOUL_SPORES_MARGIN;
    float const currentDistance = bot->GetDistance2d(boss);
    if (currentDistance >= safeDistance)
        return false;

    float const moveDist = safeDistance - currentDistance + 1.0f;
    float const awayAngle = boss->GetAngle(bot);

    auto const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");

    for (float delta : { 0.0f, float(M_PI / 8), float(-M_PI / 8), float(M_PI / 4), float(-M_PI / 4),
                         float(3 * M_PI / 8), float(-3 * M_PI / 8), float(M_PI / 2), float(-M_PI / 2) })
    {
        float const angle = awayAngle + delta;
        float dx = bot->GetPositionX() + cos(angle) * moveDist;
        float dy = bot->GetPositionY() + sin(angle) * moveDist;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
            continue;

        if (RetreatPathUnsafe(bot, mushrooms, dx, dy))
            continue;

        if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true,
                   MovementPriority::MOVEMENT_COMBAT))
            return true;
    }

    return MoveAway(boss, moveDist);
}

bool UBVacateSporeCloudAction::Execute(Event /*event*/)
{
    float const dangerRange = MushroomDangerRange(bot);
    auto const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    Creature* mushroom = GetNearestDangerousMushroom(bot, mushrooms, dangerRange);
    if (!mushroom)
        return false;

    if (PlayerbotAI::IsTank(bot))
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "hungarfen");
        if (boss && boss->GetVictim() == bot)
        {
            float const currentDistance = bot->GetDistance2d(mushroom);
            return MoveAway(mushroom, dangerRange - currentDistance + 2.0f);
        }
    }

    return FleePosition(mushroom->GetPosition(), dangerRange);
}

bool UBClearUnderbatBackAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    Creature* bat = GetNearestUnderbatInLashRange(bot, attackers);
    if (!bat)
        return false;

    bool const throttled = _lastReposition && GetMSTimeDiffToNow(_lastReposition) < UNDERBAT_REPOSITION_COOLDOWN;

    bool const melee = PlayerbotAI::IsMelee(bot);
    if (melee)
    {
        Unit* rally = UnderbatRallyUnit(bot, attackers);
        bool const parked = rally && bot->GetExactDist(rally) <= UNDERBAT_RALLY_TOLERANCE;

        if (rally && !parked)
        {
            if (throttled)
                return false;

            float const rx = rally->GetPositionX();
            float const ry = rally->GetPositionY();
            float const rz = rally->GetPositionZ();
            if (SpotClearOfUnderbats(bot, attackers, rx, ry, rz) &&
                MoveTo(bot->GetMapId(), rx, ry, rz, false, false, true, true,
                       MovementPriority::MOVEMENT_COMBAT))
            {
                _lastReposition = getMSTime();
                return true;
            }
        }

        if (!GetLashingUnderbat(bot, attackers))
            return false;
    }
    else if (bot->IsNonMeleeSpellCast(true) && !GetLashingUnderbat(bot, attackers))
        return false;

    if (throttled)
        return false;

    float const clearDistance = UNDERBAT_LASH_RANGE + UNDERBAT_LASH_MARGIN;
    float const gap = bot->GetDistance2d(bat);
    if (gap >= clearDistance)
        return false;

    if (!MoveAway(bat, clearDistance - gap + 1.0f))
        return false;

    _lastReposition = getMSTime();
    return true;
}
