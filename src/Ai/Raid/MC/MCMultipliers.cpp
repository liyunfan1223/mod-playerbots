/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MCMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MCActions.h"
#include "MCHelpers.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"
#include "SpellAuras.h"
#include "WarriorActions.h"

using namespace MoltenCoreHelpers;

static bool IsDpsBotWithAoeAction(Player* bot, Action* action)
{
    if (PlayerbotAI::IsDps(bot))
    {
        if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastConsecrationAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<CastWhirlwindAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastExplosiveTrapAction*>(action) ||
            dynamic_cast<CastDeathAndDecayAction*>(action))
            return true;

        if (auto castSpellAction = dynamic_cast<CastSpellAction*>(action))
        {
            if (castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
                return true;
        }
    }
    return false;
}

float GarrDisableDpsAoeMultiplier::GetValue(Action* action)
{
    if (AI_VALUE2(Unit*, "find target", "garr"))
    {
        if (IsDpsBotWithAoeAction(bot, action))
            return 0.0f;
    }
    return 1.0f;
}

static bool IsAllowedGeddonMovementAction(Action* action)
{
    if (dynamic_cast<MovementAction*>(action) &&
                !dynamic_cast<McMoveFromGroupAction*>(action) &&
                !dynamic_cast<McMoveFromBaronGeddonAction*>(action))
        return false;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return false;

    return true;
}

float BaronGeddonAbilityMultiplier::GetValue(Action* action)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        if (boss->HasAura(SPELL_INFERNO))
        {
            if (!IsAllowedGeddonMovementAction(action))
                return 0.0f;
        }
    }

    // No check for Baron Geddon, because bots may have the bomb even after Geddon died.
    if (bot->HasAura(SPELL_LIVING_BOMB))
    {
        if (!IsAllowedGeddonMovementAction(action))
            return 0.0f;
    }

    return 1.0f;
}

static bool IsSingleLivingTankInGroup(Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;
            if (PlayerbotAI::IsTank(member))
                return false;
        }
    }
    return true;
}

float GolemaggMultiplier::GetValue(Action* action)
{
    if (Unit* golemagg = AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        // Below 10% (soft enrage: Earthquake + converging Core Ragers) the
        // fight is a burn race, everyone dps, no splash management.
        bool burnPhase = golemagg->GetHealthPct() <= 10.0f;
        if (PlayerbotAI::IsTank(bot) && IsSingleLivingTankInGroup(bot))
        {
            // Only one tank => Pick up Golemagg and the two Core Ragers
            if (dynamic_cast<McGolemaggMainTankAttackGolemaggAction*>(action) ||
                dynamic_cast<McGolemaggAssistTankAttackCoreRagerAction*>(action))
                return 0.0f;
        }
        if (PlayerbotAI::IsAssistTank(bot))
        {
            // The first two assist tanks manage the Core Ragers. The remaining assist tanks attack the boss.
            if (dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
        if (IsDpsBotWithAoeAction(bot, action))
            return 0.0f;

        // Golemagg's Magma Splash stacks an uncapped 30s dot on anyone striking
        // him in melee. This blocks the default "melee when out of mana / in dead zone" fallback
        // that walks casters and hunters onto the boss.
        if (!burnPhase && PlayerbotAI::IsRanged(bot) && dynamic_cast<MeleeAction*>(action))
            return 0.0f;

        // Backed-off melee stay out until their whole Magma Splash stack expires (30s after the last application).
        Aura* splash = bot->GetAura(SPELL_MAGMA_SPLASH);
        bool backedOff = splash && splash->GetStackAmount() >= MAGMA_SPLASH_BACK_OFF_STACKS;
        bool engagesBoss = !dynamic_cast<McGolemaggBackOffAction*>(action) &&
                           (dynamic_cast<AttackAction*>(action) || dynamic_cast<MeleeAction*>(action) ||
                            dynamic_cast<CastReachTargetSpellAction*>(action));
        if (!burnPhase && !PlayerbotAI::IsTank(bot) && backedOff && engagesBoss)
            return 0.0f;
    }
    return 1.0f;
}
