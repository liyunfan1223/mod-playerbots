/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSMultipliers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PoSActions.h"
#include "PoSTriggers.h"

float IckAndKrickMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return 1.0f;

    // Allow the IckAndKrickAction to run
    if (dynamic_cast<IckAndKrickAction*>(action))
        return 1.0f;

    // Cancel all other actions when we need to handle Poison Nova; leaving a pool may go on, it keeps clear of the nova
    if (IsPoisonNovaCasting(boss) && bot->GetExactDist2d(boss) < 20.0f)
        return dynamic_cast<PoSAvoidGroundHazardAction*>(action) ? 1.0f : 0.0f;

    // Fixated (tank included): only run while he is close; a cast would stop the kite. Pools still get dodged.
    if (bot->HasAura(SPELL_PURSUIT) && bot->GetExactDist2d(boss) < 20.0f && !dynamic_cast<PoSAvoidGroundHazardAction*>(action))
        return 0.0f;

    if (!PlayerbotAI::IsHeal(bot) && boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) || boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK)))
        return 0.0f;  // Cancel all other actions when we need to handle Explosive Barrage

    return 1.0f;
}

float PoSHazardMultiplier::GetValue(Action* action)
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss)
        return 1.0f;

    if (dynamic_cast<PoSAvoidGroundHazardAction*>(action) || dynamic_cast<TyrannusBrandStopAction*>(action))
        return 1.0f;

    // Overlord's Brand copies the branded player's damage onto the tank and their healing onto Tyrannus x5.5
    if (bot->HasAura(SPELL_OVERLORDS_BRAND) && boss->GetEntry() == NPC_TYRANNUS && !PlayerbotAI::IsTank(bot))
    {
        if (dynamic_cast<AttackAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastHealingSpellAction*>(action) && BrandedHealerMayHeal(bot, boss))
            return 1.0f;

        // Totem drops count as buffs; a damaging one's hits would be copied onto the tank
        CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
        if (cast && IsBrandDamagingTotemSpell(cast->getSpell()))
            return 0.0f;

        if (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastBuffSpellAction*>(action))
            return 0.0f;
    }

    // Once out of a pool or patch, stay out: chasing and repositioning walked bots back in
    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        std::vector<PoSGroundHazard> hazards = FindPoSGroundHazards(bot);
        // A small margin only: with a dozen patches down, a wide one froze bots wherever they stood
        if (InPoSGroundHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), 1.5f))
            return 0.0f;

        if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot) &&
            InPoSGroundHazard(hazards, boss->GetPositionX(), boss->GetPositionY(), 0.0f))
            return 0.0f;
    }

    return 1.0f;
}

float GarfrostMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<GarfrostTakeCoverAction*>(action) || dynamic_cast<GarfrostDodgeSaroniteAction*>(action) ||
        dynamic_cast<GarfrostTankPositionAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    // Nobody follows him to the anvil: he walks back to his tank in the middle once the weapon is done
    if (IsGarfrostForging(boss) && dynamic_cast<MovementAction*>(action))
        return 0.0f;

    // Behind a boulder: casters keep casting from there, melee and the tank wait out their stacks
    if (GarfrostWantsCover(bot, boss) && IsShieldedFromGarfrost(boss, bot))
    {
        if (dynamic_cast<AttackAction*>(action))
            return PlayerbotAI::IsRanged(bot) ? 1.0f : 0.0f;

        if (dynamic_cast<MovementAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
