/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_XT002.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldEncounter_XT002.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <set>
#include <string>

using namespace EncounterHelpers;

// XT-002: the exposed Heart is the only real burst window, but bursting it kills the Heart and starts hard
// mode, so the config decides.
float XT002BurstWindowMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string const name = action->getName();
    if (!IsBurstCooldownAction(name) || IsManaReturnCooldown(bot, name))
        return 1.0f;

    uint32 now = getMSTime();
    if (now != cachedAtMs || !cachedAtMs)
    {
        cachedAtMs = now;
        cachedValue = EvaluateWindow();
    }

    bool const lust = name == "bloodlust" || name == "heroism";
    return (lust ? cachedValue.allowLust : cachedValue.allowAll) ? 1.0f : 0.0f;
}

XT002BurstWindowMultiplier::Window XT002BurstWindowMultiplier::EvaluateWindow()
{
    Unit* xt002 = GetXT002(botAI);
    if (!xt002)
        return {true, true};

    // Heartbreak is up: there will be no further Heart phase, so there is nothing left to save for.
    if (IsXT002HeartbreakActive(bot, xt002))
        return {true, true};

    bool const hardMode = IsXT002HardModeActive(botAI);

    if (GetXT002ExposedHeart(botAI))
    {
        // Hard mode burns the Heart with everything but lust: Heartbreak runs the rest of the fight, and
        // Exhaustion means there's no second lust.
        return {hardMode, false};
    }

    // Below this the last Heart phase is over. In hard mode the Heart outlived every window, so the lust held
    // for Heartbreak goes too.
    if (xt002->GetHealthPct() < ULDUAR_XT002_FINAL_PUSH_HP_PCT)
        return {true, true};

    return {false, false};
}

float XT002TargetGuardMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    // This runs on every action every bot pops anywhere in Ulduar, and finding XT is a scan. So sort
    // the action first, and only look XT up for the kinds that can come out as anything but 1.0.
    bool const movement = dynamic_cast<MovementAction*>(action) != nullptr;

    // Two aura-map walks only the immunity and mover branches read, so they sit behind those dynamic_casts and
    // a cast, buff or heal never pays for them.
    bool carryingKnown = false;
    bool carrying = false;
    auto const carryingDebuff = [&]() -> bool
    {
        if (!carryingKnown)
        {
            carrying =
                bot->HasAura(GetXT002SearingLightSpellId(bot)) || bot->HasAura(GetXT002GravityBombSpellId(bot));
            carryingKnown = true;
        }

        return carrying;
    };

    // The class-generic redirects buff whoever the group flags as main tank, which is the wrong
    // sink while a Pummeller is out; xt002 redirect threat action picks the tank that needs it.
    bool const genericRedirect = dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ||
                                 dynamic_cast<CastTricksOfTheTradeOnMainTankAction*>(action);

    // "xt002 avoid hazard action" is the only mover for this fight's hazards. Generic avoid aoe sits at the
    // same relevance and flees a flat FleeDistance its own way, so the bot slides between the two.
    bool const genericAvoidAoe = dynamic_cast<AvoidAoeAction*>(action) != nullptr;

    // Gap closers move the bot without being MovementActions, so the mover pin below misses them and a parked
    // carrier charges back out.
    bool const gapCloser = dynamic_cast<CastReachTargetSpellAction*>(action) && carryingDebuff();

    // Both bubbles cover all magic with SPELL_ATTR1_IMMUNITY_PURGES_EFFECT, so they strip the debuff and the
    // AfterEffectRemove hook drops the Void Zone or Life Spark on the spot, mid-raid. Eat the hit instead.
    bool const purgingImmunity = (dynamic_cast<CastDivineShieldAction*>(action) ||
                                  dynamic_cast<CastIceBlockAction*>(action)) &&
                                 carryingDebuff();

    // The encounter's priority action owns every target, so the generic pickers stand down. "attack rti target"
    // stays so a player's mark still wins; healers keep dps assist or they could be left targetless.
    bool const genericPicker =
        (dynamic_cast<DpsAssistAction*>(action) && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot)) ||
        (dynamic_cast<TankAssistAction*>(action) && PlayerbotAI::IsTank(bot));

    // Ranged, healers and carriers each own a destination, so generic movers stand down for them; melee keep
    // theirs unless carrying. AttackAction is a MovementAction that never moves, so it stays exempt.
    bool pinnedMover = false;
    if (movement && !dynamic_cast<AttackAction*>(action) &&
        (PlayerbotAI::IsRangedDps(bot) || PlayerbotAI::IsHeal(bot) || carryingDebuff()))
    {
        static std::set<std::string> const encounterMovers = {
            "xt002 raid position action", "xt002 debuff carrier action", "xt002 avoid hazard action"};

        pinnedMover = !encounterMovers.count(action->getName());
    }

    // Normal mode Heart floor: the priority action only stops new picks, this stops bots already on it. Only
    // hits on the Heart are blocked, or a bot would be left with no heals and no way to retarget.
    Unit* currentTarget = nullptr;
    bool heartHit = false;
    if (!IsXT002HardModeActive(botAI) && !movement && !dynamic_cast<CastHealingSpellAction*>(action))
    {
        currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget && currentTarget->GetEntry() == NPC_HEART_OF_DECONSTRUCTOR)
        {
            static std::set<std::string> const retargets = {"xt002 set dps priority action",
                                                            "xt002 pummeller taunt action",
                                                            "xt002 redirect threat action"};

            heartHit = !retargets.count(action->getName());
        }
    }

    bool const standDown =
        genericRedirect || genericAvoidAoe || purgingImmunity || genericPicker || pinnedMover || gapCloser;
    if (!standDown && !heartHit)
        return 1.0f;

    Unit* xt002 = GetXT002(botAI);
    if (!xt002)
        return 1.0f;

    // Encounter nodes don't run before the pull, so the stand-downs wait for XT's combat. The Heart floor
    // doesn't: it has to hold whenever an exposed Heart is up.
    if (standDown && xt002->IsInCombat())
        return 0.0f;

    if (!heartHit)
        return 1.0f;

    Unit* heart = GetXT002ExposedHeart(botAI);
    if (!heart || heart->GetHealthPct() > ULDUAR_XT002_HEART_SAFE_HP_PCT)
        return 1.0f;

    return currentTarget == heart ? 0.0f : 1.0f;
}
