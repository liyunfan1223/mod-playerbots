/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "UldActions_Hodir.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Hodir.h"
#include "UldMultipliers_Hodir.h"
#include "UldTriggers_Hodir.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
// Every guard below waits for the pull: the nodes they hand generic behaviour to don't run before it,
// so on sight alone they'd leave bots rooted with no target.

bool HodirTauntSuicide(PlayerbotAI* botAI)
{
    return IsHodirEngaged(botAI) && HodirTauntWouldBeSuicide(botAI, botAI->GetBot());
}

bool HodirOwnsTargets(PlayerbotAI* botAI) { return IsHodirEngaged(botAI) && !PlayerbotAI::IsHeal(botAI->GetBot()); }

bool HodirFreezeRun(PlayerbotAI* botAI) { return IsHodirEngaged(botAI) && IsHodirFlashFreezeIncoming(botAI); }

bool HodirFreezeRunInShelter(PlayerbotAI* botAI)
{
    return HodirFreezeRun(botAI) && IsHodirInLandedShelter(botAI, botAI->GetBot());
}

bool HodirFreezeRunOutsideShelter(PlayerbotAI* botAI)
{
    return HodirFreezeRun(botAI) && !IsHodirInLandedShelter(botAI, botAI->GetBot());
}

// Any target, not just Hodir: ranged walk back into pools on the way to helper ice blocks too. The
// icicle sweep is the expensive part, so it goes last.
bool HodirIcicleInPath(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!IsHodirEngaged(botAI) || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    return target && IsHodirWalkThroughLiveIcicle(bot, target->GetPosition(),
                                                  ULDUAR_HODIR_ICE_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN,
                                                  ULDUAR_HODIR_BIG_SHARDS_RADIUS + ULDUAR_HODIR_DODGE_TRIGGER_MARGIN);
}

// Only the two tanks and the ranged half stand on a spot. Melee ride the boss, so they keep every
// generic mover, SetBehindTargetAction in particular.
bool HodirAnchored(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return IsHodirEngaged(botAI) &&
           (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) || PlayerbotAI::IsRanged(bot));
}

bool HodirResistancePaladin(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_PALADIN && IsHodirEngaged(botAI) && GetHodirResistancePaladin(botAI, bot) == bot;
}

void DefineHodir(EncounterBuilder& e)
{
    // First success wins, so this is a survival ranking: a missed shelter kills, then a 14000 icicle every 2s.
    // Targeting sits above the shed, whose long walks would starve it, and costs nothing once the target's right.
    e.Node<HodirNearSnowpackedIcicleTrigger, HodirMoveSnowpackedIcicleAction>(ACTION_RAID + 6, EncounterRow::Mover);
    e.Node<HodirIcicleDodgeTrigger, HodirIcicleDodgeAction>(ACTION_RAID + 5, EncounterRow::Mover);
    e.Node<HodirFrozenBlowsSwapTrigger, HodirFrozenBlowsSwapAction>(ACTION_RAID + 4);

    // Same relevance as the swap, they never contend: that fires only for tanks, this for hunters and rogues.
    // Above targeting, or the bot switches to a block before spending the charges the tank is waiting on.
    e.Node<HodirRedirectThreatTrigger, HodirRedirectThreatAction>(ACTION_RAID + 4);

    e.Node<HodirSetDpsPriorityTrigger, HodirSetDpsPriorityAction>(ACTION_RAID + 3);
    e.Node<HodirSpreadStormCloudTrigger, HodirSpreadStormCloudAction>(ACTION_RAID + 2, EncounterRow::Mover);
    e.Node<HodirCollectStormPowerTrigger, HodirCollectStormPowerAction>(ACTION_RAID + 2, EncounterRow::Mover);
    e.Node<HodirBitingColdTrigger, HodirBitingColdShedAction>(ACTION_RAID + 1, EncounterRow::Mover);
    e.Node<HodirFrostResistanceTrigger, HodirFrostResistanceAction>(ACTION_RAID);
    e.Node<HodirRaidPositionTrigger, HodirRaidPositionAction>(ACTION_RAID, EncounterRow::Mover);

    // Without the targeting rule DpsTargetValue is never null, so the generic node retakes the target every
    // other tick and bots drift off the ice block. Without the movement rules the anchor oscillates.

    // The class "lose aggro" node taunts on the 8s cooldown for as long as somebody else holds the
    // boss, and it knows nothing about Frozen Blows.
    e.Block("hodir guard multiplier", Role::Any, HodirTauntSuicide, Family::Taunt);

    // Healers keep the generic picker. "attack rti target" stays: bots mark nothing here, but a player's mark
    // should win. "X on attacker" goes, ice blocks count as attackers and those DoTs outrank the plain ones.
    e.OwnTargeting("hodir guard multiplier", Role::Any, HodirOwnsTargets,
                   Family::DpsAssist | Family::TankAssist | Family::DebuffOnAttacker);
    e.Multiplier<HodirBreakerAoeHoldMultiplier>(Family::Spell);

    // Every role, the whole 9s cast: from its 2nd tick the shelter run answers Duplicate and descends, so
    // everything below must be off. Charge, Blink and Disengage throw the bot and aren't MovementActions.
    e.Block("hodir guard multiplier", Role::Any, HodirFreezeRun, Family::Charge | Family::Blink | Family::Disengage);

    // Reach isn't kept: "reach melee" and "reach spell" walk bots out of shelters. The shed only runs
    // inside a landed shelter, where its legs stay in the park ring.
    e.OwnMovement("hodir guard multiplier", Role::Any, HodirFreezeRunOutsideShelter, Family::Attack,
                  {HodirMoveSnowpackedIcicleAction::Name, HodirIcicleDodgeAction::Name});
    e.OwnMovement("hodir guard multiplier", Role::Any, HodirFreezeRunInShelter, Family::Attack,
                  {HodirMoveSnowpackedIcicleAction::Name, HodirIcicleDodgeAction::Name,
                   HodirBitingColdShedAction::Name});

    // Don't walk back through a pool the dodge just stepped the bot out of. Held only until the icicle
    // goes off, 3.3s at most.
    e.Block("hodir guard multiplier", Role::Any, HodirIcicleInPath,
            Family::Reach | Family::ReachHeal | Family::SetBehind);

    // ReachHeal walks a healer into range of someone standing out of it.
    e.OwnMovement("hodir guard multiplier", Role::Any, HodirAnchored,
                  Family::Attack | Family::Reach | Family::ReachHeal);

    // Holds the aura slot open for Frost Resistance, chosen paladin only. The slot is exclusive, so its own
    // buff strategy would recast Retribution or Devotion next GCD and the two would trade it all fight.
    e.Block("hodir paladin aura multiplier", Role::Any, HodirResistancePaladin, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "shadow resistance aura", "fire resistance aura"});

    // Lust waits for the first fire: from there it covers that fire's whole window, Singed at cap for most.
    e.Block("hodir lust hold", Role::Any, IsHodirLustHeld, 0, {"heroism", "bloodlust"});
}
}  // namespace

EncounterDefinition const& UldHodirDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_HODIR, &BossStateGateOpen, &DefineHodir);
    return definition;
}
