/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_IronAssembly.h"
#include "UldActions_Shared.h"

#include <cmath>

#include "AiObjectContext.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "EncounterHelpers.h"
#include "ServerFacade.h"
#include "UldData.h"
#include "UldEncounter_IronAssembly.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "Unit.h"

using namespace EncounterHelpers;

bool IronAssemblyResetEncounterStateAction::Execute(Event /*event*/)
{
    ResetIronAssemblyEncounterState(bot, false);

    // Never claims the tick: clearing state is bookkeeping, and the pull still needs every node below
    // this one to run on the same tick.
    return false;
}

namespace
{

// Where a bot stands when nothing is chasing it: its formation slot, its tank spot, or the boss the
// raid is killing.
bool TryGetIronAssemblyStation(PlayerbotAI* botAI, Player* bot, Position& station)
{
    if (TryGetIronAssemblyRaidSpot(botAI, bot, station))
        return true;

    if (TryGetIronAssemblyTankSpot(botAI, bot, station))
        return true;

    if (Unit* focus = IronAssemblyFocusTarget(botAI))
    {
        station = focus->GetPosition();
        return true;
    }

    return false;
}

// Not FleePosition: it clamps travel to Playerbots.FleeDistance, short of a 20 yd blast, and reads
// one hazard. Ties break toward the bot's station, which holds still, so the answer doesn't slide.
bool TryGetIronAssemblyEscapeSpot(PlayerbotAI* botAI, Player* bot, std::vector<HazardCircle> const& hazards,
                                  Position& spot)
{
    Position station;
    bool const hasStation = TryGetIronAssemblyStation(botAI, bot, station);

    Position const clear = FindNearestPositionClearOfHazards(
        bot, hazards, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_SEARCH_RADIUS, ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_STEP,
        ULDUAR_IRON_ASSEMBLY_ESCAPE_SWEEP_ANGLE_STEP, hasStation ? &station : nullptr);
    if (clear == Position())
        return false;

    spot = clear;
    return true;
}

void AddIronAssemblyHazards(std::vector<Position> const& centres, float clearance,
                            std::vector<HazardCircle>& hazards)
{
    for (Position const& centre : centres)
        hazards.emplace_back(centre, clearance);
}

}  // namespace

bool IronAssemblyLightningTendrilsAction::Execute(Event /*event*/)
{
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir)
        return false;

    // 2d on purpose: he is hovering while the tendrils tick, and the damage is projected on the floor
    // under him rather than measured to where he is floating.
    std::vector<HazardCircle> hazards = {{brundir->GetPosition(), ULDUAR_IRON_ASSEMBLY_TENDRILS_CLEARANCE}};

    Position spot;
    if (!TryGetIronAssemblyEscapeSpot(botAI, bot, hazards, spot))
        return false;

    // MOVEMENT_FORCED: these are the kill-you hazards, so nothing generic gets to argue about it.
    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true);
}

bool IronAssemblyOverloadAction::Execute(Event /*event*/)
{
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir)
        return false;

    std::vector<HazardCircle> hazards = {{brundir->GetPosition(), ULDUAR_IRON_ASSEMBLY_OVERLOAD_CLEARANCE}};

    // Runes of Death too, or this escape walks into one. Each at its own clearance: Overload's would waste
    // four yards per rune inside a 5.5s cast.
    std::vector<Position> const runes = GetIronAssemblyScan(botAI).RunesOfDeath();
    AddIronAssemblyHazards(runes, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE, hazards);

    Position spot;
    if (!TryGetIronAssemblyEscapeSpot(botAI, bot, hazards, spot))
        return false;

    // MOVEMENT_FORCED: these are the kill-you hazards, so nothing generic gets to argue about it.
    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true);
}

bool IronAssemblyRuneOfDeathAction::Execute(Event /*event*/)
{
    std::vector<Position> const runes = GetIronAssemblyScan(botAI).RunesOfDeath();
    if (runes.empty())
        return false;

    std::vector<HazardCircle> hazards;
    AddIronAssemblyHazards(runes, ULDUAR_IRON_ASSEMBLY_RUNE_OF_DEATH_CLEARANCE, hazards);

    Position spot;
    if (!TryGetIronAssemblyEscapeSpot(botAI, bot, hazards, spot))
        return false;

    // MOVEMENT_FORCED: these are the kill-you hazards, so nothing generic gets to argue about it.
    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_FORCED, true);
}

bool IronAssemblyInterruptAction::Execute(Event /*event*/)
{
    Unit* brundir = GetIronAssemblyMember(botAI, NPC_BRUNDIR);
    if (!brundir)
        return false;

    char const* interrupt = IronAssemblyReadyInterrupt(bot, brundir);
    if (!interrupt)
        return false;

    return botAI->CastSpell(interrupt, brundir);
}

bool IronAssemblyTankAssignmentAction::Execute(Event /*event*/)
{
    Unit* boss = IronAssemblyAssignedBoss(botAI, bot);
    if (!boss)
    {
        _spotReached = false;
        return false;
    }

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    // Shared empowered Steelbreaker: the carrier keeps him until Meltdown, an early taunt saves nobody.
    // IronAssemblyTauntGuardMultiplier covers the class taunt nodes but never sees this direct cast.
    Unit* const victim = boss->GetVictim();
    bool const shared = IsSteelbreakerEmpowered(botAI) && boss->GetEntry() == NPC_STEELBREAKER;

    // Stand, do not drag. The tank walks to its spot and the boss follows it there, which never
    // routes a boss through the ranged stack the way a dragged pull does.
    if (victim != bot && !(shared && IronAssemblyHeldByOtherTank(bot, boss)) && CastClassTaunt(botAI, boss))
        return true;

    // Same boss the assignment above returned, rather than ranking the tanks a second time.
    Position spot;
    if (!TryGetIronAssemblyBossTankSpot(bot, boss, spot))
        return false;

    float const distance = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY());

    if (_spotReached && distance > ULDUAR_IRON_ASSEMBLY_TANK_SPOT_TOLERANCE * 2.0f)
        _spotReached = false;

    if (_spotReached || distance <= ULDUAR_IRON_ASSEMBLY_TANK_SPOT_TOLERANCE)
    {
        _spotReached = true;
        return false;
    }

    // A rune on the spot holds him where the escape parked him. The ring stays on the spot, its near arc
    // ~12 yd from him inside Meltdown: fine, a rune lasts 30s and the first Meltdown lands 43s in.
    Position step;
    if (!TryGetIronAssemblyRouteStep(botAI, bot, spot, step))
        return false;

    return MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool IronAssemblyShieldOfRunesAction::Execute(Event /*event*/)
{
    Unit* molgeim = GetIronAssemblyMember(botAI, NPC_MOLGEIM);
    if (!molgeim || !IronAssemblyShieldOfRunesUp(molgeim))
        return false;

    // Offensive magic dispel: spellsteal (mage), purge (shaman), dispel magic (priest). Stripping the
    // shield denies the +50% damage buff it pays out when it is drained instead.
    static std::vector<std::string> const dispels = {"spellsteal", "purge", "dispel magic"};
    for (std::string const& dispel : dispels)
        if (botAI->CanCastSpell(dispel, molgeim))
            return botAI->CastSpell(dispel, molgeim);

    return false;
}

bool IronAssemblyFusionPunchDispelAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static std::vector<std::string> const dispels = {"dispel magic", "cleanse", "mass dispel"};

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != ULDUAR_MAP_ID)
            continue;

        if (!IronAssemblyHasFusionPunch(member))
            continue;

        for (std::string const& dispel : dispels)
            if (botAI->CanCastSpell(dispel, member))
                return botAI->CastSpell(dispel, member);
    }

    return false;
}

bool IronAssemblyRedirectThreatAction::isUseful()
{
    return bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_ROGUE;
}

Player* IronAssemblyRedirectThreatAction::GetRedirectTank()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // Feed whoever holds the kill target: with one tank per member, the group main tank is wrong for two
    // of the three.
    Unit* focus = IronAssemblyFocusTarget(botAI);
    if (focus)
    {
        if (Unit* victim = focus->GetVictim())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member == bot)
                    continue;

                if (PlayerbotAI::IsTank(member) && member == victim)
                    return member;
            }
        }
    }

    return GetGroupMainTank(bot);
}

bool IronAssemblyRedirectThreatAction::Execute(Event /*event*/)
{
    Player* tank = GetRedirectTank();
    if (!tank || tank == bot)
        return false;

    if (bot->getClass() == CLASS_ROGUE)
    {
        // Tricks redirects everything the rogue does for the next 6s, so there is no dump shot.
        return botAI->CanCastSpell("tricks of the trade", tank) && botAI->CastSpell("tricks of the trade", tank);
    }

    if (botAI->CanCastSpell("misdirection", tank))
        return botAI->CastSpell("misdirection", tank);

    // Misdirection only moves the threat of the next three shots. Spend them on the kill target
    // rather than leaving them to whatever the rotation picks.
    Unit* focus = IronAssemblyFocusTarget(botAI);
    if (focus && bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", focus))
        return botAI->CastSpell("steady shot", focus);

    return false;
}

bool IronAssemblyRuneOfPowerSoakAction::Execute(Event /*event*/)
{
    Position rune;
    if (!TryGetIronAssemblyRuneOfPowerSoakSpot(botAI, bot, rune))
        return false;

    return MoveTo(bot->GetMapId(), rune.GetPositionX(), rune.GetPositionY(), rune.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}

bool IronAssemblySetDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* focus = IronAssemblyFocusTarget(botAI);
    if (!focus)
        return false;

    // Every tick and above the early return: PetAttackAction's node is commented out globally, so a pet
    // keeps whatever it latched onto. No-op when the pet is already there.
    CommandPetAttack(botAI, focus);

    // Returning false once the bot is already on the right member is what lets the lower nodes run:
    // the engine ends the tick at the first action that succeeds.
    if (AI_VALUE(Unit*, "current target") == focus)
        return false;

    return Attack(focus);
}

bool IronAssemblyRaidPositionAction::Execute(Event /*event*/)
{
    Position spot;
    if (!TryGetIronAssemblyRaidSpot(botAI, bot, spot))
    {
        _spotReached = false;
        return false;
    }

    float const distance = bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY());

    // Reach then hold, with a deadband: a move per yard of drift restarts the spline, and a moving bot
    // never casts or kicks Lightning Whirl.
    if (_spotReached && distance > ULDUAR_IRON_ASSEMBLY_SLOT_TOLERANCE * 2.0f)
        _spotReached = false;

    if (_spotReached || distance <= ULDUAR_IRON_ASSEMBLY_SLOT_TOLERANCE)
    {
        _spotReached = true;
        return false;
    }

    Position step;
    if (!TryGetIronAssemblyRouteStep(botAI, bot, spot, step))
        return false;

    return MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT, true);
}
