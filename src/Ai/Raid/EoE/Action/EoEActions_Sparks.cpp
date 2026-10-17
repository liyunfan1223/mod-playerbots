/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEActions_Sparks.h"
#include "EoEData.h"
#include "EoEEncounter_Malygos.h"
#include "Playerbots.h"

#include <utility>

namespace
{
// Where the DK stands decides where the spark lands, so the grip is cast from the parking spot or
// not at all. The walk there belongs to MalygosPositionAction.
Unit* GetPowerSparkToGrip(PlayerbotAI* botAI)
{
    if (!IsOnPowerSparkGripDuty(botAI))
        return nullptr;

    Player* bot = botAI->GetBot();
    std::pair<float, float> const& grip = GetMalygosP1Layout(bot).grip;
    if (bot->GetDistance2d(grip.first, grip.second) > POWER_SPARK_GRIP_TOLERANCE)
        return nullptr;

    Unit* spark = GetNearestPowerSparkTo(botAI, grip.first, grip.second);
    return spark && botAI->CanCastSpell("death grip", spark) ? spark : nullptr;
}
}

bool PullPowerSparkAction::isUseful()
{
    if (Unit* spark = GetPowerSparkToGrip(botAI))
    {
        _sparkGuid = spark->GetGUID();
        _grip = true;
        return true;
    }

    // The grip only buys the distance back once, and the spark covers 6 yd/s walking it off again.
    // Its immunity mask (creature_immunities -335) carries neither root nor snare, so this lands.
    Unit* snare = GetPowerSparkToSnare(botAI);
    if (!snare || !botAI->CanCastSpell("chains of ice", snare))
        return false;

    _sparkGuid = snare->GetGUID();
    _grip = false;
    return true;
}

bool PullPowerSparkAction::Execute(Event /*event*/)
{
    Unit* spark = botAI->GetUnit(_sparkGuid);
    return spark && botAI->CastSpell(_grip ? "death grip" : "chains of ice", spark);
}

bool KillPowerSparkAction::isUseful()
{
    // Only a spark reachable standing still, nobody walks in P1. DK grips are PullPowerSparkAction.
    MalygosAggro const& aggro = GetMalygosAggro(botAI);
    if (!PlayerbotAI::IsDps(bot) || aggro.mainTank || aggro.victim)
        return false;

    Unit* spark = GetPowerSparkToKill(botAI, AI_VALUE(Unit*, "current target"));
    if (!spark)
        return false;

    _sparkGuid = spark->GetGUID();
    return true;
}

bool KillPowerSparkAction::Execute(Event /*event*/)
{
    Unit* spark = botAI->GetUnit(_sparkGuid);
    if (!spark)
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || currentTarget->GetGUID() != spark->GetGUID())
        return Attack(spark);
    return false;
}
