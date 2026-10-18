/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "PlayerbotAI.h"
#include "Strategy.h"
#include "UldActions_Ignis.h"
#include "UldData.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Ignis.h"
#include "UldMultipliers_Ignis.h"
#include "UldTriggers_Ignis.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool InUlduar(PlayerbotAI* botAI) { return botAI->GetBot()->GetMapId() == ULDUAR_MAP_ID; }

// Main tank, plus a construct tank only while it holds one: between constructs it has no spot, and blocking
// its generic movers would leave it standing still.
bool IgnisPlacedTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return PlayerbotAI::IsMainTank(bot) || GetIgnisHeldConstruct(botAI, bot);
}

// GetIgnis walks the grid, so it goes last.
bool IgnisSlagPotRide(PlayerbotAI* botAI)
{
    return InUlduar(botAI) && IsIgnisSlagPotVictim(botAI->GetBot()) && GetIgnis(botAI);
}

bool IgnisParkedTank(PlayerbotAI* botAI) { return InUlduar(botAI) && IgnisPlacedTank(botAI) && GetIgnis(botAI); }

bool IgnisTankHeld(PlayerbotAI* botAI) { return InUlduar(botAI) && IgnisPlacedTank(botAI) && IsIgnisEngaged(botAI); }

bool IgnisOwnsTargets(PlayerbotAI* botAI) { return InUlduar(botAI) && IsIgnisEngaged(botAI); }

void DefineIgnis(EncounterBuilder& e)
{
    e.Node(
        "ignis fire resistance trigger",
        [](PlayerbotAI* botAI) -> Trigger* { return new BossFireResistanceTrigger(botAI, "ignis the furnace master"); },
        "ignis fire resistance action",
        [](PlayerbotAI* botAI) -> Action* { return new BossFireResistanceAction(botAI, "ignis the furnace master"); },
        ACTION_RAID);
    e.Node<IgnisScorchedGroundTrigger, IgnisScorchedGroundAction>(ACTION_RAID + 2);

    // Tank spots outrank the raid's damage: the main tank's spot picks where every patch lands, and the kite is
    // the only way a construct dies. They share a band since no bot is ever both.
    e.Node<IgnisMainTankPositionTrigger, IgnisMainTankPositionAction>(ACTION_RAID + 4);
    e.Node<IgnisConstructTankTrigger, IgnisConstructTankAction>(ACTION_RAID + 4);
    e.Node<IgnisAttackBrittleConstructTrigger, IgnisAttackBrittleConstructAction>(ACTION_RAID + 3);
    e.Node<IgnisAttackBossTrigger, IgnisAttackBossAction>(ACTION_RAID + 0.5f);
    // Standing by a Molten construct or sitting in a Slag Pot kills a bot outright.
    e.Node<IgnisMoltenConstructAvoidTrigger, IgnisMoltenConstructAvoidAction>(ACTION_EMERGENCY);
    e.Node<IgnisSlagPotHealTrigger, IgnisSlagPotHealAction>(ACTION_EMERGENCY + 1);
    e.Node<IgnisFlameJetsTrigger, IgnisFlameJetsHoldCastAction>(ACTION_EMERGENCY + 2);

    // Slag Pot is a vehicle ride: the victim is held in place for the full duration, so movement
    // orders only fight the ride and leave the bot facing the wrong way when it drops.
    e.Exclusive("ignis", Role::Any, IgnisSlagPotRide);

    // Both dodges, the generic one too: at ACTION_EMERGENCY it outranks the kite. The construct tank sits in a
    // patch on purpose, and the main tank's arc already steps him clear.
    e.Block("ignis", Role::Any, IgnisParkedTank, Family::AvoidAoe, {IgnisScorchedGroundAction::Name});

    // Placed tanks only: the rest keep their generic movers, which spread the ranged half, and a blanket block
    // would freeze the whole raid if a replacement mover ever failed quietly.
    e.Block("ignis tank movement", Role::Any, IgnisTankHeld,
            Family::Reach | Family::ReachHeal | Family::Charge | Family::Follow | Family::Flee);

    // Whole fight, not just the Brittle window: left on, the generic pickers re-pick on any tick the settled
    // attack returns false, walking bots off the Brittle construct into the shatter blast.
    e.OwnTargeting("ignis disable default targeting", Role::Any, IgnisOwnsTargets,
                   Family::DpsAssist | Family::TankAssist | Family::AttackRti);

    e.Multiplier<IgnisFlameJetsHoldCastMultiplier>(Family::Spell);
}
}  // namespace

EncounterDefinition const& UldIgnisDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_IGNIS, &BossStateGateOpen, &DefineIgnis);
    return definition;
}
