/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include <list>

using namespace SwpHelpers;
using namespace EncounterHelpers;

// General

bool SunwellResetEncounterStatesAction::Execute(Event /*event*/)
{
    ObjectGuid const guid = bot->GetGUID();
    uint32 const instanceId = bot->GetInstanceId();

    bool reset = false;

    // Kalecgos
    Action* kalecAction = context->GetAction("kalecgos spread ranged");
    if (kalecAction && static_cast<KalecgosSpreadRangedAction*>(kalecAction)
            ->ResetInitialRangedPositionReached())
    {
        reset = true;
    }

    // Brutallus
    if (BrutallusEncounterState* brutallusState = FindBrutallusState(instanceId))
        reset |= brutallusState->rangedBurnStates.erase(guid) > 0;

    reset |= ReleaseBrutallusBurnPad(bot);

    Action* brutallusAction = context->GetAction("brutallus tanks position and swap");
    if (brutallusAction && static_cast<BrutallusTanksPositionAndSwapAction*>(brutallusAction)
            ->ResetInitialPositionReached())
    {
        reset = true;
    }

    // Eredar Twins
    Action* twinsAction = context->GetAction("eredar twins alythess tank move out of blaze");
    if (twinsAction && static_cast<EredarTwinsAlythessTankMoveOutOfBlazeAction*>(twinsAction)
            ->ResetAlythessTankState())
    {
        reset = true;
    }

    // M'uru
    Action* muruAction = context->GetAction("m'uru position ranged by phase");
    if (muruAction && static_cast<MuruPositionRangedByPhaseAction*>(muruAction)
            ->ResetEntropiusRangedPositionReached())
    {
        reset = true;
    }

    // Kil'jaeden
    if (KiljaedenEncounterState* kiljaedenState = FindKiljaedenState(instanceId))
        reset |= kiljaedenState->dragonOrbUsers.erase(guid) > 0;

    // A drake lost as Kil'jaeden dies leaves its rider stale-rooted after the encounter, when the
    // in-combat release (KiljaedenReleaseStaleRootAction) can no longer run.
    reset |= ReleaseStaleRootFlag(bot);

    // Records shared across the raid, so one bot clears them all
    if (!IsMechanicTrackerBot(bot, SWP_MAP_ID))
        return reset;

    reset |= ClearStrategyTargetIcon(
        bot, RtiTargetValue::skullIndex, { Id(SwpNpcs::NPC_HAND_OF_THE_DECEIVER) });

    reset |= KalecgosResetInstance(instanceId);
    reset |= BrutallusResetInstance(instanceId);
    reset |= FelmystResetInstance(instanceId);
    reset |= EredarTwinsResetInstance(instanceId);
    reset |= MuruResetInstance(instanceId);
    reset |= KiljaedenResetInstance(instanceId);

    return reset;
}

bool SunwellRemoveAuraAction::Execute(Event /*event*/)
{
    // Only the immunities that stop the bot from contributing should be cancelled, so Cloak of
    // Shadows and HPal bubbles are excluded.
    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (spellId && bot->getClass() != CLASS_ROGUE && !PlayerbotAI::IsHeal(bot) &&
        bot->HasAura(spellId))
    {
        bot->RemoveOwnedAura(spellId, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
        return true;
    }

    if (IsEncounterInProgress(bot, SWP_MAP_ID))
        return false;

    // It is Blizzlike for Burn to persist after the kill, but bots will murder the raid without
    // a dedicated non-combat strategy for it. That's a waste of time, so just wipe the aura.
    if (!HasBrutallusBurn(bot))
        return false;

    bot->RemoveAura(Id(SwpSpells::SPELL_BURN));
    return true;
}

// Trash

bool VolatileFiendKeepEnemyAwayFromGroupAction::Execute(Event /*event*/)
{
    Creature* volatileFiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    if (!volatileFiend || !volatileFiend->IsAlive())
        return false;

    if (PlayerbotAI::IsTank(bot))
        return AI_VALUE(Unit*, "current target") != volatileFiend && Attack(volatileFiend);

    float const currentDistance = bot->GetExactDist2d(volatileFiend);
    if (currentDistance >= VOLATILE_FIEND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(volatileFiend, VOLATILE_FIEND_SAFE_DISTANCE - currentDistance);
}

// At low health, Infernal Defense is cast, granting immunity to all damage but holy.
bool ApocalypseGuardAttackWithHolyMagicAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> apocalypseGuards;
    bot->GetCreatureListWithEntryInGrid(
        apocalypseGuards, Id(SwpNpcs::NPC_APOCALYPSE_GUARD), searchRadius);

    for (Creature* apocalypseGuard : apocalypseGuards)
    {
        if (!apocalypseGuard || !apocalypseGuard->IsAlive() ||
            !apocalypseGuard->HasAura(Id(SwpSpells::SPELL_INFERNAL_DEFENSE)))
        {
            continue;
        }

        if (!target || apocalypseGuard->GetGUID() < target->GetGUID())
            target = apocalypseGuard;
    }

    if (!target)
        return false;

    if (bot->getClass() == CLASS_PALADIN)
        return botAI->CanCastSpell("exorcism", target) && botAI->CastSpell("exorcism", target);

    if (bot->HasAura(Id(SwpSpells::SPELL_SHADOWFORM)))
    {
        bot->RemoveOwnedAura(
            Id(SwpSpells::SPELL_SHADOWFORM), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
    }

    return botAI->CanCastSpell("smite", target) && botAI->CastSpell("smite", target);
}

// Shared Bosses

// Clear Kalecgos's Arcane Buffet, the Eredar Twins' Flame Sear, and Kil'jaeden's Fire Bloom.
bool SunwellRemoveDebuffWithImmunityAction::Execute(Event /*event*/)
{
    uint32 const spellId = GetSelfImmunitySpell(bot);
    return spellId && botAI->CanCastSpell(spellId, bot) && botAI->CastSpell(spellId, bot);
}

bool SunwellMisdirectToMainTankAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    if (!boss)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || !mainTank->IsAlive())
        return false;

    return MisdirectTargetToTank(botAI, boss, mainTank);
}
