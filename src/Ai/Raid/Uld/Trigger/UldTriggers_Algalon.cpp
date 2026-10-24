/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Algalon.h"

#include "EncounterHelpers.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidTankDefensive.h"
#include "SharedDefines.h"
#include "UldData.h"
#include "UldEncounter_Algalon.h"
#include "Unit.h"
#include <RtiTargetValue.h>

using namespace EncounterHelpers;

// Algalon the Observer

// Big Bang is physical, so a tank soaks on armour and a physical button, also one with no hole in reach.
// The button follows what the bot can cast, not its slot: a promoted backup is still a priest.
bool AlgalonBigBangSoakTrigger::IsActive()
{
    if (IsAlgalonPhased(bot) || !AlgalonBigBangLatched(bot) || (!AlgalonStaysOut(bot) && !AlgalonTankCaughtOut(bot)))
        return false;

    int32 const remaining = AlgalonBigBangRemainingMs(botAI);
    if (remaining < 0)
        return false;

    if (AlgalonCanDisperse(bot))
        return remaining <= ULDUAR_ALGALON_SOAK_DISPERSION_MS;

    return remaining <= ULDUAR_ALGALON_SOAK_DEFENSIVE_MS &&
           NextTankDefensive(botAI, bot, true) != 0;
}

bool AlgalonBigBangExternalTrigger::IsActive()
{
    uint8 const playerClass = bot->getClass();
    if ((playerClass != CLASS_PRIEST && playerClass != CLASS_PALADIN) || IsAlgalonPhased(bot) ||
        !AlgalonBigBangLatched(bot))
        return false;

    if (AlgalonBigBangRemainingMs(botAI) < ULDUAR_ALGALON_EXTERNAL_MIN_REMAINING_MS)
        return false;

    // Dispersion leaves a priest about 6k of the hit; these belong on the tanks.
    Player* soaker = GetAlgalonBigBangSoaker(botAI);
    if (!soaker || soaker == bot || GetAlgalonBigBangBackup(botAI) == bot || soaker->HasSpell(SPELL_ALGALON_DISPERSION))
        return false;

    return GetAlgalonSoakerExternal(botAI, soaker) != nullptr;
}

// One layer per healer while it still has a global cooldown to spare before its walk to the hole: the
// healers are phased for a second after the hit, and whatever is already ticking on him is all he has.
bool AlgalonSoakerPrepTrigger::IsActive()
{
    if (!AlgalonBigBangLatched(bot) || IsAlgalonPhased(bot) || !PlayerbotAI::IsHeal(bot) ||
        AlgalonHideSlackMs(bot) <= ULDUAR_ALGALON_PREP_GCD_MS)
        return false;

    Player* soaker = GetAlgalonBigBangSoaker(botAI);
    if (!soaker || soaker == bot || IsAlgalonPhased(soaker) || soaker->HasSpell(SPELL_ALGALON_DISPERSION))
        return false;

    return GetAlgalonSoakerPrep(botAI, soaker) != nullptr;
}

// Once out of the field, a healer's first cast goes on the soaker while he is low.
bool AlgalonSoakerRescueTrigger::IsActive()
{
    Player* soaker = GetAlgalonRescueTarget(botAI);
    if (!soaker || soaker == bot || IsAlgalonPhased(bot) || AlgalonHoleNear(bot, ULDUAR_ALGALON_SHELTER_RADIUS) ||
        !PlayerbotAI::IsHeal(bot))
        return false;

    return GetAlgalonRescueSpell(botAI, soaker) != nullptr;
}

// A tank healer's tank gets its fastest heal ahead of the slot walk; the rest of its healing waits behind the
// mana tools. The class rotations heal whoever is lowest in percent, and the encounter vetoes those for it.
bool AlgalonTankHealCriticalTrigger::IsActive()
{
    if (!IsAlgalonTankHealer(bot))
        return false;

    AlgalonTankHealPick const pick = GetAlgalonTankHealPick(botAI);
    return pick.critical && GetAlgalonTankHealSpell(botAI, pick);
}

bool AlgalonTankHealTrigger::IsActive()
{
    if (!IsAlgalonTankHealer(bot))
        return false;

    AlgalonTankHealPick const pick = GetAlgalonTankHealPick(botAI);
    return pick.target && !pick.critical && GetAlgalonTankHealSpell(botAI, pick);
}

// Beacon on the swap tank he isn't hitting doubles every heal on the holder onto the partner.
bool AlgalonTankBeaconTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN || !IsAlgalonTankHealer(bot) || IsAlgalonPhased(bot) ||
        GetAlgalonTankHealPick(botAI).critical)
        return false;

    Player* target = GetAlgalonBeaconTarget(botAI);
    return target && botAI->CanCastSpell("beacon of light", target);
}

bool AlgalonTankDefensiveTrigger::IsActive() { return GetAlgalonTankCut(botAI).spellId != 0; }

bool AlgalonExplosionCoverTrigger::IsActive() { return GetAlgalonExplosionCover(botAI) != nullptr; }

bool AlgalonExplosionPersonalTrigger::IsActive() { return static_cast<bool>(GetAlgalonExplosionPersonal(botAI)); }

bool AlgalonShadowAuraTrigger::IsActive()
{
    return IsAlgalonShadowAuraPaladin(bot) && AlgalonInPhaseOne(botAI) &&
           !botAI->HasAura("shadow resistance aura", bot, false, true) &&
           botAI->CanCastSpell("shadow resistance aura", bot);
}

bool AlgalonBigBangHideTrigger::IsActive() { return GetAlgalonHideRole(bot) == AlgalonHideRole::Run; }

bool AlgalonCosmicSmashTrigger::IsActive()
{
    if (!AlgalonEngaged(botAI))
        return false;

    bool const smash = AlgalonCosmicSmashThreatens(bot);
    bool const star = !smash && AlgalonDyingStarThreatens(bot);

    return smash || star;
}

// A bot still inside the field when its 10s phase ends is phased again within a second, and while
// phased it can't see the hole at all, only the cached position.
bool AlgalonLeaveBlackHoleTrigger::IsActive()
{
    if (!AlgalonEngaged(botAI))
        return false;

    if (IsAlgalonPhased(bot))
        return AlgalonHoleNear(bot, ULDUAR_ALGALON_HOLE_EXIT_RADIUS) ||
               AlgalonUnderStarNear(bot, ULDUAR_ALGALON_STAR_DYING_RADIUS);

    return !AlgalonBigBangCasting(botAI) && AlgalonHoleNear(bot, ULDUAR_ALGALON_SHELTER_RADIUS);
}

// Misdirection and Tricks on the pull go to the tank holding him. Out of range the rogue waits: the class
// value would give Tricks to a dps, who then pulls him.
bool AlgalonThreatRedirectTrigger::IsActive()
{
    char const* spell = AlgalonThreatRedirectSpell(bot);
    if (!spell || IsAlgalonPhased(bot) || !AlgalonInOpener(botAI))
        return false;

    Player* tank = GetAlgalonOpenerTank(botAI);
    if (!tank || tank == bot)
        return false;

    float const range =
        bot->getClass() == CLASS_ROGUE ? ULDUAR_ALGALON_TRICKS_RANGE : ULDUAR_ALGALON_MISDIRECTION_RANGE;
    return bot->GetExactDist(tank) <= range && botAI->CanCastSpell(spell, tank);
}

// At the end of the intro he swings at a random player, and a phased tank drops him the same way. A holder
// that got him by threat still targets whatever it hit before, a constellation for a former handler.
bool AlgalonTankPickupTrigger::IsActive()
{
    if (AlgalonShouldPickUp(bot))
        return true;

    Unit* boss = GetAlgalon(botAI);
    return boss && !boss->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && boss->GetVictim() == bot &&
           IsAlgalonSwapTank(bot) && !IsAlgalonPhased(bot) && AlgalonEngaged(botAI) &&
           AI_VALUE(Unit*, "current target") != boss;
}

bool AlgalonPhasePunchSwapTrigger::IsActive() { return AlgalonShouldSwap(bot); }

// Auto-attack is no action, so the damage guard never stops it, and white swings alone take him back.
bool AlgalonYieldBossTrigger::IsActive()
{
    Unit* boss = GetAlgalon(botAI);
    return boss && AlgalonYieldsBoss(bot) && (bot->GetVictim() == boss || AI_VALUE(Unit*, "current target") == boss);
}

bool AlgalonConstellationTauntTrigger::IsActive()
{
    if (IsAlgalonPhased(bot) || !AlgalonEngaged(botAI) || AlgalonBigBangCasting(botAI) || AlgalonTakesBossNow(bot))
        return false;

    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    return constellation && constellation->GetVictim() != bot;
}

bool AlgalonDarkMatterTankTrigger::IsActive()
{
    if (IsAlgalonPhased(bot) || !AlgalonEngaged(botAI) || AlgalonBigBangCasting(botAI))
        return false;

    return GetAlgalonLooseDarkMatter(bot) != nullptr;
}

bool AlgalonConstellationKiteTrigger::IsActive()
{
    if (IsAlgalonPhased(bot) || !AlgalonEngaged(botAI) || AlgalonBigBangCasting(botAI) || AlgalonTakesBossNow(bot))
        return false;

    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    return constellation && constellation->GetVictim() == bot;
}

bool AlgalonStarTeamTrigger::IsActive()
{
    if (IsAlgalonPhased(bot) || !IsAlgalonStarTeam(bot) || !AlgalonEngaged(botAI))
        return false;

    Unit* star = GetAlgalonFocusStar(botAI);
    return star && AI_VALUE(Unit*, "current target") != star;
}

// For the humans in the raid. Not skull: every bot's "attack rti target" reads skull.
bool AlgalonStarMarkTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, ULDUAR_MAP_ID) || !AlgalonEngaged(botAI))
        return false;

    Unit* star = GetAlgalonFocusStar(botAI);
    Group* group = bot->GetGroup();
    return star && group && group->GetTargetIcon(RtiTargetValue::starIndex) != star->GetGUID();
}

// Presence gated, not combat gated: the intro is the only free window to put the raid on its spots.
bool AlgalonRaidPositionTrigger::IsActive()
{
    if (!AlgalonPresent(botAI))
        return false;

    // A hider belongs to its hole from the moment it runs; the soaker and backup keep their spots.
    if (AlgalonShouldRunForShelter(bot))
        return false;

    // The swap tank not holding him works the adds from wherever they are; its spot is for idle time.
    if (AlgalonOffTankWorking(bot))
        return false;

    // With a pickup or swap due the spot is the way back to him; the constellations follow.
    Unit* constellation = GetAlgalonHandlerConstellation(bot);
    return !constellation || constellation->GetVictim() != bot || AlgalonTakesBossNow(bot);
}
