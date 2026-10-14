/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"

using namespace GruulHelpers;
using namespace EncounterHelpers;

// General

bool GruulsLairNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, GRUUL_MAP_ID);
}

// High King Maulgar <Lord of the Ogres>

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActiveInEncounter()
{
    if (IsBlindeyeTank(bot))
        return AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsOlmTank(bot))
        return AI_VALUE2(Unit*, "find target", "olm the summoner");

    return IsMaulgarTank(bot) && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActiveInEncounter()
{
    return IsKroshMageTank(botAI) && AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActiveInEncounter()
{
    return IsKigglerMoonkinTank(botAI) && AI_VALUE2(Unit*, "find target", "kiggler the crazed");
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return false;

    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !AI_VALUE2(Unit*, "find target", "olm the summoner");

    if (IsBlindeyeTank(bot))
        return !AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsKroshMageTank(botAI))
        return !AI_VALUE2(Unit*, "find target", "krosh firehand");

    if (IsKigglerMoonkinTank(botAI))
        return !AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return true;
}

bool HighKingMaulgarChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarShouldStandBackFromKroshTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(botAI))
        return false;

    return AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActiveInEncounter()
{
    return bot->getClass() == CLASS_WARLOCK && !GetNearbyWildFelStalkers(botAI).empty();
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_ENGAGED_HEALTH_PCT;
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerInCaveInTrigger::IsActiveInEncounter()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return false;

    // The tank holding Gruul eats it. Moving him messes up the spread and puts other bots in
    // danger.
    if (gruul->GetVictim() == bot)
        return false;

    return IsInCaveIn(botAI);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActiveInEncounter()
{
    return HasGroundSlam(bot);
}
